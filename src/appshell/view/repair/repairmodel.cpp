/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include <QApplication>

#include "repairmodel.h"

#include "translation.h"
#include "ui/view/iconcodes.h"

#include "log.h"

using namespace mu::appshell;
using namespace muse::ui;

RepairModel::RepairModel(QObject* parent)
    : QAbstractItemModel(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

RepairModel::~RepairModel()
{
    cancel();

    delete m_rootItem;
    m_rootItem = nullptr;
}

QModelIndex RepairModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    PreferencePageItem* parentItem = nullptr;

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = modelIndexToItem(parent);
    }

    if (!parentItem) {
        return QModelIndex();
    }

    PreferencePageItem* childItem = parentItem->childAtRow(row);

    if (childItem) {
        return createIndex(row, column, childItem);
    }

    return QModelIndex();
}

QModelIndex RepairModel::parent(const QModelIndex& child) const
{
    PreferencePageItem* childItem = modelIndexToItem(child);
    if (!childItem) {
        return QModelIndex();
    }

    PreferencePageItem* parentItem = qobject_cast<PreferencePageItem*>(childItem->parentItem());

    if (parentItem == m_rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int RepairModel::rowCount(const QModelIndex& parent) const
{
    PreferencePageItem* parentItem = nullptr;

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = modelIndexToItem(parent);
    }

    if (!parentItem) {
        return 0;
    }

    return parentItem->childCount();
}

int RepairModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant RepairModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() && role != ItemRole) {
        return QVariant();
    }

    PreferencePageItem* item = modelIndexToItem(index);

    if (!item) {
        return QVariant();
    }

    return QVariant::fromValue(qobject_cast<QObject*>(item));
}

QHash<int, QByteArray> RepairModel::roleNames() const
{
    return { { ItemRole, "itemRole" } };
}

QString RepairModel::currentPageId() const
{
    return m_currentPageId;
}

void RepairModel::load(const QString& currentPageId)
{
    configuration()->startEditSettings();

    beginResetModel();

    if (!currentPageId.isEmpty()) {
        setCurrentPageId(currentPageId);
    } else {
        const QString& lastOpenedPageId = configuration()->preferencesDialogLastOpenedPageId();
        if (lastOpenedPageId.isEmpty()) {
            setCurrentPageId("general");
        } else {
            setCurrentPageId(lastOpenedPageId);
        }
    }

    m_rootItem = new PreferencePageItem();

    QList<PreferencePageItem*> items {
        makeItem("Manual", QT_TRANSLATE_NOOP("appshell/repair", "Manual"), IconCode::Code::CONFIGURE,
                 "Repair/ManualRepairPage.qml"),

        makeItem("Auto", QT_TRANSLATE_NOOP("appshell/repair", "Auto"), IconCode::Code::CONFIGURE,
                 "Repair/AutoRepairPage.qml"),

        //makeItem("general", QT_TRANSLATE_NOOP("appshell/preferences", "General"), IconCode::Code::SETTINGS_COG,
        //         "Preferences/GeneralPreferencesPage.qml"),

        // # BGL NOTE : The below items are the other preference pages, shown as an example for now. If we want to add items,
        //              we will need to follow this pattern. Otherwise, we can just edit the above.

        // makeItem("appearance", QT_TRANSLATE_NOOP("appshell/preferences", "Appearance"), IconCode::Code::EYE_OPEN,
        //          "Preferences/AppearancePreferencesPage.qml"),

        // makeItem("canvas", QT_TRANSLATE_NOOP("appshell/preferences", "Canvas"), IconCode::Code::NEW_FILE,
        //          "Preferences/CanvasPreferencesPage.qml"),

        // makeItem("cloud", QT_TRANSLATE_NOOP("appshell/preferences", "Save & publish"), IconCode::Code::CLOUD_FILE,
        //          "Preferences/SaveAndPublishPreferencesPage.qml"),

        // makeItem("note-input", QT_TRANSLATE_NOOP("appshell/preferences", "Note input"), IconCode::Code::EDIT,
        //          "Preferences/NoteInputPreferencesPage.qml"),

        // makeItem("score", QT_TRANSLATE_NOOP("appshell/preferences", "Score"), IconCode::Code::SCORE,
        //          "Preferences/ScorePreferencesPage.qml"),

        // makeItem("audio-midi", QT_TRANSLATE_NOOP("appshell/preferences", "Audio & MIDI"), IconCode::Code::AUDIO,
        //          "Preferences/AudioMidiPreferencesPage.qml"),

        // makeItem("midi-device-mapping", QT_TRANSLATE_NOOP("appshell/preferences", "MIDI mappings"), IconCode::Code::MIDI_INPUT,
        //          "Preferences/MidiDeviceMappingPreferencesPage.qml"),

        // makeItem("percussion", QT_TRANSLATE_NOOP("appshell/preferences", "Percussion"), IconCode::Code::PERCUSSION,
        //          "Preferences/PercussionPreferencesPage.qml"),

        // makeItem("import", QT_TRANSLATE_NOOP("appshell/preferences", "Import"), IconCode::Code::IMPORT,
        //          "Preferences/ImportPreferencesPage.qml"),

        // makeItem("shortcuts", QT_TRANSLATE_NOOP("appshell/preferences", "Shortcuts"), IconCode::Code::SHORTCUTS,
        //          "Preferences/ShortcutsPreferencesPage.qml"),

        // makeItem("update", QT_TRANSLATE_NOOP("appshell/preferences", "Update"), IconCode::Code::UPDATE,
        //          "Preferences/UpdatePreferencesPage.qml"),

        // makeItem("general-folders", QT_TRANSLATE_NOOP("appshell/preferences", "Folders"), IconCode::Code::OPEN_FILE,
        //          "Preferences/FoldersPreferencesPage.qml"),

        // makeItem("advanced", QT_TRANSLATE_NOOP("appshell/preferences", "Advanced"), IconCode::Code::CONFIGURE,
        //          "Preferences/AdvancedPreferencesPage.qml"),

        // makeItem("braille", QT_TRANSLATE_NOOP("appshell/preferences", "Braille"), IconCode::Code::BRAILLE,
        //          "Preferences/BraillePreferencesPage.qml")
    };

    for (PreferencePageItem* item: items) {
        m_rootItem->appendChild(item);
    }

    endResetModel();
}

