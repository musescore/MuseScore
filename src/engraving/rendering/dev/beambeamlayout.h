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

#ifndef MU_ENGRAVING_BEAMBEAMLAYOUT_DEV_H
#define MU_ENGRAVING_BEAMBEAMLAYOUT_DEV_H

#include <vector>
#include "draw/types/geometry.h"

#include "dom/beam.h"
#include "dom/engravingitem.h"

namespace mu::engraving {
class Chord;
class ChordRest;
class Beam;
enum class ActionIconType;
enum class SpannerSegmentType;
}

namespace mu::engraving::rendering::dev {
class BeamBeamLayout
{
public:

    static void setupLData(Beam::LayoutData* info, EngravingItem* e);

    static bool calculateAnchors(Beam::LayoutData* info, const std::vector<ChordRest*>& chordRests, const std::vector<int>& notes);

    static double chordBeamAnchorX(const Beam::LayoutData* info, const ChordRest* chord, ChordBeamAnchorType anchorType);
    static double chordBeamAnchorY(const Beam::LayoutData* info, const ChordRest* chord);
    static PointF chordBeamAnchor(const Beam::LayoutData* info, const ChordRest* chord, ChordBeamAnchorType anchorType);
    static int getMaxSlope(const Beam::LayoutData* info);
    static void extendStem(const Beam::LayoutData* info, Chord* chord, double addition);

private:
    enum class SlopeConstraint
    {
        NO_CONSTRAINT,
        FLAT,
        SMALL_SLOPE,
    };

    static int getMiddleStaffLine(const Beam::LayoutData* info, ChordRest* startChord, ChordRest* endChord, int staffLines);
    static int computeDesiredSlant(const Beam::LayoutData* info, int startNote, int endNote, int middleLine, int dictator, int pointer);
    static SlopeConstraint getSlopeConstraint(const Beam::LayoutData* info, int startNote, int endNote);
    static void offsetBeamWithAnchorShortening(const Beam::LayoutData* info, std::vector<ChordRest*> chordRests, int& dictator,
                                               int& pointer, int staffLines, bool isStartDictator, int stemLengthDictator);
    static bool isValidBeamPosition(const bool isUp, int yPos, bool isStart, bool isAscending, bool isFlat, int staffLines, bool isOuter);
    static bool isBeamInsideStaff(int yPos, int staffLines, bool allowFloater);
    static int getOuterBeamPosOffset(const Beam::LayoutData* info, int innerBeam, int beamCount, int staffLines);
    static void offsetBeamToRemoveCollisions(const Beam::LayoutData* info, std::vector<ChordRest*> chordRests, int& dictator, int& pointer,
                                             const double startX, const double endX, bool isFlat, bool isStartDictator);
    static int getBeamCount(const Beam::LayoutData* info, std::vector<ChordRest*> chordRests);
    static bool is64thBeamPositionException(const int beamSpacing, int& yPos, int staffLines);
    static int findValidBeamOffset(const Beam::LayoutData* info, int outer, int beamCount, int staffLines, bool isStart, bool isAscending,
                                   bool isFlat);
    static void setValidBeamPositions(const Beam::LayoutData* info, int& dictator, int& pointer, int beamCountD, int beamCountP,
                                      int staffLines, bool isStartDictator, bool isFlat, bool isAscending);
    static void addMiddleLineSlant(const Beam::LayoutData* info, int& dictator, int& pointer, int beamCount, int middleLine, int interval,
                                   int desiredSlant);
    static void add8thSpaceSlant(Beam::LayoutData* info, mu::PointF& dictatorAnchor, int dictator, int pointer, int beamCount, int interval,
                                 int middleLine, bool Flat);
    static bool noSlope(const Beam* beam);
    static int strokeCount(const Beam::LayoutData* info, ChordRest* cr);
    static bool calculateAnchorsCross(Beam::LayoutData* info);
};
} // namespace mu::engraving
#endif
