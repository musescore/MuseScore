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

#ifndef MUSE_AUDIO_ALLPASSMODULATEDDELAY_H
#define MUSE_AUDIO_ALLPASSMODULATEDDELAY_H

#include "circularsamplebuffer.h"

namespace muse::audio::fx {
class AllPassModulatedDelay
{
public:
    AllPassModulatedDelay()
    {
    }

    void allocateForMaxDelaySamples(int maxDelaySamples)
    {
        m_buffer.setSize(maxDelaySamples);
    }

    void setBaseDelay(float samples, float maxModulationOffset)
    {
        int allocSize = int(samples + maxModulationOffset + 16.f); // some extra space for interpolation
        if (samples != m_baseDelay || allocSize > m_buffer.getAllocatedSize()) {
            allocateForMaxDelaySamples(allocSize);
            m_baseDelay = samples;
            setModOffset(0.f);
            reset();
        }
    }

    float getBaseDelay() const
    {
        return m_baseDelay;
    }

    void setModOffset(float smpOffset)
    {
        float total = -m_baseDelay + smpOffset; // negative!
        float flr = std::floor(total);
        m_modFraction = total - flr;
        m_readOffset = int(flr);
    }

    void writeSampleAndAdvance(float sample)
    {
        m_buffer.writeOffset0(sample);
        m_buffer.advance(1);
    }

    float readSample()
    {
        auto res = m_buffer.read(m_readOffset);
        auto x1 = m_buffer.read(m_readOffset + 1);

        res += m_modFraction * (x1 - m_y1);
        m_y1 = res;
        return res;
    }

    float processSample(float sample)
    {
        writeSampleAndAdvance(sample);
        return readSample();
    }

    void reset()
    {
        m_buffer.reset();
        m_y1 = 0.f;
    }

private:
    int m_readOffset = 0;
    float m_modFraction = 0.f;
    float m_baseDelay = 0.f;

    CircularSampleBuffer<float> m_buffer;
    float m_y1 = 0.f;
};
} // namespace muse::audio::fx

#endif // MUSE_AUDIO_ALLPASSMODULATEDDELAY_H
