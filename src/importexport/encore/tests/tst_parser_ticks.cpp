/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

// Unit tests for the rhythm math (face value / real duration / dots / implied tuplets), page-setup plist
// parsing, and parser bounds helpers, exercised without loading a score.

#include <gtest/gtest.h>

#include "../internal/parser/ticks.h"
#include "../internal/parser/elem.h"
#include "../internal/parser/readers.h"
#include "../internal/importer/durations.h"

#include <vector>

using namespace mu::engraving;
using namespace mu::iex::enc;

TEST(Tst_EncoreRhythm, faceValueToTicks)
{
    EXPECT_EQ(faceValue2ticks(1), 960);
    EXPECT_EQ(faceValue2ticks(2), 480);
    EXPECT_EQ(faceValue2ticks(3), 240);
    EXPECT_EQ(faceValue2ticks(4), 120);
    EXPECT_EQ(faceValue2ticks(5), 60);
    EXPECT_EQ(faceValue2ticks(6), 30);
    EXPECT_EQ(faceValue2ticks(7), 15);
    EXPECT_EQ(faceValue2ticks(8), 7);
    EXPECT_EQ(faceValue2ticks(0), 0);
    EXPECT_EQ(faceValue2ticks(9), 0);
    EXPECT_EQ(faceValue2ticks(0xFF), 0);
    // Upper nibble must be ignored (fv & 0x0F).
    EXPECT_EQ(faceValue2ticks(0x14), 120);
    EXPECT_EQ(faceValue2ticks(0xF8), 7);
}

TEST(Tst_EncoreRhythm, faceValueToDurationType)
{
    EXPECT_EQ(faceValue2DurationType(1), DurationType::V_WHOLE);
    EXPECT_EQ(faceValue2DurationType(2), DurationType::V_HALF);
    EXPECT_EQ(faceValue2DurationType(3), DurationType::V_QUARTER);
    EXPECT_EQ(faceValue2DurationType(4), DurationType::V_EIGHTH);
    EXPECT_EQ(faceValue2DurationType(5), DurationType::V_16TH);
    EXPECT_EQ(faceValue2DurationType(6), DurationType::V_32ND);
    EXPECT_EQ(faceValue2DurationType(7), DurationType::V_64TH);
    EXPECT_EQ(faceValue2DurationType(8), DurationType::V_128TH);
    // Invalid face values fall back to V_QUARTER.
    EXPECT_EQ(faceValue2DurationType(0), DurationType::V_QUARTER);
    EXPECT_EQ(faceValue2DurationType(9), DurationType::V_QUARTER);
    // Upper nibble ignored.
    EXPECT_EQ(faceValue2DurationType(0x14), DurationType::V_EIGHTH);
}

