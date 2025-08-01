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

#ifndef MUSE_AUDIO_CIRCULARSAMPLEBUFFER_H
#define MUSE_AUDIO_CIRCULARSAMPLEBUFFER_H

#include <cassert>

#include "vectorops.h"

/*
 * Utility buffer class for delay-based effects
 * Allocates a 2^n size buffer for branchless write and read using a bitmask
 * It can read and write to past and future samples, but does not check for overlaps
 */

namespace muse::audio::fx {
template<typename SampleT>
class CircularSampleBuffer
{
public:
    CircularSampleBuffer()
    {
    }

    ~CircularSampleBuffer()
    {
        if (m_buffer) {
            free(m_buffer);
        }
    }

    void setSize(int n)
    {
        if (n > m_allocatedSize) {
            auto oldSize = m_allocatedSize;

            auto findLargerPowerOfTwo = [](int32_t number) {
                int32_t powerOf2 = 1;
                while (powerOf2 < number) {
                    powerOf2 *= 2;
                }
                return powerOf2;
            };

            m_allocatedSize = findLargerPowerOfTwo(n);
            m_bufferSizeMask = m_allocatedSize - 1;
            m_buffer = (SampleT*)std::realloc(m_buffer, m_allocatedSize * sizeof(SampleT));

            // Reset the new memory region
            assert(m_buffer);
            std::fill(m_buffer + oldSize, m_buffer + m_allocatedSize, 0.f);
        }
    }

    int getAllocatedSize() const
    {
        return m_allocatedSize;
    }

    void reset()
    {
        if (m_buffer && m_allocatedSize > 0) {
            memset(m_buffer, 0, sizeof(SampleT) * m_allocatedSize);
        }
        m_positionIndex = 0;
    }

    void write(int offset, const SampleT& sample)
    {
        m_buffer[(m_positionIndex + offset) & m_bufferSizeMask] = sample;
    }

    void writeOffset0(const SampleT& sample)
    {
        m_buffer[m_positionIndex] = sample;
    }

    const SampleT& read(int offset) const
    {
        return m_buffer[(m_positionIndex + offset) & m_bufferSizeMask];
    }

    /// change the 0 position by n
    void advance(int n)
    {
        m_positionIndex += n;
        m_positionIndex &= m_bufferSizeMask;
    }

private:
    template<typename fnc>
    void splitBlockOffsetFunction(int startOffset, int n, fnc f) const
    {
        assert(n <= m_allocatedSize);
        int firstIndex = (m_positionIndex + startOffset) & m_bufferSizeMask;
        int n_to_end = m_allocatedSize - firstIndex;
        if (n_to_end > n) {
            f(firstIndex, 0, n);
        } else {
            f(firstIndex, 0, n_to_end);
            f(0, n_to_end, n - n_to_end);
        }
    }

public:
    void writeBlock(int startOffset, int n, const SampleT* sourceBlock)
    {
        splitBlockOffsetFunction(startOffset, n, [=](int bufferOff, int sampleOff, int n) {
            vo::copy(&sourceBlock[sampleOff], &m_buffer[bufferOff], n);
        });
    }

    void readBlockWithGain(int startOffset, int n, SampleT* targetBlock, float gainFactor) const
    {
        splitBlockOffsetFunction(startOffset, n, [=](int bufferOff, int sampleOff, int n) {
            vo::constantMultiply(&m_buffer[bufferOff], gainFactor, &targetBlock[sampleOff], n);
        });
    }

    void readAddBlockWithGain(int startOffset, int n, SampleT* targetBlock, float gainFactor) const
    {
        splitBlockOffsetFunction(startOffset, n, [=](int bufferOff, int sampleOff, int n) {
            vo::constantMultiplyAndAdd(&m_buffer[bufferOff], gainFactor, &targetBlock[sampleOff], n);
        });
    }

private:
    SampleT* m_buffer = nullptr;

    int m_positionIndex = 0;   // position of sample index 0 inside _buffer. This is where the next sample will be written to
    int m_allocatedSize = 0; // 2^n buffer size
    int m_bufferSizeMask = 0; // 2^n-1 buffer mask
};
} // namespace muse::audio::fx

#endif // MUSE_AUDIO_CIRCULARSAMPLEBUFFER_H
