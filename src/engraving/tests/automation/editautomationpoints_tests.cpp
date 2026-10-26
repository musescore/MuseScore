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

#include "engraving/automation/automationdata.h"
#include "engraving/automation/internal/scoreautomationcontroller.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/staff.h"
#include "engraving/editing/editautomationpoints.h"

#include "utils/scorerw.h"
#include "automation/utils/automationtestutils.h"

using namespace mu::engraving;

static const String AUTOMATION_DATA_DIR(u"automation/data/");

// dynamics.mscx play order: M1-M5 (utick offset 0), then M5-M10 (utick offset 1920); M7 repeats M6
class EditAutomationPoints_Tests : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        s_score = ScoreRW::readScore(AUTOMATION_DATA_DIR + u"dynamics.mscx");
        ASSERT_TRUE(s_score);
    }

    static void TearDownTestSuite()
    {
        delete s_score;
        s_score = nullptr;
    }

protected:
    void SetUp() override
    {
        m_controller.init(s_score);
        m_key = AutomationCurveKey::staff(AutomationType::Dynamics, s_score->staff(0)->id());
    }

    const AutomationCurve& curve(const AutomationCurveKey& key) const
    {
        return m_controller.automationData()->curve(key);
    }

    static void checkRange(const std::optional<ChangedRange>& range, int tickFrom, int tickTo, staff_idx_t staffIdx)
    {
        ASSERT_TRUE(range.has_value());
        EXPECT_EQ(range->tickFrom, Fraction::fromTicks(tickFrom));
        EXPECT_EQ(range->tickTo, Fraction::fromTicks(tickTo));
        EXPECT_EQ(range->staffIdxFrom, staffIdx);
        EXPECT_EQ(range->staffIdxTo, staffIdx);
    }

    static MasterScore* s_score;

    ScoreAutomationController m_controller;
    AutomationCurveKey m_key;
};

MasterScore* EditAutomationPoints_Tests::s_score = nullptr;

// Redo writes the point and undo restores the curve, both reporting the edited tick on the edited staff
TEST_F(EditAutomationPoints_Tests, SetPoint_RedoUndo_RestoresCurve)
{
    // [GIVEN] M2 beat 2 (tick 2100) has no point and is played once
    const AutomationCurve curveBefore = curve(m_key);
    ASSERT_FALSE(curveBefore.contains(2100));

    const AutomationPoint point = customPoint(0.2, 0.3);
    EditAutomationPoints cmd(s_score, &m_controller, m_key, { { 2100, AutomationPointEdit::SetPoint { point } } });

    // [WHEN] The command is applied
    cmd.redo();

    // [THEN] The point is written and the range is that tick on staff 0
    const AutomationCurve& curveAfterEdit = curve(m_key);
    const auto it = curveAfterEdit.find(2100);
    ASSERT_TRUE(it != curveAfterEdit.cend());
    EXPECT_EQ(it->second, point);
    checkRange(cmd.changedRange(), 2100, 2100, 0);

    // [WHEN] The command is undone
    cmd.undo();

    // [THEN] The curve is back to its original state, with the same range
    checkCurvesMatch(curve(m_key), curveBefore);
    checkRange(cmd.changedRange(), 2100, 2100, 0);

    // [WHEN] The command is redone
    cmd.redo();

    // [THEN] The point is written again
    EXPECT_TRUE(curve(m_key).contains(2100));
}

// Moving a point reports both its old and new tick, and undo moves it back
TEST_F(EditAutomationPoints_Tests, MovePoint_RedoUndo_RangeCoversBothTicks)
{
    // [GIVEN] A user point at tick 2100
    const AutomationPoint point = customPoint(0.2, 0.3);
    EditAutomationPoints addCmd(s_score, &m_controller, m_key, { { 2100, AutomationPointEdit::SetPoint { point } } });
    addCmd.redo();
    const AutomationCurve curveBefore = curve(m_key);

    // [WHEN] It's moved to tick 2300
    EditAutomationPoints moveCmd(s_score, &m_controller, m_key,
                                 { { 2300, AutomationPointEdit::MovePoint { point, 2100 } } });
    moveCmd.redo();

    // [THEN] It's only at the new tick, and the range covers both ticks
    const AutomationCurve& curveAfterMove = curve(m_key);
    EXPECT_FALSE(curveAfterMove.contains(2100));
    EXPECT_TRUE(curveAfterMove.contains(2300));
    checkRange(moveCmd.changedRange(), 2100, 2300, 0);

    // [WHEN] The move is undone
    moveCmd.undo();

    // [THEN] The point is back at its old tick
    checkCurvesMatch(curve(m_key), curveBefore);
    checkRange(moveCmd.changedRange(), 2100, 2300, 0);

    addCmd.undo();
}

