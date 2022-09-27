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

#include "modularity/ioc.h"

#include "iaudiobuffer.h"

namespace mu::audio {
class AudioBuffer : public IAudioBuffer
{
public:
    AudioBuffer() = default;

    void init(const audioch_t audioChannelsCount, const samples_t renderStep);

    void setSource(std::shared_ptr<IAudioSource> source) override;
    void forward() override;

    void pop(float* dest, size_t sampleCount) override;
    void setMinSampleLag(size_t lag) override;

private:

    unsigned int sampleLag() const;
    void fillup();
    void updateWriteIndex(const unsigned int samplesPerChannel);

    std::mutex m_mutex;
    size_t m_minSampleLag = 0;
    size_t m_writeIndex = 0;
    size_t m_readIndex = 0;
    samples_t m_samplesPerChannel = 0;
    audioch_t m_audioChannelsCount = 0;

    samples_t m_renderStep = 0;

    std::vector<float> m_data = {};
    std::shared_ptr<IAudioSource> m_source = nullptr;
};
}

#endif // MU_AUDIO_BUFFER_H
