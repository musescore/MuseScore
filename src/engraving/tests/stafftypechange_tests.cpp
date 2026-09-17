/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 */
#include <gtest/gtest.h>

#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/page.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftypechange.h"
#include "engraving/dom/system.h"
#include "engraving/editing/editdata.h"
#include "utils/scorerw.h"

using namespace mu::engraving;

//! Exercises the actual drag callbacks within an undoable score command.
static void dragChange(StaffTypeChange* change, Measure* destination)
{
    Score* score = change->score();
    score->startCmd(muse::TranslatableString::untranslatable("Move staff type change"));
    EditData edit(nullptr);
    change->startDrag(edit);
    edit.pos = destination->staffPageBoundingRect(change->staffIdx()).center() + destination->system()->page()->pos();
    edit.evtDelta = edit.pos - change->canvasPos();
    const bool validTarget = destination == change->measure() || destination->canAddStaffTypeChange(change->staffIdx());
    change->drag(edit);
    const auto anchors = change->dragAnchorLines();
    EXPECT_EQ(anchors.size(), validTarget ? 1u : 0u);
    if (!anchors.empty()) {
        EXPECT_EQ(anchors.front().p1(), destination->staffPageBoundingRect(change->staffIdx()).topLeft()
                  + destination->system()->page()->pos());
        EXPECT_EQ(anchors.front().p2(), change->canvasPos());
    }
    change->endDrag(edit);
    EXPECT_TRUE(change->dragAnchorLines().empty());
    score->endCmd();
}

//! Moving a change must update the staff timeline and preserve its original baseline on undo.
TEST(Engraving_StaffTypeChangeTests, dragAndUndo)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(u"measure_data/checkMeasure.mscx"));
    ASSERT_NE(score, nullptr);
    Measure* first = score->firstMeasure();
    Measure* second = first->nextMeasure();
    Measure* third = second->nextMeasure();
    Staff* staff = score->staff(0);
    const int initialLines = staff->staffType(first->tick())->lines();
    StaffType* type = new StaffType(*staff->staffType(second->tick()));
    type->setLines(1);
    StaffTypeChange* change = Factory::createStaffTypeChange(second);
    change->setOwnershipParent(second);
    change->setTrack(0);
    change->setStaffType(type, true);
    score->startCmd(muse::TranslatableString::untranslatable("Add staff type change"));
    score->undoAddElement(change);
    score->endCmd();

    dragChange(change, second);
    EXPECT_EQ(change->measure(), second);
    EXPECT_EQ(change->offset(), PointF());
    EXPECT_EQ(staff->staffType(first->tick())->lines(), initialLines);
    EXPECT_EQ(staff->staffType(second->tick())->lines(), 1);

    dragChange(change, third);
    EXPECT_EQ(change->measure(), third);
    EXPECT_EQ(staff->staffType(second->tick())->lines(), initialLines);
    EXPECT_EQ(staff->staffType(third->tick())->lines(), 1);
    EXPECT_EQ(change->offset(), PointF());
    score->undoRedo(true, nullptr);
    EXPECT_EQ(change->measure(), second);
    EXPECT_EQ(staff->staffType(second->tick())->lines(), 1);

    dragChange(change, first);
    EXPECT_EQ(change->measure(), first);
    EXPECT_EQ(staff->staffType(first->tick())->lines(), 1);
    score->undoRedo(true, nullptr);
    EXPECT_EQ(change->measure(), second);
    EXPECT_EQ(staff->staffType(first->tick())->lines(), initialLines);
    score->undoRedo(false, nullptr);
    EXPECT_EQ(change->measure(), first);
    EXPECT_EQ(staff->staffType(first->tick())->lines(), 1);

    dragChange(change, third);
    EXPECT_EQ(change->measure(), third);
    EXPECT_EQ(staff->staffType(first->tick())->lines(), 1);
    score->undoRedo(true, nullptr);
    EXPECT_EQ(change->measure(), first);

    StaffTypeChange* occupied = Factory::createStaffTypeChange(second);
    occupied->setOwnershipParent(second);
    occupied->setTrack(0);
    score->startCmd(muse::TranslatableString::untranslatable("Add another staff type change"));
    score->undoAddElement(occupied);
    score->endCmd();
    dragChange(change, second);
    EXPECT_EQ(change->measure(), first);
    EXPECT_EQ(change->offset(), PointF());
}
