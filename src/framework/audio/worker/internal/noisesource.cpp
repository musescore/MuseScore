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
#include "noisesource.h"

#include <random>

using namespace muse::audio;
using namespace muse::audio::worker;

NoiseSource::NoiseSource()
{
}

void NoiseSource::setType(NoiseSource::Type type)
{
    m_type = type;
}

unsigned int NoiseSource::audioChannelsCount() const
{
    return 1;
}

samples_t NoiseSource::process(float* buffer, samples_t samplesPerChannel)
{
    auto streams = audioChannelsCount();
    std::uniform_real_distribution<float> distribution{ -1.f, 1.f };
    std::random_device randomDevice{};
    std::mt19937 gen{ randomDevice() };

    for (unsigned int i = 0; i < samplesPerChannel; ++i) {
        float sample = distribution(gen);
        switch (m_type) {
        case PINK: sample = pinkFilter(sample);
            break;
        default: break;
        }
        for (unsigned int s = 0; s < streams; ++s) {
            buffer[streams * i + s] = sample;
        }
    }

    return samplesPerChannel;
}

float NoiseSource::pinkFilter(float white)
{
    //summation of 7 first order low pass filters
    lpf[0] = 0.99886 * lpf[0] + white * 0.0555179;
    lpf[1] = 0.99332 * lpf[1] + white * 0.0750759;
    lpf[2] = 0.96900 * lpf[2] + white * 0.1538520;
    lpf[3] = 0.86650 * lpf[3] + white * 0.3104856;
    lpf[4] = 0.55000 * lpf[4] + white * 0.5329522;
    lpf[5] = -0.7616 * lpf[5] - white * 0.0168980;
    float pink = lpf[0] + lpf[1] + lpf[2] + lpf[3] + lpf[4] + lpf[5] + lpf[6] + white * 0.5362;
    lpf[6] = white * 0.115926;
    //normalization factor: 3.5
    return pink / 3.5f;
}
