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
#include "engraving/dom/part.h"
#include "engraving/dom/repeatlist.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/tempotimeline.h"
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
static const double TEMPO_1(normalizeTempo(BeatsPerSecond(1.0)));
static const double TEMPO_2_5(normalizeTempo(BeatsPerSecond(2.5)));

static constexpr double TEMPO_ERROR(0.000001);
static constexpr double TIME_ERROR(0.000001);

// Play order: M1-M5, then M5-M10 (M5 is repeated)
static AutomationCurve expectedDynamicsCurve()
{
    AutomationCurve curve;

    // 1st measure
    curve[480]  = generatedPoint(0.0,      P_VALUE);  // 2nd beat: p
                                                      // 3rd beat: ff with playback off adds no point
    curve[1440] = generatedPoint(P_VALUE,  MP_VALUE); // 4th beat: mp

    // 2nd measure
    curve[1920] = generatedPoint(MP_VALUE, F_VALUE);  // 1st beat: sf
    curve[2400] = generatedPoint(F_VALUE,  MP_VALUE); // 2nd beat: mp (sf recovery)
    curve[2880] = generatedPoint(MP_VALUE, P_VALUE);  // 3rd beat: pf start
    curve[3264] = generatedPoint(F_VALUE,  F_VALUE);  // 4th beat: pf end

    // 3rd measure
    curve[4800] = generatedPoint(F_VALUE,  P_VALUE);  // 3rd beat: p (hairpin start)

    // 4th measure
    curve[5760] = generatedPoint(FF_VALUE, FF_VALUE); // 1st beat: ff (hairpin end)

    // 6th measure
    curve[12000] = generatedPoint(FF_VALUE, MF_VALUE); // 2nd beat: mf

    // 7th measure: 1-measure repeat of the 6th
    curve[13920] = generatedPoint(MF_VALUE, MF_VALUE); // copy of the 6th measure's mf

    // 8th measure: crescendo with no end dynamic
    curve[15360] = generatedPoint(MF_VALUE, MF_VALUE); // 1st beat: hairpin start (active mf)
    curve[16320] = generatedPoint(F_VALUE,  F_VALUE);  // 3rd beat: hairpin end (one step above mf)

    // 9th measure
    curve[17280] = generatedPoint(F_VALUE,  P_VALUE);  // 1st beat: allInStaff p beats allInInstrument f
    curve[18240] = generatedPoint(P_VALUE,  P_VALUE);  // 3rd beat: crescendo start

    // 10th measure
    curve[19200] = generatedPoint(MP_VALUE, P_VALUE);  // 1st beat: p; the crescendo still rises one step into it

    return curve;
}

