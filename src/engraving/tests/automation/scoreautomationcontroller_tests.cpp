/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
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

#include <gtest/gtest.h>

#include "global/async/asyncable.h"

#include "engraving/automation/internal/scoreautomationcontroller.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/repeatlist.h"
#include "engraving/dom/staff.h"
#include "engraving/types/fraction.h"
#include "engraving/types/types.h"

#include "utils/scorerw.h"
#include "automation/utils/automationtestutils.h"

using namespace mu::engraving;

static const String AUTOMATION_DATA_DIR(u"automation/data/");

static constexpr double P_VALUE(0.425);
static constexpr double MP_VALUE(0.475);
static constexpr double MF_VALUE(0.525);
static constexpr double F_VALUE(0.575);
static constexpr double FF_VALUE(0.625);

class ScoreAutomationController_Tests : public ::testing::Test, public muse::async::Asyncable
{
public:
    static void SetUpTestSuite()
    {
        s_score = ScoreRW::readScore(AUTOMATION_DATA_DIR + u"dynamics.mscx");
        ASSERT_TRUE(s_score);
        ASSERT_FALSE(s_score->staves().empty());
    }

    static void TearDownTestSuite()
    {
        delete s_score;
        s_score = nullptr;
    }

protected:
    static MasterScore* s_score;
};

MasterScore* ScoreAutomationController_Tests::s_score = nullptr;

TEST_F(ScoreAutomationController_Tests, Init_Dynamics_CurveMatchesExpected)
{
    // [WHEN] Calculate the dynamics curve
    ScoreAutomationController controller;
    controller.init(s_score);

    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = s_score->staff(0)->id();

    // [THEN] Staff 0 all-voice curve matches expectations
    AutomationCurve expectedCurve;

    // 1st measure
    expectedCurve[480]  = generatedPoint(0.0,      P_VALUE);  // 2nd beat: p
    expectedCurve[1440] = generatedPoint(P_VALUE,  MP_VALUE); // 4th beat: mp

    // 2nd measure
    expectedCurve[1920] = generatedPoint(MP_VALUE, F_VALUE);  // 1st beat: sf
    expectedCurve[2400] = generatedPoint(F_VALUE,  MP_VALUE); // 2nd beat: mp (sf recovery)
    expectedCurve[2880] = generatedPoint(MP_VALUE, P_VALUE);  // 3rd beat: pf start
    expectedCurve[3264] = generatedPoint(F_VALUE,  F_VALUE);  // 4th beat: pf end

    // 3rd measure
    expectedCurve[4800] = generatedPoint(F_VALUE,  P_VALUE);  // 3rd beat: p (hairpin start)

    // 4th measure
    expectedCurve[5760] = generatedPoint(FF_VALUE, FF_VALUE); // 1st beat: ff (hairpin end)

    checkCurvesMatch(controller.automationData()->curve(key), expectedCurve);

    // [THEN] Voice-1 curve on staff 0 matches expectations.
    // Voice-1 has a CURRENT_VOICE_ONLY f at tick 3840. The second pass then fills in all
    // shared-curve points so the voice curve is self-contained
    key.voiceIdx = 1;
    AutomationCurve expectedVoiceCurve;

    // Shared points copied in by the second pass
    expectedVoiceCurve[480]  = generatedPoint(0.0,      P_VALUE);
    expectedVoiceCurve[1440] = generatedPoint(P_VALUE,  MP_VALUE);
    expectedVoiceCurve[1920] = generatedPoint(MP_VALUE, F_VALUE);
    expectedVoiceCurve[2400] = generatedPoint(F_VALUE,  MP_VALUE);
    expectedVoiceCurve[2880] = generatedPoint(MP_VALUE, P_VALUE);
    expectedVoiceCurve[3264] = generatedPoint(F_VALUE,  F_VALUE);

    // CURRENT_VOICE_ONLY f; inValue comes from the shared active point at tick 3264 (outValue = F_VALUE)
    expectedVoiceCurve[3840] = generatedPoint(F_VALUE,  F_VALUE);

    // Remaining shared points copied in by the second pass
    expectedVoiceCurve[4800] = generatedPoint(F_VALUE,  P_VALUE);
    expectedVoiceCurve[5760] = generatedPoint(FF_VALUE, FF_VALUE);

    checkCurvesMatch(controller.automationData()->curve(key), expectedVoiceCurve);
}

