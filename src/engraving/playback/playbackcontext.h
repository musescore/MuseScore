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

#ifndef MU_ENGRAVING_PLAYBACKCONTEXT_H
#define MU_ENGRAVING_PLAYBACKCONTEXT_H

#include "mpe/mpetypes.h"

#include "types/types.h"

#include "dom/mscore.h"

namespace mu::engraving {
class Segment;
class Dynamic;
class PlayTechAnnotation;
class Score;

class PlaybackContext
{
public:
    void update(const ID partId, const Score* score);
    void clear();

    mpe::DynamicLevelLayers dynamicLevelLayers(const Score* score) const;
    mpe::dynamic_level_t appliableDynamicLevel(const voice_idx_t voiceId, const int nominalPositionTick) const;
    mpe::ArticulationType persistentArticulationType(const int nominalPositionTick) const;

private:
    mpe::dynamic_level_t nominalDynamicLevel(const voice_idx_t voiceIdx, const int positionTick) const;

    void updateDynamicMap(const Dynamic* dynamic, const Segment* segment, const int segmentPositionTick);
    void updatePlayTechMap(const PlayTechAnnotation* annotation, const int segmentPositionTick);
    void applyDynamicToNextSegment(const Segment* currentSegment, const voice_idx_t voiceIdx, const int segmentPositionTick,
                                   const mpe::dynamic_level_t dynamicLevel);

    void handleSpanners(const ID partId, const Score* score, const int segmentStartTick, const int segmentEndTick,
                        const int tickPositionOffset);
    void handleAnnotations(const ID partId, const Segment* segment, const int segmentPositionTick);

    void setDynamicLevel(const voice_idx_t voiceIdx, const int positionTick, const mpe::dynamic_level_t lvl);
    void setDynamicLevelForAllVoices(const int positionTick, const mpe::dynamic_level_t lvl);

    using DynamicsMap = std::map<int /*nominalPositionTick*/, mpe::dynamic_level_t>;
    using DynamicsByVoice = std::array<DynamicsMap, mu::engraving::VOICES>;

    using PlayTechniquesMap = std::map<int /*nominalPositionTick*/, mpe::ArticulationType>;

    DynamicsByVoice m_dynamicsByVoice;
    PlayTechniquesMap m_playTechniquesMap;
};
}

#endif // MU_ENGRAVING_PLAYBACKCONTEXT_H
