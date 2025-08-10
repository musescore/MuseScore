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
#include "sinesource.h"

#include <cmath>

using namespace muse::audio;
using namespace muse::audio::worker;

SineSource::SineSource()
{
}

unsigned int SineSource::audioChannelsCount() const
{
    return 2;
}

samples_t SineSource::process(float* buffer, samples_t samplesPerChannel)
{
    auto streams = audioChannelsCount();
    for (unsigned int i = 0; i < samplesPerChannel; ++i) {
        m_phase += m_frequency / m_sampleRate * 2 * M_PI;
        if (m_phase > 2 * M_PI) {
            m_phase -= 2 * float(M_PI);
        }

        for (unsigned int s = 0; s < streams; ++s) {
            buffer[streams * i + s] = 0.1 * std::sin(m_phase + s * 2 * M_PI / streams);
        }
    }

    return samplesPerChannel;
}
