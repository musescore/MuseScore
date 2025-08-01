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

#ifndef MUSE_AUDIO_ALLPASSDISPERSION_H
#define MUSE_AUDIO_ALLPASSDISPERSION_H

#include "iirbiquadfilter.h"

namespace muse::audio::fx {
class AllPassDispersion
{
public:
    AllPassDispersion()
    {
        // these values have been manually tuned to produce a 50-sample IR that has a (low) peak of 0.453731
        m_processor[0].setTargetCoefficients(IirBiquadFilter::createAllpass2P(0.2492, 2.168, 1.0));
        m_processor[1].setTargetCoefficients(IirBiquadFilter::createAllpass2P(0.1119, 2.033, 1.0));
        reset();
    }

    void reset()
    {
        for (auto& p : m_processor) {
            p.reset();
        }
    }

    void process(float** signalPtr, int numCh, int n)
    {
        for (auto& p : m_processor) {
            p.process(signalPtr, numCh, n);
        }
    }

    void processMonoSample(float& s)
    {
        for (auto& p : m_processor) {
            p.processMonoSample(s);
        }
    }

    void processStereoSample(float& l, float& r)
    {
        for (auto& p : m_processor) {
            p.processStereoSample(l, r);
        }
    }

private:
    std::array<IirBiquadFilter::DF1Processor<double>, 2> m_processor;
};
} // namespace muse::audio::fx

#endif // MUSE_AUDIO_ALLPASSDISPERSION_H