TEST(Tst_EncoreRhythm, realDurationToDurationType)
{
    // Plain duration mappings: realDur == faceTicks, guard does not fire.
    EXPECT_EQ(realDuration2DurationType(960, 1), DurationType::V_WHOLE);
    EXPECT_EQ(realDuration2DurationType(480, 2), DurationType::V_HALF);
    EXPECT_EQ(realDuration2DurationType(240, 3), DurationType::V_QUARTER);
    EXPECT_EQ(realDuration2DurationType(120, 4), DurationType::V_EIGHTH);
    EXPECT_EQ(realDuration2DurationType(60, 5), DurationType::V_16TH);
    // Dotted mappings: realDur > faceTicks, guard does not fire.
    EXPECT_EQ(realDuration2DurationType(720, 2), DurationType::V_HALF);
    EXPECT_EQ(realDuration2DurationType(360, 3), DurationType::V_QUARTER);
    EXPECT_EQ(realDuration2DurationType(180, 4), DurationType::V_EIGHTH);
    // Multi-stream truncation: realDur < faceTicks means overlapping MIDI streams, so face value wins.
    EXPECT_EQ(realDuration2DurationType(120, 3), DurationType::V_QUARTER);
    EXPECT_EQ(realDuration2DurationType(240, 2), DurationType::V_HALF);
    EXPECT_EQ(realDuration2DurationType(480, 1), DurationType::V_WHOLE);
    // Triplet rdur falls back to face value; the 3:2 ratio is carried by the tuplet wrapper, not the type.
    EXPECT_EQ(realDuration2DurationType(160, 4), DurationType::V_EIGHTH);
    EXPECT_EQ(realDuration2DurationType(80, 4), DurationType::V_EIGHTH);
    EXPECT_EQ(realDuration2DurationType(80, 5), DurationType::V_16TH);
    EXPECT_EQ(realDuration2DurationType(40, 5), DurationType::V_16TH);
    // realDur <= 0 falls back to faceValue2DurationType.
    EXPECT_EQ(realDuration2DurationType(0, 5), DurationType::V_16TH);
    EXPECT_EQ(realDuration2DurationType(-1, 4), DurationType::V_EIGHTH);
    // Unknown realDur also falls back to faceValue2DurationType.
    EXPECT_EQ(realDuration2DurationType(99, 4), DurationType::V_EIGHTH);
    EXPECT_EQ(realDuration2DurationType(31000, 3), DurationType::V_QUARTER);
    // Inflated rdur (gap-to-next-event spacing) that is not a real dotted multiple of the face stays
    // the face value: a face=quarter with rdur=720 remains a quarter, not a dotted half.
    EXPECT_EQ(realDuration2DurationType(720, 3), DurationType::V_QUARTER);
    EXPECT_EQ(realDuration2DurationType(360, 4), DurationType::V_EIGHTH);
    EXPECT_EQ(realDuration2DurationType(180, 5), DurationType::V_16TH);
    // But rdur=360 on a face=quarter IS a real dotted quarter (calcDots>0), so the dotted mapping applies.
    EXPECT_EQ(realDuration2DurationType(360, 3), DurationType::V_QUARTER);
}

TEST(Tst_EncoreRhythm, dotCalculation)
{
    // Strict mode.
    EXPECT_EQ(calcDots(180, 4), 1);    // 120 * 3/2
    EXPECT_EQ(calcDots(210, 4), 2);    // 120 * 7/4
    EXPECT_EQ(calcDots(225, 4), 3);    // 120 * 15/8
    EXPECT_EQ(calcDots(120, 4), 0);    // == base
    EXPECT_EQ(calcDots(181, 4), 0);    // strict mode: no snap
    EXPECT_EQ(calcDots(0, 4), 0);      // dur <= 0
    EXPECT_EQ(calcDots(-1, 4), 0);
    EXPECT_EQ(calcDots(180, 0), 0);    // base <= 0
    EXPECT_EQ(calcDots(180, 9), 0);

    // Snap mode (±1 tick tolerance).
    EXPECT_EQ(calcDotsSnap(181, 4), 1);
    EXPECT_EQ(calcDotsSnap(179, 4), 1);
    EXPECT_EQ(calcDotsSnap(178, 4), 0);    // 2 ticks off → outside tolerance
    EXPECT_EQ(calcDotsSnap(211, 4), 2);
    EXPECT_EQ(calcDotsSnap(226, 4), 3);
    EXPECT_EQ(calcDotsSnap(121, 4), 0);    // base ± 1 → 0 dots
    EXPECT_EQ(calcDotsSnap(0, 4), 0);
    EXPECT_EQ(calcDotsSnap(180, 0), 0);
}

// The bit-0 dot fallback must fire only for genuine MIDI drift (rdur > faceTicks). When rdur <= faceTicks
// computeDotCount returns 0 even with the bit set; the true v0xC2 dotted-eighth dot is forced elsewhere
// (EncNote::forceDotted in emitters-note.cpp), not here.
TEST(Tst_EncoreRhythm, computeDotCount_v0c2_dotted_eighth)
{
    EXPECT_EQ(computeDotCount(0x60, 120, 4, /*useBit0Fallback=*/ true), 0)
        << "v0xC2 dotted-eighth without fix: dotControl=0x60 yields 0 dots";

    EXPECT_EQ(computeDotCount(0x61, 120, 4, /*useBit0Fallback=*/ true), 0)
        << "v0xC2 dotted-eighth: computeDotCount returns 0 when rdur==faceTicks; "
        "dot is forced by EncNote::forceDotted in emitters-note.cpp";

    EXPECT_EQ(computeDotCount(0x39, 60, 5, /*useBit0Fallback=*/ true), 0)
        << "Plain 16th with spurious dotControl bit0 must not be dotted (rdur==faceTicks)";
    EXPECT_EQ(computeDotCount(0x39, 60, 4, /*useBit0Fallback=*/ true), 0)
        << "rdur < faceTicks: bit-0 must not force dot even when set";

    EXPECT_EQ(computeDotCount(0x01, 160, 4, /*useBit0Fallback=*/ true), 1)
        << "Genuine drift (rdur=160 > faceTicks=120): bit0 fires → 1 dot";
}

