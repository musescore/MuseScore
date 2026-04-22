/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#pragma once

#include "iaudiocontext.h"
#include "modularity/ioc.h"

#include "mixer.h"

namespace muse::audio::engine {
class AudioContext : public IAudioContext
{
public:
    AudioContext(const modularity::IoCID& ctxId);

    Ret init(const OutputSpec& outputSpec, const RenderConstraints& consts);

    void setOutputSpec(const OutputSpec& outputSpec);

    void setIsIdle(bool idle);
    void setIsActive(bool active) override;

    samples_t process(float* buffer, samples_t samplesPerChannel) override;

    std::shared_ptr<IAudioSource> output() const override;

    void setPlayhead(std::shared_ptr<IPlayhead> playhead) override;
    void setTracksToProcessWhenIdle(const std::unordered_set<TrackId>& trackIds) override;

    AudioOutputParams masterOutputParams() const override;
    void setMasterOutputParams(const AudioOutputParams& params) override;
    void clearMasterOutputParams() override;
    async::Channel<AudioOutputParams> masterOutputParamsChanged() const override;

    Ret addChannel(ITrackAudioOutputPtr output) override;
    Ret addAuxChannel(ITrackAudioOutputPtr output) override;
    Ret removeChannel(const TrackId trackId) override;

    AudioSignalChanges masterAudioSignalChanges() const override;

private:
    modularity::IoCID m_ctxId = 0;

    std::shared_ptr<Mixer> m_mixer;
};
}
