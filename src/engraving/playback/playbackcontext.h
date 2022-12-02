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

namespace mu::engraving {
class Segment;
class Dynamic;
class PlayTechAnnotation;
class Score;

using DynamicMap = std::map<int /*nominalPositionTick*/, mpe::dynamic_level_t>;
using PlayTechniquesMap = std::map<int /*nominalPositionTick*/, mpe::ArticulationType>;

class PlaybackContext
{
public:
    mpe::dynamic_level_t appliableDynamicLevel(const int nominalPositionTick) const;
    mpe::ArticulationType persistentArticulationType(const int nominalPositionTick) const;

    void update(const ID partId, const Score* score);
    void clear();

    mpe::DynamicLevelMap dynamicLevelMap(const Score* score) const;

private:
    mpe::dynamic_level_t nominalDynamicLevel(const int positionTick) const;

    void updateDynamicMap(const Dynamic* dynamic, const Segment* segment, const int segmentPositionTick);
    void updatePlayTechMap(const PlayTechAnnotation* annotation, const int segmentPositionTick);
    void applyDynamicToNextSegment(const Segment* currentSegment, const int segmentPositionTick, const mpe::dynamic_level_t dynamicLevel);

    void handleSpanners(const ID partId, const Score* score, const int segmentStartTick, const int segmentEndTick,
                        const int tickPositionOffset);
    void handleAnnotations(const ID partId, const Segment* segment, const int segmentPositionTick);

    void removeDynamicData(const int from, const int to);
    void removePlayTechniqueData(const int from, const int to);

    DynamicMap m_dynamicsMap;
    PlayTechniquesMap m_playTechniquesMap;
};
}

#endif // MU_ENGRAVING_PLAYBACKCONTEXT_H
