/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/barline.h"
#include "engraving/dom/chordrest.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/stafftypechange.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

static const String MEASURE_DATA_DIR(u"measure_data/");

class Engraving_StaffTypeChangeTests : public ::testing::Test
{
};

/**
 * @brief Engraving_StaffTypeChangeTests_insertionTick_returnsMeasureTick
 * @details This test verifies StaffTypeChange::insertionTick(measure) for a real
 *          score measure. Expected behavior: it returns the absolute start tick of
 *          that measure (default insertion position).
 */
TEST_F(Engraving_StaffTypeChangeTests, insertionTick_returnsMeasureTick)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    EXPECT_EQ(StaffTypeChange::insertionTick(measure), measure->tick());

    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_isAtMeasureStart_true
 * @details This test verifies isAtMeasureStart() when the StaffTypeChange tick is
 *          set to the owning measure start tick. Expected behavior: it reports
 *          true.
 */
TEST_F(Engraving_StaffTypeChangeTests, isAtMeasureStart_true)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    StaffTypeChange* stc = Factory::createStaffTypeChange(measure);
    stc->setProperty(Pid::TICK, PropertyValue(measure->tick()));

    EXPECT_TRUE(stc->isAtMeasureStart());

    delete stc;
    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_isAtMeasureStart_false
 * @details This test verifies isAtMeasureStart() for a mid-measure placement
 *          (beat 2). Expected behavior: it reports false.
 */
TEST_F(Engraving_StaffTypeChangeTests, isAtMeasureStart_false)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    // beat 2 of the measure: one quarter note after the measure's start tick
    StaffTypeChange* stc = Factory::createStaffTypeChange(measure);
    stc->setProperty(Pid::TICK, PropertyValue(measure->tick() + Fraction(1, 4)));

    EXPECT_FALSE(stc->isAtMeasureStart());

    delete stc;
    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_rtick_midMeasure
 * @details This test verifies relative tick computation via rtick() for a
 *          mid-measure StaffTypeChange. Expected behavior: rtick() equals
 *          stc->tick() - measure->tick().
 */
TEST_F(Engraving_StaffTypeChangeTests, rtick_midMeasure)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    const Fraction offset(1, 4); // one quarter note into the measure

    StaffTypeChange* stc = Factory::createStaffTypeChange(measure);
    stc->setProperty(Pid::TICK, PropertyValue(measure->tick() + offset));

    EXPECT_EQ(stc->rtick(), offset);

    delete stc;
    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_saveReadScore_preservesMidMeasureTick
 * @details This test verifies full score serialization/deserialization for a
 *          mid-measure StaffTypeChange. Expected behavior: after save/read
 *          roundtrip, the element exists at the same absolute tick.
 */
TEST_F(Engraving_StaffTypeChangeTests, saveReadScore_preservesMidMeasureTick)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    const Fraction tick = measure->tick() + Fraction(1, 4);

    StaffTypeChange* stc = Factory::createStaffTypeChange(measure);
    stc->setTrack(0);
    stc->setProperty(Pid::TICK, PropertyValue(tick));
    measure->add(stc);

    // Round-trip through XML to validate persisted tick semantics.
    const auto uniqueId = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::filesystem::path roundtripPathStd
        = std::filesystem::temp_directory_path()
          / ("stafftypechange-roundtrip-" + std::to_string(uniqueId) + ".mscx");
    const String roundtripPath = String::fromStdString(roundtripPathStd.string());
    ASSERT_TRUE(ScoreRW::saveScore(score, roundtripPath));

    MasterScore* restoredScore = ScoreRW::readScore(roundtripPath, true);
    ASSERT_TRUE(restoredScore);

    Measure* restoredMeasure = restoredScore->firstMeasure();
    ASSERT_TRUE(restoredMeasure);

    const StaffTypeChange* restored = restoredMeasure->staffTypeChangeAt(0, tick);
    ASSERT_TRUE(restored);

    EXPECT_EQ(restored->tick(), tick);

    delete restoredScore;
    delete score;

    // Clean up temp file
    std::filesystem::remove(roundtripPathStd);
}

/**
 * @brief Engraving_StaffTypeChangeTests_getSetProperty_tick
 * @details This test verifies the Pid::TICK property accessors on
 *          StaffTypeChange. Expected behavior: setProperty(Pid::TICK, value)
 *          and getProperty(Pid::TICK) round-trip exactly.
 */
