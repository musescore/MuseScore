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
#include "engraving/automation/iautomation.h"
#include "engraving/dom/masterscore.h"
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

    checkCurvesMatch(controller.automation()->curve(key), expectedCurve);

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

    checkCurvesMatch(controller.automation()->curve(key), expectedVoiceCurve);
}

TEST_F(ScoreAutomationController_Tests, InsertTime_Positive_ShiftsAllPoints)
{
    // [GIVEN] A score initialised with dynamics
    ScoreAutomationController controller;
    controller.init(s_score);

    const AutomationCurveMap curvesBefore = controller.automation()->curves();
    ASSERT_FALSE(curvesBefore.empty());

    // [WHEN] Insert one full measure (1920 ticks) at tick 0
    controller.insertTime(s_score, Fraction(0, 1), Fraction(4, 4));

    // [THEN] Same number of curves, all points shifted forward by 1920 ticks
    const AutomationCurveMap& curvesAfter = controller.automation()->curves();
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

    const AutomationCurveMap curvesBefore = controller.automation()->curves();
    ASSERT_FALSE(curvesBefore.empty());

    // [WHEN] Remove the first measure: -1920 ticks starting at tick 1920 -> erases points in [0, 1920]
    controller.insertTime(s_score, Fraction(4, 4), Fraction(-4, 4));

    // [THEN] Points in the removed range are gone; points beyond it shift back by 1920 ticks
    const AutomationCurveMap& curvesAfter = controller.automation()->curves();

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

TEST_F(ScoreAutomationController_Tests, Update_IsTextEditing_DoesNothing)
{
    // [GIVEN] A score initialised with dynamics
    ScoreAutomationController controller;
    controller.init(s_score);

    const AutomationCurveMap curvesBefore = controller.automation()->curves();

    bool notified = false;
    controller.automation()->changed().onReceive(this, [&notified](const AutomationChanges&) {
        notified = true;
    });

    // [WHEN] A change arrives while text-editing is active
    ScoreChanges changes;
    changes.isTextEditing = true;
    changes.changedTypes = { ElementType::DYNAMIC };
    controller.update(s_score, changes);

    // [THEN] No change notification was sent, and automation is unchanged
    EXPECT_FALSE(notified);

    const AutomationCurveMap& curvesAfter = controller.automation()->curves();
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

    const AutomationCurveMap curvesBefore = controller.automation()->curves();

    bool notified = false;
    controller.automation()->changed().onReceive(this, [&notified](const AutomationChanges&) {
        notified = true;
    });

    // [WHEN] A change arrives with no automation-relevant element types
    ScoreChanges changes;
    changes.changedTypes = { ElementType::NOTE };
    controller.update(s_score, changes);

    // [THEN] No change notification was sent, and automation is unchanged
    EXPECT_FALSE(notified);

    const AutomationCurveMap& curvesAfter = controller.automation()->curves();
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
    controller.automation()->editPoints(key, { { 5280, customPoint(0.0, MF_VALUE) } });

    // [WHEN] The score is re-processed (simulates any subsequent score change)
    ScoreChanges changes;
    changes.changedTypes = { ElementType::HAIRPIN };
    controller.update(s_score, changes);

    // [THEN] Measures 1-2 are unaffected by the hairpin-only update, the p dynamic at the hairpin
    //        start is unchanged, the user midpoint survives with its inValue set to prev.outValue
    //        (the curve holds flat at p before the breakpoint, then steps to MF_VALUE), and the ff
    //        dynamic's inValue is overridden to the midpoint's outValue (the curve holds flat at
    //        MF_VALUE until ff fires its own outValue)
    AutomationCurve expectedCurve;
    expectedCurve[480]  = generatedPoint(0.0,      P_VALUE);
    expectedCurve[1440] = generatedPoint(P_VALUE,  MP_VALUE);
    expectedCurve[1920] = generatedPoint(MP_VALUE, F_VALUE);
    expectedCurve[2400] = generatedPoint(F_VALUE,  MP_VALUE);
    expectedCurve[2880] = generatedPoint(MP_VALUE, P_VALUE);
    expectedCurve[3264] = generatedPoint(F_VALUE,  F_VALUE);
    expectedCurve[4800] = generatedPoint(F_VALUE,  P_VALUE);
    expectedCurve[5280] = customPoint(P_VALUE,     MF_VALUE);
    expectedCurve[5760] = generatedPoint(MF_VALUE, FF_VALUE);

    checkCurvesMatch(controller.automation()->curve(key), expectedCurve);
}
