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
#ifndef MU_ENGRAVING_MEASURELAYOUT_DEV_H
#define MU_ENGRAVING_MEASURELAYOUT_DEV_H

#include "../layoutoptions.h"
#include "layoutcontext.h"

namespace mu::engraving {
class Measure;
class MeasureBase;
class Score;
class Segment;
class StaffLines;
}

namespace mu::engraving::rendering::dev {
class MeasureLayout
{
public:
    MeasureLayout() = default;

    static void layout2(Measure* item, LayoutContext& ctx);

    static void getNextMeasure(LayoutContext& ctx);
    static void computePreSpacingItems(Measure* m, LayoutContext& ctx);

    static void layoutStaffLines(Measure* m, LayoutContext& ctx);
    static void layoutMeasureNumber(Measure* m, LayoutContext& ctx);
    static void layoutMMRestRange(Measure* m, LayoutContext& ctx);
    static void layoutMeasureElements(Measure* m, LayoutContext& ctx);
    static void layoutCrossStaff(MeasureBase* mb, LayoutContext& ctx);

    static double createEndBarLines(Measure* m, bool isLastMeasureInSystem, LayoutContext& ctx);
    static void addSystemHeader(Measure* m, bool isFirstSystem, LayoutContext& ctx);
    static void removeSystemHeader(Measure* m);
    static void addSystemTrailer(Measure* m, Measure* nm, LayoutContext& ctx);
    static void removeSystemTrailer(Measure* m, LayoutContext& ctx);

    static void createSystemBeginBarLine(Measure* m, LayoutContext& ctx);

    static void stretchMeasureInPracticeMode(Measure* m, double targetWidth, LayoutContext& ctx);

    static void computeWidth(Measure* m, LayoutContext& ctx, Fraction minTicks, Fraction maxTicks, double stretchCoeff,
                             bool overrideMinMeasureWidth = false);

private:

    static void createMMRest(LayoutContext& ctx, Measure* firstMeasure, Measure* lastMeasure, const Fraction& len);

    static int adjustMeasureNo(MeasureBase* m, int measureNo);

    static void barLinesSetSpan(Segment* seg, LayoutContext& ctx);

    static void computeWidth(Measure* m, LayoutContext& ctx, Segment* s, double x, bool isSystemHeader, Fraction minTicks,
                             Fraction maxTicks, double stretchCoeff, bool overrideMinMeasureWidth = false);

    static double computeMinMeasureWidth(Measure* m, LayoutContext& ctx);

    static void layoutPartialWidth(StaffLines* lines, LayoutContext& ctx, double w, double wPartial, bool alignLeft);

    //
    static void moveToNextMeasure(LayoutContext& ctx);
    static void layoutMeasure(MeasureBase* currentMB, LayoutContext& ctx);
    static void layoutMeasureIndependentElements(const Segment& segment, track_idx_t track, const LayoutContext& ctx);
    static void checkStaffMoveValidity(const Segment& segment, track_idx_t startTrack, track_idx_t endTrack);
    static void layoutChordDrumset(const Staff* staff, const Segment& segment, track_idx_t startTrack, track_idx_t endTrack,
                                   const LayoutConfiguration& conf);
    static void setChordsMag(const Staff* staff, const Segment& segment, track_idx_t startTrack, track_idx_t endTrack,
                             const LayoutConfiguration& conf);
    static void cmdUpdateNotes(const Measure* measure, const DomAccessor& dom);
    static void createStems(const Measure* measure, LayoutContext& ctx);
    static void createMultiMeasureRestsIfNeed(MeasureBase* currentMB, LayoutContext& ctx);
};
}

#endif // MU_ENGRAVING_MEASURELAYOUT_DEV_H
