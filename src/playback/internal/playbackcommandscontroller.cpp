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

#include "playbackcommandscontroller.h"

#include <vector>

#include "rcommand/commandtypes.h"
#include "audio/common/audiotypes.h"
#include "engraving/types/constants.h"

#include "../playbackcommands.h"

using namespace muse;
using namespace muse::rcommand;
using namespace muse::audio;
using namespace mu::playback;

void PlaybackCommandsController::init()
{
    registerCommand(PLAY_TOGGLE_COMMAND, &IPlaybackController::togglePlay);
    registerCommand(PLAY_COMMAND, &IPlaybackController::play, false);
    registerCommand(PLAY_SELECTION_COMMAND, &IPlaybackController::playFromSelection, false);
    registerCommand(PAUSE_COMMAND, &IPlaybackController::pause, false);
    registerCommand(PAUSE_AND_SELECT_COMMAND, &IPlaybackController::pause, true);
    registerCommand(STOP_COMMAND, &IPlaybackController::stop);
    registerCommand(REWIND_COMMAND, [this](const rcommand::Params& params) { return rewind(params); });
    registerCommand(LOOP_TOGGLE_COMMAND, &IPlaybackController::toggleLoopPlayback);
    registerCommand(LOOP_IN_COMMAND, &IPlaybackController::addLoopBoundary, LoopBoundaryType::LoopIn);
    registerCommand(LOOP_OUT_COMMAND, &IPlaybackController::addLoopBoundary, LoopBoundaryType::LoopOut);
    registerCommand(METRONOME_TOGGLE_COMMAND, &IPlaybackController::toggleMetronome);
    registerCommand(MIDI_TOGGLE_COMMAND, &IPlaybackController::toggleMidiInput);
    registerCommand(MIDI_INPUT_WRITTEN_PITCH_COMMAND, &IPlaybackController::setMidiUseWrittenPitch, true);
    registerCommand(MIDI_INPUT_SOUNDING_PITCH_COMMAND, &IPlaybackController::setMidiUseWrittenPitch, false);
    registerCommand(REPEATS_TOGGLE_COMMAND, &IPlaybackController::togglePlayRepeats);
    registerCommand(CHORDSYMBOLS_TOGGLE_COMMAND, &IPlaybackController::togglePlayChordSymbols);
    registerCommand(HEAR_PLAYBACK_WHEN_EDITING_TOGGLE_COMMAND, &IPlaybackController::toggleHearPlaybackWhenEditing);
    registerCommand(PAN_TOGGLE_COMMAND, &IPlaybackController::toggleAutomaticallyPan);
    registerCommand(COUNTIN_TOGGLE_COMMAND, &IPlaybackController::toggleCountIn);
    registerCommand(OPEN_PLAYBACK_SETUP_COMMAND, [this]() { return showPlaybackSetup(); });
    registerCommand(RELOAD_PLAYBACK_CACHE_COMMAND, &IPlaybackController::reloadPlaybackCache);

    registerCommand(MIDI_RECORD_TOGGLE_COMMAND, &IPlaybackController::toggleMidiRecording);
    registerCommand(MIDI_RECORD_REQUANTIZE_COMMAND, &IPlaybackController::requantizeLastMidiRecording);

    using mu::engraving::Constants;
    registerCommand(MIDI_RECORD_QUANTIZATION_4_COMMAND, [this]() { return setMidiRecordingMinQuantization(Constants::DIVISION); });
    registerCommand(MIDI_RECORD_QUANTIZATION_8_COMMAND, [this]() { return setMidiRecordingMinQuantization(Constants::DIVISION / 2); });
    registerCommand(MIDI_RECORD_QUANTIZATION_16_COMMAND, [this]() { return setMidiRecordingMinQuantization(Constants::DIVISION / 4); });
    registerCommand(MIDI_RECORD_QUANTIZATION_32_COMMAND, [this]() { return setMidiRecordingMinQuantization(Constants::DIVISION / 8); });
    registerCommand(MIDI_RECORD_TUPLETS_TOGGLE_COMMAND, [this]() { return toggleMidiRecordingTuplets(); });
    registerCommand(MIDI_RECORD_LEGATO_TOGGLE_COMMAND, [this]() { return toggleMidiRecordingLegato(); });
    registerCommand(MIDI_RECORD_COUNTIN_0_COMMAND, [this]() { return setMidiRecordingCountInMeasures(0); });
    registerCommand(MIDI_RECORD_COUNTIN_1_COMMAND, [this]() { return setMidiRecordingCountInMeasures(1); });
    registerCommand(MIDI_RECORD_COUNTIN_2_COMMAND, [this]() { return setMidiRecordingCountInMeasures(2); });
    registerCommand(MIDI_RECORD_ONE_STAFF_COMMAND, [this]() { return setMidiRecordingTwoStaves(false); });
    registerCommand(MIDI_RECORD_TWO_STAVES_COMMAND, [this]() { return setMidiRecordingTwoStaves(true); });
    registerCommand(MIDI_RECORD_SPLIT_POINT_COMMAND, [this]() { return openMidiRecordingSplitPointDialog(); });
    registerCommand(MIDI_RECORD_LATENCY_COMMAND, [this]() { return openMidiRecordingLatencySettings(); });

    registerCommand(TOGGLE_MIXER_SECTION_COMMAND, [this](const rcommand::Params& params) { return toggleMixerSection(params); });
    registerCommand(TOGGLE_AUX_SEND_COMMAND, [this](const rcommand::Params& params) { return toggleAuxSend(params); });
    registerCommand(TOGGLE_AUX_CHANNEL_COMMAND, [this](const rcommand::Params& params) { return toggleAuxChannel(params); });
}