// Play order: M1-M6 twice, then M7-M8, M7, M9-M11 (M8/M9 are voltas)
static AutomationCurve expectedTempoCurve()
{
    AutomationCurve curve;
    curve[0]     = generatedPoint(0.0,        TEMPO_3);   // M1: 180bpm
    curve[1920]  = generatedPoint(TEMPO_3,    TEMPO_3);   // M2: ritardando start (flat anchor)
    curve[3840]  = generatedPoint(TEMPO_1_5,  TEMPO_1_5); // M3: ritardando end (90bpm)
    curve[5760]  = generatedPoint(TEMPO_1_5,  TEMPO_3);   // M4: a tempo -> back to 180bpm
    curve[7680]  = generatedPoint(TEMPO_3,    TEMPO_3);   // M5: Tempo I -> back to 180bpm
    curve[9600]  = generatedPoint(TEMPO_3,    TEMPO_1_5); // M6: fermata halves the tempo
    curve[11520] = generatedPoint(TEMPO_1_5,  TEMPO_3);   // 2nd pass M1: its own marking replaces the fermata recovery
    curve[13440] = generatedPoint(TEMPO_3,    TEMPO_3);   // 2nd pass M2: ritardando start
    curve[15360] = generatedPoint(TEMPO_1_5,  TEMPO_1_5); // 2nd pass M3: ritardando end
    curve[17280] = generatedPoint(TEMPO_1_5,  TEMPO_3);   // 2nd pass M4: a tempo
    curve[19200] = generatedPoint(TEMPO_3,    TEMPO_3);   // 2nd pass M5: Tempo I
    curve[21120] = generatedPoint(TEMPO_3,    TEMPO_1_5); // 2nd pass M6: fermata halves the tempo
    curve[23040] = generatedPoint(TEMPO_1_5,  TEMPO_3);   // M7: fermata recovery
    curve[25920] = generatedPoint(TEMPO_3,    TEMPO_1);   // M8 (1st volta), beat 3: 60bpm
    curve[26880] = generatedPoint(TEMPO_1,    TEMPO_3);   // M7 replayed: reset to the tempo before the 1st volta
    curve[29760] = generatedPoint(TEMPO_3,    TEMPO_1_5); // M9 (2nd volta), beat 3: 90bpm
    curve[30720] = generatedPoint(TEMPO_1_5,  TEMPO_2);   // M10: 120bpm + ritardando start (flat anchor)
    curve[32640] = generatedPoint(TEMPO_1,    TEMPO_2_5); // M11: ritardando arrives at 60bpm, then 150bpm
                                                          // M11, beat 3: fermata with playback off adds no point
    return curve;
}

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

// Builds the dynamics curves from dynamics.mscx and checks every point of the staff and voice-1 curves
TEST_F(ScoreAutomationController_Tests, Init_Dynamics_CurveMatchesExpected)
{
    // [WHEN] Calculate the dynamics curve
    ScoreAutomationController controller;
    controller.init(s_dynamicsScore);

    AutomationCurveKey key = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id());

    // [THEN] Staff 0 all-voice curve matches expectations
    checkCurvesMatch(controller.automationData()->curve(key), expectedDynamicsCurve());

    // [THEN] Voice-1 curve on staff 0 matches expectations.
    // Voice-1 has a CURRENT_VOICE_ONLY f at tick 3840. All shared-curve points (including the
    // measure repeat's copy) are filled in too, so the voice curve is self-contained
    key = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id(), size_t(1));
    AutomationCurve expectedVoiceCurve = expectedDynamicsCurve();

    // CURRENT_VOICE_ONLY f; inValue comes from the shared active point at tick 3264 (outValue = F_VALUE)
    expectedVoiceCurve[3840] = generatedPoint(F_VALUE, F_VALUE);

    checkCurvesMatch(controller.automationData()->curve(key), expectedVoiceCurve);
}

// Inserting time mid-score shifts only the points at or after the insert tick
TEST_F(ScoreAutomationController_Tests, InsertTime_ShiftsPointsAtAndAfterTick)
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

// Inserting time shifts points on both the staff and voice curves, and reports exactly those two
TEST_F(ScoreAutomationController_Tests, InsertTime_ShiftsStaffAndVoiceCurves)
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

// Removing a tick range erases the points inside it and shifts later ones back to close the gap
TEST_F(ScoreAutomationController_Tests, InsertTime_NegativeLength_RemovesRangeAndClosesGap)
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

// Removing a range holding all user points restores the generated baseline; the voice curve isn't reported
TEST_F(ScoreAutomationController_Tests, InsertTime_NegativeLength_AllPointsInRange_RestoresGeneratedCurve)
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