bool RepairModel::askForConfirmationOfRepairReset()
{
    std::string title = muse::trc("appshell", "Are you sure you want to reset preferences?");
    std::string question = muse::trc("appshell", "This action will reset all your app preferences and delete all custom shortcuts. "
                                                 "It will not delete any of your scores.\n\n"
                                                 "This action cannot be undone.");

    muse::IInteractive::ButtonData cancelBtn = interactive()->buttonData(muse::IInteractive::Button::Cancel);
    muse::IInteractive::ButtonData resetBtn = interactive()->buttonData(muse::IInteractive::Button::Reset);
    cancelBtn.accent = true;

    muse::IInteractive::Result result = interactive()->warning(title, question, { cancelBtn, resetBtn }, cancelBtn.btn,
                                                               { muse::IInteractive::Option::WithIcon },
                                                               muse::trc("appshell", "Reset preferences"));
    return result.standardButton() == muse::IInteractive::Button::Reset;
}

void RepairModel::resetFactorySettings()
{
    static constexpr bool KEEP_DEFAULT_SETTINGS = true;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();
    configuration()->revertToFactorySettings(KEEP_DEFAULT_SETTINGS);

    // Unreset the "First Launch Completed" setting so the first-time launch wizard does not appear.
    configuration()->setHasCompletedFirstLaunchSetup(true);

    configuration()->startEditSettings();
    QApplication::restoreOverrideCursor();
}

void RepairModel::apply()
{
    configuration()->applySettings();
}

void RepairModel::cancel()
{
    configuration()->rollbackSettings();
}

void RepairModel::selectRow(const QModelIndex& rowIndex)
{
    QModelIndex parentItemIndex = parent(rowIndex);
    PreferencePageItem* parentItem = nullptr;
    if (!parentItemIndex.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = modelIndexToItem(parentItemIndex);
    }

    QList<PreferencePageItem*> children = parentItem->childrenItems();
    for (PreferencePageItem* child: children) {
        child->setExpanded(false);
    }

    PreferencePageItem* selectedItem = parentItem->childAtRow(rowIndex.row());
    if (!selectedItem) {
        return;
    }

    selectedItem->setExpanded(true);
    setCurrentPageId(selectedItem->id());
}

QVariantList RepairModel::availablePages() const
{
    std::function<QVariantList(const PreferencePageItem*)> childPages;
    childPages = [&childPages](const PreferencePageItem* item) {
        QVariantList result;

        for (int i = 0; i < item->childCount(); ++i) {
            PreferencePageItem* child = item->childAtRow(i);
            QVariantMap childObj;
            childObj["id"] = child->id();
            childObj["path"] = child->path();
            result << childObj;

            QVariantList pages = childPages(child);
            for (const QVariant& page: pages) {
                result << page;
            }
        }

        return result;
    };

    return childPages(m_rootItem);
}

void RepairModel::setCurrentPageId(QString currentPageId)
{
    if (m_currentPageId == currentPageId) {
        return;
    }

    m_currentPageId = currentPageId;
    configuration()->setPreferencesDialogLastOpenedPageId(currentPageId);
    emit currentPageIdChanged(m_currentPageId);
}

PreferencePageItem* RepairModel::makeItem(const QString& id, const QString& title, muse::ui::IconCode::Code icon,
                                               const QString& path,
                                               const QList<PreferencePageItem*>& children) const
{
    PreferencePageItem* item = new PreferencePageItem();
    item->setId(id);
    item->setTitle(title);
    item->setIcon(icon);
    item->setPath(path);
    item->setExpanded(id == currentPageId());

    for (PreferencePageItem* child: children) {
        item->appendChild(child);

        if (child->id() == currentPageId()) {
            item->setExpanded(true);
        }
    }

    return item;
}

PreferencePageItem* RepairModel::modelIndexToItem(const QModelIndex& index) const
{
    return static_cast<PreferencePageItem*>(index.internalPointer());
}
