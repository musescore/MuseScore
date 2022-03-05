/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "scoreproperties.h"

#include <QScrollBar>
#include <QCloseEvent>

#include "ui/view/widgetstatestore.h"
#include "ui/view/widgetutils.h"

#include "translation.h"

using namespace mu::notation;
using namespace mu::project;
using namespace mu::framework;
using namespace mu::ui;

static const QString SP_WORK_TITLE_TAG("workTitle");
static const QString SP_WORK_NUMBER_TAG("workNumber");
static const QString SP_COMPOSER_TAG("composer");
static const QString SP_LYRICIST_TAG("lyricist");
static const QString SP_PLATFORM_TAG("platform");
static const QString SP_SOURCE_TAG("source");
static const QString SP_COPYRIGHT_TAG("copyright");
static const QString SP_TRANSLATOR_TAG("translator");
static const QString SP_ARRANGER_TAG("arranger");
static const QString SP_CREATION_DATE_TAG("creationDate");

//---------------------------------------------------------
//   MetaEditDialog
//---------------------------------------------------------

ScorePropertiesDialog::ScorePropertiesDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("ScorePropertiesDialog");

    setupUi(this);

    QDialog::setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    QDialog::setWindowFlag(Qt::WindowMinMaxButtonsHint);

    INotationProjectPtr project = this->project();

    ProjectMeta meta = project->metaInfo();

    version->setText(meta.musescoreVersion);
    level->setText(QString::number(meta.mscVersion));

    int rev = meta.musescoreRevision;
    if (rev > 99999) { // MuseScore 1.3 is decimal 5702, 2.0 and later uses a 7-digit hex SHA
        revision->setText(QString::number(rev, 16));
    } else {
        revision->setText(QString::number(rev, 10));
    }

    filePath->setText(meta.filePath.toQString());

    if (project->isNewlyCreated()) {
        filePath->setText(qtrc("project", "Project is not saved yet"));
        filePath->setEnabled(false); // Mainly for visual effect
        revealButton->setEnabled(false);
    } else if (project->isCloudProject()) {
        // TODO(save-to-cloud): it would be nice to display the URL
        // and add a button "Reveal on MuseScore.com"
        filePath->setText(qtrc("project", "Project is saved in the cloud"));
        filePath->setEnabled(false); // Mainly for visual effect
        revealButton->setEnabled(false);
    } else {
        io::path path = project->path();
        filePath->setText(path.toQString());
        revealButton->setEnabled(fileSystem()->exists(path));
    }

    initTags();

    scrollAreaLayout->setColumnStretch(1, 1); // The 'value' column should be expanding

    connect(newButton,  &QPushButton::clicked, this, &ScorePropertiesDialog::newClicked);
    connect(saveButton, &QPushButton::clicked, this, &ScorePropertiesDialog::save);

    WidgetUtils::setWidgetIcon(revealButton, IconCode::Code::OPEN_FILE);
    connect(revealButton, &QPushButton::clicked, this, &ScorePropertiesDialog::openFileLocation);

    WidgetStateStore::restoreGeometry(this);
}

ScorePropertiesDialog::ScorePropertiesDialog(const ScorePropertiesDialog& dialog)
    : ScorePropertiesDialog(dialog.parentWidget())
{
}

//---------------------------------------------------------
//   addTag
///   Add a tag to the displayed list
///   returns a pair of widget corresponding to the key and value:
///           QPair<QLineEdit* key, QLineEdit* value>
//---------------------------------------------------------

ScorePropertiesDialog::TagItem ScorePropertiesDialog::addTag(const QString& key, const QString& value)
{
    QWidget* tagWidget = nullptr;
    QLineEdit* valueWidget = new QLineEdit(value);

    connect(valueWidget, &QLineEdit::textChanged, this, [this]() { setDirty(); });

    const int numFlags = scrollAreaLayout->rowCount();
    QToolButton* deleteButton = nullptr;

    if (isStandardTag(key)) {
        QLabel* tagLabel = new QLabel(key);
        tagLabel->setToolTip(qtrc("notation", "This is a builtin tag. Its name cannot be modified."));
        tagLabel->setBuddy(valueWidget);
        tagWidget = tagLabel;
    } else {
        QLineEdit* tagLineEdit = new QLineEdit(key);
        tagLineEdit->setPlaceholderText(qtrc("notation", "Name"));

        deleteButton = new QToolButton();
        WidgetUtils::setWidgetIcon(deleteButton, IconCode::Code::DELETE_TANK);
        deleteButton->setAccessibleName(qtrc("notation", "Remove"));

        tagWidget = tagLineEdit;

        connect(tagLineEdit, &QLineEdit::textChanged, this, [this]() { setDirty(); });
        connect(deleteButton, &QToolButton::clicked, this,
                [this, tagWidget, valueWidget, deleteButton]() {
            setDirty();
            tagWidget->deleteLater();
            valueWidget->deleteLater();
            deleteButton->deleteLater();
        });
    }

    scrollAreaLayout->addWidget(tagWidget,   numFlags, 0);
    scrollAreaLayout->addWidget(valueWidget, numFlags, 1);

    if (deleteButton) {
        scrollAreaLayout->addWidget(deleteButton, numFlags, 2);
    }

    return { tagWidget, valueWidget, deleteButton };
}

