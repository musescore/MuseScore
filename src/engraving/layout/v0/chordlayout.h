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
#ifndef MU_ENGRAVING_LAYOUTCHORDS_H
#define MU_ENGRAVING_LAYOUTCHORDS_H

#include <vector>

#include "layoutcontext.h"

namespace mu::engraving {
class Chord;
class ChordRest;
class MStyle;
class Measure;
class Note;
class Rest;
class Score;
class Segment;
class Staff;
class Slur;
}

namespace mu::engraving::layout::v0 {
class ChordLayout
{
public:

    static void layout(Chord* item, LayoutContext& ctx);

    static void layoutSpanners(Chord* item, LayoutContext& ctx);
    static void layoutSpanners(Chord* item, System* system, const Fraction& stick, LayoutContext& ctx);

    static void layoutArticulations(Chord* item, LayoutContext& ctx);
    static void layoutArticulations2(Chord* item, LayoutContext& ctx, bool layoutOnCrossBeamSide = false);
    static void layoutArticulations3(Chord* item, Slur* s, LayoutContext& ctx);

    static void layoutStem(Chord* item, LayoutContext& ctx);
    static void layoutHook(Chord* item, LayoutContext& ctx);

    static void computeUp(Chord* item, LayoutContext& ctx);
    static void computeUp(ChordRest* item, LayoutContext& ctx);
    static int computeAutoStemDirection(const std::vector<int>& noteDistances);

    static void layoutChords1(Score* score, Segment* segment, staff_idx_t staffIdx, LayoutContext& ctx);
    static double layoutChords2(std::vector<Note*>& notes, bool up, LayoutContext& ctx);
    static void layoutChords3(const MStyle& style, const std::vector<Chord*>&, const std::vector<Note*>&, const Staff*, LayoutContext& ctx);
    static void updateGraceNotes(Measure* measure, LayoutContext& ctx);
    static void repositionGraceNotesAfter(Segment* segment);
    static void appendGraceNotes(Chord* chord);
    static void clearLineAttachPoints(Measure* measure);
    static void updateLineAttachPoints(Chord* chord, bool isFirstInMeasure, LayoutContext& ctx);
    static void resolveVerticalRestConflicts(Score* score, Segment* segment, staff_idx_t staffIdx, LayoutContext& ctx);
    static void resolveRestVSChord(std::vector<Rest*>& rests, std::vector<Chord*>& chords, Score* score, Segment* segment,
                                   staff_idx_t staffIdx);
    static void resolveRestVSRest(std::vector<Rest*>& rests, Score* score, Segment* segment, staff_idx_t staffIdx, LayoutContext& ctx,
                                  bool considerBeams = false);
    static void layoutChordBaseFingering(Chord* chord, System* system, LayoutContext& ctx);

private:
    static void layoutPitched(Chord* item, LayoutContext& ctx);
    static void layoutTablature(Chord* item, LayoutContext& ctx);
};
}

#endif // MU_ENGRAVING_LAYOUTCHORDS_H
