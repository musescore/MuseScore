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

#include "loopsource.h"

#include <soloud.h>
#include "log.h"
#include "realfn.h"

using namespace mu;
using namespace mu::audio;

struct LoopSource::SLInstance : public SoLoud::AudioSourceInstance {
    SoLoud::AudioSourceInstance* _origin{ nullptr };
    LoopSource* _stream{ nullptr };

    SLInstance(SoLoud::AudioSourceInstance* origin, LoopSource* stream)
        : _origin(origin), _stream(stream)
    {
    }

    ~SLInstance() override
    {
        delete _origin;
    }

    void init(SoLoud::AudioSource& aSource, int aPlayIndex) override
    {
        SoLoud::AudioSourceInstance::init(aSource, aPlayIndex);

        _origin->mPlayIndex = this->mPlayIndex;
        _origin->mBaseSamplerate = this->mBaseSamplerate;
        _origin->mSamplerate = this->mBaseSamplerate;
        _origin->mChannels = this->mChannels;
        _origin->mStreamTime = this->mStreamTime;
        _origin->mStreamPosition = this->mStreamPosition;
        _origin->init(aSource, aPlayIndex);
    }

    SoLoud::result seekFrame(double sec) override
    {
        mStreamPosition = sec;
        _origin->mStreamPosition = sec;
        _origin->seekFrame(sec);

        return SoLoud::SO_NO_ERROR;
    }

    unsigned int getAudio(float* aBuffer, unsigned int aSamplesToRead, unsigned int aBufferSize) override
    {
        if (_stream->m_loopRegion.isValid()) {
            float time = this->mStreamPosition;
            if (time < _stream->m_loopRegion.begin || RealIsEqualOrMore(time, _stream->m_loopRegion.begin)) {
                seekFrame(_stream->m_loopRegion.begin);
            }
        }

        _origin->mStreamTime = this->mStreamTime;
        _origin->mStreamPosition = this->mStreamPosition;

        aSamplesToRead = _origin->getAudio(aBuffer, aSamplesToRead, aBufferSize);
        return aSamplesToRead;
    }

    bool hasEnded() override
    {
        return _origin->hasEnded();
    }
};

struct LoopSource::SL : public SoLoud::AudioSource {
    SoLoud::AudioSource* _origin{ nullptr };
    LoopSource* _stream{ nullptr };

    SL(SoLoud::AudioSource* origin, LoopSource* stream)
        : _origin(origin), _stream(stream)
    {
        mChannels = _origin->mChannels;
        mBaseSamplerate = _origin->mBaseSamplerate;
    }

    ~SL() override {}

    SoLoud::AudioSourceInstance* createInstance() override
    {
        return new SLInstance(_origin->createInstance(), _stream);
    }
};

LoopSource::LoopSource(std::shared_ptr<IAudioSource> origin, const std::string& name)
    : m_origin(origin), m_name(name)
{
    IF_ASSERT_FAILED(origin) {
        return;
    }

    SoLoud::AudioSource* sa = origin->source();
    IF_ASSERT_FAILED(sa) {
        return;
    }

    m_sl = std::make_shared<LoopSource::SL>(sa, this);
}

LoopSource::~LoopSource()
{
}

void LoopSource::setSampleRate(float samplerate)
{
    m_origin->setSampleRate(samplerate);
}

SoLoud::AudioSource* LoopSource::source()
{
    return m_sl.get();
}

void LoopSource::setLoopRegion(const LoopRegion& loop)
{
    m_loopRegion = loop;
}
