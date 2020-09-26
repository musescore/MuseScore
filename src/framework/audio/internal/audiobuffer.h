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
#ifndef MU_AUDIO_BUFFER_H
#define MU_AUDIO_BUFFER_H

#include <vector>
#include <memory>
#include <atomic>
#include "iaudiobuffer.h"

namespace mu::audio {
class AudioBuffer : public IAudioBuffer
{
    const static unsigned int DEFAULT_SIZE = 16384;
    const static unsigned int FILL_SAMPLES = 1024;
    const static unsigned int FILL_OVER    = 1024;

public:
    AudioBuffer(unsigned int streamsPerSample = 2, unsigned int size = DEFAULT_SIZE);

    void setSource(std::shared_ptr<IAudioSource> source) override;
    void forward() override;

    void push(const float* source, int sampleCount) override;
    void pop(float* dest, unsigned int sampleCount) override;
    void setMinSampleLag(unsigned int lag) override;

private:
    unsigned int sampleLag() const;
    void fillup();

    unsigned int m_streamsPerSample = 0;
    unsigned int m_minSampleLag = FILL_SAMPLES;
    std::atomic<unsigned int> m_writeIndex = 0, m_readIndex = 0;
    std::mutex m_dataMutex;
    std::vector<float> m_data = {};
    std::shared_ptr<IAudioSource> m_source = nullptr;
};
}

#endif // MU_AUDIO_BUFFER_H
