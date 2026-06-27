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
#include <algorithm>

#include "global/async/asyncable.h"

#include "engraving/automation/internal/scoreautomationcontroller.h"
#include "engraving/automation/iautomation.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/staff.h"
#include "engraving/types/fraction.h"
#include "engraving/types/types.h"
#include "global/containers.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

using InterpolationType = AutomationPoint::InterpolationType;

static const String AUTOMATION_DATA_DIR(u"automation/data/");

static constexpr double P_VALUE(0.429);
static constexpr double MP_VALUE(0.500);
static constexpr double F_VALUE(0.643);
static constexpr double FF_VALUE(0.714);

static void checkCurvesMatch(const AutomationCurve& actualCurve, const AutomationCurve& expectedCurve)
{
    EXPECT_EQ(actualCurve.size(), expectedCurve.size());

    for (const auto& [utick, expectedPoint] : expectedCurve) {
        ASSERT_TRUE(muse::contains(actualCurve, utick)) << "Missing point at utick " << utick;
        const AutomationPoint& actualPoint = actualCurve.at(utick);

        EXPECT_NEAR(actualPoint.inValue, expectedPoint.inValue, 0.0001) << "inValue mismatch at utick " << utick;
        EXPECT_NEAR(actualPoint.outValue, expectedPoint.outValue, 0.0001) << "outValue mismatch at utick " << utick;
        EXPECT_EQ(actualPoint.interpolation, expectedPoint.interpolation) << "interpolation mismatch at utick " << utick;
    }
}

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
    expectedCurve[480]  = AutomationPoint { 0.0,      P_VALUE,  InterpolationType::Linear };      // 2nd beat: p
    expectedCurve[1440] = AutomationPoint { P_VALUE,  MP_VALUE, InterpolationType::Linear };      // 4th beat: mp

    // 2nd measure
    expectedCurve[1920] = AutomationPoint { MP_VALUE, F_VALUE,  InterpolationType::Linear };      // 1st beat: sf
    expectedCurve[2400] = AutomationPoint { F_VALUE,  MP_VALUE, InterpolationType::Linear };      // 2nd beat: mp (sf recovery)
    expectedCurve[2880] = AutomationPoint { MP_VALUE, P_VALUE,  InterpolationType::Exponential }; // 3rd beat: pf start
    expectedCurve[3264] = AutomationPoint { P_VALUE,  F_VALUE,  InterpolationType::Linear };      // 4th beat: pf end

    // 3rd measure
    expectedCurve[4800] = AutomationPoint { F_VALUE,  P_VALUE,  InterpolationType::Linear };      // 3rd beat: p (hairpin start)

    // 4th measure
    expectedCurve[5760] = AutomationPoint { FF_VALUE, FF_VALUE, InterpolationType::Linear };      // 1st beat: ff (hairpin end)

    checkCurvesMatch(controller.automation()->curve(key), expectedCurve);

    // [THEN] Voice-1 curve on staff 0 matches expectations.
    // Voice-1 has a CURRENT_VOICE_ONLY f at tick 3840. The second pass then fills in all
    // shared-curve points so the voice curve is self-contained
    key.voiceIdx = 1;
    AutomationCurve expectedVoiceCurve;

    // Shared points copied in by the second pass
    expectedVoiceCurve[480]  = AutomationPoint { 0.0,      P_VALUE,  InterpolationType::Linear };
    expectedVoiceCurve[1440] = AutomationPoint { P_VALUE,  MP_VALUE, InterpolationType::Linear };
    expectedVoiceCurve[1920] = AutomationPoint { MP_VALUE, F_VALUE,  InterpolationType::Linear };
    expectedVoiceCurve[2400] = AutomationPoint { F_VALUE,  MP_VALUE, InterpolationType::Linear };
    expectedVoiceCurve[2880] = AutomationPoint { MP_VALUE, P_VALUE,  InterpolationType::Exponential };
    expectedVoiceCurve[3264] = AutomationPoint { P_VALUE,  F_VALUE,  InterpolationType::Linear };

    // CURRENT_VOICE_ONLY f; inValue comes from the shared active point at tick 3264 (outValue = F_VALUE)
    expectedVoiceCurve[3840] = AutomationPoint { F_VALUE,  F_VALUE,  InterpolationType::Linear };

    // Remaining shared points copied in by the second pass
    expectedVoiceCurve[4800] = AutomationPoint { F_VALUE,  P_VALUE,  InterpolationType::Linear };
    expectedVoiceCurve[5760] = AutomationPoint { FF_VALUE, FF_VALUE, InterpolationType::Linear };

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
        ASSERT_TRUE(muse::contains(curvesAfter, key));
        const AutomationCurve& curveAfter = curvesAfter.at(key);
        ASSERT_EQ(curveAfter.size(), curveBefore.size());

        for (const auto& [tick, point] : curveBefore) {
            const utick_t shiftedTick = tick + 1920;
            ASSERT_TRUE(muse::contains(curveAfter, shiftedTick)) << "Missing shifted point at utick " << shiftedTick;
            EXPECT_NEAR(curveAfter.at(shiftedTick).inValue,  point.inValue,  0.0001);
            EXPECT_NEAR(curveAfter.at(shiftedTick).outValue, point.outValue, 0.0001);
        }
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
        const size_t removedCount = std::count_if(curveBefore.begin(), curveBefore.end(),
                                                   [](const auto& kv) { return kv.first <= 1920; });

        ASSERT_TRUE(muse::contains(curvesAfter, key));
        EXPECT_EQ(curvesAfter.at(key).size(), curveBefore.size() - removedCount);

        for (const auto& [tick, point] : curveBefore) {
            if (tick > 1920) {
                EXPECT_TRUE(muse::contains(curvesAfter.at(key), tick - 1920))
                    << "Expected shifted point at utick " << (tick - 1920);
            }
        }
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
        ASSERT_TRUE(muse::contains(curvesAfter, key));
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
        ASSERT_TRUE(muse::contains(curvesAfter, key));
        checkCurvesMatch(curvesAfter.at(key), curveBefore);
    }
}