// In a later repeat pass, points are shifted by their raw tick, not their utick
TEST_F(ScoreAutomationController_Tests, InsertTime_LaterRepeatPass_ShiftsByRawTick)
{
    // [GIVEN] Two user points in dynamics.mscx's 2nd repeat pass (utick = raw tick + 1920)
    ScoreAutomationController controller;
    controller.init(s_dynamicsScore);

    const AutomationCurveKey key = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id());

    AutomationPointEdits edits {
        { 16800, AutomationPointEdit::SetPoint { customPoint(0.4, 0.5) } }, // M8, raw tick 14880
        { 18000, AutomationPointEdit::SetPoint { customPoint(0.5, 0.6) } }, // M9, raw tick 16080
    };
    controller.editPoints(key, edits);

    // [WHEN] Insert 100 ticks at raw tick 15500: after the 1st point's raw tick, but before its utick
    controller.insertTime(Fraction::fromTicks(15500), Fraction::fromTicks(100), s_dynamicsScore->repeatList().segmentInfoList());

    // [THEN] The 1st point stays put, only the 2nd one shifts
    const AutomationCurve& curve = controller.automationData()->curve(key);
    EXPECT_TRUE(curve.contains(16800));
    EXPECT_FALSE(curve.contains(16900));
    EXPECT_FALSE(curve.contains(18000));
    EXPECT_TRUE(curve.contains(18100));
}

// Changes made during text editing are ignored
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

// Changes to elements that don't affect automation are ignored
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

// A user point inside a hairpin survives a rebuild, and the hairpin ramps into it and on to its end
TEST_F(ScoreAutomationController_Tests, Update_UserPointInsideHairpin_IsKept)
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

    // [THEN] The p dynamic at the hairpin start is unchanged, the user midpoint's own data is untouched
    //        by the rebuild, and the ff dynamic keeps its own independently-computed arrival value
    AutomationCurve expectedCurve = expectedDynamicsCurve();
    expectedCurve[5280] = customPoint(0.0, MF_VALUE);

    checkCurvesMatch(controller.automationData()->curve(key), expectedCurve);
}

// Undo restores the curve from before a point edit, redo reapplies it, both with matching notifications
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

// A point added in the 1st pass of a repeated measure is copied into its 2nd pass
TEST_F(ScoreAutomationController_Tests, EditPoints_PointInRepeatedMeasure_CopiedToOtherPass)
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

// A point added in a measure is copied into the measure repeat that replays it
TEST_F(ScoreAutomationController_Tests, EditPoints_PointInMeasureRepeatSource_CopiedToMeasureRepeat)
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

// Builds the tempo curve from tempo.mscx (markings, ritardandos, fermata, repeats, voltas) and checks every point
TEST_F(ScoreAutomationController_Tests, Init_Tempo_CurveMatchesExpected)
{
    // [GIVEN] tempo.mscx (see expectedTempoCurve)
    // 5 passes, the last one split in two by M10's section break
    const RepeatList& repeatList = s_tempoScore->repeatList();
    ASSERT_EQ(repeatList.size(), 6);

    // [WHEN] Calculate the tempo curve
    ScoreAutomationController controller;
    controller.init(s_tempoScore);

    // [THEN] Curve matches expectations
    checkCurvesMatch(controller.automationData()->curve(TEMPO_KEY), expectedTempoCurve());
}

// A user tempo point inside a ritardando changes what a later "a tempo" returns to, and is mirrored into the repeat
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
    //        happens after the rescan, not before it. Nothing from M7 on changes
    AutomationCurve expectedCurve = expectedTempoCurve();
    expectedCurve[2880]  = customPoint(TEMPO_3,      TEMPO_2);
    expectedCurve[5760]  = generatedPoint(TEMPO_1_5, TEMPO_2); // outValue resolves to the user's point
    expectedCurve[7680]  = generatedPoint(TEMPO_2,   TEMPO_3); // Tempo I still resolves to M1's tempo
    expectedCurve[14400] = generatedPoint(TEMPO_3,   TEMPO_2); // mirrored copy of the user's edit

    checkCurvesMatch(controller.automationData()->curve(TEMPO_KEY), expectedCurve);

    // [THEN] tickTo reaches 14400, the mirrored copy's tick, since it's the last point whose value or
    //        resolved inValue actually changed; editing a Tempo point forces a full rescan, so isFullReset is set
    EXPECT_EQ(lastChanges.affectedKeys, (std::set<AutomationCurveKey> { TEMPO_KEY }));
    EXPECT_EQ(lastChanges.tickFrom, 2880);
    EXPECT_EQ(lastChanges.tickTo, 14400);
    EXPECT_TRUE(lastChanges.isFullReset);
}