TEST_F(Engraving_StaffTypeChangeTests, getSetProperty_tick)
{
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);
    ASSERT_TRUE(score);

    StaffTypeChange* stc = Factory::createStaffTypeChange(score->dummy()->measure());

    const Fraction tick(3, 8);
    stc->setProperty(Pid::TICK, PropertyValue(tick));

    EXPECT_EQ(stc->getProperty(Pid::TICK).value<Fraction>(), tick);

    delete stc;
    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_propertyDefaultTick_matchesMeasureStart
 * @details This test verifies the default value for Pid::TICK in a
 *          measure-owned StaffTypeChange. Expected behavior:
 *          propertyDefault(Pid::TICK) equals the owning measure start tick.
 */
TEST_F(Engraving_StaffTypeChangeTests, propertyDefaultTick_matchesMeasureStart)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    StaffTypeChange* stc = Factory::createStaffTypeChange(measure);

    EXPECT_EQ(stc->propertyDefault(Pid::TICK).value<Fraction>(), measure->tick());

    delete stc;
    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_propertySetTick_updatesTickSemantics
 * @details This test verifies semantic consistency after setting Pid::TICK
 *          through setProperty at a mid-measure value. Expected behavior:
 *          tick() updates, rtick() matches relative offset, and
 *          isAtMeasureStart() becomes false.
 */
TEST_F(Engraving_StaffTypeChangeTests, propertySetTick_updatesTickSemantics)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    StaffTypeChange* stc = Factory::createStaffTypeChange(measure);
    const Fraction midMeasureTick = measure->tick() + Fraction(3, 8);

    // Drive state via property API (not direct setter) to cover undo/property path.
    ASSERT_TRUE(stc->setProperty(Pid::TICK, PropertyValue(midMeasureTick)));

    EXPECT_EQ(stc->tick(), midMeasureTick);
    EXPECT_EQ(stc->rtick(), Fraction(3, 8));
    EXPECT_FALSE(stc->isAtMeasureStart());

    delete stc;
    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_unsupportedProperty_isRejectedOrUnchanged
 * @details This test verifies behavior when setting an unrelated property
 *          (Pid::PITCH) on StaffTypeChange. Expected behavior: tick state is
 *          unchanged regardless of whether setProperty rejects or accepts.
 */
TEST_F(Engraving_StaffTypeChangeTests, unsupportedProperty_isRejectedOrUnchanged)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    StaffTypeChange* stc = Factory::createStaffTypeChange(measure);
    const Fraction originalTick = stc->tick();

    stc->setProperty(Pid::PITCH, PropertyValue(60));
    EXPECT_EQ(stc->tick(), originalTick);

    delete stc;
    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_canAddStaffTypeChange_rejectsDuplicateTick
 * @details This test verifies duplicate guarding in
 *          Measure::canAddStaffTypeChange for the same staff and tick.
 *          Expected behavior: after adding one StaffTypeChange at that tick,
 *          the same query returns false.
 */
TEST_F(Engraving_StaffTypeChangeTests, canAddStaffTypeChange_rejectsDuplicateTick)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    const staff_idx_t staffIdx = 0;
    const Fraction tick = measure->tick() + Fraction(1, 4);

    // Baseline: first insertion at this tick should be permitted.
    ASSERT_TRUE(measure->canAddStaffTypeChange(staffIdx, tick));

    // Add an STC directly into the measure's element list.
    StaffTypeChange* stc = Factory::createStaffTypeChange(measure);
    stc->setTrack(staffIdx * VOICES);
    stc->setProperty(Pid::TICK, PropertyValue(tick));
    measure->add(stc);

    EXPECT_FALSE(measure->canAddStaffTypeChange(staffIdx, tick));

    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_canAddStaffTypeChange_allowsDifferentTick
 * @details This test verifies that duplicate checks are tick-specific.
 *          Expected behavior: a second StaffTypeChange at a different tick on
 *          the same staff is still allowed.
 */
TEST_F(Engraving_StaffTypeChangeTests, canAddStaffTypeChange_allowsDifferentTick)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    const staff_idx_t staffIdx = 0;
    const Fraction tick1 = measure->tick() + Fraction(1, 4);
    const Fraction tick2 = measure->tick() + Fraction(2, 4);

    StaffTypeChange* stc = Factory::createStaffTypeChange(measure);
    stc->setTrack(staffIdx * VOICES);
    stc->setProperty(Pid::TICK, PropertyValue(tick1));
    measure->add(stc);

    // Duplicate checks must be local to a single absolute tick.
    EXPECT_TRUE(measure->canAddStaffTypeChange(staffIdx, tick2));

    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_staffTypeChangeAt_exactTick
 * @details This test verifies Measure::staffTypeChangeAt lookup behavior.
 *          Expected behavior: exact tick query returns the inserted element,
 *          while non-matching ticks return nullptr.
 */
