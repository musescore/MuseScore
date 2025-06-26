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
class ChordRest;

class PlaybackContext
{
public:
    muse::mpe::dynamic_level_t appliableDynamicLevel(const track_idx_t trackIdx, const int nominalPositionTick) const;
    muse::mpe::ArticulationType persistentArticulationType(const int nominalPositionTick) const;

    std::map<muse::mpe::timestamp_t, muse::mpe::SoundPresetChangeEventList> soundPresets(const Score* score) const;
    muse::mpe::SoundPresetChangeEventList soundPresets(const track_idx_t trackIdx, const int nominalPositionTick) const;

    std::map<muse::mpe::timestamp_t, muse::mpe::TextArticulationEventList> textArticulations(const Score* score) const;
    muse::mpe::TextArticulationEvent textArticulation(const track_idx_t trackIdx, const int nominalPositionTick) const;

    std::map<muse::mpe::timestamp_t, muse::mpe::SyllableEventList> syllables(const Score* score) const;
    muse::mpe::SyllableEvent syllable(const track_idx_t trackIdx, const int nominalPositionTick) const;

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
    using DynamicsByTrack = std::map<track_idx_t, DynamicMap>;

    using SoundPresetsMap = std::map<int /*nominalPositionTick*/, muse::mpe::SoundPresetChangeEventList>;
    using SoundPresetsByTrack = std::map<track_idx_t, SoundPresetsMap>;

    using TextArticulationMap = std::map<int /*nominalPositionTick*/, muse::mpe::TextArticulationEvent>;
    using TextArticulationsByTrack = std::map<track_idx_t, TextArticulationMap>;

    using SyllableMap = std::map<int /*nominalPositionTick*/, muse::mpe::SyllableEvent>;
    using SyllablesByTrack = std::map<track_idx_t, SyllableMap>;

    using PlayTechniquesMap = std::map<int /*nominalPositionTick*/, muse::mpe::ArticulationType>;

    using SoundFlagMap = std::unordered_map<staff_idx_t, const SoundFlag*>;

    muse::mpe::dynamic_level_t nominalDynamicLevel(const track_idx_t trackIdx, const int positionTick) const;

    void updateDynamicMap(const Dynamic* dynamic, const Segment* segment, const int segmentPositionTick);
    void updatePlayTechMap(const PlayTechAnnotation* annotation, const int segmentPositionTick);
    void updateSoundPresetAndTextArticulationMap(const SoundFlagMap& flagsOnSegment, const int segmentPositionTick);
    void updateSyllableMap(const TextBase* text, const int segmentPositionTick);

    void handleSpanners(const ID partId, const Score* score, const int segmentStartTick, const int segmentEndTick,
                        const int tickPositionOffset);
    void handleHairpin(const Hairpin* hairpin, const int tickPositionOffset);
    void handleSegmentAnnotations(const ID partId, const Segment* segment, const int segmentPositionTick);
    void handleSegmentElements(const Segment* segment, const int segmentPositionTick,
                               std::vector<const MeasureRepeat*>& foundMeasureRepeats);
    void handleMeasureRepeats(const std::vector<const MeasureRepeat*>& measureRepeats, const int tickPositionOffset);

    void applyDynamic(const EngravingItem* dynamicItem, muse::mpe::dynamic_level_t dynamicLevel, const int positionTick);

    bool shouldSkipTrack(const track_idx_t trackIdx) const;

    track_idx_t m_partStartTrack = 0;
    track_idx_t m_partEndTrack = 0;

    std::set<voice_idx_t> m_usedVoices;
    DynamicsByTrack m_dynamicsByTrack;
    SoundPresetsByTrack m_soundPresetsByTrack;
    TextArticulationsByTrack m_textArticulationsByTrack;
    SyllablesByTrack m_syllablesByTrack;
    PlayTechniquesMap m_playTechniquesMap;

    std::unordered_map<const ChordRest*, int> m_currentVerseNumByChordRest;
};

using PlaybackContextPtr = std::shared_ptr<PlaybackContext>;
}

#endif // MU_ENGRAVING_PLAYBACKCONTEXT_H