// Moving a user tempo point rebuilds the curve from its new spot, and erasing it restores the score's curve
TEST_F(ScoreAutomationController_Tests, EditPoints_Tempo_MoveAndErase_RebuildCurve)
{
    // [GIVEN] A user point mid-ritardando in the 1st pass (see EditPoints_Tempo_UserPointCascadesToLaterMarkings)
    ScoreAutomationController controller;
    controller.init(s_tempoScore);

    const AutomationPoint userPoint = customPoint(TEMPO_3, TEMPO_2);
    AutomationPointEdits setEdits { { 2880, AutomationPointEdit::SetPoint { userPoint } } };
    controller.editPoints(TEMPO_KEY, setEdits);

    // [WHEN] The point is moved to beat 4 of M2
    AutomationPointEdits moveEdits { { 3360, AutomationPointEdit::MovePoint { userPoint, 2880 } } };
    controller.editPoints(TEMPO_KEY, moveEdits);

    // [THEN] It cascades from there, and its 2nd-pass copy moves with it
    AutomationCurve expectedCurve = expectedTempoCurve();
    expectedCurve[3360]  = userPoint;
    expectedCurve[5760]  = generatedPoint(TEMPO_1_5, TEMPO_2);
    expectedCurve[7680]  = generatedPoint(TEMPO_2,   TEMPO_3);
    expectedCurve[14880] = generatedPoint(TEMPO_3,   TEMPO_2);
    checkCurvesMatch(controller.automationData()->curve(TEMPO_KEY), expectedCurve);

    // [WHEN] The point is erased
    AutomationPointEdits eraseEdits { { 3360, AutomationPointEdit::ErasePoint {} } };
    controller.editPoints(TEMPO_KEY, eraseEdits);

    // [THEN] The curve is back to the score's own, including the 2nd pass
    checkCurvesMatch(controller.automationData()->curve(TEMPO_KEY), expectedTempoCurve());
}

// A tempo text change rebuilds the whole tempo curve, replacing a stale one
TEST_F(ScoreAutomationController_Tests, Update_TempoText_RebuildsTempoCurve)
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
    checkCurvesMatch(controller.automationData()->curve(TEMPO_KEY), expectedTempoCurve());
}