TEST_F(Engraving_StaffTypeChangeTests, staffTypeChangeAt_exactTick)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    const staff_idx_t staffIdx = 0;
    const Fraction tick = measure->tick() + Fraction(1, 2);

    StaffTypeChange* stc = Factory::createStaffTypeChange(measure);
    stc->setTrack(staffIdx * VOICES);
    stc->setProperty(Pid::TICK, PropertyValue(tick));
    measure->add(stc);

    EXPECT_EQ(measure->staffTypeChangeAt(staffIdx, tick), stc);
    EXPECT_EQ(measure->staffTypeChangeAt(staffIdx, tick + Fraction(1, 4)), nullptr);
    EXPECT_EQ(measure->staffTypeChangeAt(staffIdx, measure->tick()), nullptr);

    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_moveTicks_propagatesShift
 * @details This test verifies tick shifting through StaffTypeChange::moveTicks.
 *          Expected behavior: stored tick is incremented by exactly diff.
 */
TEST_F(Engraving_StaffTypeChangeTests, moveTicks_propagatesShift)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    const Fraction initial = measure->tick() + Fraction(1, 4);
    const Fraction diff(1, 4);

    StaffTypeChange* stc = Factory::createStaffTypeChange(measure);
    stc->setProperty(Pid::TICK, PropertyValue(initial));

    stc->moveTicks(diff);

    EXPECT_EQ(stc->tick(), initial + diff);

    delete stc;
    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_endMeasureUsesStaffTypeActiveAtEnd
 * @details This test verifies the effective staff type near measure end after a
 *          mid-measure StaffTypeChange is inserted. Expected behavior: querying
 *          through the end-barline element path resolves to the new staff type,
 *          while querying at measure start resolves to the original staff type;
 *          specifically, end-barline resolution must match the staff type active
 *          at the last tick in the measure.
 */
TEST_F(Engraving_StaffTypeChangeTests, endMeasureUsesStaffTypeActiveAtEnd)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    Staff* staff = score->staff(0);
    ASSERT_TRUE(staff);

    const StaffType* originalStaffType = staff->staffType(measure->tick());
    ASSERT_TRUE(originalStaffType);
    const int originalLines = originalStaffType->lines();

    const Fraction changeTick = measure->tick() + Fraction(1, 2);

    StaffTypeChange* stc = Factory::createStaffTypeChange(measure);
    StaffType* modified = new StaffType(*originalStaffType);
    const int changedLines = originalLines == 1 ? 2 : 1;
    modified->setLines(changedLines);

    stc->setTrack(0);
    stc->setProperty(Pid::TICK, PropertyValue(changeTick));
    stc->setStaffType(modified, true);
    measure->add(stc);

    Segment* endBarSegment = measure->findSegment(SegmentType::EndBarLine, measure->endTick());
    ASSERT_TRUE(endBarSegment);

    BarLine* endBarline = toBarLine(endBarSegment->element(0));
    ASSERT_TRUE(endBarline);

    const Fraction lastTickInMeasure = Fraction::fromTicks(measure->endTick().ticks() - 1);
    const int expectedEndLines = staff->staffType(lastTickInMeasure)->lines();

    // End-barline resolution should reflect the staff type active at final in-measure tick.
    EXPECT_EQ(staff->staffType(measure->tick())->lines(), originalLines);
    EXPECT_EQ(staff->staffType(changeTick)->lines(), changedLines);
    EXPECT_EQ(staff->staffTypeForElement(endBarline)->lines(), expectedEndLines);

    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_insertionTick_dropPosMapsToExpectedTick
 * @details This test verifies x-position to tick mapping used for mid-measure
 *          drop placement. Expected behavior: a drop at tick2pos(expectedTick)
 *          resolves to the nearest tick (within one tick of expectedTick);
 *          positions left/right of measure bounds clamp to startTick and
 *          lastTickInMeasure respectively.
 */
