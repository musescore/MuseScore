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
#include "engraving/automation/tempovalues.h"
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

// Normalized tempo values for tempo.mscx, whose markings use round bps values (see tempo.mscx: 180bpm
// quarter = 3.0 bps, halved by the measure-2 ritardando to 90bpm quarter = 1.5 bps)
static const double TEMPO_3(normalizeTempo(BeatsPerSecond(3.0)));
static const double TEMPO_1_5(normalizeTempo(BeatsPerSecond(1.5)));
static const double TEMPO_2(normalizeTempo(BeatsPerSecond(2.0)));

static constexpr double TEMPO_ERROR(0.000001);
static constexpr double TIME_ERROR(0.000001);

// Restores a score's undo stack to its state at construction, even if an ASSERT_* exits the test early
class UndoRestoreGuard
{
public:
    explicit UndoRestoreGuard(MasterScore* score)
        : m_score(score) {}

    ~UndoRestoreGuard() { m_score->undoRedo(true, nullptr); }

private:
    MasterScore* m_score = nullptr;
};

class ScoreAutomationController_Tests : public ::testing::Test, public muse::async::Asyncable
{
public:
    static void SetUpTestSuite()
    {
        s_dynamicsScore = ScoreRW::readScore(AUTOMATION_DATA_DIR + u"dynamics.mscx");
        ASSERT_TRUE(s_dynamicsScore);
        ASSERT_FALSE(s_dynamicsScore->staves().empty());

        s_tempoScore = ScoreRW::readScore(AUTOMATION_DATA_DIR + u"tempo.mscx");
        ASSERT_TRUE(s_tempoScore);
        ASSERT_FALSE(s_tempoScore->staves().empty());
    }

    static void TearDownTestSuite()
    {
        delete s_dynamicsScore;
        s_dynamicsScore = nullptr;

        delete s_tempoScore;
        s_tempoScore = nullptr;
    }

protected:
    static MasterScore* s_dynamicsScore;
    static MasterScore* s_tempoScore;
};

MasterScore* ScoreAutomationController_Tests::s_dynamicsScore = nullptr;
MasterScore* ScoreAutomationController_Tests::s_tempoScore = nullptr;

TEST_F(ScoreAutomationController_Tests, Init_Dynamics_CurveMatchesExpected)
{
    // [WHEN] Calculate the dynamics curve
    ScoreAutomationController controller;
    controller.init(s_dynamicsScore);

    AutomationCurveKey key = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id());

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
    key = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id(), size_t(1));
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
    // [GIVEN] Three user-authored points on the real Dynamics curve
    ScoreAutomationController controller;
    controller.init(s_dynamicsScore);

    // Real baseline, unaffected by insertTime() since s_dynamicsScore's measures never move
    AutomationCurveKey key = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id());
    AutomationCurveKey voiceKey = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id(), size_t(1));
    AutomationCurve expected = controller.automationData()->curve(key);

    AutomationPoint p1 = customPoint(0.3, 0.4);
    AutomationPoint p2 = customPoint(0.4, 0.5);
    AutomationPoint p3 = customPoint(0.5, 0.6);

    AutomationPointEdits edits {
        { 100, AutomationPointEdit::SetPoint { p1 } },
        { 200, AutomationPointEdit::SetPoint { p2 } },
        { 300, AutomationPointEdit::SetPoint { p3 } },
    };
    controller.editPoints(key, edits);

    AutomationChanges lastChanges;
    controller.automationData()->changed().onReceive(this, [&lastChanges](const AutomationChanges& changes) {
        lastChanges = changes;
    });

    // [WHEN] Insert one full measure (1920 ticks) at tick 0
    controller.insertTime(Fraction(0, 1), Fraction(4, 4), s_dynamicsScore->repeatList().segmentInfoList());

    // [THEN] Points shift by 1920 ticks; the real baseline is unaffected
    expected[100 + 1920] = p1;
    expected[200 + 1920] = p2;
    expected[300 + 1920] = p3;
    checkCurvesMatch(controller.automationData()->curve(key), expected);

    // [THEN] The notification also covers voice-1, which mirrors the shift via fillVoiceCurvesFromBase()
    EXPECT_EQ(lastChanges.affectedKeys, (std::set<AutomationCurveKey> { key, voiceKey }));
    EXPECT_EQ(lastChanges.tickFrom, 100);
    EXPECT_EQ(lastChanges.tickTo, 2220);
    EXPECT_FALSE(lastChanges.isFullReset);
}

