/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#ifndef MUSE_AUDIO_SAMPLEDELAY_H
#define MUSE_AUDIO_SAMPLEDELAY_H

#include <cassert>

#include "circularsamplebuffer.h"

namespace muse::audio::fx {
template<typename SampleT, int maxNumChannels>
class SampleDelay
{
public:
    SampleDelay()
    {
    }

    ~SampleDelay()
    {
    }

    void allocateForMaxDelaySamples(int maxDelaySamples)
    {
        for (auto ch = 0; ch < maxNumChannels; ++ch) {
            m_buffer[ch].setSize(maxDelaySamples);
        }
    }

    void setDelaySamples(int samples)
    {
        if (m_delaySamples == samples) {
            return;
        }

        m_delaySamples = samples;
        allocateForMaxDelaySamples(samples);
        reset();
    }

    int getDelaySamples() const
    {
        return m_delaySamples;
    }

    void reset()
    {
        for (auto ch = 0; ch < maxNumChannels; ++ch) {
            m_buffer[ch].reset();
        }
    }

    SampleT processSample(const SampleT& sample, int ch)
    {
        if (m_delaySamples == 0) {
            return sample;
        }

        auto s = m_buffer[ch].read(-m_delaySamples);
        m_buffer[ch].writeOffset0(sample);
        m_buffer[ch].advance(1);
        return s;
    }

    void processBlock(SampleT** smp, int numCh, int numSamples)
    {
        assert(numCh <= maxNumChannels);
        if (m_delaySamples == 0) {
            return;
        }

        for (auto ch = 0; ch < numCh; ++ch) {
            for (auto i = 0; i < numSamples; ++i) {
                auto s = m_buffer[ch].read(-m_delaySamples);
                m_buffer[ch].writeOffset0(smp[ch][i]);
                smp[ch][i] = s;
                m_buffer[ch].advance(1);
            }
        }
    }

private:
    CircularSampleBuffer<SampleT> m_buffer[maxNumChannels];
    int m_delaySamples = 0;
};
} // namespace muse::audio::fx

#endif // MUSE_AUDIO_SAMPLEDELAY_H
