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

#ifndef MUSE_AUDIO_IMPROVEDVELVETNOISEDECORRELATION_H
#define MUSE_AUDIO_IMPROVEDVELVETNOISEDECORRELATION_H

#include <cassert>

#include "sparsefirfilter.h"

/*
 Improved Velvet Noise Decorrelation using pre-optimized impulse sequences.
*/

namespace muse::audio::fx {
extern const float ivn15_gn[64][15];
extern const float ivn15_ms[64][15];

class ImprovedVelvetNoiseDecorrelation
{
public:
    void configure(int sequence_number, double samplerate, int maxBlockSize)
    {
        assert(sequence_number >= 0 && sequence_number < 64);

        m_filter.setFormat(maxBlockSize, int(samplerate * 0.03));
        m_filter.clearImpulses();
        for (int i = 0; i < 15; ++i) {
            m_filter.appendImpulse(int(ivn15_ms[sequence_number][i] * 0.001f * samplerate + 0.5f),
                                   ivn15_gn[sequence_number][i]);
        }
    }

    float processSample(float smp)
    {
        return m_filter.processSample(smp);
    }

    void processBlock(const float* smp_in, float* smp_out, int n)
    {
        m_filter.processBlock(smp_in, smp_out, n);
    }

    void reset()
    {
        m_filter.reset();
    }

private:
    SparseFirFilter m_filter;
};
} // namespace muse::audio::fx

#endif // MUSE_AUDIO_IMPROVEDVELVETNOISEDECORRELATION_H
