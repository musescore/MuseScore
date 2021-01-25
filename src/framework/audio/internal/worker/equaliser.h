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
#ifndef MU_AUDIO_EQUALISER_H
#define MU_AUDIO_EQUALISER_H

#include "abstractaudioinsert.h"

namespace mu::audio {
class Equaliser : public AbstractAudioInsert
{
public:
    Equaliser();

    unsigned int streamCount() const override;
    void setSampleRate(unsigned int sampleRate) override;

    void setFrequency(float value);
    void setGain(float value);
    void setQ(float value);

    void process(float* input, float* output, unsigned int sampleCount) override;

private:
    void calculate();

    float m_gain = 0, m_frequency = 1'000.f, m_q = 1.f;
    float m_a[3] = { 0, 0, 0 };
    float m_b[3] = { 0, 0, 0 };
    float m_x[3] = { 0, 0, 0 };
    float m_y[3] = { 0, 0, 0 };
};
}

#endif // MU_AUDIO_EQUALISER_H