//---------------------------------------------------------
//   newClicked
///   When the 'New' button is clicked, a new tag is appended,
///   and focus is set to the QLineEdit corresponding to its name.
//---------------------------------------------------------

void ScorePropertiesDialog::newClicked()
{
    TagItem tagItem = addTag("", "");

    tagItem.titleWidget->setFocus();
    tagItem.valueLineEdit->setPlaceholderText(qtrc("notation", "Value"));

    // scroll down to see the newly created tag.
    // ugly workaround because scrolling to maximum doesn't completely scroll
    // to the maximum, for some unknow reason.
    // See https://www.qtcentre.org/threads/32852-How-can-I-always-keep-the-scroll-bar-at-the-bottom-of-a-QScrollArea
    QScrollBar* scrollBar = scrollArea->verticalScrollBar();
    scrollBar->setMaximum(scrollBar->maximum() + 1);
    scrollBar->setValue(scrollBar->maximum());

    setDirty();
    updateTabOrders(tagItem);
}

//---------------------------------------------------------
//   isStandardTag
///   returns true if the tag is one of Musescore's builtin tags
///   see also MasterScore::MasterScore()
//---------------------------------------------------------

bool ScorePropertiesDialog::isStandardTag(const QString& tag) const
{
    static const QSet<QString> standardTags {
        SP_WORK_TITLE_TAG,
        SP_WORK_NUMBER_TAG,
        SP_ARRANGER_TAG,
        SP_COMPOSER_TAG,
        SP_LYRICIST_TAG,
        SP_PLATFORM_TAG,
        SP_SOURCE_TAG,
        SP_TRANSLATOR_TAG,
        SP_COPYRIGHT_TAG,
        SP_CREATION_DATE_TAG
    };

    return standardTags.contains(tag);
}

//---------------------------------------------------------
//   setDirty
///    Sets the editor as having unsaved changes
//---------------------------------------------------------

void ScorePropertiesDialog::setDirty(const bool dirty)
{
    if (dirty == m_dirty) {
        return;
    }

    saveButton->setEnabled(dirty);

    QString title = project() ? project()->metaInfo().title : QString();
    setWindowTitle(qtrc("notation", "Score properties: %1%2").arg(title).arg((dirty ? "*" : "")));

    m_dirty = dirty;
}

//---------------------------------------------------------
//   openFileLocation
///    Opens the file location with a QMessageBox::warning on failure
//---------------------------------------------------------

void ScorePropertiesDialog::openFileLocation()
{
    Ret ret = interactive()->revealInFileBrowser(filePath->text());

    if (!ret) {
        interactive()->warning(trc("notation", "Open Containing Folder Error"),
                               trc("notation", "Could not open containing folder."));
    }
}

//---------------------------------------------------------
//   save
///   Save the currently displayed metatags
//---------------------------------------------------------

bool ScorePropertiesDialog::save()
{
    if (!m_dirty) {
        return true;
    }

    const int idx = scrollAreaLayout->rowCount();
    QVariantMap map;
    for (int i = 0; i < idx; ++i) {
        QLayoutItem* tagItem  = scrollAreaLayout->itemAtPosition(i, 0);
        QLayoutItem* valueItem = scrollAreaLayout->itemAtPosition(i, 1);
        if (tagItem && valueItem) {
            QString tagText = "";

            // Is the property a standard one?
            QLabel* labelItem = dynamic_cast<QLabel*>(tagItem->widget());
            if (labelItem) {
                tagText = labelItem->text();
            } else {
                // Is the property a none standard one (a line item) ?
                QLineEdit* lineEditItem = dynamic_cast<QLineEdit*>(tagItem->widget());
                if (lineEditItem) {
                    tagText = lineEditItem->text();

                    // Validate none standard properties names (empty / reserved or duplicates)
                    if (tagText.isEmpty()) {
                        interactive()->warning(trc("notation", "MuseScore"),
                                               trc("notation", "Tags can't have empty names."));
                        lineEditItem->setFocus();
                        return false;
                    }
                    if (map.contains(tagText)) {
                        if (isStandardTag(tagText)) {
                            interactive()->warning(trc("notation", "MuseScore"),
                                                   qtrc("notation",
                                                        "%1 is a reserved builtin tag.\n It can't be used.").arg(tagText).toStdString());
                            lineEditItem->setFocus();
                            return false;
                        }

                        interactive()->warning(trc("notation", "MuseScore"),
                                               trc("notation", "You have multiple tags with the same name."));
                        lineEditItem->setFocus();
                        return false;
                    }
                } else {
                    // Item should always be a label or an edit
                    qDebug("MetaEditDialog: unknown configuration type: %i", i);
                    continue;
                }
            }

            QLineEdit* value = static_cast<QLineEdit*>(valueItem->widget());
            map.insert(tagText, value->text());
        } else {
            qDebug("MetaEditDialog: abnormal configuration: %i", i);
        }
    }

    saveMetaTags(map);
    setDirty(false);

    return true;
}