TEST(Tst_EncoreRhythm, impliedTuplets)
{
    int normalNotes = 0;

    // Triplet (3:2): 120 * 2/3 = 80.
    EXPECT_EQ(detectImpliedTuplet(80, 4, normalNotes), 3);
    EXPECT_EQ(normalNotes, 2);

    // Quintuplet (5:4): 120 * 4/5 = 96.
    normalNotes = 0;
    EXPECT_EQ(detectImpliedTuplet(96, 4, normalNotes), 5);
    EXPECT_EQ(normalNotes, 4);

    // Exact match is not a tuplet.
    normalNotes = 7;
    EXPECT_EQ(detectImpliedTuplet(120, 4, normalNotes), 0);
    EXPECT_EQ(normalNotes, 0);

    // Invalid base resets normalNotes to 0.
    normalNotes = 7;
    EXPECT_EQ(detectImpliedTuplet(120, 0, normalNotes), 0);
    EXPECT_EQ(normalNotes, 0);

    // Non-positive realDur resets normalNotes to 0.
    normalNotes = 7;
    EXPECT_EQ(detectImpliedTuplet(0, 4, normalNotes), 0);
    EXPECT_EQ(normalNotes, 0);
}

// A dotted value that is fractional in ticks (integer division truncates base*n/d) must not be
// mistaken for a real dotted duration: e.g. a 16th's triple-dot = 60*15/8 = 112.5 truncates to 112,
// which used to be misread as a triple-dotted 16th.
TEST(Tst_EncoreRhythm, dotCalculation_noFalsePositiveForFractionalDottedValues)
{
    EXPECT_EQ(calcDots(112, 5), 0);
    EXPECT_EQ(calcDotsSnap(112, 5), 0);
    EXPECT_EQ(calcDotsSnap(111, 5), 0);
    EXPECT_EQ(calcDotsSnap(113, 5), 0);

    EXPECT_EQ(calcDots(52, 6), 0);
    EXPECT_EQ(calcDotsSnap(52, 6), 0);

    EXPECT_EQ(calcDots(56, 6), 0);
    EXPECT_EQ(calcDotsSnap(56, 6), 0);

    // Exact-integer dotted durations must still resolve.
    EXPECT_EQ(calcDots(90, 5), 1);
    EXPECT_EQ(calcDotsSnap(90, 5), 1);
    EXPECT_EQ(calcDots(105, 5), 2);
    EXPECT_EQ(calcDotsSnap(105, 5), 2);
    // 8th triple-dotted (225 = 120*15/8 exact).
    EXPECT_EQ(calcDots(225, 4), 3);
    EXPECT_EQ(calcDotsSnap(225, 4), 3);
    EXPECT_EQ(calcDotsSnap(226, 4), 3);
}

TEST(Tst_EncoreRhythm, dottedAdvance)
{
    EXPECT_EQ(dottedAdvance(DurationType::V_EIGHTH, 0), Fraction(1, 8));
    EXPECT_EQ(dottedAdvance(DurationType::V_EIGHTH, 1), Fraction(3, 16));
    EXPECT_EQ(dottedAdvance(DurationType::V_EIGHTH, 2), Fraction(7, 32));
    EXPECT_EQ(dottedAdvance(DurationType::V_EIGHTH, 3), Fraction(15, 64));
    // dots >= 3 clamps to 15/8 multiplier.
    EXPECT_EQ(dottedAdvance(DurationType::V_EIGHTH, 4), Fraction(15, 64));
    EXPECT_EQ(dottedAdvance(DurationType::V_EIGHTH, 99), Fraction(15, 64));
    // Quarter base.
    EXPECT_EQ(dottedAdvance(DurationType::V_QUARTER, 0), Fraction(1, 4));
    EXPECT_EQ(dottedAdvance(DurationType::V_QUARTER, 1), Fraction(3, 8));
}

