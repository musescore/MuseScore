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
#include "compressor.h"

#include "audiomathutils.h"

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::dsp;

static constexpr float RATIO = 4.f;

Compressor::Compressor(const unsigned int sampleRate)
    : m_filterConfig(sampleRate, RATIO),
    m_feedbackFactor(1.f)   // 0 = feed forward, 1 = feed back
{
}

bool Compressor::isActive() const
{
    return m_isActive;
}

void Compressor::setIsActive(const bool active)
{
    m_isActive = active;
}

volume_db_t Compressor::gainSmoothing(const float& newGainReduction) const
{
    float coefficient = 0.f;

    if (newGainReduction <= m_previousGainReduction) {
        coefficient = m_filterConfig.attackTimeCoefficient();
    } else {
        coefficient = m_filterConfig.releaseTimeCoefficient();
    }

    return (coefficient * m_previousGainReduction) + ((1 - coefficient) * newGainReduction);
}

volume_db_t Compressor::computeGain(const volume_db_t& logarithmSample) const
{
    if (logarithmSample < m_softThresholdLower) {
        return logarithmSample;
    }

    if (logarithmSample >= m_softThresholdLower && logarithmSample <= m_softThresholdUpper) {
        return logarithmSample
               + ((((1 / m_filterConfig.ratio()) - 1) * std::pow(logarithmSample - m_softThresholdUpper, 2))
                  / (2 * m_filterConfig.kneeWidth()));
    }

    return m_filterConfig.logarithmicThreshold()
           + ((logarithmSample - m_filterConfig.logarithmicThreshold()) / m_filterConfig.ratio());
}

void Compressor::process(const float linearRms, float* buffer, const audioch_t& audioChannelsCount, const samples_t samplesPerChannel)
{
    float dbGain = muse::linear_to_db(linearRms);

    if (dbGain <= m_filterConfig.minimumOperableLevel()) {
        return;
    }

    // trying to predict the next sample by the previous gain reduction
    dbGain += m_feedbackFactor * m_feedbackGain;

    float dbDiff = computeGain(dbGain) - dbGain;

    m_feedbackGain = dbDiff;
    float gainFact = muse::db_to_linear(dbDiff * (1.f + m_feedbackFactor));

    float currentGainReduction = std::min(gainFact, m_previousGainReduction);

    // apply gain
    for (audioch_t audioChNum = 0; audioChNum < audioChannelsCount; ++audioChNum) {
        multiplySamples(buffer, audioChannelsCount, audioChNum, samplesPerChannel, currentGainReduction);
    }

    m_previousGainReduction = currentGainReduction;
}
