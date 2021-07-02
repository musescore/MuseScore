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
#ifndef MU_AUDIO_EQUALISER_H
#define MU_AUDIO_EQUALISER_H

#include "ifxprocessor.h"

namespace mu::audio {
class Equaliser : public IFxProcessor
{
public:
    Equaliser();

    FxProcessorId id() const override;

    unsigned int streamCount() const override;
    void setSampleRate(unsigned int sampleRate) override;

    bool active() const override;
    void setActive(bool active) override;

    void setFrequency(float value);
    void setGain(float value);
    void setQ(float value);

    void process(float* input, float* output, unsigned int sampleCount) override;

private:
    void calculate();

    unsigned int m_sampleRate = 0;
    bool m_active = true;

    float m_gain = 0, m_frequency = 1'000.f, m_q = 1.f;
    float m_a[3] = { 0, 0, 0 };
    float m_b[3] = { 0, 0, 0 };
    float m_x[3] = { 0, 0, 0 };
    float m_y[3] = { 0, 0, 0 };
};
}

#endif // MU_AUDIO_EQUALISER_H
