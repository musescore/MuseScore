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

#ifndef MUSE_AUDIO_SPARSEFIRFILTER_H
#define MUSE_AUDIO_SPARSEFIRFILTER_H

#include <cassert>
#include <vector>

#include "circularsamplebuffer.h"

namespace muse::audio::fx {
class SparseFirFilter
{
public:
    SparseFirFilter()
    {
    }

    void reset()
    {
        m_buffer.reset();
    }

    void setFormat(int maxBlockSize, int maxImpulseOffset)
    {
        m_buffer.setSize(maxBlockSize + maxImpulseOffset + 1);
        m_blockSize = maxBlockSize;
        m_maxAllowedOffset = maxImpulseOffset;
    }

    void processBlock(const float* signal_in, float* signal_out, int n)
    {
        assert(n <= m_blockSize);
        assert(m_impulses.size() > 0);

        m_buffer.writeBlock(0, n, signal_in);

        m_buffer.readBlockWithGain(-m_impulses[0].offset, n, signal_out, m_impulses[0].gainFact);
        for (size_t i = 1; i < m_impulses.size(); ++i) {
            m_buffer.readAddBlockWithGain(-m_impulses[i].offset, n, signal_out, m_impulses[i].gainFact);
        }

        m_buffer.advance(n);
    }

    float processSample(float smp)
    {
        m_buffer.writeOffset0(smp);

        smp = 0.f;
        for (const auto& i : m_impulses) {
            smp += m_buffer.read(-i.offset) * i.gainFact;
        }

        m_buffer.advance(1);
        return smp;
    }

    void clearImpulses()
    {
        m_impulses.clear();
    }

    void appendImpulse(int offset, float value)
    {
        assert(offset >= 0 && offset <= m_maxAllowedOffset);

        m_impulses.push_back({ offset, value });
        m_buffer.setSize(offset + 1);
    }

private:
    struct Impulse
    {
        int offset;
        float gainFact;
    };

    std::vector<Impulse> m_impulses;

    int m_blockSize = 0;
    int m_maxAllowedOffset = 0;
    CircularSampleBuffer<float> m_buffer;
};
} // namespace muse::audio::fx

#endif // MUSE_AUDIO_SPARSEFIRFILTER_H
