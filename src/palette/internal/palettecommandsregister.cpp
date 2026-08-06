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

 #include "palettecommandsregister.h"

 #include "../palettecommands.h"
#include "rcommand/commandtypes.h"

using namespace muse;
using namespace muse::rcommand;
using namespace mu::palette;

static const std::vector<CommandInfo> s_commandInfos = {
    CommandInfo{
        TOGGLE_MASTER_PALETTE_COMMAND,
        TranslatableString("palette", "&Master palette…"),
        TranslatableString("palette", "Open master palette"),
        InputSchema({
            { "palette_name", Arg(DataType::String, u"Selected palette name") }
        }),
        Decoration()
    },
    CommandInfo{
        TOGGLE_SPECIAL_CHARACTERS_COMMAND,
        TranslatableString("palette", "&Special characters…"),
        TranslatableString("palette", "Open special characters"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        OPEN_TIME_SIGNATURE_PROPERTIES_COMMAND,
        TranslatableString("palette", "&Time signature properties…"),
        TranslatableString("palette", "Open time signature properties"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        OPEN_CUSTOMIZE_KIT_COMMAND,
        TranslatableString("palette", "&Customize kit…"),
        TranslatableString("palette", "Open customize kit"),
        InputSchema(),
        Decoration()
    },

    CommandInfo{
        PALETTE_SEARCH_COMMAND,
        TranslatableString("palette", "Palette search"),
        TranslatableString("palette", "Search palettes"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PALETTE_APPLY_CURRENT_ELEMENT_COMMAND,
        TranslatableString("palette", "Apply current palette element"),
        TranslatableString("palette", "Apply current palette element"),
        InputSchema(),
        Decoration()
    },

    CommandInfo{
        PALETTE_TOGGLE_SINGLE_CLICK_TO_OPEN_COMMAND,
        TranslatableString("palette", "Single-click to open a palette"),
        TranslatableString("palette", "Toggle single click to open a palette"),
        InputSchema(),
        Decoration(rcommand::Checkable::Yes)
    },
    CommandInfo{
        PALETTE_TOGGLE_SINGLE_PALETTE_COMMAND,
        TranslatableString("palette", "Open only one palette at a time"),
        TranslatableString("palette", "Toggle open only one palette at a time"),
        InputSchema(),
        Decoration(rcommand::Checkable::Yes)
    },
    CommandInfo{
        PALETTE_TOGGLE_DRAG_ENABLED_COMMAND,
        TranslatableString("palette", "Allow reordering palettes"),
        TranslatableString("palette", "Toggle allow reordering palettes by dragging"),
        InputSchema(),
        Decoration(rcommand::Checkable::Yes)
    },
    CommandInfo{
        PALETTE_EXPAND_ALL_COMMAND,
        TranslatableString("palette", "Expand all palettes"),
        TranslatableString("palette", "Expand all palettes"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PALETTE_COLLAPSE_ALL_COMMAND,
        TranslatableString("palette", "Collapse all palettes"),
        TranslatableString("palette", "Collapse all palettes"),
        InputSchema(),
        Decoration()
    },
};

std::string PaletteCommandsRegister::moduleName() const
{
    return "palette";
}

const std::vector<muse::rcommand::Command>& PaletteCommandsRegister::commandList() const
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

const std::vector<CommandInfo>& PaletteCommandsRegister::commandInfoList() const
{
    return s_commandInfos;
}