//---------------------------------------------------------
//   accept
///   Reimplemented to save modifications before closing the dialog.
//---------------------------------------------------------

void ScorePropertiesDialog::accept()
{
    if (!save()) {
        return;
    }

    QDialog::accept();
}

//---------------------------------------------------------
//   hideEvent
///   Reimplemented to notify the user that he/she is quitting without saving
//---------------------------------------------------------

void ScorePropertiesDialog::closeEvent(QCloseEvent* event)
{
    auto apply = [this, event]() {
        WidgetStateStore::saveGeometry(this);
        event->accept();
    };

    if (!m_dirty) {
        apply();
        return;
    }

    IInteractive::Result result = interactive()->question(trc("notation", "MuseScore"), trc("notation",
                                                                                            "You have unsaved changes.\nSave?"), {
        IInteractive::Button::Save, IInteractive::Button::Discard, IInteractive::Button::Cancel
    }, IInteractive::Button::Save);

    if (result.standardButton() == IInteractive::Button::Save) {
        if (!save()) {
            event->ignore();
            return;
        }
    } else if (result.standardButton() == IInteractive::Button::Cancel) {
        event->ignore();
        return;
    }

    apply();
}

mu::project::INotationProjectPtr ScorePropertiesDialog::project() const
{
    return context()->currentProject();
}

void ScorePropertiesDialog::initTags()
{
    if (!project()) {
        return;
    }

    ProjectMeta meta = project()->metaInfo();

    TagItem firstTag = addTag(SP_WORK_TITLE_TAG, meta.title);
    addTag(SP_ARRANGER_TAG, meta.arranger);
    addTag(SP_COMPOSER_TAG, meta.composer);
    addTag(SP_COPYRIGHT_TAG, meta.copyright);
    addTag(SP_CREATION_DATE_TAG, meta.creationDate.toString());
    addTag(SP_LYRICIST_TAG, meta.lyricist);
    addTag(SP_TRANSLATOR_TAG, meta.translator);
    addTag(SP_PLATFORM_TAG, meta.platform);
    addTag(SP_SOURCE_TAG, meta.source);

    TagItem lastTag;
    for (const QString& key : meta.additionalTags.keys()) {
        lastTag = addTag(key, meta.additionalTags[key].toString());
    }

    scrollArea->setFocusProxy(firstTag.titleWidget);

    updateTabOrders(lastTag);
}

void ScorePropertiesDialog::saveMetaTags(const QVariantMap& tagsMap)
{
    if (!project()) {
        return;
    }

    ProjectMeta meta;

    meta.title = tagsMap[SP_WORK_TITLE_TAG].toString();
    meta.arranger = tagsMap[SP_ARRANGER_TAG].toString();
    meta.composer = tagsMap[SP_COMPOSER_TAG].toString();
    meta.copyright = tagsMap[SP_COPYRIGHT_TAG].toString();
    meta.creationDate = QDate::fromString(tagsMap[SP_CREATION_DATE_TAG].toString());
    meta.lyricist = tagsMap[SP_LYRICIST_TAG].toString();
    meta.translator = tagsMap[SP_TRANSLATOR_TAG].toString();
    meta.source = tagsMap[SP_SOURCE_TAG].toString();
    meta.platform = tagsMap[SP_PLATFORM_TAG].toString();

    for (const QString& key : tagsMap.keys()) {
        if (isStandardTag(key)) {
            continue;
        }

        meta.additionalTags[key] = tagsMap[key];
    }

    project()->setMetaInfo(meta);
}

void ScorePropertiesDialog::updateTabOrders(const TagItem& lastTagItem)
{
    if (lastTagItem.deleteButton) {
        QWidget::setTabOrder(lastTagItem.deleteButton, newButton);
    } else {
        QWidget::setTabOrder(lastTagItem.valueLineEdit, newButton);
    }

    QWidget::setTabOrder(newButton, okButton);
    QWidget::setTabOrder(okButton, cancelButton);
    QWidget::setTabOrder(cancelButton, saveButton);
}