TEST_F(ScoreAutomationController_Tests, InsertTime_Negative_RemovesMeasurePoints)
{
    // [GIVEN] One user-authored point inside the range about to be removed, one after it
    ScoreAutomationController controller;
    controller.init(s_dynamicsScore);

    // Real baseline, unaffected by insertTime() since s_dynamicsScore's measures never move
    AutomationCurveKey key = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id());
    AutomationCurveKey voiceKey = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id(), size_t(1));
    AutomationCurve expected = controller.automationData()->curve(key);

    AutomationPoint pInside = customPoint(0.4, 0.5);
    AutomationPoint pAfter = customPoint(0.5, 0.6);

    AutomationPointEdits edits {
        { 1000, AutomationPointEdit::SetPoint { pInside } },
        { 2500, AutomationPointEdit::SetPoint { pAfter } },
    };
    controller.editPoints(key, edits);

    AutomationChanges lastChanges;
    controller.automationData()->changed().onReceive(this, [&lastChanges](const AutomationChanges& changes) {
        lastChanges = changes;
    });

    // [WHEN] Remove the first measure: -1920 ticks starting at tick 0 -> erases points in [0, 1920)
    controller.insertTime(Fraction(0, 1), Fraction(-4, 4), s_dynamicsScore->repeatList().segmentInfoList());

    // [THEN] pInside is gone; pAfter shifts back by 1920 ticks (2500 -> 580)
    expected[2500 - 1920] = pAfter;
    checkCurvesMatch(controller.automationData()->curve(key), expected);

    // [THEN] The notification also covers voice-1, which mirrors pAfter's shift via fillVoiceCurvesFromBase()
    EXPECT_EQ(lastChanges.affectedKeys, (std::set<AutomationCurveKey> { key, voiceKey }));
    EXPECT_EQ(lastChanges.tickFrom, 580);
    EXPECT_EQ(lastChanges.tickTo, 2500);
    EXPECT_FALSE(lastChanges.isFullReset);
}

TEST_F(ScoreAutomationController_Tests, MoveTicks_ShiftsPointsAtAndAfterFrom)
{
    // [GIVEN] Three user-authored points on the real Dynamics curve
    ScoreAutomationController controller;
    controller.init(s_dynamicsScore);

    // Real baseline, unaffected by insertTime() since s_dynamicsScore's measures never move
    AutomationCurveKey key = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id());
    AutomationCurveKey voiceKey = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id(), size_t(1));
    AutomationCurve expected = controller.automationData()->curve(key);

    AutomationPoint p1 = customPoint(0.3, 0.4);
    AutomationPoint p2 = customPoint(0.4, 0.6);
    AutomationPoint p3 = customPoint(0.6, 0.7);

    AutomationPointEdits edits {
        { 100, AutomationPointEdit::SetPoint { p1 } },
        { 200, AutomationPointEdit::SetPoint { p2 } },
        { 300, AutomationPointEdit::SetPoint { p3 } },
    };
    controller.editPoints(key, edits);

    AutomationChanges lastChanges;
    controller.automationData()->changed().onReceive(this, [&lastChanges](const AutomationChanges& changes) {
        lastChanges = changes;
    });

    // [WHEN] Move ticks starting at 200 by +100
    controller.insertTime(Fraction::fromTicks(200), Fraction::fromTicks(100), s_dynamicsScore->repeatList().segmentInfoList());

    // [THEN] p1 (before 200) is unchanged; p2/p3 shift to 300/400
    expected[100] = p1;
    expected[300] = p2;
    expected[400] = p3;
    checkCurvesMatch(controller.automationData()->curve(key), expected);

    // [THEN] The notification also covers voice-1, which mirrors p2/p3's shift via fillVoiceCurvesFromBase()
    EXPECT_EQ(lastChanges.affectedKeys, (std::set<AutomationCurveKey> { key, voiceKey }));
    EXPECT_EQ(lastChanges.tickFrom, 100);
    EXPECT_EQ(lastChanges.tickTo, 400);
    EXPECT_FALSE(lastChanges.isFullReset);
}

