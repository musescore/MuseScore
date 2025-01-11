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
#pragma once

#include "layoutcontext.h"

namespace mu::engraving {
class LaissezVib;
class LaissezVibSegment;
class Slur;
class SlurSegment;
struct SlurTiePos;
class SlurTieSegment;
class SpannerSegment;
class System;
class Chord;
class TieSegment;
class Tie;
class TremoloTwoChord;
enum class Grip : signed char;
class Note;
class PartialTie;
class PartialTieSegment;
}

namespace muse::draw {
class Transform;
}

namespace mu::engraving::rendering::score {
class SlurTieLayout
{
public:
    static SpannerSegment* layoutSystem(Slur* item, System* system, LayoutContext& ctx);

    static TieSegment* layoutTieFor(Tie* item, System* system);
    static TieSegment* layoutTieBack(Tie* item, System* system, LayoutContext& ctx);
    static void resolveVerticalTieCollisions(const std::vector<TieSegment*>& stackedTies);

    static void computeUp(Slur* slur, LayoutContext& ctx);
    static void calculateDirection(Tie* item);

    static void computeBezier(TieSegment* tieSeg, PointF shoulderOffset = PointF());
    static void computeBezier(SlurSegment* slurSeg, PointF shoulderOffset = PointF());
    static double noteOpticalCenterForTie(const Note* note, bool up);
    static void createSlurSegments(Slur* item, LayoutContext& ctx);

    static void layoutLaissezVibChord(Chord* chord, LayoutContext& ctx);
private:

    static void slurPos(Slur* item, SlurTiePos* sp, LayoutContext& ctx);
    static void avoidPreBendsOnTab(const Chord* sc, const Chord* ec, SlurTiePos* sp);
    static void fixArticulations(Slur* item, PointF& pt, Chord* c, double up, bool stemSide);
    static void adjustEndPoints(SlurSegment* slurSeg);

    static void avoidCollisions(SlurSegment* slurSeg, PointF& pp1, PointF& p2, PointF& p3, PointF& p4,
                                muse::draw::Transform& toSystemCoordinates, double& slurAngle);
    static Shape getSegmentShapes(SlurSegment* slurSeg, ChordRest* startCR, ChordRest* endCR);
    static Shape getSegmentShape(SlurSegment* slurSeg, Segment* seg, ChordRest* startCR, ChordRest* endCR);
    static void addMinClearanceToShapes(Shape& segShapes, double spatium, bool slurUp, const ChordRest* startCR, const ChordRest* endCR);
    static double computeArcClearance(double spatium, double slurLength, double slurAngle);
    static void computeAdjustmentBalance(SlurSegment* slurSeg, const ChordRest* startCR, const ChordRest* endCR, double& leftBalance,
                                         double& rightBalance);
    static bool hasArticulationAbove(SlurSegment* slurSeg, const ChordRest* chordRest);
    static double computeAdjustmentStep(int upSign, double spatium, double slurLength);
    static bool stemSideForBeam(Slur* slur, bool start);
    static bool stemSideStartForBeam(Slur* slur) { return stemSideForBeam(slur, true); }
    static bool stemSideEndForBeam(Slur* slur) { return stemSideForBeam(slur, false); }
    static bool isOverBeams(Slur* slur);

    static void computeStartAndEndSystem(Tie* item, SlurTiePos& slurTiePos);
    static PointF computeDefaultStartOrEndPoint(const Tie* tie, Grip startOrEnd);
    static void correctForCrossStaff(Tie* tie, SlurTiePos& sPos, SpannerSegmentType type);
    static void forceHorizontal(Tie* tie, SlurTiePos& sPos);
    static void adjustX(TieSegment* tieSegment, SlurTiePos& sPos, Grip startOrEnd);
    static void adjustXforLedgerLines(TieSegment* tieSegment, bool start, Chord* chord, Note* note, const PointF& chordSystemPos,
                                      double padding, double& resultingX);
    static void adjustYforLedgerLines(TieSegment* tieSegment, SlurTiePos& sPos);
    static void adjustY(TieSegment* tieSegment);
    static bool hasEndPointAboveNote(TieSegment* tieSegment);

    static double defaultStemLengthStart(TremoloTwoChord* tremolo);
    static double defaultStemLengthEnd(TremoloTwoChord* tremolo);

    static bool isDirectionMixture(const Chord* c1, const Chord* c2, LayoutContext& ctx);

    static void layoutSegment(SlurSegment* item, LayoutContext& ctx, const PointF& p1, const PointF& p2);

    static void computeMidThickness(SlurTieSegment* slurTieSeg, double slurTieLengthInSp);
    static void fillShape(SlurTieSegment* slurTieSeg, double slurTieLengthInSp);
    static bool shouldHideSlurSegment(SlurSegment* item, LayoutContext& ctx);

    static void addLineAttachPoints(TieSegment* segment);
    static void addLineAttachPoints(PartialTieSegment* segment);

    static void calculateIsInside(Tie* item);

    static LaissezVibSegment* createLaissezVibSegment(LaissezVib* item);
    static void calculateLaissezVibX(LaissezVibSegment* segment, SlurTiePos& sPos, bool smufl);
    static void calculateLaissezVibY(LaissezVibSegment* segment, SlurTiePos& sPos);

    static PartialTieSegment* createPartialTieSegment(PartialTie* item);
    static PartialTieSegment* layoutPartialTie(PartialTie* item);

    static void setPartialTieEndPos(PartialTie* item, SlurTiePos& sPos);
};
}
