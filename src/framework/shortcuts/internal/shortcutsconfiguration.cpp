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

using namespace mu::shortcuts;

static const std::string SHORTCUTS_FILE_NAME("shortcuts.xml");
static const std::string SHORTCUTS_DEFAULT_FILE_PATH(":/data/" + SHORTCUTS_FILE_NAME);

mu::io::path ShortcutsConfiguration::shortcutsUserPath() const
{
    return globalConfiguration()->dataPath() + "/" + SHORTCUTS_FILE_NAME;
}

mu::io::path ShortcutsConfiguration::shortcutsDefaultPath() const
{
    return SHORTCUTS_DEFAULT_FILE_PATH;
}
