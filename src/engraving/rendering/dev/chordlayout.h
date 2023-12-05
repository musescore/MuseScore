/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#ifndef MU_ENGRAVING_LAYOUTCHORDS_DEV_H
#define MU_ENGRAVING_LAYOUTCHORDS_DEV_H

#include <vector>

#include "layoutcontext.h"

#include "dom/chord.h"
#include "dom/rest.h"
#include "dom/measurerepeat.h"
#include "dom/mmrest.h"

namespace mu::engraving {
class MStyle;
class Measure;
class Note;
class Rest;
class Score;
class Segment;
class Staff;
class Slur;
}

namespace mu::engraving::rendering::dev {
class ChordLayout
{
public:

    static void layout(Chord* item, LayoutContext& ctx);

    static void layoutSpanners(Chord* item, LayoutContext& ctx);

    static void layoutArticulations(Chord* item, LayoutContext& ctx);
    static void layoutArticulations2(Chord* item, LayoutContext& ctx, bool layoutOnCrossBeamSide = false);
    static void layoutArticulations3(Chord* item, Slur* s, LayoutContext& ctx);

    static void layoutStem(Chord* item, LayoutContext& ctx);

    static void computeUp(Chord* item, LayoutContext& ctx);
    static void computeUp(ChordRest* item, LayoutContext& ctx);
    static int computeAutoStemDirection(const std::vector<int>& noteDistances);

    static void layoutChords1(LayoutContext& ctx, Segment* segment, staff_idx_t staffIdx);
    static double layoutChords2(std::vector<Note*>& notes, bool up, LayoutContext& ctx);
    static void layoutChords3(const MStyle& style, const std::vector<Chord*>&, const std::vector<Note*>&, const Staff*, LayoutContext& ctx);
    static void getNoteListForDots(Chord* c, std::vector<Note*>&, std::vector<Note*>&, std::vector<int>&);
    static void updateGraceNotes(Measure* measure, LayoutContext& ctx);
    static void repositionGraceNotesAfter(Segment* segment, size_t tracks);
    static void appendGraceNotes(Chord* chord);
    static void clearLineAttachPoints(Measure* measure);
    static void updateLineAttachPoints(Chord* chord, bool isFirstInMeasure, LayoutContext& ctx);
    static void resolveVerticalRestConflicts(LayoutContext& ctx, Segment* segment, staff_idx_t staffIdx);
    static void resolveRestVSChord(std::vector<Rest*>& rests, std::vector<Chord*>& chords, const Staff* staff, Segment* segment);
    static void resolveRestVSRest(std::vector<Rest*>& rests, const Staff* staff, Segment* segment, LayoutContext& ctx,
                                  bool considerBeams = false);
    static void layoutChordBaseFingering(Chord* chord, System* system, LayoutContext& ctx);
    static void layoutStretchedBends(Chord* chord, LayoutContext& ctx);

    static void crossMeasureSetup(Chord* chord, bool on, LayoutContext& ctx);

    static void checkStartEndSlurs(Chord* chord, LayoutContext& ctx);

    static void checkAndFillShape(const ChordRest* item, ChordRest::LayoutData* ldata, const LayoutConfiguration& conf);
    static void fillShape(const ChordRest* item, Chord::LayoutData* ldata, const LayoutConfiguration& conf);
    static void fillShape(const Chord* item, Chord::LayoutData* ldata, const LayoutConfiguration& conf);
    static void fillShape(const Rest* item, Rest::LayoutData* ldata, const LayoutConfiguration& conf);
    static void fillShape(const MeasureRepeat* item, MeasureRepeat::LayoutData* ldata, const LayoutConfiguration& conf);
    static void fillShape(const MMRest* item, MMRest::LayoutData* ldata, const LayoutConfiguration& conf);

private:
    static void layoutPitched(Chord* item, LayoutContext& ctx);
    static void layoutTablature(Chord* item, LayoutContext& ctx);

    static void layoutNote2(Note* note, LayoutContext& ctx);

    static void placeDots(const std::vector<Chord*>& chords, const std::vector<Note*>& notes);

    static void skipAccidentals(Segment* segment, track_idx_t startTrack, track_idx_t endTrack);

    static Shape chordRestShape(const ChordRest* item, const LayoutConfiguration& conf);
};
}

#endif // MU_ENGRAVING_LAYOUTCHORDS_DEV_H
