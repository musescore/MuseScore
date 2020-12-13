//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2008 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "metaedit.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"
#include "musescore.h"
#include "preferences.h"
#include "icons.h"
#include "openfilelocation.h"

namespace Ms {

//---------------------------------------------------------
//   MetaEditDialog
//---------------------------------------------------------

MetaEditDialog::MetaEditDialog(Score* score, QWidget* parent)
   : QDialog(parent),
     m_score(score),
     m_dirty(false)
      {
      setObjectName("MetaEditDialog");

      setupUi(this);
      QDialog::setWindowFlag(Qt::WindowContextHelpButtonHint, false);
      QDialog::setWindowFlag(Qt::WindowMinMaxButtonsHint);

      version->setText(m_score->mscoreVersion());
      level->setText(QString::number(m_score->mscVersion()));

      int rev = m_score->mscoreRevision();
      if (rev > 99999)  // MuseScore 1.3 is decimal 5702, 2.0 and later uses a 7-digit hex SHA
            revision->setText(QString::number(rev, 16));
      else
            revision->setText(QString::number(rev, 10));

      QString currentFileName  = score->masterScore()->fileInfo()->absoluteFilePath();
      QString previousFileName = score->importedFilePath();
      if (previousFileName.isEmpty() || previousFileName == currentFileName) // New score or no "Save as" used
            filePath->setText(previousFileName);
      else
            filePath->setText(QString("%1\n%2\n%3")
                              .arg(QFileInfo::exists(currentFileName) ? currentFileName : "<b>" + tr("Not saved yet,") + "</b>",
                                   tr("initially read from:"), previousFileName));
      filePath->setTextInteractionFlags(Qt::TextSelectableByMouse);

      QMapIterator<QString, QString> iterator(score->metaTags());
      while (iterator.hasNext()) {
            iterator.next();
            const QString key = iterator.key();
            addTag(key, iterator.value(), isBuiltinTag(key));
            }

      scrollAreaLayout->setColumnStretch(1, 1); // The 'value' column should be expanding

      connect(newButton,  &QPushButton::clicked, this, &MetaEditDialog::newClicked);
      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(buttonBoxClicked(QAbstractButton*)));
      buttonBox->button(QDialogButtonBox::Save)->setEnabled(m_dirty);
      if (!QFileInfo::exists(score->importedFilePath()))
            revealButton->setEnabled(false);

      revealButton->setIcon(*icons[int(Icons::fileOpen_ICON)]);
      revealButton->setToolTip(OpenFileLocation::platformText());

