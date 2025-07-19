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

namespace mu::engraving::rendering::score {
struct ChordPosInfo {
    std::vector<Chord*> chords;
    std::vector<Note*> upStemNotes;
    std::vector<Note*> downStemNotes;
    int upVoices       = 0;
    int downVoices     = 0;
    double maxUpWidth   = 0.0;
    double maxDownWidth = 0.0;
    double maxUpMag     = 0.0;
    double maxDownMag   = 0.0;

    // dots and hooks can affect layout of notes as well as vice versa
    int upDots         = 0;
    int downDots       = 0;
    bool upHooks       = false;
    bool downHooks     = false;

    // also check for grace notes
    bool upGrace       = false;
    bool downGrace     = false;
};

struct OffsetInfo {
    double upOffset           = 0.0;      // offset to apply to upstem chords
    double downOffset         = 0.0;      // offset to apply to downstem chords
    double dotAdjust          = 0.0;      // additional chord offset to account for dots
    double dotAdjustThreshold = 0.0;      // if it exceeds this amount

    // centering adjustments for whole note, breve, and small chords
    double centerUp          = 0.0;      // offset to apply in order to center upstem chords
    double oversizeUp        = 0.0;      // adjustment to oversized upstem chord needed if laid out to the right
    double centerDown        = 0.0;      // offset to apply in order to center downstem chords
    double centerAdjustUp    = 0.0;      // adjustment to upstem chord needed after centering donwstem chord
    double centerAdjustDown  = 0.0;      // adjustment to downstem chord needed after centering upstem chord

    std::set<track_idx_t> tracksToAdjust;
};

class ChordLayout
{
public:

    static void layout(Chord* item, LayoutContext& ctx);

    static void layoutSpanners(Chord* item, LayoutContext& ctx);

    static void layoutArticulations(Chord* item, LayoutContext& ctx);
    static void layoutArticulations2(Chord* item, LayoutContext& ctx, bool layoutOnCrossBeamSide = false);
    static void layoutArticulations3(Chord* item, Slur* s, LayoutContext& ctx);

    static void layoutStem(Chord* item, const LayoutContext& ctx);

    static void computeUp(const Chord* item, ChordRest::LayoutData* ldata, const LayoutContext& ctx);
    static void computeUp(ChordRest* item, const LayoutContext& ctx);
    static int computeAutoStemDirection(const std::vector<int>& noteDistances);
    static bool isChordPosBelowBeam(Chord* item, Beam* beam);
    static bool isChordPosBelowTrem(const Chord* item, TremoloTwoChord* trem);

    static void layoutChords1(LayoutContext& ctx, Segment* segment, staff_idx_t staffIdx);
    static double layoutChords2(std::vector<Note*>& notes, bool up, LayoutContext& ctx);
    static void layoutChords3(const std::vector<Chord*>&, const std::vector<Note*>&, const Staff*, LayoutContext& ctx);
    static void layoutLedgerLines(const std::vector<Chord*>& chords, LayoutContext& ctx);
    static void getNoteListForDots(Chord* c, std::vector<Note*>&, std::vector<Note*>&, std::vector<int>&);
    static void repositionGraceNotesAfter(Segment* segment, size_t tracks);
    static void appendGraceNotes(Chord* chord);
    static void clearLineAttachPoints(Measure* measure);
    static void updateLineAttachPoints(Chord* chord, bool isFirstInMeasure, LayoutContext& ctx);
    static void resolveVerticalRestConflicts(LayoutContext& ctx, Segment* segment, staff_idx_t staffIdx);
    static void resolveRestVSChord(std::vector<Rest*>& rests, std::vector<Chord*>& chords, const Staff* staff, Segment* segment);
    static void resolveRestVSRest(std::vector<Rest*>& rests, const Staff* staff, Segment* segment, LayoutContext& ctx,
                                  bool considerBeams = false);
    static void layoutChordBaseFingering(Chord* chord, System* system, LayoutContext& ctx);

    static void crossMeasureSetup(Chord* chord, bool on, LayoutContext& ctx);

    static void checkStartEndSlurs(Chord* chord, LayoutContext& ctx);

    static void checkAndFillShape(const ChordRest* item, ChordRest::LayoutData* ldata, const LayoutConfiguration& conf);
    static void fillShape(const ChordRest* item, Chord::LayoutData* ldata, const LayoutConfiguration& conf);
    static void fillShape(const Chord* item, Chord::LayoutData* ldata);
    static void fillShape(const Rest* item, Rest::LayoutData* ldata);
    static void fillShape(const MeasureRepeat* item, MeasureRepeat::LayoutData* ldata, const LayoutConfiguration& conf);
    static void fillShape(const MMRest* item, MMRest::LayoutData* ldata, const LayoutConfiguration& conf);

    static void addLineAttachPoints(Spanner* spanner);

    static bool chordHasDotsAllInvisible(Chord* chord);

    static double centerX(const Chord* chord);

private:
    static void layoutPitched(Chord* item, LayoutContext& ctx);
    static void layoutTablature(Chord* item, LayoutContext& ctx);

    static void layoutLvArticulation(Chord* item, LayoutContext& ctx);

    static void layoutNote2(Note* note, LayoutContext& ctx);

    static void placeDots(const std::vector<Chord*>& chords, const std::vector<Note*>& notes);

    static void setDotX(const std::vector<Chord*>& chords, const std::array<double, 3 * VOICES>& dotPos, const Staff* staff,
                        const double upDotPosX, const double downDotPosX);

    static void skipAccidentals(Segment* segment, track_idx_t startTrack, track_idx_t endTrack);

    static Shape chordRestShape(const ChordRest* item);

    static bool leaveSpaceForTie(const Articulation* item);

    static void computeUpBeamCase(Chord* item, Beam* beam);

    static void updateLedgerLines(Chord* item, LayoutContext& ctx);

    static ChordPosInfo calculateChordPosInfo(Segment* segment, staff_idx_t staffIdx, track_idx_t partStartTrack, track_idx_t partEndTrack,
                                              LayoutContext& ctx);
    static void calculateMaxNoteWidths(ChordPosInfo& posInfo, const Fraction& tick, const Staff* staff, LayoutContext& ctx);
    static OffsetInfo centreChords(const Segment* segment, ChordPosInfo& posInfo, staff_idx_t staffIdx, const Fraction& tick,
                                   LayoutContext& ctx);
    static void calculateChordOffsets(Segment* segment, staff_idx_t staffIdx, const Fraction& tick, OffsetInfo& offsetInfo,
                                      ChordPosInfo& posInfo, LayoutContext& ctx);
    static void applyChordOffsets(Segment* segment, staff_idx_t staffIdx, track_idx_t partStartTrack, track_idx_t partEndTrack,
                                  OffsetInfo& offsetInfo, const ChordPosInfo& posInfo, LayoutContext& ctx);
};
}

#endif // MU_ENGRAVING_LAYOUTCHORDS_DEV_H
