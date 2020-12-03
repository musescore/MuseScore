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

#include "midisource.h"

#include <soloud.h>

#include "log.h"

using namespace mu::audio;
using namespace mu::midi;

struct MidiSource::SLInstance : public SoLoud::AudioSourceInstance {
    std::shared_ptr<ISequencer> seq;
    std::shared_ptr<midi::ISequencer::Context> seqContext = nullptr;

    SLInstance(std::shared_ptr<ISequencer> s, std::shared_ptr<midi::ISequencer::Context> ctx)
        : seq(s), seqContext(ctx)
    {
        seq->run(mStreamTime);
    }

    ~SLInstance() override
    {
        seq->stop();
    }

    SoLoud::result seekFrame(double sec) override
    {
        seq->seek(sec);
        return SoLoud::SO_NO_ERROR;
    }

    unsigned int getAudio(float* aBuffer, unsigned int aSamplesToRead, unsigned int /*aBufferSize*/) override
    {
        seq->getAudio(mStreamPosition, aBuffer, aSamplesToRead, seqContext.get());
        return aSamplesToRead;
    }

    bool hasEnded() override
    {
        return seq->hasEnded();
    }
};

struct MidiSource::SL : public SoLoud::AudioSource {
    std::shared_ptr<ISequencer> seq;
    std::shared_ptr<midi::ISequencer::Context> seqContext;

    ~SL() override {}

    SoLoud::AudioSourceInstance* createInstance() override
    {
        return new SLInstance(seq, seqContext);
    }
};

MidiSource::MidiSource(const std::string& name)
    : m_name(name)
{
    m_seq = sequencer();
    m_seqContext = std::make_shared<ISequencer::Context>();

    m_sl = std::make_shared<MidiSource::SL>();
    m_sl->mChannels = 2;
    m_sl->seq = m_seq;
    m_sl->seqContext = m_seqContext;
}

MidiSource::~MidiSource()
{
}

void MidiSource::setSampleRate(float samplerate)
{
    m_sl->mBaseSamplerate = samplerate;
}

SoLoud::AudioSource* MidiSource::source()
{
    return m_sl.get();
}

void MidiSource::loadMIDI(const std::shared_ptr<midi::MidiStream>& stream)
{
    m_seq->loadMIDI(stream);
}

void MidiSource::fillPlayContext(Context* ctx)
{
    IF_ASSERT_FAILED(ctx) {
        return;
    }

    ctx->set<tick_t>(CtxKey::PlayTick, m_seqContext->playTick);
    ctx->set<tick_t>(CtxKey::FromTick, m_seqContext->fromTick);
    ctx->set<tick_t>(CtxKey::ToTick, m_seqContext->toTick);
}

float MidiSource::playbackSpeed() const
{
    return m_seq->playbackSpeed();
}

void MidiSource::setPlaybackSpeed(float speed)
{
    m_seq->setPlaybackSpeed(speed);
}

void MidiSource::setIsTrackMuted(int ti, bool mute)
{
    m_seq->setIsTrackMuted(ti, mute);
}

void MidiSource::setTrackVolume(int ti, float volume)
{
    m_seq->setTrackVolume(ti, volume);
}

void MidiSource::setTrackBalance(int ti, float balance)
{
    m_seq->setTrackBalance(ti, balance);
}
