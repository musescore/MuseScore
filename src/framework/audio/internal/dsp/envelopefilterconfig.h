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

#ifndef MUSE_AUDIO_DSP_ENVELOPEFILTERCONFIG_H
#define MUSE_AUDIO_DSP_ENVELOPEFILTERCONFIG_H

#include "../../audiotypes.h"
#include "audiomathutils.h"

namespace muse::audio::dsp {
struct EnvelopeFilterConfig
{
    explicit EnvelopeFilterConfig(const unsigned int sampleRate,
                                  const float ratio = DEFAULT_RATIO,
                                  const volume_db_t logarithmicThreshold = DEFAULT_THRESHOLD_DB,
                                  const volume_db_t kneeWidth = DEFAULT_KNEE_WIDTH,
                                  const float attackTime = DEFAULT_ATTACK_TIME,
                                  const float releaseTime = DEFAULT_RELEASE_TIME,
                                  const volume_db_t minimumOperableLevel = DEFAULT_MINIMUM_OPERABLE_LEVEL,
                                  const volume_db_t makeUpGain = DEFAULT_MAKE_UP_GAIN)
    {
        m_sampleRate = sampleRate;
        m_ratio = ratio;
        m_kneeWidth = kneeWidth;
        m_logarithmicThreshold = logarithmicThreshold;
        m_attackTime = attackTime;
        m_releaseTime = releaseTime;

        m_attackTimeCoefficient = sampleAttackTimeCoefficient(sampleRate, attackTime);
        m_releaseTimeCoefficient = sampleReleaseTimeCoefficient(sampleRate, releaseTime);

        m_minimumOperableLevel = minimumOperableLevel;

        m_softThresholdLower = m_logarithmicThreshold - (m_kneeWidth / 2.f);
        m_softThresholdUpper = m_logarithmicThreshold + (m_kneeWidth / 2.f);

        m_makeUpGain = makeUpGain;
    }

    static constexpr float DEFAULT_KNEE_WIDTH = 6.f; // [0, 20.f]
    static constexpr float DEFAULT_RATIO = 4.f; // [1, 50.f]
    static constexpr volume_db_t DEFAULT_THRESHOLD_DB = volume_dbfs_t::make(-10.f);
    static constexpr float DEFAULT_ATTACK_TIME = 0.005f; // [0, 4.f]
    static constexpr float DEFAULT_RELEASE_TIME = 0.02f; // [0, 4.f]
    static constexpr volume_db_t DEFAULT_MINIMUM_OPERABLE_LEVEL = volume_dbfs_t::make(-60.f);
    static constexpr volume_db_t DEFAULT_MAKE_UP_GAIN = volume_dbfs_t::make(-10.f); // [-10.f, 12.f]

    unsigned int sampleRate() const
    {
        return m_sampleRate;
    }

    /// Compression ratio is the input/output ratio for signals that overshoot the operation threshold.
    float ratio() const
    {
        return m_ratio;
    }

    volume_db_t kneeWidth() const
    {
        return m_kneeWidth;
    }

    /// Operation threshold is the level above which gain is applied to the input signal.
    volume_db_t logarithmicThreshold() const
    {
        return m_logarithmicThreshold;
    }

    float attackTimeCoefficient() const
    {
        return m_attackTimeCoefficient;
    }

    float releaseTimeCoefficient() const
    {
        return m_releaseTimeCoefficient;
    }

    volume_db_t minimumOperableLevel() const
    {
        return m_minimumOperableLevel;
    }

    volume_db_t softThresholdUpper() const
    {
        return m_softThresholdUpper;
    }

    volume_db_t softThresholdLower() const
    {
        return m_softThresholdLower;
    }

    /// Make-up gain compensates for gain lost during limiting. It is applied at the output of the dynamic range limiter.
    volume_db_t makeUpGain() const
    {
        return m_makeUpGain;
    }

private:
    unsigned int m_sampleRate = 0;
    float m_ratio = 0.f;
    volume_db_t m_kneeWidth = 0.f;
    volume_db_t m_logarithmicThreshold = 0.f;
    float m_attackTime = 0.f;
    float m_releaseTime = 0.f;
    float m_attackTimeCoefficient = 0.f;
    float m_releaseTimeCoefficient = 0.f;
    float m_minimumOperableLevel = 0.f;

    volume_db_t m_softThresholdUpper = 0.f;
    volume_db_t m_softThresholdLower = 0.f;
    volume_db_t m_makeUpGain = 0.f;
};
}

#endif // MUSE_AUDIO_DSP_ABSTRACTENVELOPEFILTER_H