TEST_F(ScoreAutomationController_Tests, InsertTime_Positive_ShiftsAllPoints)
{
    // [GIVEN] A score initialised with dynamics
    ScoreAutomationController controller;
    controller.init(s_score);

    const AutomationCurveMap curvesBefore = controller.automationData()->curves();
    ASSERT_FALSE(curvesBefore.empty());

    // [WHEN] Insert one full measure (1920 ticks) at tick 0
    controller.insertTime(Fraction(0, 1), Fraction(4, 4));

    // [THEN] Same number of curves, all points shifted forward by 1920 ticks
    const AutomationCurveMap& curvesAfter = controller.automationData()->curves();
    EXPECT_EQ(curvesAfter.size(), curvesBefore.size());

    for (const auto& [key, curveBefore] : curvesBefore) {
        ASSERT_TRUE(curvesAfter.contains(key));

        AutomationCurve expectedCurve;
        for (const auto& [tick, point] : curveBefore) {
            expectedCurve[tick + 1920] = point;
        }

        checkCurvesMatch(curvesAfter.at(key), expectedCurve);
    }
}

TEST_F(ScoreAutomationController_Tests, InsertTime_Negative_RemovesMeasurePoints)
{
    // [GIVEN] A score initialised with dynamics
    ScoreAutomationController controller;
    controller.init(s_score);

    const AutomationCurveMap curvesBefore = controller.automationData()->curves();
    ASSERT_FALSE(curvesBefore.empty());

    // [WHEN] Remove the first measure: -1920 ticks starting at tick 1920 -> erases points in [0, 1920]
    controller.insertTime(Fraction(4, 4), Fraction(-4, 4));

    // [THEN] Points in the removed range are gone; points beyond it shift back by 1920 ticks
    const AutomationCurveMap& curvesAfter = controller.automationData()->curves();

    for (const auto& [key, curveBefore] : curvesBefore) {
        ASSERT_TRUE(curvesAfter.contains(key));

        AutomationCurve expectedCurve;
        for (const auto& [tick, point] : curveBefore) {
            if (tick > 1920) {
                expectedCurve[tick - 1920] = point;
            }
        }

        checkCurvesMatch(curvesAfter.at(key), expectedCurve);
    }
}

TEST_F(ScoreAutomationController_Tests, MoveTicks_ShiftsPointsAtAndAfterFrom)
{
    // [GIVEN] Three points on a single curve
    ScoreAutomationController controller;
    controller.init(s_score);

    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = muse::ID(1);

    AutomationPoint p1 = generatedPoint(0.3, 0.4);
    AutomationPoint p2 = generatedPoint(0.4, 0.6);
    AutomationPoint p3 = generatedPoint(0.6, 0.7);

    AutomationCurveMap curves;
    curves[key] = { { 100, p1 }, { 200, p2 }, { 300, p3 } };

    AutomationDataPtr data = std::make_shared<AutomationData>();
    data->setCurves(curves);
    controller.setAutomationData(data);

    // [WHEN] Move ticks starting at 200 by +100
    controller.insertTime(Fraction::fromTicks(200), Fraction::fromTicks(100));

    // [THEN] Point before 200 is unchanged; points at 200 and 300 shift to 300 and 400
    AutomationCurve expected;
    expected[100] = p1;
    expected[300] = p2;
    expected[400] = p3;
    checkCurvesMatch(controller.automationData()->curve(key), expected);
}

TEST_F(ScoreAutomationController_Tests, MoveTicks_AcrossMultipleCurves)
{
    // [GIVEN] Two curves with points on different staves
    ScoreAutomationController controller;
    controller.init(s_score);

    AutomationCurveKey key1;
    key1.type = AutomationType::Dynamics;
    key1.staffId = muse::ID(1);

    AutomationCurveKey key2;
    key2.type = AutomationType::Dynamics;
    key2.staffId = muse::ID(2);

    AutomationPoint p1 = generatedPoint(0.3, 0.5);
    AutomationPoint p2 = generatedPoint(0.4, 0.6);

    AutomationCurveMap curves;
    curves[key1] = { { 200, p1 } };
    curves[key2] = { { 200, p2 } };

    AutomationDataPtr data = std::make_shared<AutomationData>();
    data->setCurves(curves);
    controller.setAutomationData(data);

    // [WHEN] Ticks from 100 shifted by +500
    controller.insertTime(Fraction::fromTicks(100), Fraction::fromTicks(500));

    // [THEN] Both curves shift from 200 to 700
    AutomationCurve expected1;
    expected1[700] = p1;
    AutomationCurve expected2;
    expected2[700] = p2;
    checkCurvesMatch(controller.automationData()->curve(key1), expected1);
    checkCurvesMatch(controller.automationData()->curve(key2), expected2);
}