TEST_F(ScoreAutomationController_Tests, MoveTicks_AcrossMultipleCurves)
{
    // [GIVEN] Two real curves on staff 0: the shared (all-voice) curve, and the voice-1 curve
    //         (which already carries its own baseline - see Init_Dynamics_CurveMatchesExpected)
    ScoreAutomationController controller;
    controller.init(s_dynamicsScore);

    AutomationCurveKey key1 = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id());
    AutomationCurveKey key2 = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id(), size_t(1));

    // Real baselines, unaffected by insertTime() since s_dynamicsScore's measures never move
    AutomationCurve expected1 = controller.automationData()->curve(key1);
    AutomationCurve expected2 = controller.automationData()->curve(key2);

    AutomationPoint p1 = customPoint(0.3, 0.5);
    AutomationPoint p2 = customPoint(0.4, 0.6);

    AutomationPointEdits edits1 { { 200, AutomationPointEdit::SetPoint { p1 } } };
    controller.editPoints(key1, edits1);

    AutomationPointEdits edits2 { { 200, AutomationPointEdit::SetPoint { p2 } } };
    controller.editPoints(key2, edits2);

    AutomationChanges lastChanges;
    controller.automationData()->changed().onReceive(this, [&lastChanges](const AutomationChanges& changes) {
        lastChanges = changes;
    });

    // [WHEN] Ticks from 100 shifted by +500
    controller.insertTime(Fraction::fromTicks(100), Fraction::fromTicks(500), s_dynamicsScore->repeatList().segmentInfoList());

    // [THEN] Both points shift from 200 to 700
    expected1[700] = p1;
    expected2[700] = p2;
    checkCurvesMatch(controller.automationData()->curve(key1), expected1);
    checkCurvesMatch(controller.automationData()->curve(key2), expected2);

    // [THEN] The notification reports exactly the two curves that actually shifted
    EXPECT_EQ(lastChanges.affectedKeys, (std::set<AutomationCurveKey> { key1, key2 }));
    EXPECT_EQ(lastChanges.tickFrom, 200);
    EXPECT_EQ(lastChanges.tickTo, 700);
    EXPECT_FALSE(lastChanges.isFullReset);
}

TEST_F(ScoreAutomationController_Tests, RemoveTicks_RemovesRangeAndClosesGap)
{
    // [GIVEN] Four user-authored points on the real Dynamics curve
    ScoreAutomationController controller;
    controller.init(s_dynamicsScore);

    // Real baseline, unaffected by insertTime() since s_dynamicsScore's measures never move
    AutomationCurveKey key = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id());
    AutomationCurveKey voiceKey = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id(), size_t(1));
    AutomationCurve expected = controller.automationData()->curve(key);

    AutomationPoint p100 = customPoint(0.3, 0.4);
    AutomationPoint p300 = customPoint(0.4, 0.5);
    AutomationPoint p500 = customPoint(0.5, 0.6);
    AutomationPoint p700 = customPoint(0.6, 0.7);

    AutomationPointEdits edits {
        { 100, AutomationPointEdit::SetPoint { p100 } },
        { 300, AutomationPointEdit::SetPoint { p300 } },
        { 500, AutomationPointEdit::SetPoint { p500 } },
        { 700, AutomationPointEdit::SetPoint { p700 } },
    };
    controller.editPoints(key, edits);

    AutomationChanges lastChanges;
    controller.automationData()->changed().onReceive(this, [&lastChanges](const AutomationChanges& changes) {
        lastChanges = changes;
    });

    // [WHEN] Remove ticks [300, 500) (a 200-tick gap, upper bound exclusive)
    controller.insertTime(Fraction::fromTicks(300), Fraction::fromTicks(-200), s_dynamicsScore->repeatList().segmentInfoList());

    // [THEN] p300 (inside the removed range) is gone; p500/p700 shift back by 200 to 300/500
    expected[100] = p100;
    expected[300] = p500;
    expected[500] = p700;
    checkCurvesMatch(controller.automationData()->curve(key), expected);

    // [THEN] The notification also covers voice-1, which mirrors p100/p500/p700's positions via fillVoiceCurvesFromBase()
    EXPECT_EQ(lastChanges.affectedKeys, (std::set<AutomationCurveKey> { key, voiceKey }));
    EXPECT_EQ(lastChanges.tickFrom, 100);
    EXPECT_EQ(lastChanges.tickTo, 700);
    EXPECT_FALSE(lastChanges.isFullReset);
}

