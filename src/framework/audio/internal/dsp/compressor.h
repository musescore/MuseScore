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

#ifndef MUSE_AUDIO_COMPRESSOR_H
#define MUSE_AUDIO_COMPRESSOR_H

#include <memory>

#include "envelopefilterconfig.h"

namespace muse::audio::dsp {
class Compressor
{
public:
    Compressor(const unsigned int sampleRate);

    bool isActive() const;
    void setIsActive(const bool active);

    void process(const float linearRms, float* buffer, const audioch_t& audioChannelsCount, const samples_t samplesPerChannel);
private:
    float attackTimeCoefficient() const;
    float releaseTimeCoefficient() const;

    volume_db_t gainSmoothing(const float& newGainReduction) const;
    volume_db_t computeGain(const volume_db_t& logarithmSample) const;

    EnvelopeFilterConfig m_filterConfig;

    float m_softThresholdLower = 0.f;
    float m_softThresholdUpper = 0.f;
    float m_previousGainReduction = 1.f;

    float m_feedbackGain = 0.f;
    float m_feedbackFactor = 0.f;

    bool m_isActive = false;
};

using CompressorPtr = std::unique_ptr<Compressor>;
}

#endif // MUSE_AUDIO_COMPRESSOR_H
