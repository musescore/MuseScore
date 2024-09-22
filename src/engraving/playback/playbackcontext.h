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
class Hairpin;
class PlayTechAnnotation;
class SoundFlag;
class Score;
class MeasureRepeat;
class TextBase;

class PlaybackContext
{
public:
    muse::mpe::dynamic_level_t appliableDynamicLevel(const track_idx_t trackIdx, const int nominalPositionTick) const;
    muse::mpe::ArticulationType persistentArticulationType(const int nominalPositionTick) const;

    muse::mpe::PlaybackParamList playbackParams(const track_idx_t trackIdx, const int nominalPositionTick) const;

    muse::mpe::PlaybackParamLayers playbackParamLayers(const Score* score) const;
    muse::mpe::DynamicLevelLayers dynamicLevelLayers(const Score* score) const;

    void update(const ID partId, const Score* score, bool expandRepeats = true);
    void clear();

    bool hasSoundFlags() const;

private:
    struct DynamicInfo {
        muse::mpe::dynamic_level_t level = 0;
        int priority = -1;
    };

    using DynamicMap = std::map<int /*nominalPositionTick*/, DynamicInfo>;
    using DynamicsByTrack = std::unordered_map<track_idx_t, DynamicMap>;

    using SoundFlagMap = std::unordered_map<staff_idx_t, const SoundFlag*>;
    using ParamMap = std::map<int /*nominalPositionTick*/, muse::mpe::PlaybackParamList>;
    using ParamsByTrack = std::unordered_map<track_idx_t, ParamMap>;

    using PlayTechniquesMap = std::map<int /*nominalPositionTick*/, muse::mpe::ArticulationType>;

    muse::mpe::dynamic_level_t nominalDynamicLevel(const track_idx_t trackIdx, const int positionTick) const;

    void updateDynamicMap(const Dynamic* dynamic, const Segment* segment, const int segmentPositionTick);
    void updatePlayTechMap(const PlayTechAnnotation* annotation, const int segmentPositionTick);
    void updatePlaybackParamsForSoundFlags(const SoundFlagMap& flagsOnSegment, const int segmentPositionTick);
    void updatePlaybackParamsForText(const TextBase* text, const int segmentPositionTick);

    void handleSpanners(const ID partId, const Score* score, const int segmentStartTick, const int segmentEndTick,
                        const int tickPositionOffset);
    void handleHairpin(const Hairpin* hairpin, const int tickPositionOffset);
    void handleSegmentAnnotations(const ID partId, const Segment* segment, const int segmentPositionTick);
    void handleSegmentElements(const Segment* segment, const int segmentPositionTick,
                               std::vector<const MeasureRepeat*>& foundMeasureRepeats);
    void handleMeasureRepeats(const std::vector<const MeasureRepeat*>& measureRepeats, const int tickPositionOffset);

    void applyDynamic(const EngravingItem* dynamicItem, muse::mpe::dynamic_level_t dynamicLevel, const int positionTick);

    static void copyDynamicsInRange(DynamicsByTrack& source, const int rangeStartTick, const int rangeEndTick,
                                    const int newDynamicsOffsetTick);
    static void copyPlaybackParamsInRange(ParamsByTrack& source, const int rangeStartTick, const int rangeEndTick,
                                          const int newParamsOffsetTick);
    static void copyPlayTechniquesInRange(PlayTechniquesMap& source, const int rangeStartTick, const int rangeEndTick,
                                          const int newPlayTechOffsetTick);

    static const muse::mpe::PlaybackParamList& findParams(const ParamsByTrack& all, const track_idx_t trackIdx,
                                                          const int nominalPositionTick, bool* startAtNominalTick = nullptr);

    track_idx_t m_partStartTrack = 0;
    track_idx_t m_partEndTrack = 0;

    DynamicsByTrack m_dynamicsByTrack;
    ParamsByTrack m_soundFlagParamsByTrack;
    ParamsByTrack m_textParamsByTrack;
    PlayTechniquesMap m_playTechniquesMap;
};

using PlaybackContextPtr = std::shared_ptr<PlaybackContext>;
}

#endif // MU_ENGRAVING_PLAYBACKCONTEXT_H