TEST_F(ScoreAutomationController_Tests, RemoveTicks_PointsInsideRangeAreRemoved)
{
    // [GIVEN] Two user-authored points, both inside the range about to be removed. A real
    //         Dynamics curve never ends up empty (the trailing rebuild regenerates it), so
    //         this checks the points are gone and the real baseline is untouched instead
    ScoreAutomationController controller;
    controller.init(s_dynamicsScore);

    AutomationCurveKey key = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id());
    // Real baseline, unaffected by insertTime() since s_dynamicsScore's measures never move
    AutomationCurve expected = controller.automationData()->curve(key);

    AutomationPointEdits edits {
        { 200, AutomationPointEdit::SetPoint { customPoint(0.4, 0.5) } },
        { 400, AutomationPointEdit::SetPoint { customPoint(0.5, 0.6) } },
    };
    controller.editPoints(key, edits);

    AutomationChanges lastChanges;
    controller.automationData()->changed().onReceive(this, [&lastChanges](const AutomationChanges& changes) {
        lastChanges = changes;
    });

    // [WHEN] Remove ticks in range [100, 500)
    controller.insertTime(Fraction::fromTicks(100), Fraction::fromTicks(-400), s_dynamicsScore->repeatList().segmentInfoList());

    // [THEN] Both points are gone; the curve is back to exactly the real baseline
    checkCurvesMatch(controller.automationData()->curve(key), expected);

    // [THEN] Only the base curve is reported: voice-1 never carried these points, so it's untouched
    EXPECT_EQ(lastChanges.affectedKeys, (std::set<AutomationCurveKey> { key }));
    EXPECT_EQ(lastChanges.tickFrom, 200);
    EXPECT_EQ(lastChanges.tickTo, 400);
    EXPECT_FALSE(lastChanges.isFullReset);
}

TEST_F(ScoreAutomationController_Tests, Update_IsTextEditing_DoesNothing)
{
    // [GIVEN] A score initialised with dynamics
    ScoreAutomationController controller;
    controller.init(s_dynamicsScore);

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
    controller.init(s_dynamicsScore);

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
    controller.init(s_dynamicsScore);

    AutomationCurveKey key = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id());

    // [WHEN] The user inserts a custom automation point at the midpoint of the hairpin
    AutomationPointEdits edits { { 5280, AutomationPointEdit::SetPoint { customPoint(0.0, MF_VALUE) } } };
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

TEST_F(ScoreAutomationController_Tests, EditPoints_UndoRedo_RestoresAndReappliesEdit)
{
    s_dynamicsScore->initAutomation();

    // [GIVEN] A score initialised with dynamics
    AutomationCurveKey key = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id());

    const AutomationCurve curveBefore = s_dynamicsScore->automationData()->curve(key);

    AutomationChanges lastChanges;
    s_dynamicsScore->automationData()->changed().onReceive(this, [&lastChanges](const AutomationChanges& changes) {
        lastChanges = changes;
    });

    // [WHEN] The user edits a point (in an undoable command)
    const AutomationPoint edited = customPoint(0.2, 0.3);
    AutomationPointEdits edits { { 2100, AutomationPointEdit::SetPoint { edited } } };
    s_dynamicsScore->startCmd(TranslatableString::untranslatable("ScoreAutomationController tests"));
    s_dynamicsScore->editAutomationPoints(key, edits);
    s_dynamicsScore->endCmd();

    // Restore s_dynamicsScore to its original state even if an ASSERT_* below fails early
    UndoRestoreGuard restoreScoreGuard(s_dynamicsScore);

    const AutomationCurve curveAfterEdit = s_dynamicsScore->automationData()->curve(key);
    const auto editedIt = curveAfterEdit.find(2100);
    ASSERT_TRUE(editedIt != curveAfterEdit.cend());
    EXPECT_EQ(editedIt->second, edited);

    // [THEN] The edit notification reports exactly the touched tick
    EXPECT_EQ(lastChanges.affectedKeys, (std::set<AutomationCurveKey> { key }));
    EXPECT_EQ(lastChanges.tickFrom, 2100);
    EXPECT_EQ(lastChanges.tickTo, 2100);
    EXPECT_FALSE(lastChanges.isFullReset);

    // [WHEN] The edit is undone
    lastChanges = AutomationChanges();
    s_dynamicsScore->undoRedo(true, nullptr);

    // [THEN] The curve is exactly what it was before the edit, and the undo notification matches the edit's
    checkCurvesMatch(s_dynamicsScore->automationData()->curve(key), curveBefore);
    EXPECT_EQ(lastChanges.affectedKeys, (std::set<AutomationCurveKey> { key }));
    EXPECT_EQ(lastChanges.tickFrom, 2100);
    EXPECT_EQ(lastChanges.tickTo, 2100);
    EXPECT_FALSE(lastChanges.isFullReset);

    // [WHEN] The edit is redone
    lastChanges = AutomationChanges();
    s_dynamicsScore->undoRedo(false, nullptr);

    // [THEN] The curve exactly matches the post-edit state again, and the redo notification matches too
    checkCurvesMatch(s_dynamicsScore->automationData()->curve(key), curveAfterEdit);
    EXPECT_EQ(lastChanges.affectedKeys, (std::set<AutomationCurveKey> { key }));
    EXPECT_EQ(lastChanges.tickFrom, 2100);
    EXPECT_EQ(lastChanges.tickTo, 2100);
    EXPECT_FALSE(lastChanges.isFullReset);
}

