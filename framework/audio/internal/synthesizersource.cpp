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
#include "synthesizersource.h"
#include <soloud.h>
using namespace mu::audio;
using namespace mu::midi;

class SynthesizerSource::SynthesizerSLASI : public SoLoud::AudioSourceInstance
{
public:
    SynthesizerSLASI(std::shared_ptr<ISynthesizer> synthesizer)
        : m_synthesizer(synthesizer)
    {
        IF_ASSERT_FAILED(m_synthesizer) {
            LOGE() << "m_synthesizer is nullptr";
        }
    }

    unsigned int getAudio(float* aBuffer, unsigned int aSamplesToRead, unsigned int /*aBufferSize*/) override
    {
        m_synthesizer->writeBuf(aBuffer, aSamplesToRead);
        return aSamplesToRead;
    }

    bool hasEnded() override
    {
        return false;
    }

private:
    std::shared_ptr<ISynthesizer> m_synthesizer;
};

class SynthesizerSource::SynthesizerSLAS : public SoLoud::AudioSource
{
public:
    SynthesizerSLAS(std::shared_ptr<ISynthesizer> synthesizer)
        : SoLoud::AudioSource(), m_synthesizer(synthesizer) {}

    SoLoud::AudioSourceInstance* createInstance() override
    {
        IF_ASSERT_FAILED(m_synthesizer) {
            LOGE() << "m_synthesizer is nullptr";
        }
        return new SynthesizerSLASI(m_synthesizer);
    }

private:
    std::shared_ptr<ISynthesizer> m_synthesizer;
};

SynthesizerSource::SynthesizerSource(std::shared_ptr<ISynthesizer> synthesizer)
    : m_synthesizer(synthesizer)
{
    m_sosource = std::make_shared<SynthesizerSLAS>(m_synthesizer);
}

void SynthesizerSource::setSampleRate(float sampleRate)
{
    m_synthesizer->init(sampleRate);
}

SoLoud::AudioSource* SynthesizerSource::source()
{
    return m_sosource.get();
}
