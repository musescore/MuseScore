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

#include "rpcmidistream.h"

#include "log.h"
#include "internal/worker/workertypes.h"

using namespace mu::audio::engine;

RpcMidiStream::RpcMidiStream(const std::string& name)
    : RpcStreamBase(CallType::Midi, name)
{
    listen([this](const CallMethod& /*method*/, const Args& /*args*/) {});
}

void RpcMidiStream::loadMIDI(const std::shared_ptr<midi::MidiStream>& midi)
{
    m_midiStream = midi;
    call(CallMethod::LoadMidi, Args::make_arg1<std::shared_ptr<midi::MidiStream> >(midi));
}

void RpcMidiStream::init(float samplerate)
{
    call(CallMethod::InitMidi, Args::make_arg1<float>(samplerate));
}

float RpcMidiStream::playbackSpeed() const
{
    return m_playbackSpeed;
}

void RpcMidiStream::setPlaybackSpeed(float speed)
{
    m_playbackSpeed = speed;
    call(CallMethod::SetPlaybackSpeed, Args::make_arg1<float>(speed));
    truncate();
}

void RpcMidiStream::setIsTrackMuted(uint16_t ti, bool mute)
{
    call(CallMethod::SetIsTrackMuted, Args::make_arg2<uint16_t, bool>(ti, mute));
    truncate();
}

void RpcMidiStream::setTrackVolume(uint16_t ti, float volume)
{
    call(CallMethod::SetTrackVolume, Args::make_arg2<uint16_t, float>(ti, volume));
    truncate();
}

void RpcMidiStream::setTrackBalance(uint16_t ti, float balance)
{
    call(CallMethod::SetTrackBalance, Args::make_arg2<uint16_t, float>(ti, balance));
    truncate();
}

void RpcMidiStream::onGetAudio(const Context&)
{
    //LOGI() << ctx.dump();
}