TEST_F(Engraving_StaffTypeChangeTests, insertionTick_dropPosMapsToExpectedTick)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    score->doLayout();

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    const Fraction expectedTick = measure->tick() + Fraction(1, 2);

    // Exact coordinate at expectedTick should map to the same (or nearest) tick.
    const PointF exactDrop(
        measure->canvasPos().x() + measure->tick2pos(expectedTick),
        measure->canvasPos().y());
    const int expectedTicks = expectedTick.ticks();
    const int mappedTicks = StaffTypeChange::insertionTick(measure, exactDrop).ticks();
    EXPECT_LE(std::abs(mappedTicks - expectedTicks), 1);

    const PointF leftDrop(measure->canvasPos().x() - 100.0, measure->canvasPos().y());

    // Drops left of the measure should clamp to measure start.
    EXPECT_EQ(StaffTypeChange::insertionTick(measure, leftDrop), measure->tick());

    const int lastTickInMeasure = measure->endTick().ticks() - 1;
    const PointF rightDrop(
        measure->canvasPos().x() + measure->tick2pos(Fraction::fromTicks(lastTickInMeasure)) + 100.0,
        measure->canvasPos().y());

    // Drops right of the measure should clamp to the last in-measure tick.
    EXPECT_EQ(StaffTypeChange::insertionTick(measure, rightDrop), Fraction::fromTicks(lastTickInMeasure));

    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_twoSTCs_differentTicks_bothAccepted
 * @details This test verifies coexistence of multiple StaffTypeChange elements
 *          on one staff at different ticks. Expected behavior: both insertion
 *          checks pass for distinct ticks, and both are retrievable by
 *          staffTypeChangeAt.
 */
TEST_F(Engraving_StaffTypeChangeTests, twoSTCs_differentTicks_bothAccepted)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    const staff_idx_t staffIdx = 0;
    const Fraction tick1 = measure->tick() + Fraction(1, 4);
    const Fraction tick2 = measure->tick() + Fraction(3, 4);

    ASSERT_TRUE(measure->canAddStaffTypeChange(staffIdx, tick1));
    ASSERT_TRUE(measure->canAddStaffTypeChange(staffIdx, tick2));

    StaffTypeChange* stc1 = Factory::createStaffTypeChange(measure);
    stc1->setTrack(staffIdx * VOICES);
    stc1->setProperty(Pid::TICK, PropertyValue(tick1));
    measure->add(stc1);

    // Adding the first STC must not block the second tick.
    EXPECT_TRUE(measure->canAddStaffTypeChange(staffIdx, tick2));

    StaffTypeChange* stc2 = Factory::createStaffTypeChange(measure);
    stc2->setTrack(staffIdx * VOICES);
    stc2->setProperty(Pid::TICK, PropertyValue(tick2));
    measure->add(stc2);

    EXPECT_NE(measure->staffTypeChangeAt(staffIdx, tick1), nullptr);
    EXPECT_NE(measure->staffTypeChangeAt(staffIdx, tick2), nullptr);

    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_transitionMeasureReportsMidMeasureTick
 * @details Verifies branch selection metadata for a start measure containing a
 *          mid-measure StaffTypeChange. Expected behavior: the measure is
 *          reported as a transition measure and transitionTick matches the
 *          inserted StaffTypeChange tick.
 */
TEST_F(Engraving_StaffTypeChangeTests, transitionMeasureReportsMidMeasureTick)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    ASSERT_TRUE(m1);
    Measure* m2 = m1->nextMeasure();
    ASSERT_TRUE(m2);

    Staff* staff = score->staff(0);
    ASSERT_TRUE(staff);

    const Fraction changeTick = m1->tick() + Fraction(1, 4);
    const StaffType* original = staff->staffType(m1->tick());
    ASSERT_TRUE(original);

    StaffTypeChange* stc = Factory::createStaffTypeChange(m1);
    StaffType* modified = new StaffType(*original);
    modified->setStepOffset(original->stepOffset() + 2);

    stc->setTrack(0);
    stc->setProperty(Pid::TICK, PropertyValue(changeTick));
    stc->setStaffType(modified, true);
    m1->add(stc);

    Fraction transitionTick;

    // Transition metadata should report the earliest in-measure STC tick.
    EXPECT_TRUE(m1->isStaffTypeTransitionMeasure(0, &transitionTick));
    EXPECT_EQ(transitionTick, changeTick);

    // Only the start measure with a mid-measure STC should be marked transition.
    EXPECT_FALSE(m2->isStaffTypeTransitionMeasure(0));

    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_followingMeasureUsesChangedTypeWithoutTransitionFlag
 * @details Verifies the post-transition branch contract. Expected behavior:
 *          following measures are not transition measures, but the changed
 *          staff type is active for their elements from measure start.
 */