// With repeats expanded, both passes take the same time, and the flattened timeline matches the 1st pass
TEST_F(ScoreAutomationController_Tests, TempoTimeline_ExpandRepeats_BothPassesTakeSameTime)
{
    // [GIVEN] tempo.mscx's first end repeat plays M1-M6 (raw ticks 0-11520) twice (see expectedTempoCurve)
    ScoreAutomationController controller;
    controller.init(s_tempoScore);

    const TempoTimeline& flattened = controller.tempoTimeline(false);
    const TempoTimeline& expanded = controller.tempoTimeline(true);

    ASSERT_FALSE(flattened.points().empty());
    ASSERT_FALSE(expanded.points().empty());

    // [THEN] Both passes of M1-M6 are tempo-identical, so the expanded timeline takes exactly twice as long
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

// The 2nd volta and the jump back after the 1st volta both use the tempo from before the 1st volta
TEST_F(ScoreAutomationController_Tests, TempoTimeline_SecondVolta_StartsAtPreVoltaTempo)
{
    // [GIVEN] tempo.mscx M7-M9: the 1st volta (M8) sets 60bpm on beat 3, the 2nd volta (M9) sets 90bpm
    //         on beat 3; the tempo before the voltas is 180bpm
    ScoreAutomationController controller;
    controller.init(s_tempoScore);

    const TempoTimeline& flattened = controller.tempoTimeline(false);
    const TempoTimeline& expanded = controller.tempoTimeline(true);

    // [THEN] Flattened: the 2nd volta (raw tick 15360) starts at 180bpm, not at the 1st volta's 60bpm
    EXPECT_NEAR(flattened.tempo(15359).val, 1.0, TEMPO_ERROR);
    EXPECT_NEAR(flattened.tempo(15360).val, 3.0, TEMPO_ERROR);

    // [THEN] Expanded: jumping back to M7 after the 1st volta (utick 26880) restores 180bpm,
    //        which carries on into the 2nd volta (utick 28800)
    EXPECT_NEAR(expanded.tempo(26879).val, 1.0, TEMPO_ERROR);
    EXPECT_NEAR(expanded.tempo(26880).val, 3.0, TEMPO_ERROR);
    EXPECT_NEAR(expanded.tempo(28800).val, 3.0, TEMPO_ERROR);
}

// A ritardando starting on a tempo marking ramps from that marking, not from an earlier ramp's end
// See https://github.com/musescore/MuseScore/issues/35008
TEST_F(ScoreAutomationController_Tests, TempoTimeline_GradualChangeOnTempoMarking_RampStartsAtMarking)
{
    // [GIVEN] tempo.mscx M10: a 120bpm marking and a ritardando (factor 0.5) start together, after M9's 90bpm
    //         marking; the ritardando arrives at M11 (utick 32640, raw tick 19200)
    ScoreAutomationController controller;
    controller.init(s_tempoScore);

    const TempoTimeline& flattened = controller.tempoTimeline(false);
    const TempoTimeline& expanded = controller.tempoTimeline(true);

    // [THEN] 90bpm holds up to M10, the ramp starts at the marking's 120bpm and is halfway to 60bpm
    //        in the middle of M10, then M11's marking sets 150bpm
    EXPECT_NEAR(expanded.tempo(30719).val, 1.5, TEMPO_ERROR);
    EXPECT_NEAR(expanded.tempo(30720).val, 2.0, TEMPO_ERROR);
    EXPECT_NEAR(expanded.tempo(31680).val, 1.5, TEMPO_ERROR);
    EXPECT_NEAR(expanded.tempo(32640).val, 2.5, TEMPO_ERROR);

    // [THEN] The flattened timeline ramps the same way
    EXPECT_NEAR(flattened.tempo(17279).val, 1.5, TEMPO_ERROR);
    EXPECT_NEAR(flattened.tempo(17280).val, 2.0, TEMPO_ERROR);
    EXPECT_NEAR(flattened.tempo(18240).val, 1.5, TEMPO_ERROR);
}

// A user tempo point at a ritardando's start wins over the marking there and sets the ramp's start tempo
TEST_F(ScoreAutomationController_Tests, EditPoints_Tempo_UserPointAtRampStart_IsKept)
{
    // [GIVEN] The tempo curve calculated from tempo.mscx (see Init_Tempo_CurveMatchesExpected)
    ScoreAutomationController controller;
    controller.init(s_tempoScore);

    // [WHEN] The user sets 180bpm at M10's 120bpm marking, where the ritardando also starts
    AutomationPointEdits edits { { 30720, AutomationPointEdit::SetPoint { customPoint(TEMPO_1_5, TEMPO_3) } } };
    controller.editPoints(TEMPO_KEY, edits);

    // [THEN] The user point is kept, and the ritardando halves the user's tempo instead of the marking's
    AutomationCurve expectedCurve = expectedTempoCurve();
    expectedCurve[30720] = customPoint(TEMPO_1_5, TEMPO_3);
    expectedCurve[32640] = generatedPoint(TEMPO_1_5, TEMPO_2_5);
    checkCurvesMatch(controller.automationData()->curve(TEMPO_KEY), expectedCurve);

    // [THEN] Both timelines ramp from the user's 180bpm and are halfway to 90bpm in the middle of M10
    EXPECT_NEAR(controller.tempoTimeline(true).tempo(30720).val, 3.0, TEMPO_ERROR);
    EXPECT_NEAR(controller.tempoTimeline(true).tempo(31680).val, 2.25, TEMPO_ERROR);
    EXPECT_NEAR(controller.tempoTimeline(false).tempo(17280).val, 3.0, TEMPO_ERROR);
    EXPECT_NEAR(controller.tempoTimeline(false).tempo(18240).val, 2.25, TEMPO_ERROR);
}

// A breath mark's pause is added to the playback time
TEST_F(ScoreAutomationController_Tests, TempoTimeline_Breath_AddsPause)
{
    // [GIVEN] tempo.mscx M11 (150bpm) has a breath with a 1s pause after beat 2
    ScoreAutomationController controller;
    controller.init(s_tempoScore);

    const TempoTimeline& flattened = controller.tempoTimeline(false);
    const TempoTimeline& expanded = controller.tempoTimeline(true);

    // [THEN] Playing M11 takes 4 beats at 2.5 bps (1.6s) plus the pause
    const double expectedDuration = 4 / 2.5 + 1.0;
    EXPECT_NEAR(expanded.utick2utime(34560) - expanded.utick2utime(32640), expectedDuration, TIME_ERROR);
    EXPECT_NEAR(flattened.utick2utime(21120) - flattened.utick2utime(19200), expectedDuration, TIME_ERROR);
}

// A section break's pause is added to the playback time at the start of the next section
TEST_F(ScoreAutomationController_Tests, TempoTimeline_SectionBreak_AddsPause)
{
    // [GIVEN] tempo.mscx M10 ends with a section break with a 2s pause; M11 starts at utick 32640 (raw tick 19200)
    ScoreAutomationController controller;
    controller.init(s_tempoScore);

    const TempoTimeline& flattened = controller.tempoTimeline(false);
    const TempoTimeline& expanded = controller.tempoTimeline(true);

    // [THEN] The last tick before M11 takes the pause plus about 2ms (one tick at 60bpm)
    const double pauseError = 0.01;
    EXPECT_NEAR(expanded.utick2utime(32640) - expanded.utick2utime(32639), 2.0, pauseError);
    EXPECT_NEAR(flattened.utick2utime(19200) - flattened.utick2utime(19199), 2.0, pauseError);
}

// The tempo multiplier scales playback time but not the tempo values, and survives a rebuild
TEST_F(ScoreAutomationController_Tests, SetTempoMultiplier_ScalesPlaybackTime)
{
    // [GIVEN] The tempo timelines calculated from tempo.mscx
    ScoreAutomationController controller;
    controller.init(s_tempoScore);

    const TempoTimeline& flattened = controller.tempoTimeline(false);
    const TempoTimeline& expanded = controller.tempoTimeline(true);
    const double expandedTime = expanded.utick2utime(11520);
    const double flattenedTime = flattened.utick2utime(11520);

    // [WHEN] Playback is set to double speed
    controller.setTempoMultiplier(BeatsPerSecond(2.0));

    // [THEN] Both timelines take half as long, and the tempo itself is unchanged
    EXPECT_NEAR(expanded.utick2utime(11520), expandedTime / 2, TIME_ERROR);
    EXPECT_NEAR(flattened.utick2utime(11520), flattenedTime / 2, TIME_ERROR);
    EXPECT_NEAR(expanded.tempo(0).val, 3.0, TEMPO_ERROR);

    // [WHEN] The tempo curve is rebuilt
    ScoreChanges changes;
    changes.changedTypes = { ElementType::TEMPO_TEXT };
    controller.update(changes);

    // [THEN] The multiplier still applies
    EXPECT_NEAR(expanded.utick2utime(11520), expandedTime / 2, TIME_ERROR);
    EXPECT_NEAR(flattened.utick2utime(11520), flattenedTime / 2, TIME_ERROR);
}

// A timeline override replaces both timelines until it's cleared
TEST_F(ScoreAutomationController_Tests, SetTempoTimelineOverride_ReplacesBothTimelines)
{
    // [GIVEN] The tempo timelines calculated from tempo.mscx (180bpm at tick 0)
    ScoreAutomationController controller;
    controller.init(s_tempoScore);

    // [WHEN] An override with a constant 60bpm is set
    TempoTimeline overrideTimeline;
    overrideTimeline.rebuild(TempoValues { { 0, 1.0 } }, PausesMap());
    controller.setTempoTimelineOverride(overrideTimeline);

    // [THEN] Both the expanded and the flattened timeline return the override
    EXPECT_NEAR(controller.tempoTimeline(true).tempo(0).val, 1.0, TEMPO_ERROR);
    EXPECT_NEAR(controller.tempoTimeline(false).tempo(0).val, 1.0, TEMPO_ERROR);

    // [WHEN] The override is cleared
    controller.setTempoTimelineOverride(std::nullopt);

    // [THEN] The score's own timelines are back
    EXPECT_NEAR(controller.tempoTimeline(true).tempo(0).val, 3.0, TEMPO_ERROR);
    EXPECT_NEAR(controller.tempoTimeline(false).tempo(0).val, 3.0, TEMPO_ERROR);
}

// A volta change rebuilds both dynamics and tempo
TEST_F(ScoreAutomationController_Tests, Update_Volta_RebuildsDynamicsAndTempo)
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
    checkCurvesMatch(controller.automationData()->curve(key), expectedDynamicsCurve());
}

