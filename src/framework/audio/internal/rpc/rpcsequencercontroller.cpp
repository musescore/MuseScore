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
#include "rpcsequencercontroller.h"

#include "log.h"
#include "internal/worker/audioengine.h"

using namespace mu::audio;
using namespace mu::audio::rpc;

static Target rpcSeqTarget = Target(TargetName::Sequencer);

TargetName RpcSequencerController::target() const
{
    return TargetName::Sequencer;
}

ISequencerPtr RpcSequencerController::sequencer() const
{
    return AudioEngine::instance()->sequencer();
}

void RpcSequencerController::doBind()
{
    bindMethod("initMIDITrack", [this](const Args& args) {
        sequencer()->initMIDITrack(args.arg<ISequencer::TrackID>(0));
    });

    bindMethod("initAudioTrack", [this](const Args& args) {
        sequencer()->initAudioTrack(args.arg<ISequencer::TrackID>(0));
    });

    bindMethod("setMIDITrack", [this](const Args& args) {
        if (isSerialized()) {
            NOT_IMPLEMENTED;
        } else {
            sequencer()->setMIDITrack(args.arg<ISequencer::TrackID>(0), args.arg<std::shared_ptr<midi::MidiStream> >(1));
        }
    });

    bindMethod("setAudioTrack", [this](const Args& args) {
        if (isSerialized()) {
            NOT_IMPLEMENTED;
        } else {
            sequencer()->setAudioTrack(args.arg<ISequencer::TrackID>(0), args.arg<std::shared_ptr<audio::IAudioStream> >(1));
        }
    });

    bindMethod("play", [this](const Args&) {
        sequencer()->play();
    });

    bindMethod("pause", [this](const Args&) {
        sequencer()->pause();
    });

    bindMethod("stop", [this](const Args&) {
        sequencer()->stop();
    });

    bindMethod("seek", [this](const Args& args) {
        sequencer()->seek(args.arg<uint64_t>(0));
    });

    bindMethod("rewind", [this](const Args&) {
        sequencer()->rewind();
    });

    bindMethod("setLoop", [this](const Args& args) {
        sequencer()->setLoop(args.arg<uint64_t>(0), args.arg<uint64_t>(1));
    });

    bindMethod("unsetLoop", [this](const Args&) {
        sequencer()->unsetLoop();
    });

    bindMethod("instantlyPlayMidi", [this](const Args& args) {
        if (isSerialized()) {
            NOT_IMPLEMENTED;
        } else {
            sequencer()->instantlyPlayMidi(args.arg<midi::MidiData>(0));
        }
    });

    // Bind from sequencer

    sequencer()->statusChanged().onReceive(this, [this](const ISequencer::Status& status) {
        sendToMain(Msg(rpcSeqTarget, "statusChanged", Args::make_arg1<ISequencer::Status>(status)));
    });

    //! TODO I highly doubt that the position and play tick should be transmitted at this point from the sequencer.
    //! There is also a buffer between the sequencer and the audio driver, which introduces, though not large (at the moment), but the delay.
    //! I think both the position and the tick should be transferred together with the audio data
    //! to the audio driver and from there already taken, then they will always be synchronized with the sound that is currently being played.

    bindMethod("bindMidiTickPlayed", [this](const Args& args) {
        ISequencer::TrackID trackID = args.arg<ISequencer::TrackID>(0);
        async::Channel<midi::tick_t> ch = sequencer()->midiTickPlayed(trackID);
        ch.onReceive(this, [this, trackID](midi::tick_t tick) {
            sendToMain(Msg(rpcSeqTarget, "midiTickPlayed", Args::make_arg2<ISequencer::TrackID, midi::tick_t>(trackID, tick)));
        });
    });

    sequencer()->positionChanged().onNotify(this, [this]() {
        float pos = sequencer()->playbackPositionInSeconds();
        sendToMain(Msg(rpcSeqTarget, "positionChanged", Args::make_arg1<float>(pos)));
    });
}