TEST_F(Engraving_StaffTypeChangeTests, followingMeasureUsesChangedTypeWithoutTransitionFlag)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    ASSERT_TRUE(m1);
    Measure* m2 = m1->nextMeasure();
    ASSERT_TRUE(m2);

    Staff* staff = score->staff(0);
    ASSERT_TRUE(staff);

    const Fraction changeTick = m1->tick() + Fraction(1, 4);
    const StaffType* original = staff->staffType(m1->tick());
    ASSERT_TRUE(original);

    const int changedStepOffset = original->stepOffset() + 3;

    StaffTypeChange* stc = Factory::createStaffTypeChange(m1);
    StaffType* modified = new StaffType(*original);
    modified->setStepOffset(changedStepOffset);

    stc->setTrack(0);
    stc->setProperty(Pid::TICK, PropertyValue(changeTick));
    stc->setStaffType(modified, true);
    m1->add(stc);

    score->doLayout();

    EXPECT_TRUE(m1->isStaffTypeTransitionMeasure(0));
    EXPECT_FALSE(m2->isStaffTypeTransitionMeasure(0));

    const StaffType* m2StartType = staff->staffType(m2->tick());
    ASSERT_TRUE(m2StartType);
    EXPECT_EQ(m2StartType->stepOffset(), changedStepOffset);

    ChordRest* m2CR = score->findCR(m2->tick(), 0);
    ASSERT_TRUE(m2CR);

    const StaffType* m2ElementType = staff->staffTypeForElement(m2CR);
    ASSERT_TRUE(m2ElementType);
    EXPECT_EQ(m2ElementType->stepOffset(), changedStepOffset);

    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_midMeasureTicks_excludeStartAndEnd
 * @details Verifies that transition tick collection only reports strict
 *          mid-measure StaffTypeChange ticks. Expected behavior: measure start
 *          and measure end ticks are excluded.
 */
TEST_F(Engraving_StaffTypeChangeTests, midMeasureTicks_excludeStartAndEnd)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    const Fraction startTick = measure->tick();
    const Fraction midTick = measure->tick() + Fraction(1, 4);
    const Fraction endTick = measure->endTick();

    StaffTypeChange* startStc = Factory::createStaffTypeChange(measure);
    startStc->setTrack(0);
    startStc->setProperty(Pid::TICK, PropertyValue(startTick));
    measure->add(startStc);

    StaffTypeChange* midStc = Factory::createStaffTypeChange(measure);
    midStc->setTrack(0);
    midStc->setProperty(Pid::TICK, PropertyValue(midTick));
    measure->add(midStc);

    StaffTypeChange* endStc = Factory::createStaffTypeChange(measure);
    endStc->setTrack(0);
    endStc->setProperty(Pid::TICK, PropertyValue(endTick));
    measure->add(endStc);

    const std::vector<Fraction> ticks = measure->midMeasureStaffTypeChangeTicks(0);
    const std::vector<Fraction> expected { midTick };
    EXPECT_EQ(ticks, expected);

    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_midMeasureTicks_reportsEarliest
 * @details Verifies current transition tick collection behavior when multiple
 *          mid-measure StaffTypeChange elements are inserted.
 *          Expected behavior: all mid-measure ticks are reported in order,
 *          while transition metadata still resolves to the earliest tick.
 */
TEST_F(Engraving_StaffTypeChangeTests, midMeasureTicks_reportsEarliest)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    const Fraction tick1 = measure->tick() + Fraction(1, 4);
    const Fraction tick2 = measure->tick() + Fraction(3, 8);

    StaffTypeChange* stc1 = Factory::createStaffTypeChange(measure);
    stc1->setTrack(0);
    stc1->setProperty(Pid::TICK, PropertyValue(tick1));
    measure->add(stc1);

    StaffTypeChange* stc2 = Factory::createStaffTypeChange(measure);
    stc2->setTrack(0);
    stc2->setProperty(Pid::TICK, PropertyValue(tick2));
    measure->add(stc2);

    const std::vector<Fraction> ticks = measure->midMeasureStaffTypeChangeTicks(0);
    const std::vector<Fraction> expected { tick1, tick2 };
    EXPECT_EQ(ticks, expected);

    Fraction transitionTick;
    EXPECT_TRUE(measure->isStaffTypeTransitionMeasure(0, &transitionTick));
    EXPECT_EQ(transitionTick, tick1);

    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_isPostTransitionTick_boundary
 * @details Verifies boundary behavior around the first transition tick.
 *          Expected behavior: false before, true at and after transition tick.
 */
TEST_F(Engraving_StaffTypeChangeTests, isPostTransitionTick_boundary)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    const Fraction transitionTick = measure->tick() + Fraction(1, 4);

    StaffTypeChange* stc = Factory::createStaffTypeChange(measure);
    stc->setTrack(0);
    stc->setProperty(Pid::TICK, PropertyValue(transitionTick));
    measure->add(stc);

    EXPECT_FALSE(measure->isPostStaffTypeTransitionTick(0, transitionTick - Fraction(1, 16)));
    EXPECT_TRUE(measure->isPostStaffTypeTransitionTick(0, transitionTick));
    EXPECT_TRUE(measure->isPostStaffTypeTransitionTick(0, transitionTick + Fraction(1, 16)));

    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_isPostTransitionTick_falseWithoutTransition
 * @details Verifies behavior when a measure has no mid-measure transition.
 *          Expected behavior: query is always false.
 */