// A dynamics change from a given tick keeps earlier points and rebuilds everything after it
TEST_F(ScoreAutomationController_Tests, Update_DynamicFromTick_RebuildsOnlyFromTick)
{
    // [GIVEN] A score initialised with dynamics
    ScoreAutomationController controller;
    controller.init(s_dynamicsScore);

    const AutomationCurveKey key = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id());

    // [GIVEN] The live dynamics curve is replaced by a stale one: a generated ff before the edited range, and a
    //         generated point inside it that no longer matches the score
    AutomationCurveMap staleCurves;
    staleCurves[key][100] = generatedPoint(0.0, FF_VALUE);
    staleCurves[key][16000] = generatedPoint(0.0, P_VALUE);
    AutomationDataPtr staleData = std::make_shared<AutomationData>();
    staleData->setCurves(staleCurves);
    controller.setAutomationData(staleData);

    // [WHEN] A dynamics-only change arrives for staff 0 from M8 (raw tick 13440, utick 15360) on
    ScoreChanges changes;
    changes.changedTypes = { ElementType::DYNAMIC };
    changes.tickFrom = 13440;
    changes.staffIdxFrom = 0;
    changes.staffIdxTo = 0;
    controller.update(changes);

    // [THEN] The generated point before utick 15360 is kept as is, the stale one after it is gone, and
    //        M8-M10 are rebuilt; M8's hairpin starts from the kept ff, since that's the active point there
    const double ffPlusStep = FF_VALUE + 0.05;

    AutomationCurve expectedCurve;
    expectedCurve[100]   = generatedPoint(0.0,        FF_VALUE);
    expectedCurve[15360] = generatedPoint(FF_VALUE,   FF_VALUE);   // M8 hairpin start
    expectedCurve[16320] = generatedPoint(ffPlusStep, ffPlusStep); // M8 hairpin end
    expectedCurve[17280] = generatedPoint(ffPlusStep, P_VALUE);    // M9: allInStaff p
    expectedCurve[18240] = generatedPoint(P_VALUE,    P_VALUE);    // M9 crescendo start
    expectedCurve[19200] = generatedPoint(MP_VALUE,   P_VALUE);    // M10: p

    checkCurvesMatch(controller.automationData()->curve(key), expectedCurve);
}

