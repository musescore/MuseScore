/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "notationcommandsregister.h"

#include "../notationcommands.h"

using namespace muse;
using namespace muse::rcommand;
using namespace muse::ui;
using namespace mu::notation;

static const std::vector<CommandInfo> s_commandInfos = {
    // copy, cut, paste, delete, cancel
    CommandInfo{
        COPY_COMMAND,
        TranslatableString("notation", "&Copy"),
        TranslatableString("notation", "Copy the current notation element"),
        InputSchema(),
        Decoration(IconCode::Code::COPY)
    },
    CommandInfo{
        CUT_COMMAND,
        TranslatableString("notation", "Cu&t"),
        TranslatableString("notation", "Cut the current notation element"),
        InputSchema(),
        Decoration(IconCode::Code::CUT)
    },
    CommandInfo{
        PASTE_COMMAND,
        TranslatableString("notation", "Past&e"),
        TranslatableString("notation", "Paste the current notation element"),
        InputSchema(),
        Decoration(IconCode::Code::PASTE)
    },
    CommandInfo{
        DELETE_COMMAND,
        TranslatableString("notation", "De&lete"),
        TranslatableString("notation", "Delete the current notation element"),
        InputSchema(),
        Decoration(IconCode::Code::DELETE_TANK)
    },
    CommandInfo{
        CANCEL_COMMAND,
        TranslatableString("notation", "Cancel"),
        TranslatableString("notation", "Cancel the current notation operation"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        UNDO_COMMAND,
        TranslatableString("notation", "Undo"),
        TranslatableString("notation", "Undo the last notation operation"),
        InputSchema(),
        Decoration(IconCode::Code::UNDO)
    },
    CommandInfo{
        REDO_COMMAND,
        TranslatableString("notation", "Redo"),
        TranslatableString("notation", "Redo the last notation operation"),
        InputSchema(),
        Decoration(IconCode::Code::REDO)
    },

    // navigation
    CommandInfo{
        MOVE_RIGHT_COMMAND,
        TranslatableString("notation", "Move right"),
        TranslatableString("notation", "Go to next notation element"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        MOVE_RIGHT_QUICKLY_COMMAND,
        TranslatableString("notation", "Move right quickly"),
        TranslatableString("notation", "Go to next notation element quickly"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        MOVE_LEFT_COMMAND,
        TranslatableString("notation", "Move left"),
        TranslatableString("notation", "Go to previous notation element"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        MOVE_LEFT_QUICKLY_COMMAND,
        TranslatableString("notation", "Move left quickly"),
        TranslatableString("notation", "Go to previous notation element quickly"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PITCH_UP_COMMAND,
        TranslatableString("notation", "Pitch up"),
        TranslatableString("notation", "Pitch up the current note"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PITCH_DOWN_COMMAND,
        TranslatableString("notation", "Pitch down"),
        TranslatableString("notation", "Pitch down the current note"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PITCH_UP_OCTAVE_COMMAND,
        TranslatableString("notation", "Pitch up octave"),
        TranslatableString("notation", "Pitch up the current note by an octave"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PITCH_DOWN_OCTAVE_COMMAND,
        TranslatableString("notation", "Pitch down octave"),
        TranslatableString("notation", "Pitch down the current note by an octave"),
        InputSchema(),
        Decoration()
    },

    // text editing
    CommandInfo{
        EDIT_NEXT_WORD_COMMAND,
        TranslatableString("notation", "Edit next word"),
        TranslatableString("notation", "Go to edit next notation word"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDIT_NEXT_TEXT_ELEMENT_COMMAND,
        TranslatableString("notation", "Edit next text element"),
        TranslatableString("notation", "Go to edit next notation text element"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        EDIT_PREV_TEXT_ELEMENT_COMMAND,
        TranslatableString("notation", "Edit previous text element"),
        TranslatableString("notation", "Go to edit previous notation text element"),
        InputSchema(),
        Decoration()
    }
};

std::string NotationCommandsRegister::moduleName() const
{
    return "notation";
}

const std::vector<Command>& NotationCommandsRegister::commandList() const
{
    static std::vector<Command> commands;
    if (commands.empty()) {
        commands.reserve(s_commandInfos.size());
        for (const auto& info : s_commandInfos) {
            commands.push_back(info.command);
        }
    }
    return commands;
}

const std::vector<CommandInfo>& NotationCommandsRegister::commandInfoList() const
{
    return s_commandInfos;
}