      connect(revealButton, &QPushButton::clicked, this, &MetaEditDialog::openFileLocation);
      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   addTag
///   Add a tag to the displayed list
///   returns a pair of widget corresponding to the key and value:
///           QPair<QLineEdit* key, QLineEdit* value>
//---------------------------------------------------------

QPair<QLineEdit*, QLineEdit*> MetaEditDialog::addTag(const QString& key, const QString& value, const bool builtinTag)
      {
      QLineEdit* tagWidget = new QLineEdit(key);
      QLineEdit* valueWidget = new QLineEdit(value);

      connect(valueWidget, &QLineEdit::textChanged, this, [this]() { setDirty(); });

      const int numFlags = scrollAreaLayout->rowCount();
      if (builtinTag) {
            tagWidget->setReadOnly(true);
            // Make it clear that builtin tags are not editable
            tagWidget->setStyleSheet("QLineEdit { background: transparent; }");
            tagWidget->setFrame(false);
            tagWidget->setFocusPolicy(Qt::NoFocus);
            tagWidget->setToolTip(tr("This is a builtin tag. Its name cannot be modified."));
            }
      else {
            tagWidget->setPlaceholderText(tr("Name"));
            QToolButton* deleteButton = new QToolButton();
            deleteButton->setIcon(*icons[int (Icons::bin_ICON)]);

            // follow gui scaling. The '+ 2' at the end is the margin. (2 * 1px).for top and bottoms.
            const double size = preferences.getInt(PREF_UI_THEME_ICONWIDTH) * guiScaling * .5 + 2;
            deleteButton->setIconSize(QSize(size, size));

            connect(tagWidget, &QLineEdit::textChanged, this, [this]() { setDirty(); });
            connect(deleteButton, &QToolButton::clicked, this,
                    [this, tagWidget, valueWidget, deleteButton]() { setDirty();
                                                         tagWidget->deleteLater();
                                                         valueWidget->deleteLater();
                                                         deleteButton->deleteLater(); });

            scrollAreaLayout->addWidget(deleteButton, numFlags, 2);
            }
      scrollAreaLayout->addWidget(tagWidget,   numFlags, 0);
      scrollAreaLayout->addWidget(valueWidget, numFlags, 1);

      return QPair<QLineEdit*, QLineEdit*>(tagWidget, valueWidget);
      }

//---------------------------------------------------------
//   newClicked
///   When the 'New' button is clicked, a new tag is appended,
///   and focus is set to the QLineEdit corresponding to its name.
//---------------------------------------------------------

void MetaEditDialog::newClicked()
      {
      QPair<QLineEdit*, QLineEdit*> pair = addTag("", "", false);

      pair.first->setFocus();
      pair.second->setPlaceholderText(tr("Value"));
      // scroll down to see the newly created tag.
      // ugly workaround because scrolling to maximum doesn't completely scroll
      // to the maximum, for some unknow reason.
      // See https://www.qtcentre.org/threads/32852-How-can-I-always-keep-the-scroll-bar-at-the-bottom-of-a-QScrollArea
      QScrollBar* scrollBar = scrollArea->verticalScrollBar();
      scrollBar->setMaximum(scrollBar->maximum() + 1);
      scrollBar->setValue(scrollBar->maximum());

      setDirty();
      }

//---------------------------------------------------------
//   isBuiltinTag
///   returns true if the tag is one of Musescore's builtin tags
///   see also MasterScore::MasterScore()
//---------------------------------------------------------

bool MetaEditDialog::isBuiltinTag(const QString& tag) const
      {
      return (tag ==  "platform"      || tag ==  "movementNumber" || tag ==  "movementTitle"
              || tag ==  "workNumber" || tag ==  "workTitle"      || tag ==  "arranger"
              || tag ==  "composer"   || tag ==  "lyricist"       || tag ==  "poet"
              || tag ==  "translator" || tag ==  "source"         || tag ==  "copyright"
              || tag ==  "creationDate");
      }

//---------------------------------------------------------
//   setDirty
///    Sets the editor as having unsaved changes
//---------------------------------------------------------

void MetaEditDialog::setDirty(const bool dirty)
      {
      if (dirty == m_dirty)
            return;

      buttonBox->button(QDialogButtonBox::Save)->setEnabled(dirty);
      setWindowTitle(tr("Score properties: %1%2").arg(m_score->title()).arg((dirty ? "*" : "")));

      m_dirty = dirty;
      }

//---------------------------------------------------------
//   openFileLocation
///    Opens the file location with a QMessageBox::warning on failure
//---------------------------------------------------------

void MetaEditDialog::openFileLocation()
      {
      if (!OpenFileLocation::openFileLocation(filePath->text()))
            QMessageBox::warning(this, tr("Open Containing Folder Error"),
                                       tr("Could not open containing folder"));
      }

//---------------------------------------------------------
//   buttonBoxClicked
//---------------------------------------------------------

void MetaEditDialog::buttonBoxClicked(QAbstractButton* button)
      {
      switch (buttonBox->buttonRole(button)) {
            case QDialogButtonBox::ApplyRole:
                  save();
                  break;
            case QDialogButtonBox::AcceptRole:
                  accept();
                  // fall through
            case QDialogButtonBox::RejectRole:
                  close();
            default:
                  break;
            }
      }

//---------------------------------------------------------
//   save
///   Save the currently displayed metatags
//---------------------------------------------------------

bool MetaEditDialog::save()
      {
      if (m_dirty) {
            const int idx = scrollAreaLayout->rowCount();
            QMap<QString, QString> map;
            for (int i = 0; i < idx; ++i) {
                  QLayoutItem *tagItem   = scrollAreaLayout->itemAtPosition(i, 0);
                  QLayoutItem *valueItem = scrollAreaLayout->itemAtPosition(i, 1);
                  if (tagItem && valueItem) {
                        QLineEdit *tag   = static_cast<QLineEdit*>(tagItem->widget());
                        QLineEdit *value = static_cast<QLineEdit*>(valueItem->widget());

                        QString tagText = tag->text();
                        if (tagText.isEmpty()) {
                              QMessageBox::warning(this, tr("MuseScore"),
                                                   tr("Tags can't have empty names."),
                                                   QMessageBox::Ok, QMessageBox::Ok);
                              tag->setFocus();
                              return false;
                              }
                        if (map.contains(tagText)) {
                              if (isBuiltinTag(tagText)) {
                                    QMessageBox::warning(this, tr("MuseScore"),
                                                         tr("%1 is a reserved builtin tag.\n"
                                                            "It can't be used.").arg(tagText),
                                                         QMessageBox::Ok, QMessageBox::Ok);
                                    tag->setFocus();
                                    return false;
                                    }
                              QMessageBox::warning(this, tr("MuseScore"),
                                                   tr("You have multiple tags with the same name."),
                                                   QMessageBox::Ok, QMessageBox::Ok);
                              tag->setFocus();
                              return false;
                              }
                        map.insert(tagText, value->text());
                        }
                  else
                        qDebug("MetaEditDialog: abnormal configuration: %i", i);
                  }
            m_score->undo(new ChangeMetaTags(m_score, map));
            setDirty(false);
            }
      return true;
      }

//---------------------------------------------------------
//   accept
///   Reimplemented to save modifications before closing the dialog.
//---------------------------------------------------------

void MetaEditDialog::accept()
      {
      if (!save())
            return;

      QDialog::accept();
      }

//---------------------------------------------------------
//   hideEvent
///   Reimplemented to notify the user that he/she is quitting without saving
//---------------------------------------------------------

void MetaEditDialog::closeEvent(QCloseEvent* event)
      {
      if (m_dirty) {
            QMessageBox::StandardButton button = QMessageBox::warning(this, tr("MuseScore"),
                                                                      tr("You have unsaved changes.\nSave?"),
                                                                      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                                                      QMessageBox::Save);
            if (button == QMessageBox::Save) {
                  if (!save()) {
                        event->ignore();
                        return;
                        }
                  }
            else if (button == QMessageBox::Cancel) {
                  event->ignore();
                  return;
                  }
            }
      MuseScore::saveGeometry(this);
      event->accept();
      }

} // namespace Ms