muse::Ret PlaybackCommandsController::rewind(const muse::rcommand::Params& params)
{
    double secs = params.at("position").toDouble();
    return playbackController()->rewind(secs_t(secs));
}

muse::Ret PlaybackCommandsController::showPlaybackSetup()
{
    interactive()->open("musescore://playback/soundprofiles");
    return make_ok();
}

muse::Ret PlaybackCommandsController::toggleMixerSection(const muse::rcommand::Params& params)
{
    MixerSectionType section = str_conv(params.at("section").toString(), MixerSectionType::Unknown);
    if (section == MixerSectionType::Unknown) {
        return make_ret(Ret::Code::BadArgs);
    }

    bool visible = configuration()->isMixerSectionVisible(section);
    configuration()->setMixerSectionVisible(section, !visible);
    return make_ok();
}

muse::Ret PlaybackCommandsController::toggleAuxSend(const muse::rcommand::Params& params)
{
    if (!params.contains("auxsend-index")) {
        return make_ret(Ret::Code::BadArgs);
    }

    aux_channel_idx_t auxSendIndex = static_cast<aux_channel_idx_t>(params.at("auxsend-index").toInt());
    bool visible = configuration()->isAuxSendVisible(auxSendIndex);
    configuration()->setAuxSendVisible(auxSendIndex, !visible);
    return make_ok();
}

muse::Ret PlaybackCommandsController::toggleAuxChannel(const muse::rcommand::Params& params)
{
    if (!params.contains("auxchannel-index")) {
        return make_ret(Ret::Code::BadArgs);
    }

    aux_channel_idx_t auxChannelIndex = static_cast<aux_channel_idx_t>(params.at("auxchannel-index").toInt());
    bool visible = configuration()->isAuxChannelVisible(auxChannelIndex);
    configuration()->setAuxChannelVisible(auxChannelIndex, !visible);
    return make_ok();
}

muse::Ret PlaybackCommandsController::setMidiRecordingMinQuantization(int ticks)
{
    notationConfiguration()->setMidiRecordingMinQuantizationTicks(ticks);
    return make_ok();
}

muse::Ret PlaybackCommandsController::setMidiRecordingCountInMeasures(int measures)
{
    notationConfiguration()->setMidiRecordingCountInMeasures(measures);
    return make_ok();
}

muse::Ret PlaybackCommandsController::toggleMidiRecordingTuplets()
{
    notationConfiguration()->setMidiRecordingSearchTuplets(!notationConfiguration()->midiRecordingSearchTuplets());
    return make_ok();
}

muse::Ret PlaybackCommandsController::toggleMidiRecordingLegato()
{
    notationConfiguration()->setMidiRecordingLegato(!notationConfiguration()->midiRecordingLegato());
    return make_ok();
}

muse::Ret PlaybackCommandsController::setMidiRecordingTwoStaves(bool twoStaves)
{
    notationConfiguration()->setMidiRecordingTwoStaves(twoStaves);
    return make_ok();
}

muse::Ret PlaybackCommandsController::openMidiRecordingSplitPointDialog()
{
    if (playbackController()->isMidiRecording()) {
        return make_ret(Ret::Code::NotSupported);
    }

    interactive()->open("musescore://playback/midirecordingsplitpoint");
    return make_ok();
}

muse::Ret PlaybackCommandsController::openMidiRecordingLatencySettings()
{
    if (playbackController()->isMidiRecording() || playbackController()->isPlaying()) {
        playbackController()->stop();
    }

    //! NOTE The latency compensation is in Preferences > Note input > MIDI devices > Recording with metronome
    UriQuery preferencesUri("muse://preferences");
    preferencesUri.addParam("currentPageId", Val("note-input"));
    interactive()->open(preferencesUri);
    return make_ok();
}

void PlaybackCommandsController::registerCommand(const muse::rcommand::Command& command, const std::function<Ret()>& handler)
{
    dispatcher()->onRequest(this, command, [this, command, handler]() {
        if (!commandsState()->commandState(command).enabled) {
            return muse::make_ret(Ret::Code::NotSupported);
        }

        return handler();
    });
}

void PlaybackCommandsController::registerCommand(const muse::rcommand::Command& command,
                                                 const std::function<muse::Ret(const muse::rcommand::Params&)>& handler)
{
    dispatcher()->onRequest(this, command, [this, command, handler](const muse::rcommand::Params& params) {
        if (!commandsState()->commandState(command).enabled) {
            return muse::make_ret(Ret::Code::NotSupported);
        }

        return handler(params);
    });
}

void PlaybackCommandsController::registerCommand(const muse::rcommand::Command& command, Ret (IPlaybackController::* handler)())
{
    registerCommand(command, [this, handler]()
    {
        return (playbackController().get()->*handler)();
    });
}

template<typename P1>
void PlaybackCommandsController::registerCommand(const muse::rcommand::Command& command, Ret (IPlaybackController::* handler)(P1), P1 p1)
{
    registerCommand(command, [this, handler, p1]()
    {
        return (playbackController().get()->*handler)(p1);
    });
}