TEST_F(ScoreAutomationController_Tests, RemoveTicks_RemovesRangeAndClosesGap)
{
    // [GIVEN] Points at 100, 300, 500, 700
    ScoreAutomationController controller;
    controller.init(s_score);

    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = muse::ID(1);

    AutomationPoint p100 = generatedPoint(0.3, 0.4);
    AutomationPoint p300 = generatedPoint(0.4, 0.5);
    AutomationPoint p500 = generatedPoint(0.5, 0.6);
    AutomationPoint p700 = generatedPoint(0.6, 0.7);

    AutomationCurveMap curves;
    curves[key] = { { 100, p100 }, { 300, p300 }, { 500, p500 }, { 700, p700 } };

    AutomationDataPtr data = std::make_shared<AutomationData>();
    data->setCurves(curves);
    controller.setAutomationData(data);

    // [WHEN] Remove ticks [300, 500] (a 200-tick gap)
    controller.insertTime(Fraction::fromTicks(500), Fraction::fromTicks(-200));

    // [THEN] Points at 300 and 500 removed; 700 shifts to 500
    AutomationCurve expected;
    expected[100] = p100;
    expected[500] = p700;
    checkCurvesMatch(controller.automationData()->curve(key), expected);
}

TEST_F(ScoreAutomationController_Tests, RemoveTicks_CleansUpEmptyCurves)
{
    // [GIVEN] A curve with all its points inside the removed range
    ScoreAutomationController controller;
    controller.init(s_score);

    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = muse::ID(1);

    AutomationCurveMap curves;
    curves[key] = { { 200, generatedPoint(0.4, 0.5) }, { 400, generatedPoint(0.5, 0.6) } };

    AutomationDataPtr data = std::make_shared<AutomationData>();
    data->setCurves(curves);
    controller.setAutomationData(data);

    // [WHEN] Remove ticks in range [100, 500]
    controller.insertTime(Fraction::fromTicks(500), Fraction::fromTicks(-400));

    // [THEN] Curve entry is removed from the map
    EXPECT_TRUE(controller.automationData()->isEmpty());
}

TEST_F(ScoreAutomationController_Tests, Update_IsTextEditing_DoesNothing)
{
    // [GIVEN] A score initialised with dynamics
    ScoreAutomationController controller;
    controller.init(s_score);

    const AutomationCurveMap curvesBefore = controller.automationData()->curves();

    bool notified = false;
    controller.automationData()->changed().onReceive(this, [&notified](const AutomationChanges&) {
        notified = true;
    });

    // [WHEN] A change arrives while text-editing is active
    ScoreChanges changes;
    changes.isTextEditing = true;
    changes.changedTypes = { ElementType::DYNAMIC };
    controller.update(changes);

    // [THEN] No change notification was sent, and automation is unchanged
    EXPECT_FALSE(notified);

    const AutomationCurveMap& curvesAfter = controller.automationData()->curves();
    ASSERT_EQ(curvesAfter.size(), curvesBefore.size());

    for (const auto& [key, curveBefore] : curvesBefore) {
        ASSERT_TRUE(curvesAfter.contains(key));
        checkCurvesMatch(curvesAfter.at(key), curveBefore);
    }
}

TEST_F(ScoreAutomationController_Tests, Update_NoRelevantTypes_DoesNothing)
{
    // [GIVEN] A score initialised with dynamics
    ScoreAutomationController controller;
    controller.init(s_score);

    const AutomationCurveMap curvesBefore = controller.automationData()->curves();

    bool notified = false;
    controller.automationData()->changed().onReceive(this, [&notified](const AutomationChanges&) {
        notified = true;
    });

    // [WHEN] A change arrives with no automation-relevant element types
    ScoreChanges changes;
    changes.changedTypes = { ElementType::NOTE };
    controller.update(changes);

    // [THEN] No change notification was sent, and automation is unchanged
    EXPECT_FALSE(notified);

    const AutomationCurveMap& curvesAfter = controller.automationData()->curves();
    ASSERT_EQ(curvesAfter.size(), curvesBefore.size());

    for (const auto& [key, curveBefore] : curvesBefore) {
        ASSERT_TRUE(curvesAfter.contains(key));
        checkCurvesMatch(curvesAfter.at(key), curveBefore);
    }
}

