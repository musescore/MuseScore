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
#ifndef MU_AUDIO_BUFFER_H
#define MU_AUDIO_BUFFER_H

#include <vector>
#include <memory>
#include <atomic>
#include "iaudiobuffer.h"
#include "iaudioconfiguration.h"
#include "modularity/ioc.h"

namespace mu::audio {
class AudioBuffer : public IAudioBuffer
{
    static const int DEFAULT_SIZE = 16384;
    static const unsigned int FILL_SAMPLES = 1024;
    static const unsigned int FILL_OVER    = 1024;

    INJECT(audio, IAudioConfiguration, config)

public:
    AudioBuffer() = default;

    void init(int samplesPerChannel = DEFAULT_SIZE);

    void setSource(std::shared_ptr<IAudioSource> source) override;
    void forward() override;

    void pop(float* dest, size_t sampleCount) override;
    void setMinSampleLag(size_t lag) override;

private:

    unsigned int sampleLag() const;
    void fillup();
    void updateWriteIndex(const unsigned int samplesPerChannel);

    std::recursive_mutex m_mutex; //! TODO get rid *recursive*
    size_t m_minSampleLag = FILL_SAMPLES;
    size_t m_writeIndex = 0;
    size_t m_readIndex = 0;
    int m_audioChannelsCount = 2;

    std::vector<float> m_data = {};
    std::shared_ptr<IAudioSource> m_source = nullptr;
};
}

#endif // MU_AUDIO_BUFFER_H