// A point in a repeated measure is copied to the other pass; the range stays on the edited raw tick
TEST_F(EditAutomationPoints_Tests, SetPoint_InRepeatedMeasure_UndoRemovesCopy)
{
    // [GIVEN] M5 is played in both passes: raw tick 7700 is utick 7700 and 9620
    const AutomationCurve curveBefore = curve(m_key);
    EditAutomationPoints cmd(s_score, &m_controller, m_key, { { 7700, AutomationPointEdit::SetPoint { customPoint(0.4, 0.4) } } });

    // [WHEN] The command is applied
    cmd.redo();

    // [THEN] Both passes get the point, and both uticks map back to raw tick 7700
    const AutomationCurve& curveAfterEdit = curve(m_key);
    EXPECT_TRUE(curveAfterEdit.contains(7700));
    EXPECT_TRUE(curveAfterEdit.contains(9620));
    checkRange(cmd.changedRange(), 7700, 7700, 0);

    // [WHEN] The command is undone
    cmd.undo();

    // [THEN] The point and its copy are gone
    checkCurvesMatch(curve(m_key), curveBefore);
    checkRange(cmd.changedRange(), 7700, 7700, 0);
}

// A point in a measure-repeat source is copied to the repeat measure, and the range covers the copy
TEST_F(EditAutomationPoints_Tests, SetPoint_InMeasureRepeatSource_RangeCoversCopy)
{
    // [GIVEN] M6 raw tick 9700 (utick 11620) is replayed by M7 at raw tick 11620 (utick 13540)
    const AutomationCurve curveBefore = curve(m_key);
    EditAutomationPoints cmd(s_score, &m_controller, m_key, { { 11620, AutomationPointEdit::SetPoint { customPoint(0.4, 0.4) } } });

    // [WHEN] The command is applied
    cmd.redo();

    // [THEN] M7 gets a copy, and the range spans from the edit to the copy
    const AutomationCurve& curveAfterEdit = curve(m_key);
    EXPECT_TRUE(curveAfterEdit.contains(11620));
    EXPECT_TRUE(curveAfterEdit.contains(13540));
    checkRange(cmd.changedRange(), 9700, 11620, 0);

    // [WHEN] The command is undone
    cmd.undo();

    // [THEN] The point and its copy are gone, and the range still covers the copy
    checkCurvesMatch(curve(m_key), curveBefore);
    checkRange(cmd.changedRange(), 9700, 11620, 0);
}

// Erasing a user point and undoing brings it back
TEST_F(EditAutomationPoints_Tests, ErasePoint_RedoUndo_RestoresPoint)
{
    // [GIVEN] A user point at tick 2100
    const AutomationPoint point = customPoint(0.2, 0.3);
    EditAutomationPoints addCmd(s_score, &m_controller, m_key, { { 2100, AutomationPointEdit::SetPoint { point } } });
    addCmd.redo();
    const AutomationCurve curveBefore = curve(m_key);

    // [WHEN] It's erased
    EditAutomationPoints eraseCmd(s_score, &m_controller, m_key, { { 2100, AutomationPointEdit::ErasePoint {} } });
    eraseCmd.redo();

    // [THEN] It's gone, and the range is its tick
    EXPECT_FALSE(curve(m_key).contains(2100));
    checkRange(eraseCmd.changedRange(), 2100, 2100, 0);

    // [WHEN] The erase is undone
    eraseCmd.undo();

    // [THEN] The point is back
    checkCurvesMatch(curve(m_key), curveBefore);

    addCmd.undo();
}

// A tempo edit isn't tied to a staff and affects the timing up to the end of the score
TEST_F(EditAutomationPoints_Tests, SetPoint_Tempo_RangeExtendsToScoreEnd)
{
    // [GIVEN] The score ends at the end of its last measure
    const Measure* lastMeasure = s_score->lastMeasure();
    ASSERT_TRUE(lastMeasure);
    const AutomationCurve curveBefore = curve(TEMPO_KEY);

    EditAutomationPoints cmd(s_score, &m_controller, TEMPO_KEY, { { 2100, AutomationPointEdit::SetPoint { customPoint(0.1, 0.1) } } });

    // [WHEN] The command is applied
    cmd.redo();

    // [THEN] The range runs from the edit to the end of the score, with no staff
    EXPECT_TRUE(curve(TEMPO_KEY).contains(2100));
    checkRange(cmd.changedRange(), 2100, lastMeasure->endTick().ticks(), muse::nidx);

    // [WHEN] The command is undone
    cmd.undo();

    // [THEN] The tempo curve is back to its original state
    checkCurvesMatch(curve(TEMPO_KEY), curveBefore);
}
