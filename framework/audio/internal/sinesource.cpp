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
#include "sinesource.h"

#include <cmath>
#include <cstring>

#include <soloud.h>

#include "log.h"

using namespace mu::audio;

struct SineSource::SLInstance : public SoLoud::AudioSourceInstance {
    std::shared_ptr<SineSource::Samples> samples;
    size_t position = 0;

    SLInstance(std::shared_ptr<SineSource::Samples> s)
        : samples(s) {}
    ~SLInstance() override = default;

    SoLoud::result seekFrame(double sec) override
    {
        position = sec * mBaseSamplerate;
        return SoLoud::SO_NO_ERROR;
    }

    unsigned int getAudio(float* aBuffer, unsigned int aSamplesToRead, unsigned int /*aBufferSize*/) override
    {
        size_t rest = samples->size() - position;
        int readSamples = rest < aSamplesToRead ? rest : aSamplesToRead;
        size_t blockSizeInBytes = readSamples * sizeof(float);
        std::memcpy(aBuffer, &samples->operator [](position), blockSizeInBytes);
        std::memcpy(aBuffer + readSamples, &samples->operator [](position), blockSizeInBytes);

        position += readSamples;

        return readSamples;
    }

    bool hasEnded() override
    {
        return position >= samples->size();
    }
};

struct SineSource::SL : public SoLoud::AudioSource {
    std::shared_ptr<SineSource::Samples> samples;
    ~SL() override = default;

    SoLoud::AudioSourceInstance* createInstance() override
    {
        return new SLInstance(samples);
    }
};

SineSource::SineSource()
{
    m_samples = std::make_shared<Samples>();

    m_sl = std::make_shared<SL>();
    m_sl->mChannels = 2;
    m_sl->samples = m_samples;
}

void SineSource::generateSine(Samples& samples, float samplerate, float freq, int seconds) const
{
    size_t buf_size = seconds * samplerate;
    samples.clear();
    samples.resize(buf_size);

    for (size_t i = 0; i < buf_size; ++i) {
        samples[i] = 32760 * std::sin((2.f * float(M_PI) * freq) / samplerate * i);
    }
}

void SineSource::setSampleRate(float samplerate)
{
    m_sl->mBaseSamplerate = samplerate;
    generateSine(*m_samples.get(), samplerate, 340.0, 10);
}

SoLoud::AudioSource* SineSource::source()
{
    return m_sl.get();
}
