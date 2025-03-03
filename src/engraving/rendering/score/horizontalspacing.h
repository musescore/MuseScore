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

#include "types/fraction.h"

namespace mu::engraving {
class Chord;
class EngravingItem;
class Fraction;
class Lyrics;
class Note;
class Rest;
class Shape;
class StemSlash;
class Segment;
struct Spring;
class Measure;
class System;
enum class ElementType : unsigned char;
enum class KerningType : unsigned char;
}

namespace mu::engraving::rendering::score {
class HorizontalSpacing
{
public:

    static double computeSpacingForFullSystem(System* system, double stretchReduction = 1.0, double squeezeFactor = 1.0,
                                              bool overrideMinMeasureWidth = false);
    static double updateSpacingForLastAddedMeasure(System* system);
    static void squeezeSystemToFit(System* system, double& curSysWidth, double targetSysWidth);
    static void justifySystem(System* system, double curSysWidth, double targetSystemWidth);

    static double minHorizontalDistance(const Shape& f, const Shape& s, double spatium, double squeezeFactor = 1.0);
    //! NOTE Temporary solution
    static double shapeSpatium(const Shape& s);

    static double minHorizontalDistance(const Segment* f, const Segment* ns, double squeezeFactor);
    static double minLeft(const Segment* seg, const Shape& ls);

    static double computePadding(const EngravingItem* item1, const EngravingItem* item2);
    static KerningType computeKerning(const EngravingItem* item1, const EngravingItem* item2);
    static double computeVerticalClearance(const EngravingItem* item1, const EngravingItem* item2, double spatium);

private:
    struct HorizontalSpacingContext {
        System* system = nullptr;
        double spatium = 0.0;
        double xCur = 0.0;
        double xLeftBarrier = 0.0;
        bool systemIsFull = false;
        Measure* startMeas = nullptr;
        double stretchReduction = 1.0;
        double squeezeFactor = 1.0;
        bool overrideMinMeasureWidth = false;
    };

    struct SegmentPosition {
        Segment* segment;
        double xPosInSystemCoords;
        SegmentPosition(Segment* s, double x)
            : segment(s), xPosInSystemCoords(x) {}
    };

    struct CrossBeamSpacing
    {
        bool upDown = false;
        bool downUp = false;
        bool canBeAdjusted = true;
        bool hasOpposingBeamlets = false;
        bool preventCrossStaffKerning = false;
        bool ensureMinStemDistance = false;
    };

    static void spaceMeasureGroup(const std::vector<Measure*>& measureGroup, HorizontalSpacingContext& ctx);
    static double getFirstSegmentXPos(Segment* segment, HorizontalSpacingContext& ctx);
    static std::vector<SegmentPosition> spaceSegments(const std::vector<Segment*>& segList, int startSegIdx, HorizontalSpacingContext& ctx);
    static bool ignoreSegmentForSpacing(const Segment* segment);
    static bool ignoreAllSegmentsForSpacing(const std::vector<SegmentPosition>& segmentPositions);
    static void spaceAgainstPreviousSegments(Segment* segment, std::vector<SegmentPosition>& prevSegPositions,
                                             HorizontalSpacingContext& ctx);
    static bool stopCheckingPreviousSegments(const SegmentPosition& prev, const SegmentPosition& curSegPos);
    static void checkLyricsAgainstLeftMargin(Segment* segment, double& x, HorizontalSpacingContext& ctx);
    static void checkLyricsAgainstRightMargin(std::vector<SegmentPosition>& segPositions);
    static double spaceLyricsAgainstBarlines(Segment* firstSeg, Segment* secondSeg, const HorizontalSpacingContext& ctx);
    static void checkLargeTimeSigAgainstRightMargin(std::vector<SegmentPosition>& segPositions);
    static void moveRightAlignedSegments(std::vector<SegmentPosition>& placedSegments, const HorizontalSpacingContext& ctx);
    static void checkCollisionsWithCrossStaffStems(const Segment* thisSeg, const Segment* nextSeg, staff_idx_t staffIdx,
                                                   double& curMinDist);

    static double chordRestSegmentNaturalWidth(Segment* segment, HorizontalSpacingContext& ctx);
    static double computeSegmentDurationStretch(const Segment* curSeg, const Segment* prevSeg);
    static double durationStretchForMMRests(const Segment* segment);
    static double durationStretchForTicks(double slope, const Fraction& ticks);
    static bool needsCueSizeSpacing(const Segment* segment);
    static bool needsHeaderSpacingExceptions(const Segment* seg, const Segment* nextSeg);

    static void applyCrossBeamSpacingCorrection(Segment* thisSeg, Segment* nextSeg, double& width);
    static CrossBeamSpacing computeCrossBeamSpacing(Segment* thisSeg, Segment* nextSeg);

    static void enforceMinimumMeasureWidths(const std::vector<Measure*> measureGroup);
    static double computeMinMeasureWidth(Measure* m);
    static void stretchMeasureToTargetWidth(Measure* m, double targetWidth);

    static void stretchSegmentsToWidth(std::vector<Spring>& springs, double width);

    static void setPositionsAndWidths(const std::vector<SegmentPosition>& segmentPositions);

    static bool isSpecialNotePaddingType(ElementType type);
    static void computeNotePadding(const Note* note, const EngravingItem* item2, double& padding, double scaling);
    static void computeLedgerRestPadding(const Rest* rest2, double& padding);
    static bool isSpecialLyricsPaddingType(ElementType type);
    static void computeLyricsPadding(const Lyrics* lyrics1, const EngravingItem* item2, double& padding);

    static bool isSameVoiceKerningLimited(const EngravingItem* item);
    static bool isNeverKernable(const EngravingItem* item);
    static bool isAlwaysKernable(const EngravingItem* item);
    static bool ignoreItems(const EngravingItem* item1, const EngravingItem* item2);

    static KerningType doComputeKerningType(const EngravingItem* item1, const EngravingItem* item2);
    static KerningType computeNoteKerningType(const Note* note, const EngravingItem* item2);
    static KerningType computeStemSlashKerningType(const StemSlash* stemSlash, const EngravingItem* item2);
    static KerningType computeLyricsKerningType(const Lyrics* lyrics1, const EngravingItem* item2);

    static void computeHangingLineWidth(const Segment* firstSeg, const Segment* nextSeg, double& width, bool systemHeaderGap,
                                        bool systemEnd);
};
} // namespace mu::engraving::layout
