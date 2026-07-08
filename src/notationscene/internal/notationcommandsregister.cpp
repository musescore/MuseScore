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
    },

    // note input
    CommandInfo{
        TOGGLE_NOTE_INPUT_COMMAND,
        TranslatableString("notation", "Note input"),
        TranslatableString("notation", "Toggle note input mode"),
        InputSchema(),
        Decoration(IconCode::Code::EDIT, rcommand::Checkable::Yes)
    },
    CommandInfo{
        TOGGLE_NOTE_INPUT_BY_NOTE_NAME_COMMAND,
        TranslatableString("notation", "Note input by note name"),
        TranslatableString("notation", "Toggle note input mode by note name"),
        InputSchema(),
        Decoration(IconCode::Code::EDIT, rcommand::Checkable::Yes)
    },
    CommandInfo{
        TOGGLE_NOTE_INPUT_BY_DURATION_COMMAND,
        TranslatableString("notation", "Note input by duration"),
        TranslatableString("notation", "Toggle note input mode by duration"),
        InputSchema(),
        Decoration(IconCode::Code::DURATION_CURSOR, rcommand::Checkable::Yes)
    },
    CommandInfo{
        TOGGLE_NOTE_INPUT_RHYTHM_COMMAND,
        TranslatableString("notation", "Rhythm only (not pitch)"),
        TranslatableString("notation", "Toggle note input mode: rhythm only (not pitch)"),
        InputSchema(),
        Decoration(IconCode::Code::RHYTHM_ONLY, rcommand::Checkable::Yes)
    },
    CommandInfo{
        TOGGLE_NOTE_INPUT_REPITCH_COMMAND,
        TranslatableString("notation", "Re-pitch existing notes"),
        TranslatableString("notation", "Toggle note input mode: re-pitch existing notes"),
        InputSchema(),
        Decoration(IconCode::Code::RE_PITCH, rcommand::Checkable::Yes)
    },
    CommandInfo{
        TOGGLE_NOTE_INPUT_REALTIME_AUTO_COMMAND,
        TranslatableString("notation", "Real-time (metronome)"),
        TranslatableString("notation", "Toggle note input mode: real-time (metronome)"),
        InputSchema(),
        Decoration(IconCode::Code::METRONOME, rcommand::Checkable::Yes)
    },
    CommandInfo{
        TOGGLE_NOTE_INPUT_REALTIME_MANUAL_COMMAND,
        TranslatableString("notation", "Real-time (foot pedal)"),
        TranslatableString("notation", "Toggle note input mode: real-time (foot pedal)"),
        InputSchema(),
        Decoration(IconCode::Code::FOOT_PEDAL, rcommand::Checkable::Yes)
    },
    CommandInfo{
        TOGGLE_NOTE_INPUT_TIMEWISE_COMMAND,
        TranslatableString("notation", "Insert"),
        TranslatableString("notation", "Toggle note input mode: insert (increases measure duration)"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_PLUS, rcommand::Checkable::Yes)
    },
    CommandInfo{
        REALTIME_ADVANCE_COMMAND,
        TranslatableString("notation", "Real-time advance"),
        TranslatableString("notation", "Real-time advance"),
        InputSchema(),
        Decoration(IconCode::Code::METRONOME)
    },
    CommandInfo{
        NOTE_LONGA_COMMAND,
        TranslatableString("notation", "Longa"),
        TranslatableString("notation", "Set duration: longa"),
        InputSchema(),
        Decoration(IconCode::Code::LONGO)
    },
    CommandInfo{
        NOTE_BREVE_COMMAND,
        TranslatableString("notation", "Breve"),
        TranslatableString("notation", "Set duration: breve"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_WHOLE_DOUBLE)
    },
    CommandInfo{
        PAD_NOTE_1_COMMAND,
        TranslatableString("notation", "Whole note"),
        TranslatableString("notation", "Set duration: whole note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_WHOLE)
    },
    CommandInfo{
        PAD_NOTE_2_COMMAND,
        TranslatableString("notation", "Half note"),
        TranslatableString("notation", "Set duration: half note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_HALF)
    },
    CommandInfo{
        PAD_NOTE_4_COMMAND,
        TranslatableString("notation", "Quarter note"),
        TranslatableString("notation", "Set duration: quarter note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_QUARTER)
    },
    CommandInfo{
        PAD_NOTE_8_COMMAND,
        TranslatableString("notation", "Eighth note"),
        TranslatableString("notation", "Set duration: eighth note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_8TH)
    },
    CommandInfo{
        PAD_NOTE_16_COMMAND,
        TranslatableString("notation", "16th note"),
        TranslatableString("notation", "Set duration: 16th note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_16TH)
    },
    CommandInfo{
        PAD_NOTE_32_COMMAND,
        TranslatableString("notation", "32nd note"),
        TranslatableString("notation", "Set duration: 32nd note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_32ND)
    },
    CommandInfo{
        PAD_NOTE_64_COMMAND,
        TranslatableString("notation", "64th note"),
        TranslatableString("notation", "Set duration: 64th note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_64TH)
    },
    CommandInfo{
        PAD_NOTE_128_COMMAND,
        TranslatableString("notation", "128th note"),
        TranslatableString("notation", "Set duration: 128th note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_128TH)
    },
    CommandInfo{
        PAD_NOTE_256_COMMAND,
        TranslatableString("notation", "256th note"),
        TranslatableString("notation", "Set duration: 256th note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_256TH)
    },
    CommandInfo{
        PAD_NOTE_512_COMMAND,
        TranslatableString("notation", "512th note"),
        TranslatableString("notation", "Set duration: 512th note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_512TH)
    },
    CommandInfo{
        PAD_NOTE_1024_COMMAND,
        TranslatableString("notation", "1024th note"),
        TranslatableString("notation", "Set duration: 1024th note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_1024TH)
    },
    CommandInfo{
        PAD_DOT_COMMAND,
        TranslatableString("notation", "Augmentation dot"),
        TranslatableString("notation", "Toggle duration dot"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_DOTTED)
    },
    CommandInfo{
        PAD_DOT2_COMMAND,
        TranslatableString("notation", "Double augmentation dot"),
        TranslatableString("notation", "Toggle duration dot: double"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_DOTTED_2)
    },
    CommandInfo{
        PAD_DOT3_COMMAND,
        TranslatableString("notation", "Triple augmentation dot"),
        TranslatableString("notation", "Toggle duration dot: triple"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_DOTTED_3)
    },
    CommandInfo{
        PAD_DOT4_COMMAND,
        TranslatableString("notation", "Quadruple augmentation dot"),
        TranslatableString("notation", "Toggle duration dot: quadruple"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_DOTTED_4)
    },
    CommandInfo{
        PAD_REST_COMMAND,
        TranslatableString("notation", "Rest"),
        TranslatableString("notation", "Toggle rest"),
        InputSchema(),
        Decoration(IconCode::Code::REST)
    },
    CommandInfo{
        TOGGLE_FLAT2_COMMAND,
        TranslatableString("notation", "Toggle double-flat"),
        TranslatableString("notation", "Toggle accidental: double-flat"),
        InputSchema(),
        Decoration(IconCode::Code::FLAT_DOUBLE)
    },
    CommandInfo{
        TOGGLE_FLAT_COMMAND,
        TranslatableString("notation", "Toggle flat"),
        TranslatableString("notation", "Toggle accidental: flat"),
        InputSchema(),
        Decoration(IconCode::Code::FLAT)
    },
    CommandInfo{
        TOGGLE_NAT_COMMAND,
        TranslatableString("notation", "Toggle natural"),
        TranslatableString("notation", "Toggle accidental: natural"),
        InputSchema(),
        Decoration(IconCode::Code::NATURAL)
    },
    CommandInfo{
        TOGGLE_SHARP_COMMAND,
        TranslatableString("notation", "Toggle sharp"),
        TranslatableString("notation", "Toggle accidental: sharp"),
        InputSchema(),
        Decoration(IconCode::Code::SHARP)
    },
    CommandInfo{
        TOGGLE_SHARP2_COMMAND,
        TranslatableString("notation", "Toggle double-sharp"),
        TranslatableString("notation", "Toggle accidental: double-sharp"),
        InputSchema(),
        Decoration(IconCode::Code::SHARP_DOUBLE)
    },
    CommandInfo{
        ADD_TIE_COMMAND,
        TranslatableString("notation", "Tie"),
        TranslatableString("notation", "Add tied note"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_TIE)
    },
    CommandInfo{
        ADD_SLUR_COMMAND,
        TranslatableString("notation", "Slur"),
        TranslatableString("notation", "Add slur"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_SLUR)
    },
    CommandInfo{
        ADD_LV_COMMAND,
        TranslatableString("notation", "Laissez vibrer"),
        TranslatableString("notation", "Add laissez vibrer"),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_LV)
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
