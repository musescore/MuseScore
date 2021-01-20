//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#include "rpccontroller.h"

#include "log.h"

using namespace mu::audio;
using namespace mu::audio::rpc;

static Target rpcSeqTarget = Target(TargetName::Sequencer);

void RpcController::init()
{
    //! NOTE We could use a by target controller factory here,
    //! but we have not abstract RPC system, but a specialized one for audio, and the targets are known in advance.
    //! So, a factory is deliberately not used to have everything in one place.
    rpcChannel()->listen([this](const Msg& msg) {
        switch (msg.target.name) {
        case TargetName::Sequencer:
            sequencerHandle(msg);
            break;
        case TargetName::Undefined: {
            LOGE() << "received message for undefined target, will be ignored";
        }
        }
    });
}

ISequencerPtr RpcController::sequencer() const
{
    return audioEngine()->sequencer();
}

void RpcController::sequencerHandle(const Msg& msg)
{
    static std::map<Method, Call> calls;
    auto bindMethod = [](std::map<Method, Call>& calls, const Method& method, const Call& call) {
        calls.insert({ method, call });
    };

    //! NOTE We could use reflection here so as not to write this code of the same type.
    //! But it's not that simple. Rpc can work in serialization mode,
    //! and then some arguments cannot be simply passed to the target class, they should be deserialized first.
    //! The second problem is subscription to channels for the given argument.
    //! The target class just has a method to get the channel.
    //! But to use the channel in RPC, we need to do the same as in bindMidiTickPlayed
    //! So, it is better to have the preparation of arguments and their conversion in one place.

    if (calls.empty()) {
        // Bind to sequencer

        bindMethod(calls, "initMIDITrack", [this](const Args& args) {
            sequencer()->initMIDITrack(args.arg<ISequencer::TrackID>(0));
        });

        bindMethod(calls, "initAudioTrack", [this](const Args& args) {
            sequencer()->initAudioTrack(args.arg<ISequencer::TrackID>(0));
        });

        bindMethod(calls, "setMIDITrack", [this](const Args& args) {
            if (rpcChannel()->isSerialized()) {
                NOT_IMPLEMENTED;
            } else {
                sequencer()->setMIDITrack(args.arg<ISequencer::TrackID>(0), args.arg<std::shared_ptr<midi::MidiStream> >(1));
            }
        });

        bindMethod(calls, "setAudioTrack", [this](const Args& args) {
            if (rpcChannel()->isSerialized()) {
                NOT_IMPLEMENTED;
            } else {
                sequencer()->setAudioTrack(args.arg<ISequencer::TrackID>(0), args.arg<std::shared_ptr<audio::IAudioStream> >(1));
            }
        });

        bindMethod(calls, "play", [this](const Args&) {
            sequencer()->play();
        });

        bindMethod(calls, "pause", [this](const Args&) {
            sequencer()->pause();
        });

        bindMethod(calls, "stop", [this](const Args&) {
            sequencer()->stop();
        });

        bindMethod(calls, "seek", [this](const Args& args) {
            sequencer()->seek(args.arg<uint64_t>(0));
        });

        bindMethod(calls, "rewind", [this](const Args&) {
            sequencer()->rewind();
        });

        bindMethod(calls, "setLoop", [this](const Args& args) {
            sequencer()->setLoop(args.arg<uint64_t>(0), args.arg<uint64_t>(1));
        });

        bindMethod(calls, "unsetLoop", [this](const Args&) {
            sequencer()->unsetLoop();
        });

        bindMethod(calls, "instantlyPlayMidi", [this](const Args& args) {
            if (rpcChannel()->isSerialized()) {
                NOT_IMPLEMENTED;
            } else {
                sequencer()->instantlyPlayMidi(args.arg<midi::MidiData>(0));
            }
        });

        // Bind from sequencer

        sequencer()->statusChanged().onReceive(this, [this](const ISequencer::Status& status) {
            rpcChannel()->send(Msg(rpcSeqTarget, "statusChanged", Args::make_arg1<ISequencer::Status>(status)));
        });

        //! TODO I highly doubt that the position and play tick should be transmitted at this point from the sequencer.
        //! There is also a buffer between the sequencer and the audio driver, which introduces, though not large (at the moment), but the delay.
        //! I think both the position and the tick should be transferred together with the audio data
        //! to the audio driver and from there already taken, then they will always be synchronized with the sound that is currently being played.

        bindMethod(calls, "bindMidiTickPlayed", [this](const Args& args) {
            ISequencer::TrackID trackID = args.arg<ISequencer::TrackID>(0);
            async::Channel<midi::tick_t> ch = sequencer()->midiTickPlayed(trackID);
            ch.onReceive(this, [this, trackID](midi::tick_t tick) {
                rpcChannel()->send(Msg(rpcSeqTarget, "midiTickPlayed", Args::make_arg2<ISequencer::TrackID, midi::tick_t>(trackID, tick)));
            });
        });

        sequencer()->positionChanged().onNotify(this, [this]() {
            float pos = sequencer()->playbackPosition();
            rpcChannel()->send(Msg(rpcSeqTarget, "positionChanged", Args::make_arg1<float>(pos)));
        });
    }

    auto it = calls.find(msg.method);
    if (it == calls.end()) {
        LOGE() << "not found method: " << msg.method;
        return;
    }

    it->second(msg.args);
}