// Volume/Pan curves keep their user points through a rebuild, and drop stale generated ones
TEST_F(ScoreAutomationController_Tests, Update_InstrumentCurve_KeepsUserPointsOnly)
{
    // [GIVEN] A score initialised with dynamics, plus a Volume curve with two user points and a stale generated one
    ScoreAutomationController controller;
    controller.init(s_dynamicsScore);

    const Part* part = s_dynamicsScore->staff(0)->part();
    const AutomationCurveKey volumeKey = AutomationCurveKey::instrument(AutomationType::Volume,
                                                                        InstrumentTrackId { part->id(), part->instrumentId() });

    AutomationCurveMap curves = controller.automationData()->curves();
    curves[volumeKey][3000] = generatedPoint(0.0, 0.3);
    curves[volumeKey][8000] = customPoint(0.0, 0.8);  // M5, 1st pass
    curves[volumeKey][12000] = customPoint(0.8, 0.4); // M6, the source of M7's measure repeat
    AutomationDataPtr data = std::make_shared<AutomationData>();
    data->setCurves(curves);
    controller.setAutomationData(data);

    // [WHEN] A dynamics change triggers a rebuild
    ScoreChanges changes;
    changes.changedTypes = { ElementType::DYNAMIC };
    controller.update(changes);

    // [THEN] The generated point is gone, the user points are kept, and the M5 point is copied into M5's 2nd pass;
    //        the M6 point isn't copied into M7, since measure repeats only apply to staff curves
    AutomationCurve expectedCurve;
    expectedCurve[8000] = customPoint(0.0, 0.8);
    expectedCurve[9920] = generatedPoint(0.0, 0.8);
    expectedCurve[12000] = customPoint(0.8, 0.4);
    checkCurvesMatch(controller.automationData()->curve(volumeKey), expectedCurve);
}