TEST_F(ScoreAutomationController_Tests, MirrorEdit_OtherRepeatSegment_CopiesPoint)
{
    // [GIVEN] Measure 5 (tick 7680-9600) is played twice via a real repeat barline: once as part
    //         of the 1st RepeatSegment (utick offset 0), once as part of the 2nd (utick offset 1920)
    const RepeatList& repeatList = s_dynamicsScore->repeatList();
    ASSERT_EQ(repeatList.size(), 2);
    const int secondPassOffset = repeatList.at(1)->utick - repeatList.at(1)->tick;
    ASSERT_EQ(secondPassOffset, 1920);

    ScoreAutomationController controller;
    controller.init(s_dynamicsScore);

    AutomationCurveKey key = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id());

    AutomationChanges lastChanges;
    controller.automationData()->changed().onReceive(this, [&lastChanges](const AutomationChanges& changes) {
        lastChanges = changes;
    });

    // [WHEN] The user adds a custom point during the 1st pass through measure 5
    const AutomationPoint edited = customPoint(0.4, 0.4);
    AutomationPointEdits edits { { 7700, AutomationPointEdit::SetPoint { edited } } };
    controller.editPoints(key, edits);

    // [THEN] The same point is mirrored into the 2nd pass through the same measure, marked generated
    //        since it is a copy, not something the user edited directly at that tick
    AutomationPoint expectedMirrored = edited;
    expectedMirrored.generated = true;

    const AutomationCurve& curve = controller.automationData()->curve(key);
    const auto mirroredIt = curve.find(7700 + secondPassOffset);
    ASSERT_TRUE(mirroredIt != curve.cend());
    EXPECT_EQ(mirroredIt->second, expectedMirrored);

    // [THEN] The notification spans both the user's edit and its mirrored copy
    EXPECT_EQ(lastChanges.affectedKeys, (std::set<AutomationCurveKey> { key }));
    EXPECT_EQ(lastChanges.tickFrom, 7700);
    EXPECT_EQ(lastChanges.tickTo, 7700 + secondPassOffset);
    EXPECT_FALSE(lastChanges.isFullReset);
}

TEST_F(ScoreAutomationController_Tests, MirrorEdit_MeasureRepeat_CopiesPoint)
{
    // [GIVEN] Measure 7 is a 1-measure repeat of measure 6 (tick 9600-11520); both only ever play
    //         once, during the 2nd RepeatSegment (utick offset 1920)
    const RepeatList& repeatList = s_dynamicsScore->repeatList();
    ASSERT_EQ(repeatList.size(), 2);
    const int passOffset = repeatList.at(1)->utick - repeatList.at(1)->tick;
    ASSERT_EQ(passOffset, 1920);

    ScoreAutomationController controller;
    controller.init(s_dynamicsScore);

    AutomationCurveKey key = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id());

    AutomationChanges lastChanges;
    controller.automationData()->changed().onReceive(this, [&lastChanges](const AutomationChanges& changes) {
        lastChanges = changes;
    });

    // [WHEN] The user adds a custom point inside measure 6, the measure-repeat's source measure
    const AutomationPoint edited = customPoint(0.4, 0.4);
    const utick_t editUTick = 9700 + passOffset;
    AutomationPointEdits edits { { editUTick, AutomationPointEdit::SetPoint { edited } } };
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

    // [THEN] The notification spans both the user's edit and its mirrored copy
    EXPECT_EQ(lastChanges.affectedKeys, (std::set<AutomationCurveKey> { key }));
    EXPECT_EQ(lastChanges.tickFrom, editUTick);
    EXPECT_EQ(lastChanges.tickTo, editUTick + 1920);
    EXPECT_FALSE(lastChanges.isFullReset);
}

