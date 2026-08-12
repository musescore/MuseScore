/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) MuseScore Limited and others
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

#include "appshellcommandsregister.h"

#include "../appshellcommands.h"
#include "rcommand/commandtypes.h"

using namespace muse;
using namespace muse::rcommand;
using namespace muse::ui;
using namespace mu::appshell;

static const std::vector<CommandInfo> s_commandInfos = {
    CommandInfo(
        APP_QUIT_COMMAND,
        TranslatableString("action", "Quit"),
        TranslatableString("action", "Quit"),
        InputSchema({
        { "all_instances", Arg(DataType::Boolean, u"All instances (optional)") },
        { "installer_path", Arg(DataType::String, u"Installer path (optional)") } }),
        Decoration()
        ),
    CommandInfo(
        APP_RESTART_COMMAND,
        TranslatableString("action", "Restart"),
        TranslatableString("action", "Restart"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        APP_FULLSCREEN_COMMAND,
        TranslatableString("action", "&Full screen"),
        TranslatableString("action", "Full screen"),
        InputSchema(),
        Decoration(rcommand::Checkable::Yes)
        ),

    CommandInfo(
        APP_ABOUT_MUSESCORE_COMMAND,
        TranslatableString("action", "&About MuseScore Studio…"),
        TranslatableString("action", "About MuseScore Studio"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        APP_ABOUT_QT_COMMAND,
        TranslatableString("action", "About &Qt…"),
        TranslatableString("action", "About Qt"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        APP_ABOUT_MUSICXML_COMMAND,
        TranslatableString("action", "About &MusicXML…"),
        TranslatableString("action", "About MusicXML"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        APP_ONLINE_HANDBOOK_COMMAND,
        TranslatableString("action", "Online &handbook"),
        TranslatableString("action", "Open online handbook"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        APP_ASK_HELP_COMMAND,
        TranslatableString("action", "As&k for help"),
        TranslatableString("action", "Ask for help"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        APP_ACCESSIBILITY_STATEMENT_COMMAND,
        TranslatableString("action", "Accessibility &statement"),
        TranslatableString("action", "Accessibility statement"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        APP_PREFERENCES_COMMAND,
        TranslatableString("action", "&Preferences…"),
        TranslatableString("action", "Preferences"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        APP_REVERT_TO_FACTORY_COMMAND,
        TranslatableString("action", "Revert to factory settings"),
        TranslatableString("action", "Revert to factory settings"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        APP_EXTENSIONS_COMMAND,
        TranslatableString("action", "Manage &extensions"),
        TranslatableString("action", "Manage extensions"),
        InputSchema(),
        Decoration()
        ),

    // docks commands
    CommandInfo(
        DOCK_TOGGLE_PLAYBACK_COMMAND,
        TranslatableString("action", "Toggle playback"),
        TranslatableString("action", "Dock: Toggle playback"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        DOCK_TOGGLE_NOTEINPUT_COMMAND,
        TranslatableString("action", "Toggle note input"),
        TranslatableString("action", "Dock: Toggle note input"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        DOCK_TOGGLE_PALETTES_COMMAND,
        TranslatableString("action", "Toggle palettes"),
        TranslatableString("action", "Dock: Toggle palettes"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        DOCK_TOGGLE_INSTRUMENTS_COMMAND,
        TranslatableString("action", "Toggle instruments"),
        TranslatableString("action", "Dock: Toggle instruments"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        DOCK_TOGGLE_PROPERTIES_COMMAND,
        TranslatableString("action", "Toggle properties"),
        TranslatableString("action", "Dock: Toggle properties"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        DOCK_TOGGLE_SELECTION_FILTER_COMMAND,
        TranslatableString("action", "Toggle selection filter"),
        TranslatableString("action", "Dock: Toggle selection filter"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        DOCK_TOGGLE_UNDO_HISTORY_COMMAND,
        TranslatableString("action", "Toggle undo history"),
        TranslatableString("action", "Dock: Toggle undo history"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        DOCK_TOGGLE_NAVIGATOR_COMMAND,
        TranslatableString("action", "Toggle navigator"),
        TranslatableString("action", "Dock: Toggle navigator"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        DOCK_TOGGLE_BRAILLE_COMMAND,
        TranslatableString("action", "Toggle braille"),
        TranslatableString("action", "Dock: Toggle braille"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        DOCK_TOGGLE_TIMELINE_COMMAND,
        TranslatableString("action", "Toggle timeline"),
        TranslatableString("action", "Dock: Toggle timeline"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        DOCK_TOGGLE_MIXER_COMMAND,
        TranslatableString("action", "Toggle mixer"),
        TranslatableString("action", "Dock: Toggle mixer"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        DOCK_TOGGLE_PIANO_KEYBOARD_COMMAND,
        TranslatableString("action", "Toggle piano keyboard"),
        TranslatableString("action", "Dock: Toggle piano keyboard"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        DOCK_TOGGLE_PERCUSSION_COMMAND,
        TranslatableString("action", "Toggle percussion"),
        TranslatableString("action", "Dock: Toggle percussion"),
        InputSchema(),
        Decoration()
        ),
    CommandInfo(
        DOCK_TOGGLE_STATUSBAR_COMMAND,
        TranslatableString("action", "Toggle statusbar"),
        TranslatableString("action", "Dock: Toggle statusbar"),
        InputSchema(),
        Decoration()
        ),
};

std::string AppshellCommandsRegister::moduleName() const
{
    return "appshell";
}

const std::vector<muse::rcommand::Command>& AppshellCommandsRegister::commandList() const
{
    static std::vector<muse::rcommand::Command> commands;
    if (commands.empty()) {
        commands.reserve(s_commandInfos.size());
        for (const auto& info : s_commandInfos) {
            commands.push_back(info.command);
        }
    }
    return commands;
}

const std::vector<CommandInfo>& AppshellCommandsRegister::commandInfoList() const
{
    return s_commandInfos;
}