// A compound dynamic like pf keeps its change length in seconds, so a faster tempo multiplier stretches it in ticks
TEST_F(ScoreAutomationController_Tests, SetTempoMultiplier_StretchesCompoundDynamic)
{
    // [GIVEN] dynamics.mscx M2's pf (tick 2880), whose change takes 384 ticks at the default 120bpm
    ScoreAutomationController controller;
    controller.init(s_dynamicsScore);

    const AutomationCurveKey key = AutomationCurveKey::staff(AutomationType::Dynamics, s_dynamicsScore->staff(0)->id());

    // [WHEN] Playback is set to double speed and the dynamics are rebuilt
    controller.setTempoMultiplier(BeatsPerSecond(2.0));

    ScoreChanges changes;
    changes.changedTypes = { ElementType::DYNAMIC };
    controller.update(changes);

    // [THEN] The pf's end point moves from 3264 to 3648 (2 * 384 ticks after its start)
    const AutomationCurve& curve = controller.automationData()->curve(key);
    EXPECT_FALSE(curve.contains(3264));

    const auto endIt = curve.upper_bound(2880);
    ASSERT_TRUE(endIt != curve.cend());
    EXPECT_NEAR(endIt->first, 3648, 1);
    EXPECT_NEAR(endIt->second.value.outValue, F_VALUE, 0.0001);
}

// ensureInitialized builds the curves only on its first call
TEST_F(ScoreAutomationController_Tests, EnsureInitialized_InitializesOnlyOnce)
{
    // [WHEN] A new controller is asked to ensure it's initialised
    ScoreAutomationController controller;
    controller.ensureInitialized(s_tempoScore);

    // [THEN] It builds the tempo curve
    checkCurvesMatch(controller.automationData()->curve(TEMPO_KEY), expectedTempoCurve());

    // [WHEN] Its tempo curve is replaced by a stale one, and it's asked again
    AutomationCurveMap staleCurves;
    staleCurves[TEMPO_KEY][0] = generatedPoint(0.0, TEMPO_1_5);
    AutomationDataPtr staleData = std::make_shared<AutomationData>();
    staleData->setCurves(staleCurves);
    controller.setAutomationData(staleData);

    controller.ensureInitialized(s_tempoScore);

    // [THEN] Nothing is rebuilt
    checkCurvesMatch(controller.automationData()->curve(TEMPO_KEY), staleCurves[TEMPO_KEY]);
}
