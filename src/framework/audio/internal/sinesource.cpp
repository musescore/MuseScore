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
#include "sinesource.h"
#include <cmath>

using namespace mu::audio;

SineSource::SineSource()
{
}

unsigned int SineSource::streamCount() const
{
    return 1;
}

void SineSource::forward(unsigned int sampleCount)
{
    auto streams = streamCount();
    for (unsigned int i = 0; i < sampleCount; ++i) {
        m_phase += m_frequency / m_sampleRate * 2 * M_PI;
        if (m_phase > 2 * M_PI) {
            m_phase -= 2 * M_PI;
        }

        for (unsigned int s = 0; s < streams; ++s) {
            m_buffer[streams * i + s] = 0.1 * std::sin(m_phase + s * 2 * M_PI / streams);
        }
    }
}