TEST_F(ScoreAutomationController_Tests, UserMidpoint_InsideHairpin_CorrectInValues)
{
    // [GIVEN] The score has a crescendo hairpin in measures 3–4: p at tick 4800, ff at tick 5760.
    ScoreAutomationController controller;
    controller.init(s_score);

    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = s_score->staff(0)->id();

    // [WHEN] The user inserts a custom automation point at the midpoint of the hairpin
    AutomationPointEdits edits { { 5280, customPoint(0.0, MF_VALUE) } };
    controller.editPoints(key, edits);

    // [WHEN] The score is re-processed (simulates any subsequent score change)
    ScoreChanges changes;
    changes.changedTypes = { ElementType::HAIRPIN };
    controller.update(changes);

    // [THEN] Measures 1-2 are unaffected by the hairpin-only update, the p dynamic at the hairpin
    //        start is unchanged, the user midpoint's own data is untouched by the rebuild, and the
    //        ff dynamic keeps its own independently-computed arrival value regardless of the midpoint
    AutomationCurve expectedCurve;
    expectedCurve[480]  = generatedPoint(0.0,      P_VALUE);
    expectedCurve[1440] = generatedPoint(P_VALUE,  MP_VALUE);
    expectedCurve[1920] = generatedPoint(MP_VALUE, F_VALUE);
    expectedCurve[2400] = generatedPoint(F_VALUE,  MP_VALUE);
    expectedCurve[2880] = generatedPoint(MP_VALUE, P_VALUE);
    expectedCurve[3264] = generatedPoint(F_VALUE,  F_VALUE);
    expectedCurve[4800] = generatedPoint(F_VALUE,  P_VALUE);
    expectedCurve[5280] = customPoint(0.0,         MF_VALUE);
    expectedCurve[5760] = generatedPoint(FF_VALUE, FF_VALUE);

    checkCurvesMatch(controller.automationData()->curve(key), expectedCurve);
}

TEST_F(ScoreAutomationController_Tests, MirrorEdit_OtherRepeatSegment_CopiesPoint)
{
    // [GIVEN] Measure 5 (tick 7680-9600) is played twice via a real repeat barline: once as part
    //         of the 1st RepeatSegment (utick offset 0), once as part of the 2nd (utick offset 1920)
    const RepeatList& repeatList = s_score->repeatList();
    ASSERT_EQ(repeatList.size(), 2);
    const int secondPassOffset = repeatList.at(1)->utick - repeatList.at(1)->tick;
    ASSERT_EQ(secondPassOffset, 1920);

    ScoreAutomationController controller;
    controller.init(s_score);

    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = s_score->staff(0)->id();

    // [WHEN] The user adds a custom point during the 1st pass through measure 5
    const AutomationPoint edited = customPoint(0.4, 0.4);
    AutomationPointEdits edits { { 7700, edited } };
    controller.editPoints(key, edits);

    // [THEN] The same point is mirrored into the 2nd pass through the same measure, marked generated
    //        since it is a copy, not something the user edited directly at that tick
    AutomationPoint expectedMirrored = edited;
    expectedMirrored.generated = true;

    const AutomationCurve& curve = controller.automationData()->curve(key);
    const auto mirroredIt = curve.find(7700 + secondPassOffset);
    ASSERT_TRUE(mirroredIt != curve.cend());
    EXPECT_EQ(mirroredIt->second, expectedMirrored);
}

TEST_F(ScoreAutomationController_Tests, MirrorEdit_MeasureRepeat_CopiesPoint)
{
    // [GIVEN] Measure 7 is a 1-measure repeat of measure 6 (tick 9600-11520); both only ever play
    //         once, during the 2nd RepeatSegment (utick offset 1920)
    const RepeatList& repeatList = s_score->repeatList();
    ASSERT_EQ(repeatList.size(), 2);
    const int passOffset = repeatList.at(1)->utick - repeatList.at(1)->tick;
    ASSERT_EQ(passOffset, 1920);

    ScoreAutomationController controller;
    controller.init(s_score);

    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = s_score->staff(0)->id();

    // [WHEN] The user adds a custom point inside measure 6, the measure-repeat's source measure
    const AutomationPoint edited = customPoint(0.4, 0.4);
    const utick_t editUTick = 9700 + passOffset;
    AutomationPointEdits edits { { editUTick, edited } };
    controller.editPoints(key, edits);

    // [THEN] The same point is mirrored into measure 7, the measure-repeat's occurrence
    //        (measure 7 immediately follows measure 6, so tickShift is also 1 measure: 1920 ticks),
    //        marked generated since it is a copy, not something the user edited directly at that tick
    AutomationPoint expectedMirrored = edited;
    expectedMirrored.generated = true;

    const AutomationCurve& curve = controller.automationData()->curve(key);
    const auto mirroredIt = curve.find(editUTick + 1920);
    ASSERT_TRUE(mirroredIt != curve.cend());
    EXPECT_EQ(mirroredIt->second, expectedMirrored);
}
