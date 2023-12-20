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
#ifndef MU_ENGRAVING_SLURTIELAYOUT_STABLE_H
#define MU_ENGRAVING_SLURTIELAYOUT_STABLE_H

#include "layoutcontext.h"

namespace mu::engraving {
class Slur;
class SlurSegment;
struct SlurTiePos;
class SlurTieSegment;
class SpannerSegment;
class System;
class Chord;
class TieSegment;
class Tie;
class TremoloDispatcher;
enum class Grip;
class Note;
}

namespace mu::draw {
class Transform;
}

namespace mu::engraving::rendering::stable {
class SlurTieLayout
{
public:
    static void layout(Slur* item, LayoutContext& ctx);
    static SpannerSegment* layoutSystem(Slur* item, System* system, LayoutContext& ctx);

    static TieSegment* tieLayoutFor(Tie* item, System* system);
    static TieSegment* tieLayoutBack(Tie* item, System* system);
    static void resolveVerticalTieCollisions(const std::vector<TieSegment*>& stackedTies);

    static void computeUp(Slur* slur, LayoutContext& ctx);

    static void computeBezier(TieSegment* tieSeg, PointF shoulderOffset = PointF());
    static void computeBezier(SlurSegment* slurSeg, PointF shoulderOffset = PointF());

private:

    static void slurPos(Slur* item, SlurTiePos* sp, LayoutContext& ctx);
    static void avoidPreBendsOnTab(const Chord* sc, const Chord* ec, SlurTiePos* sp);
    static void fixArticulations(Slur* item, PointF& pt, Chord* c, double up, bool stemSide);
    static void adjustEndPoints(SlurSegment* slurSeg);
    static void avoidCollisions(SlurSegment* slurSeg, PointF& pp1, PointF& p2, PointF& p3, PointF& p4,
                                mu::draw::Transform& toSystemCoordinates, double& slurAngle);
    static Shape getSegmentShape(SlurSegment* slurSeg, Segment* seg, ChordRest* startCR, ChordRest* endCR);

    static void computeStartAndEndSystem(Tie* item, SlurTiePos& slurTiePos);
    static PointF computeDefaultStartOrEndPoint(const Tie* tie, Grip startOrEnd);
    static double noteOpticalCenterForTie(const Note* note, bool up);
    static void correctForCrossStaff(Tie* tie, SlurTiePos& sPos);
    static void forceHorizontal(Tie* tie, SlurTiePos& sPos);
    static void adjustX(TieSegment* tieSegment, SlurTiePos& sPos, Grip startOrEnd);
    static void adjustForLedgerLines(TieSegment* tieSegment, SlurTiePos& sPos);
    static void adjustY(TieSegment* tieSegment);
    static TieSegment* layoutTieWithNoEndNote(Tie* item);

    static double defaultStemLengthStart(TremoloDispatcher* tremolo);
    static double defaultStemLengthEnd(TremoloDispatcher* tremolo);

    static void layoutSegment(SlurSegment* item, LayoutContext& ctx, const PointF& p1, const PointF& p2);

    static void computeMidThickness(SlurTieSegment* slurTieSeg, double slurTieLengthInSp);
};
}

#endif // MU_ENGRAVING_SLURTIELAYOUT_STABLE_H
