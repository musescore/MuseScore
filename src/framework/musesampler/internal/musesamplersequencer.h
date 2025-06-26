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

#ifndef MUSE_MUSESAMPLER_MUSESAMPLERSEQUENCER_H
#define MUSE_MUSESAMPLER_MUSESAMPLERSEQUENCER_H

#include "audio/internal/abstracteventsequencer.h"
#include "imusesamplertracks.h"

#include "internal/apitypes.h"
#include "internal/libhandler.h"

#include "global/timer.h"

typedef typename std::variant<muse::mpe::NoteEvent, muse::musesampler::AuditionStartNoteEvent,
                              muse::musesampler::AuditionStopNoteEvent> MuseSamplerEvent;

template<>
struct std::less<MuseSamplerEvent>
{
    bool operator()(const MuseSamplerEvent& first,
                    const MuseSamplerEvent& second) const
    {
        if (first.index() != second.index()) {
            return first.index() < second.index();
        }

        if (std::holds_alternative<muse::musesampler::AuditionStartNoteEvent>(first)) {
            auto& e1 = std::get<muse::musesampler::AuditionStartNoteEvent>(first);
            auto& e2 = std::get<muse::musesampler::AuditionStartNoteEvent>(second);
            if (e1.msEvent._pitch == e2.msEvent._pitch) {
                return e1.msEvent._offset_cents < e2.msEvent._offset_cents;
            }
            return e1.msEvent._pitch < e2.msEvent._pitch;
        }

        if (std::holds_alternative<muse::musesampler::AuditionStopNoteEvent>(first)) {
            return std::get<muse::musesampler::AuditionStopNoteEvent>(first).msEvent._pitch
                   < std::get<muse::musesampler::AuditionStopNoteEvent>(second).msEvent._pitch;
        }

        return false;
    }
};

namespace muse::musesampler {
class MuseSamplerSequencer : public muse::audio::AbstractEventSequencer<mpe::NoteEvent, AuditionStartNoteEvent, AuditionStopNoteEvent>
{
public:
    void init(MuseSamplerLibHandlerPtr samplerLib, ms_MuseSampler sampler, IMuseSamplerTracks* tracks, std::string&& defaultPresetCode);
    void deinit();

    void setRenderingProgress(audio::InputProcessingProgress* progress);
    void setAutoRenderInterval(double secs);
    void triggerRender();

private:
    void updateOffStreamEvents(const mpe::PlaybackEventsMap& events, const mpe::DynamicLevelLayers& dynamics) override;
    void updateMainStreamEvents(const mpe::PlaybackEventsMap& events, const mpe::DynamicLevelLayers& dynamics) override;

    void pollRenderingProgress();
    void doPollProgress();

    void clearAllTracks();
    void finalizeAllTracks();

    ms_Track findOrCreateTrack(mpe::layer_idx_t layerIdx);
    ms_Track findTrack(mpe::layer_idx_t layerIdx) const;

    const TrackList& allTracks() const;

    void loadEvents(const mpe::PlaybackEventsMap& changes);
    void loadDynamicEvents(const mpe::DynamicLevelLayers& changes);

    void addNoteEvent(const mpe::NoteEvent& noteEvent);
    void addTextArticulationEvent(const mpe::TextArticulationEvent& event, long long startUs);
    void addSoundPresetEvent(const mpe::SoundPresetChangeEvent& event, long long positionUs);
    void addSyllableEvent(const mpe::SyllableEvent& event, long long positionUs);
    void addPitchBends(const mpe::NoteEvent& noteEvent, long long noteEventId, ms_Track track);
    void addVibrato(const mpe::NoteEvent& noteEvent, long long noteEventId, ms_Track track);

    void pitchAndTuning(const mpe::pitch_level_t nominalPitch, int& pitch, int& centsOffset) const;
    int pitchLevelToCents(const mpe::pitch_level_t pitchLevel) const;
    double dynamicLevelRatio(const mpe::dynamic_level_t level) const;

    ms_NoteArticulation convertArticulationType(mpe::ArticulationType articulation) const;
    void parseArticulations(const mpe::ArticulationMap& articulations, ms_NoteArticulation& articulationFlag, ms_NoteHead& notehead) const;

    struct OffStreamParams {
        std::string presets;
        std::string textArticulation;
        std::string syllable;
        bool textArticulationStartsAtNote = false;
        bool syllableStartsAtNote = false;

        void clear()
        {
            *this = OffStreamParams();
        }
    };

    void parseOffstreamParams(const mpe::PlaybackEvent& event, OffStreamParams& out) const;

    struct RenderingInfo {
        long long initialChunksDurationUs = 0;
        int errorCode = 0;
        int64_t percentage = 0;
        audio::InputProcessingProgress::ChunkInfoList lastReceivedChunks;

        void clear()
        {
            initialChunksDurationUs = 0;
            errorCode = 0;
            percentage = 0;
            lastReceivedChunks.clear();
        }
    };

    MuseSamplerLibHandlerPtr m_samplerLib = nullptr;
    ms_MuseSampler m_sampler = nullptr;
    IMuseSamplerTracks* m_tracks = nullptr;

    std::unordered_map<mpe::layer_idx_t, track_idx_t> m_layerIdxToTrackIdx;
    std::unordered_map<ms_Track, std::map<long long /*startUs*/, ms_PresetChange> > m_presetChangesByTrack;

    std::string m_defaultPresetCode;
    OffStreamParams m_offStreamCache;

    std::unique_ptr<Timer> m_pollRenderingProgressTimer;
    audio::InputProcessingProgress* m_renderingProgress = nullptr;
    RenderingInfo m_renderingInfo;
};
}

#endif // MUSE_MUSESAMPLER_MUSESAMPLERSEQUENCER_H
