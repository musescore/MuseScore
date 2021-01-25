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
#include "equaliser.h"
#include "log.h"
#include <cmath>
using namespace mu::audio;

Equaliser::Equaliser()
{
}

unsigned int Equaliser::streamCount() const
{
    return 1;
}

void Equaliser::setSampleRate(unsigned int sampleRate)
{
    AbstractAudioInsert::setSampleRate(sampleRate);
    calculate();
}

void Equaliser::process(float* input, float* output, unsigned int sampleCount)
{
    for (unsigned int i = 0; i < sampleCount; ++i) {
        m_x[2] = m_x[1];
        m_x[1] = m_x[0];
        m_x[0] = input[i];

        m_y[2] = m_y[1];
        m_y[1] = m_y[0];
        m_y[0] = (m_b[2] * m_x[2] + m_b[1] * m_x[1] + m_b[0] * m_x[0] - m_a[1] * m_y[1] - m_a[2] * m_y[2]) / m_a[0];

        output[i] = m_y[0];
    }
}

void Equaliser::calculate()
{
    if (!m_sampleRate) {
        return;
    }
    float a = std::pow(10.f, m_gain / 40.f);
    float w0 = 2 * M_PI * m_frequency / m_sampleRate;
    float alpha = std::sin(w0) * a / (2 * m_q);

    m_b[0] = 1 + alpha * a;
    m_b[1] = -2 * std::cos(w0);
    m_b[2] = 1 - alpha * a;

    m_a[0] = 1 + alpha / a;
    m_a[1] = -2 * std::cos(w0);
    m_a[2] = 1 - alpha / a;
}

void mu::audio::Equaliser::setFrequency(float value)
{
    m_frequency = value;
    calculate();
}

void mu::audio::Equaliser::setGain(float value)
{
    m_gain = value;
    calculate();
}

void mu::audio::Equaliser::setQ(float value)
{
    m_q = value;
    calculate();
}
