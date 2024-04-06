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

typedef typename std::variant<mu::mpe::NoteEvent, muse::musesampler::AuditionStartNoteEvent,
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
class MuseSamplerSequencer : public muse::audio::AbstractEventSequencer<mu::mpe::NoteEvent, AuditionStartNoteEvent, AuditionStopNoteEvent>
{
public:
    void init(MuseSamplerLibHandlerPtr samplerLib, ms_MuseSampler sampler, IMuseSamplerTracksPtr tracks, std::string&& defaultPresetCode);

    void updateOffStreamEvents(const mpe::PlaybackEventsMap& events, const mpe::PlaybackParamMap& params) override;
    void updateMainStreamEvents(const mpe::PlaybackEventsMap& events, const mpe::DynamicLevelMap& dynamics,
                                const mpe::PlaybackParamMap& params) override;

private:
    void clearAllTracks();
    void finalizeAllTracks();

    ms_Track resolveTrack(mu::mpe::staff_layer_idx_t staffLayerIdx);
    const TrackList& allTracks() const;

    void loadParams(const mpe::PlaybackParamMap& changes);
    void loadNoteEvents(const mpe::PlaybackEventsMap& changes);
    void loadDynamicEvents(const mpe::DynamicLevelMap& changes);

    void addNoteEvent(const mpe::NoteEvent& noteEvent);
    void addTextArticulation(const std::string& articulationCode, long long startUs);
    void addPresets(const std::vector<std::string>& presets, long long startUs);
    void addPitchBends(const mpe::NoteEvent& noteEvent, long long noteEventId, ms_Track track);
    void addVibrato(const mpe::NoteEvent& noteEvent, long long noteEventId, ms_Track track);

    void pitchAndTuning(const mpe::pitch_level_t nominalPitch, int& pitch, int& centsOffset) const;
    int pitchLevelToCents(const mpe::pitch_level_t pitchLevel) const;
    double dynamicLevelRatio(const mpe::dynamic_level_t level) const;

    ms_NoteArticulation convertArticulationType(mpe::ArticulationType articulation) const;
    void parseArticulations(const mu::mpe::ArticulationMap& articulations, ms_NoteArticulation& articulationFlag,
                            ms_NoteHead& notehead) const;

    void parseOffStreamParams(const mu::mpe::PlaybackParamMap& params, std::string& presets, std::string& textArticulation) const;

    MuseSamplerLibHandlerPtr m_samplerLib = nullptr;
    ms_MuseSampler m_sampler = nullptr;
    IMuseSamplerTracksPtr m_tracks = nullptr;

    using layer_idx_t = size_t;
    std::unordered_map<layer_idx_t, track_idx_t> m_layerIdxToTrackIdx;

    struct {
        std::string presets;
        std::string textArticulation;

        void clear()
        {
            presets.clear();
            textArticulation.clear();
        }
    } m_offStreamCache;

    std::string m_defaultPresetCode;
};
}

#endif // MUSE_MUSESAMPLER_MUSESAMPLERSEQUENCER_H