TEST_F(ScoreAutomationController_Tests, Init_Tempo_CurveMatchesExpected)
{
    // [GIVEN] tempo.mscx: M1 sets 180bpm (3.0 bps); M2 has a ritardando (factor 0.5) ending at the
    //         start of M3; M4 is "a tempo"; M5 is "Tempo I"; M6 has a fermata (2x stretch) on a whole
    //         note and an end repeat with no start repeat, so the whole piece (M1-M6) plays twice
    // [WHEN] Calculate the tempo curve
    ScoreAutomationController controller;
    controller.init(s_tempoScore);

    // [THEN] Curve matches expectations: one set of points per repeat pass, the 2nd offset by 11520
    //        (the utick length of one pass). M1's own marking at utick 11520 overwrites the generated
    //        point the 1st pass's fermata recovery would otherwise leave there, but resolves to the
    //        same value either way, so the boundary is a single point, not a duplicate
    AutomationCurve expectedCurve;
    expectedCurve[0]     = generatedPoint(0.0,        TEMPO_3);   // M1: 180bpm
    expectedCurve[1920]  = generatedPoint(TEMPO_3,    TEMPO_3);   // M2: ritardando start (flat anchor)
    expectedCurve[3840]  = generatedPoint(TEMPO_1_5,  TEMPO_1_5); // M3: ritardando end (90bpm)
    expectedCurve[5760]  = generatedPoint(TEMPO_1_5,  TEMPO_3);   // M4: a tempo -> back to 180bpm
    expectedCurve[7680]  = generatedPoint(TEMPO_3,    TEMPO_3);   // M5: Tempo I -> back to 180bpm
    expectedCurve[9600]  = generatedPoint(TEMPO_3,    TEMPO_1_5); // M6: fermata halves the tempo
    expectedCurve[11520] = generatedPoint(TEMPO_1_5,  TEMPO_3);   // End of M6 / repeat back to M1
    expectedCurve[13440] = generatedPoint(TEMPO_3,    TEMPO_3);   // 2nd pass M2: ritardando start
    expectedCurve[15360] = generatedPoint(TEMPO_1_5,  TEMPO_1_5); // 2nd pass M3: ritardando end
    expectedCurve[17280] = generatedPoint(TEMPO_1_5,  TEMPO_3);   // 2nd pass M4: a tempo
    expectedCurve[19200] = generatedPoint(TEMPO_3,    TEMPO_3);   // 2nd pass M5: Tempo I
    expectedCurve[21120] = generatedPoint(TEMPO_3,    TEMPO_1_5); // 2nd pass M6: fermata halves the tempo
    expectedCurve[23040] = generatedPoint(TEMPO_1_5,  TEMPO_3);   // 2nd pass end of M6: fermata recovery

    checkCurvesMatch(controller.automationData()->curve(TEMPO_KEY), expectedCurve);
}

