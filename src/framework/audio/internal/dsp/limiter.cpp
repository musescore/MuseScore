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
#include "limiter.h"

#include "audiomathutils.h"

using namespace muse::audio;
using namespace muse::audio::dsp;

static constexpr volume_db_t THRESHOLD = volume_dbfs_t::make(0.f);

Limiter::Limiter(const unsigned int sampleRate)
    : m_filterConfig(sampleRate, 1.f, THRESHOLD)
{
}

bool Limiter::isActive() const
{
    return m_isActive;
}

void Limiter::setIsActive(const bool active)
{
    m_isActive = active;
}

volume_db_t Limiter::gainSmoothing(const float newGainReduction) const
{
    float coefficient = 0.f;

    if (newGainReduction <= m_previousGainReduction) {
        coefficient = m_filterConfig.attackTimeCoefficient();
    } else {
        coefficient = m_filterConfig.releaseTimeCoefficient();
    }

    return (coefficient * m_previousGainReduction) + ((1 - coefficient) * newGainReduction);
}

volume_db_t Limiter::computeGain(const volume_db_t& logarithmSample) const
{
    if (logarithmSample < m_filterConfig.softThresholdLower()) {
        return logarithmSample;
    }

    if (logarithmSample >= m_filterConfig.softThresholdLower() && logarithmSample <= m_filterConfig.softThresholdUpper()) {
        return logarithmSample
               - (std::pow((logarithmSample - m_filterConfig.softThresholdUpper()), 2)
                  / (2 * m_filterConfig.kneeWidth()));
    }

    return m_filterConfig.logarithmicThreshold();
}

void Limiter::process(const float& linearRms, float* buffer, const audioch_t& audioChannelsCount,
                      const samples_t samplesPerChannel)
{
    volume_db_t rmsDb = muse::linear_to_db(linearRms);

    if (rmsDb <= m_filterConfig.minimumOperableLevel()) {
        return;
    }

    // gain computing
    float computedGain = computeGain(rmsDb) - rmsDb;

    // gain smoothing
    float smoothedGain = gainSmoothing(computedGain);
    m_previousGainReduction = smoothedGain;

    // gain make-up
    float makeUpGain = smoothedGain + m_filterConfig.makeUpGain();

    // total linear gain
    float totalLinearGain = muse::db_to_linear(makeUpGain);

    // apply linear gain
    for (audioch_t audioChNum = 0; audioChNum < audioChannelsCount; ++audioChNum) {
        multiplySamples(buffer, audioChannelsCount, audioChNum, samplesPerChannel, totalLinearGain);
    }
}
