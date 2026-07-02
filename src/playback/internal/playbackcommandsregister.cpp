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

#include "playbackcommandsregister.h"

#include "../playbackcommands.h"

using namespace muse;
using namespace muse::rcommand;
using namespace muse::ui;
using namespace mu::playback;

static const std::vector<CommandInfo> s_commandInfos = {
    CommandInfo{
        PLAY_TOGGLE_COMMAND,
        TranslatableString("playback", "Play toggle"),
        TranslatableString("playback", "Toggle playback of current score"),
        InputSchema(),
        Decoration(IconCode::Code::PLAY)
    },
    CommandInfo{
        PLAY_COMMAND,
        TranslatableString("playback", "Play"),
        TranslatableString("playback", "Play the current score"),
        InputSchema(),
        Decoration(IconCode::Code::PLAY)
    },
    CommandInfo{
        PLAY_SELECTION_COMMAND,
        TranslatableString("playback", "Play from selection"),
        TranslatableString("playback", "Play from selection"),
        InputSchema(),
        Decoration(IconCode::Code::PLAY)
    },
    CommandInfo{
        PAUSE_COMMAND,
        TranslatableString("playback", "Pause"),
        TranslatableString("playback", "Pause playback"),
        InputSchema(),
        Decoration(IconCode::Code::PAUSE)
    },
    CommandInfo{
        PAUSE_AND_SELECT_COMMAND,
        TranslatableString("playback", "Pause and select"),
        TranslatableString("playback", "Pause and select playback position"),
        InputSchema(),
        Decoration(IconCode::Code::PAUSE)
    },
    CommandInfo{
        STOP_COMMAND,
        TranslatableString("playback", "Stop"),
        TranslatableString("playback", "Stop playback"),
        InputSchema(),
        Decoration(IconCode::Code::STOP)
    },
    CommandInfo{
        REWIND_COMMAND,
        TranslatableString("playback", "Rewind"),
        TranslatableString("playback", "Rewind"),
        InputSchema({ { "position", Arg(DataType::Float, u"Playback position in seconds", Val(0)) } }),
        Decoration(IconCode::Code::REWIND)
    },
    CommandInfo{
        LOOP_TOGGLE_COMMAND,
        TranslatableString("playback", "Loop toggle"),
        TranslatableString("playback", "Toggle loop playback"),
        InputSchema(),
        Decoration(IconCode::Code::LOOP, rcommand::Checkable::Yes)
    },

    CommandInfo{
        LOOP_IN_COMMAND,
        TranslatableString("playback", "Loop in"),
        TranslatableString("playback", "Set loop marker left"),
        InputSchema(),
        Decoration(IconCode::Code::LOOP_IN)
    },
    CommandInfo{
        LOOP_OUT_COMMAND,
        TranslatableString("playback", "Loop out"),
        TranslatableString("playback", "Set loop marker right"),
        InputSchema(),
        Decoration(IconCode::Code::LOOP_OUT)
    },
    CommandInfo{
        METRONOME_TOGGLE_COMMAND,
        TranslatableString("playback", "Metronome toggle"),
        TranslatableString("playback", "Toggle metronome playback"),
        InputSchema(),
        Decoration(IconCode::Code::METRONOME, rcommand::Checkable::Yes)
    },
    CommandInfo{
        SHOW_PLAYBACK_SETUP_COMMAND,
        TranslatableString("playback", "Playback setup"),
        TranslatableString("playback", "Show playback setup"),
        InputSchema(),
        Decoration(IconCode::Code::NONE)
    },
    CommandInfo{
        MIDI_TOGGLE_COMMAND,
        TranslatableString("playback", "MIDI toggle"),
        TranslatableString("playback", "Toggle MIDI input"),
        InputSchema(),
        Decoration(IconCode::Code::MIDI_INPUT, rcommand::Checkable::Yes)
    },
    CommandInfo{
        MIDI_INPUT_WRITTEN_PITCH_COMMAND,
        TranslatableString("playback", "Written pitch"),
        TranslatableString("playback", "Input written pitch"),
        InputSchema(),
        Decoration(IconCode::Code::NONE, rcommand::Checkable::Yes)
    },
    CommandInfo{
        MIDI_INPUT_SOUNDING_PITCH_COMMAND,
        TranslatableString("playback", "Sounding pitch"),
        TranslatableString("playback", "Input sounding pitch"),
        InputSchema(),
        Decoration(IconCode::Code::NONE, rcommand::Checkable::Yes)
    },
    CommandInfo{
        REPEATS_TOGGLE_COMMAND,
        TranslatableString("playback", "Play repeats"),
        TranslatableString("playback", "Toggle play repeats"),
        InputSchema(),
        Decoration(IconCode::Code::PLAY_REPEATS, rcommand::Checkable::Yes)
    },
    CommandInfo{
        CHORDSYMBOLS_TOGGLE_COMMAND,
        TranslatableString("playback", "Play chord symbols"),
        TranslatableString("playback", "Toggle play chord symbols"),
        InputSchema(),
        Decoration(IconCode::Code::CHORD_SYMBOL, rcommand::Checkable::Yes)
    },
    CommandInfo{
        HEAR_PLAYBACK_WHEN_EDITING_TOGGLE_COMMAND,
        TranslatableString("playback", "Hear playback when editing"),
        TranslatableString("playback", "Toggle hear playback when editing"),
        InputSchema(),
        Decoration(IconCode::Code::AUDIO, rcommand::Checkable::Yes)
    },
    CommandInfo{
        PAN_TOGGLE_COMMAND,
        TranslatableString("playback", "Pan score automatically"),
        TranslatableString("playback", "Toggle pan score automatically during playback"),
        InputSchema(),
        Decoration(IconCode::Code::PAN_SCORE, rcommand::Checkable::Yes)
    },
    CommandInfo{
        COUNTIN_TOGGLE_COMMAND,
        TranslatableString("playback", "Count-in when playing"),
        TranslatableString("playback", "Toggle count-in when playing"),
        InputSchema(),
        Decoration(IconCode::Code::COUNT_IN, rcommand::Checkable::Yes)
    },
    CommandInfo{
        CLEAR_ONLINESOUNDS_CACHE_COMMAND,
        TranslatableString("playback", "Clear online sounds cache"),
        TranslatableString("playback", "Clear online sounds cache"),
        InputSchema(),
        Decoration(IconCode::Code::NONE)
    },
    CommandInfo{
        PROCESS_ONLINESOUNDS_COMMAND,
        TranslatableString("playback", "Process online sounds"),
        TranslatableString("playback", "Process online sounds"),
        InputSchema(),
        Decoration(IconCode::Code::NONE)
    },
    CommandInfo{
        RELOAD_PLAYBACK_CACHE_COMMAND,
        TranslatableString("playback", "Reload playback cache"),
        TranslatableString("playback", "Reload playback cache"),
        InputSchema(),
        Decoration(IconCode::Code::NONE)
    }
};

std::string PlaybackCommandsRegister::moduleName() const
{
    return "playback";
}

const std::vector<muse::rcommand::Command>& PlaybackCommandsRegister::commandList() const
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

const std::vector<CommandInfo>& PlaybackCommandsRegister::commandInfoList() const
{
    return s_commandInfos;
}