TEST_F(Engraving_StaffTypeChangeTests, isPostTransitionTick_falseWithoutTransition)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    EXPECT_FALSE(measure->isPostStaffTypeTransitionTick(0, measure->tick()));
    EXPECT_FALSE(measure->isPostStaffTypeTransitionTick(0, measure->tick() + Fraction(1, 2)));

    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_transitionTick_usesFirstMidMeasureChange
 * @details Verifies transition tick selection with multiple mid-measure changes.
 *          Expected behavior: first transition tick is reported.
 */
TEST_F(Engraving_StaffTypeChangeTests, transitionTick_usesFirstMidMeasureChange)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    const Fraction tick1 = measure->tick() + Fraction(1, 4);
    const Fraction tick2 = measure->tick() + Fraction(3, 4);

    StaffTypeChange* stc2 = Factory::createStaffTypeChange(measure);
    stc2->setTrack(0);
    stc2->setProperty(Pid::TICK, PropertyValue(tick2));
    measure->add(stc2);

    StaffTypeChange* stc1 = Factory::createStaffTypeChange(measure);
    stc1->setTrack(0);
    stc1->setProperty(Pid::TICK, PropertyValue(tick1));
    measure->add(stc1);

    Fraction transitionTick;
    EXPECT_TRUE(measure->isStaffTypeTransitionMeasure(0, &transitionTick));
    EXPECT_EQ(transitionTick, tick1);

    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_roundTrip_preservesFirstTransitionTick
 * @details Verifies save/read roundtrip preserves transition semantics for
 *          measures with multiple inserted mid-measure StaffTypeChange values.
 *          Expected behavior: transition metadata remains valid and reports
 *          the first transition tick.
 */
TEST_F(Engraving_StaffTypeChangeTests, roundTrip_preservesFirstTransitionTick)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    const Fraction tick1 = measure->tick() + Fraction(1, 4);
    const Fraction tick2 = measure->tick() + Fraction(3, 4);

    StaffTypeChange* stc1 = Factory::createStaffTypeChange(measure);
    stc1->setTrack(0);
    stc1->setProperty(Pid::TICK, PropertyValue(tick1));
    measure->add(stc1);

    StaffTypeChange* stc2 = Factory::createStaffTypeChange(measure);
    stc2->setTrack(0);
    stc2->setProperty(Pid::TICK, PropertyValue(tick2));
    measure->add(stc2);

    // Persist and reload to ensure transition ordering survives serialization.
    const auto uniqueId = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::filesystem::path roundtripPathStd
        = std::filesystem::temp_directory_path()
          / ("stafftypechange-roundtrip-multi-" + std::to_string(uniqueId) + ".mscx");
    const String roundtripPath = String::fromStdString(roundtripPathStd.string());
    ASSERT_TRUE(ScoreRW::saveScore(score, roundtripPath));

    MasterScore* restoredScore = ScoreRW::readScore(roundtripPath, true);
    ASSERT_TRUE(restoredScore);

    Measure* restoredMeasure = restoredScore->firstMeasure();
    ASSERT_TRUE(restoredMeasure);

    Fraction transitionTick;
    EXPECT_TRUE(restoredMeasure->isStaffTypeTransitionMeasure(0, &transitionTick));
    EXPECT_EQ(transitionTick, tick1);

    delete restoredScore;
    delete score;

    // Clean up temp file
    std::filesystem::remove(roundtripPathStd);
}

/**
 * @brief Engraving_StaffTypeChangeTests_insertionTick_atEndTick_isRejected
 * @details Verifies that an STC placed at the measure's end tick is excluded
 *          from the mid-measure tick collection. Expected behavior:
 *          midMeasureStaffTypeChangeTicks does not include endTick.
 */
TEST_F(Engraving_StaffTypeChangeTests, insertionTick_atEndTick_isRejected)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    const Fraction endTick = measure->endTick();
    const staff_idx_t staffIdx = 0;

    // A StaffTypeChange placed at the end tick is not a valid mid-measure tick.
    StaffTypeChange* stc = Factory::createStaffTypeChange(measure);
    stc->setTrack(staffIdx * VOICES);
    stc->setProperty(Pid::TICK, PropertyValue(endTick));
    measure->add(stc);

    const std::vector<Fraction> midTicks = measure->midMeasureStaffTypeChangeTicks(staffIdx);
    const auto it = std::find(midTicks.begin(), midTicks.end(), endTick);
    EXPECT_EQ(it, midTicks.end());

    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_canAddStaffTypeChange_sameTick_differentStaff_isAllowed
 * @details Verifies that duplicate guarding is per-staff. Expected behavior:
 *          adding an STC on staff 0 at a given tick does not block the same
 *          tick on staff 1.
 */
