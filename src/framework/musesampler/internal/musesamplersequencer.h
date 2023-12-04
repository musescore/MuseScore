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

#ifndef MU_MUSESAMPLER_MUSESAMPLERSEQUENCER_H
#define MU_MUSESAMPLER_MUSESAMPLERSEQUENCER_H

#include "audio/internal/abstracteventsequencer.h"

#include "internal/apitypes.h"
#include "internal/libhandler.h"

typedef typename std::variant<mu::mpe::NoteEvent, ms_AuditionStartNoteEvent_2, ms_AuditionStopNoteEvent> MuseSamplerEvent;

template<>
struct std::less<MuseSamplerEvent>
{
    bool operator()(const MuseSamplerEvent& first,
                    const MuseSamplerEvent& second) const
    {
        if (first.index() != second.index()) {
            return first.index() < second.index();
        }

        if (std::holds_alternative<ms_AuditionStartNoteEvent_2>(first)) {
            auto& e1 = std::get<ms_AuditionStartNoteEvent_2>(first);
            auto& e2 = std::get<ms_AuditionStartNoteEvent_2>(second);
            if (e1._pitch == e2._pitch) {
                return e1._offset_cents < e2._offset_cents;
            }
            return e1._pitch < e2._pitch;
        }

        if (std::holds_alternative<ms_AuditionStopNoteEvent>(first)) {
            return std::get<ms_AuditionStopNoteEvent>(first)._pitch < std::get<ms_AuditionStopNoteEvent>(second)._pitch;
        }

        return false;
    }
};

namespace mu::musesampler {
class MuseSamplerSequencer : public audio::AbstractEventSequencer<mu::mpe::NoteEvent, ms_AuditionStartNoteEvent_2, ms_AuditionStopNoteEvent>
{
public:
    void init(MuseSamplerLibHandlerPtr samplerLib, ms_MuseSampler sampler, ms_Track track);

    void updateOffStreamEvents(const mpe::PlaybackEventsMap& changes) override;
    void updateMainStreamEvents(const mpe::PlaybackEventsMap& changes) override;
    void updateDynamicChanges(const mpe::DynamicLevelMap& changes) override;

private:
    void reloadTrack();

    void loadNoteEvents(const mpe::PlaybackEventsMap& changes);
    void loadDynamicEvents(const mpe::DynamicLevelMap& changes);

    void addNoteEvent(const mpe::NoteEvent& noteEvent);
    void addPitchBends(const mpe::NoteEvent& noteEvent, long long noteEventId);
    void addVibrato(const mpe::NoteEvent& noteEvent, long long noteEventId);

    void pitchAndTuning(const mpe::pitch_level_t nominalPitch, int& pitch, int& centsOffset) const;
    int pitchLevelToCents(const mpe::pitch_level_t pitchLevel) const;
    double dynamicLevelRatio(const mpe::dynamic_level_t level) const;

    ms_NoteArticulation convertArticulationType(mpe::ArticulationType articulation) const;
    ms_NoteArticulation noteArticulationTypes(const mpe::NoteEvent& noteEvent) const;

    MuseSamplerLibHandlerPtr m_samplerLib = nullptr;
    ms_MuseSampler m_sampler = nullptr;
    ms_Track m_track = nullptr;

    mpe::PlaybackEventsMap m_eventsMap;
};
}

#endif // MU_MUSESAMPLER_MUSESAMPLERSEQUENCER_H