TEST_F(ScoreAutomationController_Tests, EditPoints_Tempo_UserPointCascadesToLaterMarkings)
{
    // [GIVEN] The tempo curve calculated from tempo.mscx (see Init_Tempo_CurveMatchesExpected)
    ScoreAutomationController controller;
    controller.init(s_tempoScore);

    AutomationChanges lastChanges;
    controller.automationData()->changed().onReceive(this, [&lastChanges](const AutomationChanges& changes) {
        lastChanges = changes;
    });

    // [WHEN] The user inserts a custom point mid-ritardando in the 1st pass (tick 2880, beat 3 of M2)
    AutomationPointEdits edits { { 2880, AutomationPointEdit::SetPoint { customPoint(TEMPO_3, TEMPO_2) } } };
    controller.editPoints(TEMPO_KEY, edits);

    // [THEN] Editing a Tempo point triggers a full rescan, so M4's "a tempo" cascades to the user's
    //        value within the 1st pass (see the single-pass version of this test for why). The repeat
    //        also mirrors the edit into the 2nd pass (tick 14400 = 2880 + 11520 utick offset), marked
    //        generated since it's a copy - but that pass's own M4 does NOT cascade, since mirroring
    //        happens after the rescan, not before it.
    AutomationCurve expectedCurve;
    expectedCurve[0]     = generatedPoint(0.0,        TEMPO_3);
    expectedCurve[1920]  = generatedPoint(TEMPO_3,    TEMPO_3);
    expectedCurve[2880]  = customPoint(TEMPO_3,       TEMPO_2);
    expectedCurve[3840]  = generatedPoint(TEMPO_1_5,  TEMPO_1_5);
    expectedCurve[5760]  = generatedPoint(TEMPO_1_5,  TEMPO_2);   // outValue resolves to the user's point
    expectedCurve[7680]  = generatedPoint(TEMPO_2,    TEMPO_3);   // Tempo I still resolves to M1's tempo
    expectedCurve[9600]  = generatedPoint(TEMPO_3,    TEMPO_1_5);
    expectedCurve[11520] = generatedPoint(TEMPO_1_5,  TEMPO_3);
    expectedCurve[13440] = generatedPoint(TEMPO_3,    TEMPO_3);
    expectedCurve[14400] = generatedPoint(TEMPO_3,    TEMPO_2);   // mirrored copy of the user's edit
    expectedCurve[15360] = generatedPoint(TEMPO_1_5,  TEMPO_1_5);
    expectedCurve[17280] = generatedPoint(TEMPO_1_5,  TEMPO_3);   // 2nd pass's own "a tempo" - not cascaded
    expectedCurve[19200] = generatedPoint(TEMPO_3,    TEMPO_3);
    expectedCurve[21120] = generatedPoint(TEMPO_3,    TEMPO_1_5);
    expectedCurve[23040] = generatedPoint(TEMPO_1_5,  TEMPO_3);

    checkCurvesMatch(controller.automationData()->curve(TEMPO_KEY), expectedCurve);

    // [THEN] tickTo reaches 14400, the mirrored copy's tick, since it's the last point whose value or
    //        resolved inValue actually changed; editing a Tempo point forces a full rescan, so isFullReset is set
    EXPECT_EQ(lastChanges.affectedKeys, (std::set<AutomationCurveKey> { TEMPO_KEY }));
    EXPECT_EQ(lastChanges.tickFrom, 2880);
    EXPECT_EQ(lastChanges.tickTo, 14400);
    EXPECT_TRUE(lastChanges.isFullReset);
}

TEST_F(ScoreAutomationController_Tests, Update_TempoRelevantType_FullRescan)
{
    // [GIVEN] A score initialised with tempo
    ScoreAutomationController controller;
    controller.init(s_tempoScore);

    // [GIVEN] The live tempo curve is replaced by a stale/incorrect one, so notification requires a real rescan
    AutomationCurveMap staleCurves;
    staleCurves[TEMPO_KEY][0] = generatedPoint(0.0, TEMPO_1_5);
    AutomationDataPtr staleData = std::make_shared<AutomationData>();
    staleData->setCurves(staleCurves);
    controller.setAutomationData(staleData);

    bool notified = false;
    controller.automationData()->changed().onReceive(this, [&notified](const AutomationChanges&) {
        notified = true;
    });

    // [WHEN] A change arrives with a tempo-relevant element type
    ScoreChanges changes;
    changes.changedTypes = { ElementType::TEMPO_TEXT };
    controller.update(changes);

    // [THEN] A change notification was sent, and the tempo curve matches a from-scratch rebuild
    EXPECT_TRUE(notified);

    AutomationCurve expectedCurve;
    expectedCurve[0]     = generatedPoint(0.0,        TEMPO_3);
    expectedCurve[1920]  = generatedPoint(TEMPO_3,    TEMPO_3);
    expectedCurve[3840]  = generatedPoint(TEMPO_1_5,  TEMPO_1_5);
    expectedCurve[5760]  = generatedPoint(TEMPO_1_5,  TEMPO_3);
    expectedCurve[7680]  = generatedPoint(TEMPO_3,    TEMPO_3);
    expectedCurve[9600]  = generatedPoint(TEMPO_3,    TEMPO_1_5);
    expectedCurve[11520] = generatedPoint(TEMPO_1_5,  TEMPO_3);
    expectedCurve[13440] = generatedPoint(TEMPO_3,    TEMPO_3);
    expectedCurve[15360] = generatedPoint(TEMPO_1_5,  TEMPO_1_5);
    expectedCurve[17280] = generatedPoint(TEMPO_1_5,  TEMPO_3);
    expectedCurve[19200] = generatedPoint(TEMPO_3,    TEMPO_3);
    expectedCurve[21120] = generatedPoint(TEMPO_3,    TEMPO_1_5);
    expectedCurve[23040] = generatedPoint(TEMPO_1_5,  TEMPO_3);

    checkCurvesMatch(controller.automationData()->curve(TEMPO_KEY), expectedCurve);
}

