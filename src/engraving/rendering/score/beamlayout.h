/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#ifndef MU_ENGRAVING_BEAMLAYOUT_DEV_H
#define MU_ENGRAVING_BEAMLAYOUT_DEV_H

#include <vector>

#include "../../types/types.h"
#include "../../dom/beam.h"

#include "layoutcontext.h"

namespace mu::engraving {
class Beam;
class Chord;
class Rest;
class ChordRest;
class Measure;
class Score;
class Segment;
}

namespace mu::engraving::rendering::score {
class BeamLayout
{
public:

    static void layout(Beam* item, const LayoutContext& ctx);
    static void layoutIfNeed(Beam* item, const LayoutContext& ctx);
    static void layout1(Beam* item, LayoutContext& ctx);

    static bool isTopBeam(ChordRest* cr);
    static bool notTopBeam(ChordRest* cr);
    static void createBeams(LayoutContext& ctx, Measure* measure);
    static void restoreBeams(Measure* m, LayoutContext& ctx);
    static bool measureMayHaveBeamsJoinedIntoNext(const Measure* measure);
    static void breakCrossMeasureBeams(Measure* measure, LayoutContext& ctx);
    static void layoutNonCrossBeams(ChordRest* cr, LayoutContext& ctx);
    static void verticalAdjustBeamedRests(Rest* rest, Beam* beam, LayoutContext& ctx);
    static void checkCrossPosAndStemConsistency(Beam* beam, LayoutContext& ctx);

    static PointF chordBeamAnchor(const Beam* item, const ChordRest* chord, ChordBeamAnchorType anchorType);
    static double chordBeamAnchorY(const Beam* item, const ChordRest* chord);
    static void setTremAnchors(Beam* item, const LayoutContext& ctx);

private:
    static void beamGraceNotes(LayoutContext& ctx, Chord* mainNote, bool after);

    static void layout2(Beam* item, const LayoutContext& ctx, const std::vector<ChordRest*>& chordRests, SpannerSegmentType, int frag);

    static void createBeamSegments(Beam* item, const LayoutContext& ctx, const std::vector<ChordRest*>& chordRests);
    static bool calcIsBeamletBefore(const Beam* item, Chord* chord, int i, int level, bool isAfter32Break, bool isAfter64Break);
    static void createBeamSegment(Beam* item, ChordRest* startChord, ChordRest* endChord, int level, bool shortStems);
    static void createBeamletSegment(Beam* item, const LayoutContext& ctx, ChordRest* chord, bool isBefore, int level);

    static bool computeUpForMovedCross(Beam* item);
};
}

#endif // MU_ENGRAVING_BEAMLAYOUT_DEV_H