TEST_F(Engraving_StaffTypeChangeTests, canAddStaffTypeChange_sameTick_differentStaff_isAllowed)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);
    ASSERT_GT(score->nstaves(), 1);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    const Fraction tick = measure->tick() + Fraction(1, 4);

    StaffTypeChange* stc0 = Factory::createStaffTypeChange(measure);
    stc0->setTrack(0);
    stc0->setProperty(Pid::TICK, PropertyValue(tick));
    measure->add(stc0);

    // Same tick, same staff — must be blocked.
    EXPECT_FALSE(measure->canAddStaffTypeChange(0, tick));

    // Same tick, different staff — allowed.
    EXPECT_TRUE(measure->canAddStaffTypeChange(1, tick));

    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_insertionTick_nullMeasure_returnsFractionZero
 * @details Verifies that insertionTick(nullptr) is robust and does not crash.
 *          Expected behavior: returns Fraction(0, 1).
 */
TEST_F(Engraving_StaffTypeChangeTests, insertionTick_nullMeasure_returnsFractionZero)
{
    const Fraction result = StaffTypeChange::insertionTick(nullptr);
    EXPECT_EQ(result, Fraction(0, 1));
}

/**
 * @brief Engraving_StaffTypeChangeTests_insertionTick_nullMeasureWithPos_returnsFractionZero
 * @details Verifies that insertionTick(nullptr, pos) is robust and does not crash.
 *          Expected behavior: returns Fraction(0, 1).
 */
TEST_F(Engraving_StaffTypeChangeTests, insertionTick_nullMeasureWithPos_returnsFractionZero)
{
    const Fraction result = StaffTypeChange::insertionTick(nullptr, PointF(100.0, 0.0));
    EXPECT_EQ(result, Fraction(0, 1));
}

/**
 * @brief Engraving_StaffTypeChangeTests_moveTicks_zeroDiff_isNoOp
 * @details Verifies that moveTicks with Fraction(0, 1) leaves the stored tick
 *          unchanged. Expected behavior: tick is identical before and after.
 */
TEST_F(Engraving_StaffTypeChangeTests, moveTicks_zeroDiff_isNoOp)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    const Fraction initial = measure->tick() + Fraction(1, 4);

    StaffTypeChange* stc = Factory::createStaffTypeChange(measure);
    stc->setProperty(Pid::TICK, PropertyValue(initial));

    stc->moveTicks(Fraction(0, 1));

    EXPECT_EQ(stc->tick(), initial);

    delete stc;
    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_multipleSTCs_sameTick_deduplicatedInMidMeasureTicks
 * @details Verifies behavior when multiple StaffTypeChange elements are forced
 *          to the same staff/tick by direct insertion into the measure. This
 *          documents the defensive behavior expected when callers bypass
 *          canAddStaffTypeChange(). Expected behavior: exact tick lookup still
 *          resolves, and midMeasureStaffTypeChangeTicks reports that tick once
 *          (deduplicated for transition calculations).
 */
TEST_F(Engraving_StaffTypeChangeTests, multipleSTCs_sameTick_deduplicatedInMidMeasureTicks)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    const staff_idx_t staffIdx = 0;
    const Fraction sharedTick = measure->tick() + Fraction(1, 4);

    // Intentionally bypass duplicate guard to validate downstream robustness.
    StaffTypeChange* stc1 = Factory::createStaffTypeChange(measure);
    stc1->setTrack(staffIdx * VOICES);
    stc1->setProperty(Pid::TICK, PropertyValue(sharedTick));
    measure->add(stc1);

    StaffTypeChange* stc2 = Factory::createStaffTypeChange(measure);
    stc2->setTrack(staffIdx * VOICES);
    stc2->setProperty(Pid::TICK, PropertyValue(sharedTick));
    measure->add(stc2);

    // Exact lookup remains valid even when duplicates exist at the same tick.
    EXPECT_NE(measure->staffTypeChangeAt(staffIdx, sharedTick), nullptr);

    // Transition tick collection must expose a stable deduplicated view.
    const std::vector<Fraction> ticks = measure->midMeasureStaffTypeChangeTicks(staffIdx);
    const std::vector<Fraction> expected { sharedTick };
    EXPECT_EQ(ticks, expected);

    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_staffTypeChange_coexistsWithClefAndKeySig
 * @details Verifies StaffTypeChange behavior in a measure containing other
 *          measure elements. Expected behavior: adding clef and key signature
 *          elements at measure start does not affect StaffTypeChange lookup or
 *          duplicate guarding at a mid-measure tick. This protects against
 *          regressions where segment-level element additions interfere with
 *          measure-level StaffTypeChange bookkeeping.
 */
