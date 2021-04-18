//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#include "shortcutsconfiguration.h"

#include "settings.h"

using namespace mu::shortcuts;
using namespace mu::framework;

static const std::string SHORTCUTS_FILE_NAME("shortcuts.xml");
static const std::string SHORTCUTS_DEFAULT_FILE_PATH(":/data/" + SHORTCUTS_FILE_NAME);

static const Settings::Key USER_PATH_KEY("shortcuts", "application/paths/myShortcuts");

void ShortcutsConfiguration::init()
{
    io::path defaultUserPath = globalConfiguration()->dataPath() + "/" + SHORTCUTS_FILE_NAME;
    settings()->setDefaultValue(USER_PATH_KEY, Val(defaultUserPath.toStdString()));

    settings()->valueChanged(USER_PATH_KEY).onReceive(this, [this](const Val& val) {
        m_userPathChanged.send(val.toString());
    });
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
