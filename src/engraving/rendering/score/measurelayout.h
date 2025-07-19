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

#include "../layoutoptions.h"
#include "layoutcontext.h"

namespace mu::engraving {
class Measure;
class MeasureBase;
class Parenthesis;
class Score;
class Segment;
class StaffLines;
enum class SegmentType;
enum class CourtesySegmentPosition : char;
}

namespace mu::engraving::rendering::score {
class MeasureLayout
{
public:
    struct MeasureStartEndPos {
        MeasureStartEndPos(double x1, double x2)
            : x1(x1), x2(x2) {}
        double x1;
        double x2;
    };

    MeasureLayout() = default;

    static void layout2(Measure* item, LayoutContext& ctx);

    static void getNextMeasure(LayoutContext& ctx);
    static void computePreSpacingItems(Measure* m, LayoutContext& ctx);

    static void layoutStaffLines(Measure* m, LayoutContext& ctx);
    static void layoutMeasureNumber(Measure* m, LayoutContext& ctx);
    static void layoutMMRestRange(Measure* m, LayoutContext& ctx);
    static void layoutMeasureElements(Measure* m, LayoutContext& ctx);

    static void createEndBarLines(Measure* m, bool isLastMeasureInSystem, LayoutContext& ctx);
    static Segment* addHeaderClef(Measure* m, bool isFirstClef, const Staff* staff, LayoutContext& ctx);
    static Segment* addHeaderKeySig(Measure* m, bool isFirstKeysig, const Staff* staff, LayoutContext& ctx);
    static void addSystemHeader(Measure* m, bool isFirstSystem, LayoutContext& ctx);
    static void removeSystemHeader(Measure* m);
    static void addSystemTrailer(Measure* m, Measure* nm, LayoutContext& ctx);
    static void removeSystemTrailer(Measure* m);

    static void setRepeatCourtesiesAndParens(Measure* m, LayoutContext& ctx);

    static void updateGraceNotes(Measure* measure, LayoutContext& ctx);

    static void createSystemBeginBarLine(Measure* m, LayoutContext& ctx);

    static void stretchMeasureInPracticeMode(Measure* m, double targetWidth, LayoutContext& ctx);

    static void layoutTimeTickAnchors(Measure* m, LayoutContext& ctx);

    static MeasureStartEndPos getMeasureStartEndPos(const Measure* measure, const Segment* firstCrSeg, const staff_idx_t staffIdx,
                                                    const bool needsHeaderException, const bool modernMMRest, const LayoutContext& ctx);

private:

    static void createMMRest(LayoutContext& ctx, Measure* firstMeasure, Measure* lastMeasure, const Fraction& len);

    static int adjustMeasureNo(MeasureBase* m, int measureNo);

    static void barLinesSetSpan(Segment* seg, LayoutContext& ctx);

    static void layoutPartialWidth(StaffLines* lines, LayoutContext& ctx, double w, double wPartial, bool alignLeft);

    static void moveToNextMeasure(LayoutContext& ctx);
    static void layoutMeasure(MeasureBase* currentMB, LayoutContext& ctx);
    static void checkStaffMoveValidity(Measure* measure, const LayoutContext& ctx);

    static void createMultiMeasureRestsIfNeed(MeasureBase* currentMB, LayoutContext& ctx);
    static void removeMMRestElements(Measure* mmRestMeasure);

    static void setClefSegVisibility(Measure* m, bool isLastMeasureInSystem, LayoutContext& ctx);

    static void setCourtesyTimeSig(Measure* m, const Fraction& refSigTick, const Fraction& courtesySigTick, const SegmentType segType,
                                   LayoutContext& ctx);
    static void setCourtesyKeySig(Measure* m, const Fraction& refSigTick, const Fraction& courtesySigTick, const SegmentType segType,
                                  LayoutContext& ctx);
    static void setCourtesyClef(Measure* m, const Fraction& refSigTick, const Fraction& courtesySigTick, const SegmentType segType,
                                LayoutContext& ctx);

    static void removeRepeatCourtesyParenthesesMeasure(Measure* m, const bool continuation, LayoutContext& ctx);
    static void removeRepeatCourtesyParenthesis(EngravingItem* item, const DirectionH direction = DirectionH::AUTO);
    static void addRepeatCourtesyParentheses(Measure* m, const bool continuation,  LayoutContext& ctx);
    static void addRepeatCourtesies(Measure* m, LayoutContext& ctx);
    static void removeRepeatCourtesies(Measure* m);
    static void addRepeatContinuationCourtesies(Measure* m, LayoutContext& ctx);
    static void removeRepeatContinuationCourtesies(Measure* m);
};
}
