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

#ifndef MU_AUDIO_ISYNTHESIZER_H
#define MU_AUDIO_ISYNTHESIZER_H

#include "async/channel.h"
#include "async/asyncable.h"
#include "mpe/events.h"

#include "synthtypes.h"
#include "audiotypes.h"
#include "iaudiosource.h"
#include "abstracteventsequencer.h"

namespace mu::audio::synth {
class AbstractSynthesizer : public IAudioSource, public async::Asyncable
{
public:
    AbstractSynthesizer(const audio::AudioInputParams& params);
    virtual ~AbstractSynthesizer();

    virtual std::string name() const = 0;
    virtual AudioSourceType type() const = 0;
    const audio::AudioInputParams& params() const;
    async::Channel<audio::AudioInputParams> paramsChanged() const;

    virtual msecs_t playbackPosition() const = 0;
    virtual void setPlaybackPosition(const msecs_t newPosition) = 0;

    void setup(const mpe::PlaybackData& playbackData);
    virtual void flushSound() = 0;
    virtual void revokePlayingNotes();

    virtual bool isValid() const = 0;
    virtual bool isActive() const = 0;
    virtual void setIsActive(bool arg) = 0;

protected:

    virtual void setupSound(const mpe::PlaybackSetupData& setupData) = 0;
    virtual void setupEvents(const mpe::PlaybackData& playbackData) = 0;

    msecs_t samplesToMsecs(const samples_t samplesPerChannel, const samples_t sampleRate) const;
    samples_t microSecsToSamples(const msecs_t msec, const samples_t sampleRate) const;

    mpe::PlaybackSetupData m_setupData;

    audio::AudioInputParams m_params;
    async::Channel<audio::AudioInputParams> m_paramsChanges;

    samples_t m_sampleRate = 0;
};

using ISynthesizerPtr = std::shared_ptr<AbstractSynthesizer>;
}

#endif // MU_AUDIO_ISYNTHESIZER_H