// SCO5 (macOS Encore 5) stores the PREC page setup as an NSPrintInfo XML plist, not a Windows DEVMODE;
// parsePrecPlist extracts orientation, paper size and scale (no margins). See ENCORE_FORMAT.md §PREC block.
TEST(Tst_EncorePrecPlist, letter_portrait_scale_120)
{
    const QByteArray plist
        ="<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         "<plist version=\"1.0\"><dict>\n"
         "<key>com.apple.print.PageFormat.PMOrientation</key><integer>1</integer>\n"
         "<key>com.apple.print.PageFormat.PMScaling</key><real>1.2</real>\n"
         "<key>PMTiogaPaperName</key><string>na-letter</string>\n"
         "</dict></plist>\n";
    EncPrintSetup ps;
    EXPECT_TRUE(parsePrecPlist(plist, ps));
    EXPECT_TRUE(ps.hasData);
    EXPECT_EQ(ps.orientation, 1);     // portrait
    EXPECT_EQ(ps.paperSize, 1);       // Letter (dmPaper code)
    EXPECT_EQ(ps.scale, 120);         // 1.2 -> 120%
}

TEST(Tst_EncorePrecPlist, a4_landscape)
{
    const QByteArray plist
        ="<?xml version=\"1.0\"?><plist><dict>\n"
         "<key>com.apple.print.PageFormat.PMOrientation</key><integer>2</integer>\n"
         "<key>PMTiogaPaperName</key><string>iso-a4</string>\n"
         "</dict></plist>\n";
    EncPrintSetup ps;
    EXPECT_TRUE(parsePrecPlist(plist, ps));
    EXPECT_EQ(ps.orientation, 2);     // landscape
    EXPECT_EQ(ps.paperSize, 9);       // A4
}

TEST(Tst_EncorePrecPlist, rejects_non_plist)
{
    EncPrintSetup ps;
    EXPECT_FALSE(parsePrecPlist(QByteArray("not a plist at all"), ps));
    EXPECT_FALSE(ps.hasData);
}

TEST(Tst_EncoreParserBounds, clampMeasureEnd_clamps_oversized_varsize)
{
    // Normal case: end = measStart + varsize + elemBlockOffset.
    EXPECT_EQ(clampMeasureEnd(100, 50, 0x36, 10000), 100 + 50 + 0x36);
    // An oversized (attacker-controlled) varsize must clamp to the device size, never past EOF.
    EXPECT_EQ(clampMeasureEnd(100, 0xFFFFFFFFu, 0x36, 1000), 1000);
    // A varsize that fits stays unclamped even for a large device.
    EXPECT_EQ(clampMeasureEnd(0, 200, 0x36, 100000), 200 + 0x36);
}

TEST(Tst_EncoreParserDurations, computeElementDurations_gap_and_boundary)
{
    // Two notes 480 ticks apart in a 960-tick (4/4) measure: each spans the gap to the next event,
    // and the last spans the gap to the measure end.
    EncNote a(0, 9, 0);
    EncNote b(480, 9, 0);
    std::vector<EncMeasureElem*> elems { &a, &b };
    computeElementDurations(elems, 960, /*hasGraceTimeBorrowing*/ false);
    EXPECT_EQ(a.realDuration, 480);
    EXPECT_EQ(b.realDuration, 480);

    // A boundary tick (e.g. a mid-measure clef change) caps the preceding note's gap.
    EncNote c(0, 9, 0);
    EncNote d(480, 9, 0);
    std::vector<EncMeasureElem*> elems2 { &c, &d };
    computeElementDurations(elems2, 960, false, { 240 });
    EXPECT_EQ(c.realDuration, 240);
    EXPECT_EQ(d.realDuration, 480);
}
