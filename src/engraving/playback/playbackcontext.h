/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_ENGRAVING_PLAYBACKCONTEXT_H
#define MU_ENGRAVING_PLAYBACKCONTEXT_H

#include "mpe/mpetypes.h"
#include "mpe/events.h"

#include "../types/types.h"

namespace mu::engraving {
class EngravingItem;
class Segment;
class Dynamic;
class PlayTechAnnotation;
class SoundFlag;
class Score;
class MeasureRepeat;

class PlaybackContext
{
public:
    muse::mpe::dynamic_level_t appliableDynamicLevel(const track_idx_t trackIdx, const int nominalPositionTick) const;
    muse::mpe::ArticulationType persistentArticulationType(const int nominalPositionTick) const;

    muse::mpe::PlaybackParamMap playbackParamMap(const Score* score, const int nominalPositionTick, const staff_idx_t staffIdx) const;
    muse::mpe::PlaybackParamMap playbackParamMap(const Score* score) const;
    muse::mpe::DynamicLevelLayers dynamicLevelLayers(const Score* score) const;

    void update(const ID partId, const Score* score);
    void clear();

    bool hasSoundFlags() const;

private:
    using DynamicMap = std::map<int /*nominalPositionTick*/, muse::mpe::dynamic_level_t>;
    using DynamicsByTrack = std::unordered_map<track_idx_t, DynamicMap>;

    using PlayTechniquesMap = std::map<int /*nominalPositionTick*/, muse::mpe::ArticulationType>;
    using ParamMap = std::map<int /*nominalPositionTick*/, muse::mpe::PlaybackParamList>;
    using SoundFlagMap = std::map<staff_idx_t, const SoundFlag*>;

    muse::mpe::dynamic_level_t nominalDynamicLevel(const track_idx_t trackIdx, const int positionTick) const;

    void updateDynamicMap(const Dynamic* dynamic, const Segment* segment, const int segmentPositionTick);
    void updatePlayTechMap(const ID partId, const Score* score, const PlayTechAnnotation* annotation, const int segmentPositionTick);
    void updatePlaybackParamMap(const ID partId, const Score* score, const SoundFlagMap& flagsOnSegment, const int segmentPositionTick);

    void handleSpanners(const ID partId, const Score* score, const int segmentStartTick, const int segmentEndTick,
                        const int tickPositionOffset);
    void handleAnnotations(const ID partId, const Score* score, const Segment* segment, const int segmentPositionTick);
    void handleMeasureRepeats(const std::vector<const MeasureRepeat*>& measureRepeats, const int tickPositionOffset);

    void copyDynamicsInRange(const int rangeStartTick, const int rangeEndTick, const int newDynamicsOffsetTick);
    void copyPlaybackParamsInRange(const int rangeStartTick, const int rangeEndTick, const int newParamsOffsetTick);
    void copyPlayTechniquesInRange(const int rangeStartTick, const int rangeEndTick, const int newPlayTechOffsetTick);

    void applyDynamic(const EngravingItem* dynamicItem, muse::mpe::dynamic_level_t dynamicLevel, const int positionTick);

    track_idx_t m_partStartTrack = 0;
    track_idx_t m_partEndTrack = 0;

    DynamicsByTrack m_dynamicsByTrack;
    PlayTechniquesMap m_playTechniquesMap;
    ParamMap m_playbackParamMap;
};
}

#endif // MU_ENGRAVING_PLAYBACKCONTEXT_H