TEST_F(Engraving_StaffTypeChangeTests, staffTypeChange_coexistsWithClefAndKeySig)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    const staff_idx_t staffIdx = 0;
    const track_idx_t track = staffIdx * VOICES;
    const Fraction startTick = measure->tick();
    const Fraction stcTick = startTick + Fraction(1, 4);

    // Add representative segment-based elements at measure start.
    Segment* clefSeg = measure->undoGetSegment(SegmentType::Clef, startTick);
    ASSERT_TRUE(clefSeg);
    Clef* clef = Factory::createClef(score->dummy()->segment());
    clef->setTrack(track);
    clefSeg->add(clef);

    Segment* keySigSeg = measure->undoGetSegment(SegmentType::KeySig, startTick);
    ASSERT_TRUE(keySigSeg);
    KeySig* keySig = Factory::createKeySig(score->dummy()->segment());
    keySig->setTrack(track);
    keySig->setKey(Key::C);
    keySigSeg->add(keySig);

    // Add a mid-measure staff type change on the same staff/track.
    StaffTypeChange* stc = Factory::createStaffTypeChange(measure);
    stc->setTrack(track);
    stc->setProperty(Pid::TICK, PropertyValue(stcTick));
    measure->add(stc);

    // STC APIs should behave as if other measure elements were absent.
    EXPECT_NE(measure->staffTypeChangeAt(staffIdx, stcTick), nullptr);
    EXPECT_FALSE(measure->canAddStaffTypeChange(staffIdx, stcTick));

    // Sanity check that the companion elements are really present.
    ASSERT_TRUE(clefSeg->element(track));
    EXPECT_TRUE(clefSeg->element(track)->isClef());
    ASSERT_TRUE(keySigSeg->element(track));
    EXPECT_TRUE(keySigSeg->element(track)->isKeySig());

    delete score;
}

/**
 * @brief Engraving_StaffTypeChangeTests_largeCountQueries_completeWithinBudget
 * @details Stress/performance sanity test for measures with many mid-measure
 *          StaffTypeChange elements. Expected behavior: query APIs return
 *          expected results and complete within a conservative time budget.
 *          This is intended as a regression tripwire for algorithmic slowdowns,
 *          not as a microbenchmark.
 */
TEST_F(Engraving_StaffTypeChangeTests, largeCountQueries_completeWithinBudget)
{
    MasterScore* score = ScoreRW::readScore(MEASURE_DATA_DIR + u"measure-1.mscx");
    ASSERT_TRUE(score);

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    const staff_idx_t staffIdx = 0;
    const track_idx_t track = staffIdx * VOICES;
    const int startTick = measure->tick().ticks();
    const int endTick = measure->endTick().ticks();
    const int availableMidTicks = std::max(0, endTick - startTick - 1);
    ASSERT_GT(availableMidTicks, 0);

    // Keep count bounded for stable execution on developer machines and CI.
    const int insertedCount = std::min(availableMidTicks, 512);
    for (int i = 0; i < insertedCount; ++i) {
        StaffTypeChange* stc = Factory::createStaffTypeChange(measure);
        stc->setTrack(track);
        stc->setProperty(Pid::TICK, PropertyValue(Fraction::fromTicks(startTick + 1 + i)));
        measure->add(stc);
    }

    // Exercise hot query paths repeatedly to detect major complexity regressions.
    const Fraction probeTick = Fraction::fromTicks(startTick + 1 + insertedCount / 2);
    const auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < 200; ++i) {
        const std::vector<Fraction> ticks = measure->midMeasureStaffTypeChangeTicks(staffIdx);
        EXPECT_EQ(static_cast<int>(ticks.size()), insertedCount);
        EXPECT_NE(measure->staffTypeChangeAt(staffIdx, probeTick), nullptr);
    }
    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - t0).count();

    // Optional perf guard: keep wall-clock assertions out of default CI/unit runs.
    if (std::getenv("MUSE_PERF_TESTS") != nullptr) {
        EXPECT_LT(elapsedMs, 5000);
    }

    delete score;
}
