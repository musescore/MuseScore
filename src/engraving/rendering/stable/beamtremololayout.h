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

#ifndef MU_ENGRAVING_BEAMTREMOLOLAYOUT_STABLE_H
#define MU_ENGRAVING_BEAMTREMOLOLAYOUT_STABLE_H

#include <vector>
#include "draw/types/geometry.h"

#include "dom/beam.h"
#include "dom/engravingitem.h"

#include "layoutcontext.h"

namespace mu::engraving {
class Chord;
class ChordRest;
class Beam;
class TremoloTwoChord;
enum class ActionIconType;
enum class SpannerSegmentType;
}

enum class SlopeConstraint
{
    NO_CONSTRAINT,
    FLAT,
    SMALL_SLOPE,
};

namespace mu::engraving::rendering::stable {
class BeamTremoloLayout
{
public:
    BeamTremoloLayout() {}

    static void setupLData(const BeamBase* item, BeamBase::LayoutData* ldata, const LayoutContext& ctx);

    static bool calculateAnchors(const BeamBase* item, BeamBase::LayoutData* ldata, LayoutContext& ctx,
                                 const std::vector<ChordRest*>& chordRests, const std::vector<int>& notes);

    static double chordBeamAnchorX(const BeamBase::LayoutData* ldata, const ChordRest* chord, ChordBeamAnchorType anchorType);
    static double chordBeamAnchorY(const BeamBase::LayoutData* ldata, const ChordRest* chord);
    static PointF chordBeamAnchor(const BeamBase::LayoutData* ldata, const ChordRest* chord, ChordBeamAnchorType anchorType);
    static int getMaxSlope(const BeamBase::LayoutData* ldata);
    static void extendStem(const BeamBase::LayoutData* ldata, Chord* chord, double addition);

private:

    static int getMiddleStaffLine(const BeamBase::LayoutData* ldata, const LayoutConfiguration& conf, ChordRest* startChord,
                                  ChordRest* endChord, int staffLines);
    static int computeDesiredSlant(const BeamBase* item, const BeamBase::LayoutData* ldata, int startNote, int endNote, int middleLine,
                                   int dictator, int pointer);
    static SlopeConstraint getSlopeConstraint(const BeamBase::LayoutData* ldata, int startNote, int endNote);
    static void offsetBeamWithAnchorShortening(const BeamBase::LayoutData* ldata, const LayoutConfiguration& conf,
                                               std::vector<ChordRest*> chordRests, int& dictator, int& pointer, int staffLines,
                                               bool isStartDictator, int stemLengthDictator);
    static bool isValidBeamPosition(const bool isUp, int yPos, bool isStart, bool isAscending, bool isFlat, int staffLines, bool isOuter);
    static bool isBeamInsideStaff(int yPos, int staffLines, bool allowFloater);
    static int getOuterBeamPosOffset(const BeamBase::LayoutData* ldata, int innerBeam, int beamCount, int staffLines);
    static void offsetBeamToRemoveCollisions(const BeamBase* item, const BeamBase::LayoutData* ldata, std::vector<ChordRest*> chordRests,
                                             int& dictator, int& pointer, const double startX, const double endX, bool isFlat,
                                             bool isStartDictator);
    static int getBeamCount(const BeamBase::LayoutData* ldata, std::vector<ChordRest*> chordRests);
    static bool is64thBeamPositionException(const int beamSpacing, int& yPos, int staffLines);
    static int findValidBeamOffset(const BeamBase::LayoutData* ldata, int outer, int beamCount, int staffLines, bool isStart,
                                   bool isAscending, bool isFlat);
    static void setValidBeamPositions(const BeamBase::LayoutData* ldata, int& dictator, int& pointer, int beamCountD, int beamCountP,
                                      int staffLines, bool isStartDictator, bool isFlat, bool isAscending);
    static void addMiddleLineSlant(const BeamBase::LayoutData* ldata, int& dictator, int& pointer, int beamCount, int middleLine,
                                   int interval, int desiredSlant);
    static void add8thSpaceSlant(BeamBase::LayoutData* ldata, mu::PointF& dictatorAnchor, int dictator, int pointer, int beamCount,
                                 int interval, int middleLine, bool Flat);
    static bool noSlope(const Beam* beam);
    static int strokeCount(const BeamBase::LayoutData* ldata, ChordRest* cr);
    static bool calculateAnchorsCross(BeamBase::LayoutData* ldata, const LayoutConfiguration& conf);
    static bool computeTremoloUp(const BeamBase::LayoutData* ldata);
};
} // namespace mu::engraving
#endif
