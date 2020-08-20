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

#include "rpcmidisource.h"

#include "log.h"
#include "internal/worker/workertypes.h"

using namespace mu::audio;
using namespace mu::audio::worker;

RpcMidiSource::RpcMidiSource(const std::string& name)
    : RpcSourceBase(CallType::Midi, name)
{
    listen([this](const CallMethod& /*method*/, const Args& /*args*/) {});
}

std::shared_ptr<IAudioSource> RpcMidiSource::audioSource()
{
    return shared_from_this();
}

void RpcMidiSource::loadMIDI(const std::shared_ptr<midi::MidiStream>& midi)
{
    call(CallMethod::LoadMidi, Args::make_arg1<std::shared_ptr<midi::MidiStream> >(midi));

    midiPortDataSender()->setMidiStream(midi);
}

float RpcMidiSource::playbackSpeed() const
{
    return m_playbackSpeed;
}

void RpcMidiSource::setPlaybackSpeed(float speed)
{
    m_playbackSpeed = speed;
    call(CallMethod::SetPlaybackSpeed, Args::make_arg1<float>(speed));
    truncate();
}

void RpcMidiSource::setIsTrackMuted(uint16_t ti, bool mute)
{
    call(CallMethod::SetIsTrackMuted, Args::make_arg2<uint16_t, bool>(ti, mute));
    truncate();
}

void RpcMidiSource::setTrackVolume(uint16_t ti, float volume)
{
    call(CallMethod::SetTrackVolume, Args::make_arg2<uint16_t, float>(ti, volume));
    truncate();
}

void RpcMidiSource::setTrackBalance(uint16_t ti, float balance)
{
    call(CallMethod::SetTrackBalance, Args::make_arg2<uint16_t, float>(ti, balance));
    truncate();
}

void RpcMidiSource::onGetAudio(const Context& ctx)
{
    // LOGI() << ctx.dump();

    if (ctx.hasVal(CtxKey::HasEnded)) {
        return;
    }

    midiPortDataSender()->sendEvents(ctx.get<midi::tick_t>(CtxKey::FromTick), ctx.get<midi::tick_t>(CtxKey::ToTick));
}
