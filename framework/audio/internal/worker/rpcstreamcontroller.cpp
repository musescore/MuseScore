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

#include "rpcstreamcontroller.h"

#include <sstream>

#include "log.h"

using namespace mu::audio::worker;

RpcStreamController::~RpcStreamController()
{
    channel()->unlistenAll();
}

void RpcStreamController::setup()
{
    m_midi = std::make_shared<MidiStreamController>();
    //_wav = std::make_shared<track::WavStreamController>();

    channel()->listenAll([this](const StreamID& id, CallID method, const Args& args) {
        callRpc(id, method, args);
    });

    channel()->onGetAudio([this](const StreamID& id, float* buf, uint32_t samples, uint32_t bufSize, Context* ctx) {
        getAudio(id, buf, samples, bufSize, ctx);
    });
}

void RpcStreamController::getAudio(const StreamID& id, float* buf, uint32_t samples, uint32_t bufSize, Context* ctx)
{
    if (m_midi->hasStream(id)) {
        m_midi->getAudio(id, buf, samples, bufSize, ctx);
    } else {
        m_wav->getAudio(id, buf, samples, bufSize, ctx);
    }
}

void RpcStreamController::callRpc(const StreamID& id, CallID method, const Args& args)
{
#define CHECK_ARGS(args, n) IF_ASSERT_FAILED(args.count() >= n) { return; }

    auto bindMethod = [this](CallID method, const Call& call) {
                          m_calls.insert({ method, call });
                      };

    if (m_calls.empty()) {
        bindMethod(callID(CallType::Midi, CallMethod::Create), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 1)
            const std::string& name = args.arg<std::string>(0);
            m_midi->createStream(id, name);
        });

        bindMethod(callID(CallType::Midi, CallMethod::Destroy), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 0)
            m_midi->destroyStream(id);
        });

        bindMethod(callID(CallType::Midi, CallMethod::SetSamplerate), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 1)
            float samplerate = args.arg<float>(0);
            m_midi->setSampleRate(id, samplerate);
        });

        bindMethod(callID(CallType::Midi, CallMethod::SetLoopRegion), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 1)
            LoopRegion loop = args.arg<LoopRegion>(0);
            m_midi->setLoopRegion(id, loop);
        });

        bindMethod(callID(CallType::Midi, CallMethod::LoadMidi), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 1)
            std::shared_ptr<midi::MidiStream> midiData = args.arg<std::shared_ptr<midi::MidiStream> >(0);
            m_midi->loadMIDI(id, midiData);
        });

        bindMethod(callID(CallType::Midi, CallMethod::SetPlaybackSpeed), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 1)
            float speed = args.arg<float>(0);
            m_midi->setPlaybackSpeed(id, speed);
        });

        bindMethod(callID(CallType::Midi, CallMethod::SetIsTrackMuted), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 2)
            uint16_t ti = args.arg<uint16_t>(0);
            bool mute = args.arg<bool>(1);
            m_midi->setIsTrackMuted(id, ti, mute);
        });

        bindMethod(callID(CallType::Midi, CallMethod::SetTrackVolume), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 2)
            uint16_t ti = args.arg<uint16_t>(0);
            float vol = args.arg<float>(1);
            m_midi->setTrackVolume(id, ti, vol);
        });

        bindMethod(callID(CallType::Midi, CallMethod::SetTrackBalance), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 2)
            uint16_t ti = args.arg<uint16_t>(0);
            float bal = args.arg<float>(1);
            m_midi->setTrackBalance(id, ti, bal);
        });

        // Instance
        bindMethod(callID(CallType::Midi, CallMethod::InstanceCreate), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 0)
            m_midi->streamInstance_create(id);
        });

        bindMethod(callID(CallType::Midi, CallMethod::InstanceDestroy), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 0)
            m_midi->streamInstance_destroy(id);
        });

        bindMethod(callID(CallType::Midi, CallMethod::InstanceInit), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 4)
            float samplerate = args.arg<float>(0);
            uint16_t channels = args.arg<uint16_t>(1);
            double streamTime = args.arg<double>(2);
            double streamPosition = args.arg<double>(3);

            m_midi->streamInstance_init(id, samplerate, channels, streamTime, streamPosition);
        });

        bindMethod(callID(CallType::Midi, CallMethod::InstanceSeekFrame), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 1)
            float sec = args.arg<float>(0);
            m_midi->streamInstance_seek_frame(id, sec);
            channel()->send(id, callID(CallType::Midi, CallMethod::InstaneOnSeek), {});
        });

        // Wav
        bindMethod(callID(CallType::Wav, CallMethod::Create), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 1)
            const std::string& name = args.arg<std::string>(0);
            m_wav->createStream(id, name);
        });

        bindMethod(callID(CallType::Wav, CallMethod::Destroy), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 0)
            m_wav->destroyStream(id);
        });

        bindMethod(callID(CallType::Wav, CallMethod::SetSamplerate), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 1)
            float samplerate = args.arg<float>(0);
            m_wav->setSampleRate(id, samplerate);
        });

        bindMethod(callID(CallType::Wav, CallMethod::SetLoopRegion), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 1)
            LoopRegion loop = args.arg<LoopRegion>(0);
            m_wav->setLoopRegion(id, loop);
        });

        bindMethod(callID(CallType::Wav, CallMethod::LoadTrack), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 1)
            const std::string& url = args.arg<std::string>(0);
            const uint16_t trackNum = args.arg<uint16_t>(1);

            m_wav->load(id, url, trackNum, [this, id, url](bool success) {
                channel()->send(id, callID(CallType::Wav, CallMethod::OnTrackLoaded), Args::make_arg1<bool>(success));
            });
        });

        // Instance
        bindMethod(callID(CallType::Wav, CallMethod::InstanceCreate), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 0)
            m_wav->streamInstance_create(id);
        });

        bindMethod(callID(CallType::Wav, CallMethod::InstanceDestroy), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 0)
            m_wav->streamInstance_destroy(id);
        });

        bindMethod(callID(CallType::Wav, CallMethod::InstanceInit), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 4)
            float samplerate = args.arg<float>(0);
            uint16_t channels = args.arg<uint16_t>(1);
            double streamTime = args.arg<double>(2);
            double streamPosition = args.arg<double>(3);

            m_wav->streamInstance_init(id, samplerate, channels, streamTime, streamPosition);
        });

        bindMethod(callID(CallType::Wav, CallMethod::InstanceSeekFrame), [this](const StreamID& id, const Args& args) {
            CHECK_ARGS(args, 1)
            float sec = args.arg<float>(0);
            m_wav->streamInstance_seek_frame(id, sec);
            channel()->send(id, callID(CallType::Wav, CallMethod::InstaneOnSeek), {});
        });
    }

    auto it = m_calls.find(method);
    if (it == m_calls.end()) {
        LOGE() << "not found method: " << method;
        return;
    }

    it->second(id, args);
    //LOGI() << "[RpcStreamController] called method: " << method;
}
