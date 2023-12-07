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

#ifndef MU_ENGRAVING_TREMOLOBEAMLAYOUT_DEV_H
#define MU_ENGRAVING_TREMOLOBEAMLAYOUT_DEV_H

#include <vector>
#include "draw/types/geometry.h"

#include "dom/tremolo.h"
#include "dom/engravingitem.h"

namespace mu::engraving {
class Chord;
class ChordRest;
class Beam;
enum class ActionIconType;
enum class SpannerSegmentType;
}

namespace mu::engraving::rendering::dev {
class TremoloBeamLayout
{
public:

    static void setupLData(Tremolo::LayoutData* info, EngravingItem* e);

    static bool calculateAnchors(Tremolo::LayoutData* info, const std::vector<ChordRest*>& chordRests, const std::vector<int>& notes);

    static double chordBeamAnchorX(const Tremolo::LayoutData* info, const ChordRest* chord, ChordBeamAnchorType anchorType);
    static double chordBeamAnchorY(const Tremolo::LayoutData* info, const ChordRest* chord);
    static PointF chordBeamAnchor(const Tremolo::LayoutData* info, const ChordRest* chord, ChordBeamAnchorType anchorType);
    static int getMaxSlope(const Tremolo::LayoutData* info);
    static void extendStem(const Tremolo::LayoutData* info, Chord* chord, double addition);

private:
    enum class SlopeConstraint
    {
        NO_CONSTRAINT,
        FLAT,
        SMALL_SLOPE,
    };

    static int getMiddleStaffLine(const Tremolo::LayoutData* info, ChordRest* startChord, ChordRest* endChord, int staffLines);
    static int computeDesiredSlant(const Tremolo::LayoutData* info, int startNote, int endNote, int middleLine, int dictator, int pointer);
    static SlopeConstraint getSlopeConstraint(const Tremolo::LayoutData* info, int startNote, int endNote);
    static void offsetBeamWithAnchorShortening(const Tremolo::LayoutData* info, std::vector<ChordRest*> chordRests, int& dictator,
                                               int& pointer, int staffLines, bool isStartDictator, int stemLengthDictator);
    static bool isValidBeamPosition(const bool isUp, int yPos, bool isStart, bool isAscending, bool isFlat, int staffLines, bool isOuter);
    static bool isBeamInsideStaff(int yPos, int staffLines, bool allowFloater);
    static int getOuterBeamPosOffset(const Tremolo::LayoutData* info, int innerBeam, int beamCount, int staffLines);
    static void offsetBeamToRemoveCollisions(const Tremolo::LayoutData* info, std::vector<ChordRest*> chordRests, int& dictator,
                                             int& pointer, const double startX, const double endX, bool isFlat, bool isStartDictator);
    static int getBeamCount(const Tremolo::LayoutData* info, std::vector<ChordRest*> chordRests);
    static bool is64thBeamPositionException(const int beamSpacing, int& yPos, int staffLines);
    static int findValidBeamOffset(const Tremolo::LayoutData* info, int outer, int beamCount, int staffLines, bool isStart,
                                   bool isAscending, bool isFlat);
    static void setValidBeamPositions(const Tremolo::LayoutData* info, int& dictator, int& pointer, int beamCountD, int beamCountP,
                                      int staffLines, bool isStartDictator, bool isFlat, bool isAscending);
    static void addMiddleLineSlant(const Tremolo::LayoutData* info, int& dictator, int& pointer, int beamCount, int middleLine,
                                   int interval, int desiredSlant);
    static void add8thSpaceSlant(Tremolo::LayoutData* info, mu::PointF& dictatorAnchor, int dictator, int pointer, int beamCount,
                                 int interval, int middleLine, bool Flat);
    static bool noSlope(const Beam* beam);
    static int strokeCount(const Tremolo::LayoutData* info, ChordRest* cr);
    static bool calculateAnchorsCross(Tremolo::LayoutData* info);
    static bool computeTremoloUp(const Tremolo::LayoutData* info);
};
} // namespace mu::engraving
#endif