TEST_F(ScoreAutomationController_Tests, TempoTimeline_ExpandRepeats_TrueVsFalse)
{
    // [GIVEN] tempo.mscx's end repeat plays the whole piece twice (see Init_Tempo_CurveMatchesExpected)
    ScoreAutomationController controller;
    controller.init(s_tempoScore);

    const TempoTimeline& flattened = controller.tempoTimeline(false);
    const TempoTimeline& expanded = controller.tempoTimeline(true);

    // [THEN] The flattened timeline ignores the repeat: it only ever covers one playthrough,
    //        ending exactly at the raw tick length of the piece (11520)
    ASSERT_FALSE(flattened.points().empty());
    EXPECT_EQ(flattened.points().back().utick, 11520);

    // [THEN] The expanded timeline covers both passes, ending at the repeated utick length (23040)
    ASSERT_FALSE(expanded.points().empty());
    EXPECT_EQ(expanded.points().back().utick, 23040);

    // [THEN] Both passes are tempo-identical, so the expanded timeline takes exactly twice as long
    //        to reach the end of the 2nd pass as it does the end of the 1st, and the 1st pass's
    //        own elapsed time matches the flattened (single-pass) timeline exactly
    const double singlePassTime = flattened.utick2utime(11520);
    EXPECT_GT(singlePassTime, 0.0);
    EXPECT_NEAR(expanded.utick2utime(11520), singlePassTime, TIME_ERROR);
    EXPECT_NEAR(expanded.utick2utime(23040), 2 * singlePassTime, TIME_ERROR);

    // [THEN] Sampled mid-ritardando (M2, beat 3), the 2nd pass's tempo matches the 1st exactly,
    //        and the flattened timeline agrees with the expanded one's 1st pass
    EXPECT_NEAR(expanded.tempo(2880).val, expanded.tempo(2880 + 11520).val, TEMPO_ERROR);
    EXPECT_NEAR(flattened.tempo(2880).val, expanded.tempo(2880).val, TEMPO_ERROR);
}

TEST_F(ScoreAutomationController_Tests, Update_RepeatStructureChange_ForcesFullReprocessing)
{
    // [GIVEN] A score initialised with dynamics
    ScoreAutomationController controller;
    controller.init(s_dynamicsScore);

    const AutomationCurveKey key = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id());

    // [GIVEN] The live dynamics curve is replaced by a stale/incorrect one, so notification requires a real rescan
    AutomationCurveMap staleCurves;
    staleCurves[key][0] = generatedPoint(0.0, P_VALUE);
    AutomationDataPtr staleData = std::make_shared<AutomationData>();
    staleData->setCurves(staleCurves);
    controller.setAutomationData(staleData);

    bool notified = false;
    controller.automationData()->changed().onReceive(this, [&notified](const AutomationChanges&) {
        notified = true;
    });

    // [WHEN] A change arrives with a repeat-structure-relevant element type (Volta), even though
    //        this score has no Volta and no relevant Pid changed
    ScoreChanges changes;
    changes.changedTypes = { ElementType::VOLTA };
    controller.update(changes);

    // [THEN] A change notification was sent (repeat-structure changes force both Dynamics and Tempo
    //        reprocessing, per classifyChanges()), and Dynamics matches a from-scratch rebuild
    EXPECT_TRUE(notified);

    AutomationCurve expectedCurve;
    expectedCurve[480]  = generatedPoint(0.0,      P_VALUE);
    expectedCurve[1440] = generatedPoint(P_VALUE,  MP_VALUE);
    expectedCurve[1920] = generatedPoint(MP_VALUE, F_VALUE);
    expectedCurve[2400] = generatedPoint(F_VALUE,  MP_VALUE);
    expectedCurve[2880] = generatedPoint(MP_VALUE, P_VALUE);
    expectedCurve[3264] = generatedPoint(F_VALUE,  F_VALUE);
    expectedCurve[4800] = generatedPoint(F_VALUE,  P_VALUE);
    expectedCurve[5760] = generatedPoint(FF_VALUE, FF_VALUE);

    checkCurvesMatch(controller.automationData()->curve(key), expectedCurve);
}
