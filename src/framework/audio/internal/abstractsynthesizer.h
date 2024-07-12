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

#ifndef MUSE_AUDIO_ISYNTHESIZER_H
#define MUSE_AUDIO_ISYNTHESIZER_H

#include "global/async/channel.h"
#include "global/async/asyncable.h"
#include "global/modularity/ioc.h"
#include "mpe/events.h"

#include "../audiotypes.h"
#include "../isynthesizer.h"
#include "../iaudioconfiguration.h"
#include "worker/iaudioengine.h"

namespace muse::audio::synth {
class AbstractSynthesizer : public ISynthesizer, public Injectable, public async::Asyncable
{
public:
    muse::Inject<IAudioConfiguration> config = { this };
    muse::Inject<IAudioEngine> audioEngine = { this };

public:
    AbstractSynthesizer(const audio::AudioInputParams& params, const modularity::ContextPtr& iocCtx);
    virtual ~AbstractSynthesizer() = default;

    const audio::AudioInputParams& params() const override;
    async::Channel<audio::AudioInputParams> paramsChanged() const override;

    void setup(const mpe::PlaybackData& playbackData) override;

    void revokePlayingNotes() override;

protected:

    virtual void setupSound(const mpe::PlaybackSetupData& setupData) = 0;
    virtual void setupEvents(const mpe::PlaybackData& playbackData) = 0;
    virtual void updateRenderingMode(const RenderMode mode);

    audio::RenderMode currentRenderMode() const;

    msecs_t samplesToMsecs(const samples_t samplesPerChannel, const samples_t sampleRate) const;
    samples_t microSecsToSamples(const msecs_t msec, const samples_t sampleRate) const;

    mpe::PlaybackSetupData m_setupData;

    audio::AudioInputParams m_params;
    async::Channel<audio::AudioInputParams> m_paramsChanges;

    samples_t m_sampleRate = 0;
};
}

#endif // MUSE_AUDIO_ISYNTHESIZER_H
