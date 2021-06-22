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
#include "shortcutsconfiguration.h"

#include "settings.h"

using namespace mu::shortcuts;
using namespace mu::framework;

static const std::string module_name("shortcuts");

static const std::string SHORTCUTS_FILE_NAME("shortcuts.xml");
static const std::string SHORTCUTS_DEFAULT_FILE_PATH(":/data/" + SHORTCUTS_FILE_NAME);

static const std::string MIDIMAPPINGS_FILE_NAME("midi_mappings.xml");

static const Settings::Key USER_PATH_KEY(module_name, "application/paths/myShortcuts");
static const Settings::Key ADVANCE_TO_NEXT_NOTE_ON_KEY_RELEASE(module_name, "io/midi/advanceOnRelease");

void ShortcutsConfiguration::init()
{
    io::path defaultUserPath = globalConfiguration()->userAppDataPath() + "/" + SHORTCUTS_FILE_NAME;
    settings()->setDefaultValue(USER_PATH_KEY, Val(defaultUserPath.toStdString()));

    settings()->valueChanged(USER_PATH_KEY).onReceive(this, [this](const Val& val) {
        m_userPathChanged.send(val.toString());
    });

    settings()->setDefaultValue(ADVANCE_TO_NEXT_NOTE_ON_KEY_RELEASE, Val(true));
}

mu::ValCh<mu::io::path> ShortcutsConfiguration::shortcutsUserPath() const
{
    ValCh<io::path> result;
    result.ch = m_userPathChanged;
    result.val = settings()->value(USER_PATH_KEY).toString();

    return result;
}

void ShortcutsConfiguration::setShortcutsUserPath(const io::path& path)
{
    settings()->setValue(USER_PATH_KEY, Val(path.toStdString()));
}

mu::io::path ShortcutsConfiguration::shortcutsDefaultPath() const
{
    return SHORTCUTS_DEFAULT_FILE_PATH;
}

mu::io::path ShortcutsConfiguration::midiMappingsPath() const
{
    return globalConfiguration()->userAppDataPath() + "/" + MIDIMAPPINGS_FILE_NAME;
}

bool ShortcutsConfiguration::advanceToNextNoteOnKeyRelease() const
{
    return settings()->value(ADVANCE_TO_NEXT_NOTE_ON_KEY_RELEASE).toBool();
}

void ShortcutsConfiguration::setAdvanceToNextNoteOnKeyRelease(bool value)
{
    settings()->setValue(ADVANCE_TO_NEXT_NOTE_ON_KEY_RELEASE, Val(value));
}
