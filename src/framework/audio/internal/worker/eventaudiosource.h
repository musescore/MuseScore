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

#ifndef MU_AUDIO_EVENTAUDIOSOURCE_H
#define MU_AUDIO_EVENTAUDIOSOURCE_H

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "mpe/events.h"

#include "audiotypes.h"
#include "isynthresolver.h"
#include "track.h"

namespace mu::audio {
class EventAudioSource : public ITrackAudioInput, public async::Asyncable
{
    INJECT(synth::ISynthResolver, synthResolver)

public:
    explicit EventAudioSource(const TrackId trackId, const mpe::PlaybackData& playbackData);

    ~EventAudioSource() override;

    bool isActive() const override;
    void setIsActive(const bool active) override;

    void setSampleRate(unsigned int sampleRate) override;
    unsigned int audioChannelsCount() const override;
    async::Channel<unsigned int> audioChannelsCountChanged() const override;
    samples_t process(float* buffer, size_t bufferSize, samples_t samplesPerChannel) override;

    void seek(const msecs_t newPositionMsecs) override;

    const AudioInputParams& inputParams() const override;
    void applyInputParams(const AudioInputParams& requiredParams) override;
    async::Channel<AudioInputParams> inputParamsChanged() const override;

private:
    struct SynthCtx
    {
        bool isActive = false;
        msecs_t playbackPosition = -1;

        bool isValid() const
        {
            return playbackPosition >= 0;
        }
    };

    void setupSource();
    SynthCtx currentSynthCtx() const;
    void restoreSynthCtx(SynthCtx&& ctx);

    TrackId m_trackId = -1;
    mpe::PlaybackData m_playbackData;
    synth::ISynthesizerPtr m_synth = nullptr;
    AudioInputParams m_params;
    async::Channel<AudioInputParams> m_paramsChanges;

    samples_t m_sampleRate = 0;
};
}

#endif // EVENTAUDIOSOURCE_H
