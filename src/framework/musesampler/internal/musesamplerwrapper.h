/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#ifndef MUSE_MUSESAMPLER_MUSESAMPLERWRAPPER_H
#define MUSE_MUSESAMPLER_MUSESAMPLERWRAPPER_H

#include <memory>

#include "audio/internal/abstractsynthesizer.h"
#include "async/channel.h"

#include "libhandler.h"
#include "musesamplersequencer.h"

#include "imusesamplertracks.h"

namespace muse::musesampler {
class MuseSamplerWrapper : public muse::audio::synth::AbstractSynthesizer, public IMuseSamplerTracks,
    public std::enable_shared_from_this<MuseSamplerWrapper>
{
public:
    MuseSamplerWrapper(MuseSamplerLibHandlerPtr samplerLib, const InstrumentInfo& instrument, const muse::audio::AudioSourceParams& params);
    ~MuseSamplerWrapper() override;

    void setSampleRate(unsigned int sampleRate) override;
    unsigned int audioChannelsCount() const override;
    async::Channel<unsigned int> audioChannelsCountChanged() const override;
    muse::audio::samples_t process(float* buffer, muse::audio::samples_t samplesPerChannel) override;

    std::string name() const override;
    muse::audio::AudioSourceType type() const override;
    void flushSound() override;
    bool isValid() const override;

    void revokePlayingNotes() override;

private:
    void setupSound(const mpe::PlaybackSetupData& setupData) override;
    void setupEvents(const mpe::PlaybackData& playbackData) override;
    void updateRenderingMode(const muse::audio::RenderMode mode) override;

    // IMuseSamplerTracks
    const TrackList& allTracks() const override;
    ms_Track addTrack() override;

    muse::audio::msecs_t playbackPosition() const override;
    void setPlaybackPosition(const muse::audio::msecs_t newPosition) override;
    bool isActive() const override;
    void setIsActive(bool arg) override;

    InstrumentInfo resolveInstrument(const mpe::PlaybackSetupData& setupData) const;
    std::string resolveDefaultPresetCode(const InstrumentInfo& instrument) const;

    void handleAuditionEvents(const MuseSamplerSequencer::EventType& event);
    void setCurrentPosition(const muse::audio::samples_t samples);
    void extractOutputSamples(muse::audio::samples_t samples, float* output);

    async::Channel<unsigned int> m_audioChannelsCountChanged;

    MuseSamplerLibHandlerPtr m_samplerLib = nullptr;
    ms_MuseSampler m_sampler = nullptr;
    InstrumentInfo m_instrument;
    TrackList m_tracks;
    ms_OutputBuffer m_bus;

    muse::audio::samples_t m_currentPosition = 0;

    std::vector<float> m_leftChannel;
    std::vector<float> m_rightChannel;

    std::array<float*, 2> m_internalBuffer;

    bool m_offlineModeStarted = false;

    MuseSamplerSequencer m_sequencer;
};

using MuseSamplerWrapperPtr = std::shared_ptr<MuseSamplerWrapper>;
}

#endif // MUSE_MUSESAMPLER_MUSESAMPLERWRAPPER_H
