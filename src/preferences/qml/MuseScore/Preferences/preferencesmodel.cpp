/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <QGuiApplication>

#include "preferencesmodel.h"

#include "translation.h"
#include "ui/view/iconcodes.h"

#include "muse_framework_config.h"

using namespace mu::preferences;
using namespace muse::ui;

PreferencesModel::PreferencesModel(QObject* parent)
    : QAbstractListModel(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

PreferencesModel::~PreferencesModel()
{
    cancel();
}

int PreferencesModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QVariant PreferencesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size() || role != ItemRole) {
        return QVariant();
    }

    return QVariant::fromValue(qobject_cast<QObject*>(m_items.at(index.row())));
}

QHash<int, QByteArray> PreferencesModel::roleNames() const
{
    return { { ItemRole, "item" } };
}

QString PreferencesModel::currentPageId() const
{
    return m_currentPageId;
}

void PreferencesModel::load(const QString& currentPageId)
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

    qDeleteAll(m_items);

    m_items = {
        makeItem("general", QT_TRANSLATE_NOOP("preferences", "General"), IconCode::Code::SETTINGS_COG,
                 "Preferences/GeneralPreferencesPage.qml"),

        makeItem("appearance", QT_TRANSLATE_NOOP("preferences", "Appearance"), IconCode::Code::EYE_OPEN,
                 "Preferences/AppearancePreferencesPage.qml"),

        makeItem("canvas", QT_TRANSLATE_NOOP("preferences", "Canvas"), IconCode::Code::NEW_FILE,
                 "Preferences/CanvasPreferencesPage.qml"),

        makeItem("cloud", QT_TRANSLATE_NOOP("preferences", "Save & publish"), IconCode::Code::CLOUD_FILE,
                 "Preferences/SaveAndPublishPreferencesPage.qml"),

        makeItem("note-input", QT_TRANSLATE_NOOP("preferences", "Note input"), IconCode::Code::EDIT,
                 "Preferences/NoteInputPreferencesPage.qml"),

        makeItem("score", QT_TRANSLATE_NOOP("preferences", "Score"), IconCode::Code::SCORE,
                 "Preferences/ScorePreferencesPage.qml"),

        makeItem("audio-midi", QT_TRANSLATE_NOOP("preferences", "Audio & MIDI"), IconCode::Code::AUDIO,
                 "Preferences/AudioMidiPreferencesPage.qml"),

#ifdef MUSE_MODULE_MIDIREMOTE
        makeItem("midi-device-mapping", QT_TRANSLATE_NOOP("preferences", "MIDI mappings"), IconCode::Code::MIDI_INPUT,
                 "Preferences/MidiDeviceMappingPreferencesPage.qml"),
#endif

        makeItem("video", QT_TRANSLATE_NOOP("preferences", "Video"), IconCode::Code::VIDEO,
                 "Preferences/VideoPreferencesPage.qml"),

        makeItem("percussion", QT_TRANSLATE_NOOP("preferences", "Percussion"), IconCode::Code::PERCUSSION,
                 "Preferences/PercussionPreferencesPage.qml"),

        makeItem("import", QT_TRANSLATE_NOOP("preferences", "Import"), IconCode::Code::IMPORT,
                 "Preferences/ImportPreferencesPage.qml"),

        makeItem("shortcuts", QT_TRANSLATE_NOOP("preferences", "Shortcuts"), IconCode::Code::SHORTCUTS,
                 "Preferences/ShortcutsPreferencesPage.qml"),

        makeItem("general-folders", QT_TRANSLATE_NOOP("preferences", "Folders"), IconCode::Code::OPEN_FILE,
                 "Preferences/FoldersPreferencesPage.qml"),

        makeItem("advanced", QT_TRANSLATE_NOOP("preferences", "Advanced"), IconCode::Code::CONFIGURE,
                 "Preferences/AdvancedPreferencesPage.qml"),

        makeItem("braille", QT_TRANSLATE_NOOP("preferences", "Braille"), IconCode::Code::BRAILLE,
                 "Preferences/BraillePreferencesPage.qml")
    };

    endResetModel();
}

void PreferencesModel::askForConfirmationOfPreferencesReset()
{
    std::string title = muse::trc("appshell", "Are you sure you want to reset preferences?");
    std::string question = muse::trc("appshell", "This action will reset all your app preferences and delete all custom shortcuts. "
                                                 "It will not delete any of your scores.\n\n"
                                                 "This action cannot be undone.");

    muse::IInteractive::ButtonData cancelBtn = interactive()->buttonData(muse::IInteractive::Button::Cancel);
    muse::IInteractive::ButtonData resetBtn = interactive()->buttonData(muse::IInteractive::Button::Reset);
    cancelBtn.accent = true;

    interactive()->warning(title, question, { cancelBtn, resetBtn }, cancelBtn.btn,
                           { muse::IInteractive::Option::WithIcon },
                           muse::trc("appshell", "Reset preferences"))
    .onResolve(this, [this](const muse::IInteractive::Result& res) {
        if (res.isButton(muse::IInteractive::Button::Reset)) {
            emit userAgreesToResetPreferences();
        }
    });
}

void PreferencesModel::resetFactorySettings()
{
    static constexpr bool KEEP_DEFAULT_SETTINGS = true;
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QGuiApplication::processEvents();
    configuration()->revertToFactorySettings(KEEP_DEFAULT_SETTINGS);

    // Unreset the "First Launch Completed" setting so the first-time launch wizard does not appear.
    configuration()->setHasCompletedFirstLaunchSetup(true);

    configuration()->startEditSettings();
    QGuiApplication::restoreOverrideCursor();
}

void PreferencesModel::apply()
{
    configuration()->applySettings();
}

void PreferencesModel::cancel()
{
    configuration()->rollbackSettings();
}

void PreferencesModel::selectRow(int row)
{
    if (row < 0 || row >= m_items.size()) {
        return;
    }

    setCurrentPageId(m_items.at(row)->id());
}

QVariantList PreferencesModel::availablePages() const
{
    QVariantList result;

    for (const PreferencePageItem* item : m_items) {
        QVariantMap page;
        page["id"] = item->id();
        page["path"] = item->path();
        result << page;
    }

    return result;
}

void PreferencesModel::setCurrentPageId(QString currentPageId)
{
    if (m_currentPageId == currentPageId) {
        return;
    }

    m_currentPageId = currentPageId;
    configuration()->setPreferencesDialogLastOpenedPageId(currentPageId);
    emit currentPageIdChanged(m_currentPageId);
}

PreferencePageItem* PreferencesModel::makeItem(const QString& id, const QString& title, muse::ui::IconCode::Code icon,
                                               const QString& path)
{
    PreferencePageItem* item = new PreferencePageItem(this);
    item->setId(id);
    item->setTitle(title);
    item->setIcon(icon);
    item->setPath(path);

    return item;
}
