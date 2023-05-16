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
#ifndef MU_ENGRAVING_BEAMLAYOUT_H
#define MU_ENGRAVING_BEAMLAYOUT_H

#include <vector>

#include "types/types.h"

namespace mu::engraving {
class Beam;
class Chord;
class Rest;
class ChordRest;
class Measure;
class Score;
class Segment;

class LayoutContext;
class BeamLayout
{
public:

    static void layout(Beam* item, LayoutContext& ctx);
    static void layout1(Beam* item, LayoutContext& ctx);

    static bool isTopBeam(ChordRest* cr);
    static bool notTopBeam(ChordRest* cr);
    static void createBeams(Score* score, LayoutContext& lc, Measure* measure);
    static void restoreBeams(Measure* m);
    static void breakCrossMeasureBeams(Measure* measure, LayoutContext& ctx);
    static void layoutNonCrossBeams(Segment* s, LayoutContext& ctx);
    static void verticalAdjustBeamedRests(Rest* rest, Beam* beam, LayoutContext& ctx);

private:
    static void beamGraceNotes(Score* score, Chord* mainNote, bool after);

    static void layout2(Beam* item, LayoutContext& ctx, const std::vector<ChordRest*>& chordRests, SpannerSegmentType, int frag);

    static void createBeamSegments(Beam* item, const std::vector<ChordRest*>& chordRests);
    static bool calcIsBeamletBefore(const Beam* item, Chord* chord, int i, int level, bool isAfter32Break, bool isAfter64Break);
    static void createBeamSegment(Beam* item, ChordRest* startChord, ChordRest* endChord, int level);
    static void createBeamletSegment(Beam* item, ChordRest* chord, bool isBefore, int level);

    static bool layout2Cross(Beam* item, const std::vector<ChordRest*>& chordRests, int frag);
};
}

#endif // MU_ENGRAVING_BEAMLAYOUT_H
