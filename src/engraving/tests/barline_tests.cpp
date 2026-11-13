/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <memory>

#include "engraving/dom/barline.h"
#include "engraving/dom/bracket.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/range.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/system.h"
#include "engraving/dom/timesig.h"

#include "engraving/editing/editproperty.h"
#include "engraving/editing/edittimesig.h"
#include "engraving/editing/transaction/transaction.h"
#include "engraving/editing/transaction/undostack.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu::engraving;

static const String BARLINE_DATA_DIR(u"barline_data/");

//---------------------------------------------------------
//   BarlineTests
//---------------------------------------------------------

class Engraving_BarlineTests : public ::testing::Test
{
public:
};

//---------------------------------------------------------
//  barline01
//  Check bar line and brackets presence and length with hidden empty staves:
//    A score with:
//          3 staves,
//          bracket across all 3 staves
//          bar lines across all 3 staves
//          systems with each staff hidden in turn because empty
//    is loaded, laid out and bracket/bar line sizes are checked.
//
//    NO REFERENCE SCORE IS USED: the test has to do with layout/formatting,
//    not with edit or read/save operations.
//---------------------------------------------------------

// actual 3-staff bracket should be high 28.6 SP ca.: allow for some layout margin
static const double BRACKET0_HEIGHT_MIN     = 27;
static const double BRACKET0_HEIGHT_MAX     = 30;
// actual 2-staff bracket should be high 18.1 SP ca.
static const double BRACKET_HEIGHT_MIN      = 17;
static const double BRACKET_HEIGHT_MAX      = 20;

TEST_F(Engraving_BarlineTests, barline01)
{
    Score* score = ScoreRW::readScore(BARLINE_DATA_DIR + "barline01.mscx");
    ASSERT_TRUE(score);

    double height, heightMin, heightMax;
    double spatium = score->style().spatium();
    int sysNo = 0;
    for (System* sys : score->systems()) {
        // check number of the brackets of each system
        EXPECT_EQ(sys->brackets().size(), 1);

        // check height of the bracket of each system
        // (bracket height is different between first system (3 staves) and other systems (2 staves) )
        Bracket* bracket = sys->brackets().at(0);
        height      = bracket->ldata()->bbox().height() / spatium;
        heightMin   = (sysNo == 0) ? BRACKET0_HEIGHT_MIN : BRACKET_HEIGHT_MIN;
        heightMax   = (sysNo == 0) ? BRACKET0_HEIGHT_MAX : BRACKET_HEIGHT_MAX;

        EXPECT_GT(height, heightMin);
        EXPECT_LT(height, heightMax);

        // check presence and height of the bar line of each measure of each system
        // (2 measure for each system)
        for (int msrNo=0; msrNo < 2; ++msrNo) {
            Measure* msr = toMeasure(sys->measure(msrNo));
            Segment* seg = msr->findSegment(SegmentType::EndBarLine, msr->endTick());
            ASSERT_TRUE(seg);

            BarLine* bar = toBarLine(seg->element(0));
            ASSERT_TRUE(bar);
        }
        sysNo++;
    }

    delete score;
}

//---------------------------------------------------------
//   barline02
//   add a 3/4 time signature in the second measure and check bar line 'generated' status
//
//    NO REFERENCE SCORE IS USED.
//---------------------------------------------------------
TEST_F(Engraving_BarlineTests, barline02)
{
    Score* score = ScoreRW::readScore(BARLINE_DATA_DIR + "barline02.mscx");
    ASSERT_TRUE(score);

    Measure* msr = score->firstMeasure()->nextMeasure();
    TimeSig* ts  = Factory::createTimeSig(score->dummy());
    ts->setSig(Fraction(3, 4), TimeSigType::NORMAL);

    EditTimeSig::addTimeSig(score->transactionManager()->currentOrDummyTransaction(), score, msr, 0, ts, false);
    score->doLayout();

    msr = score->firstMeasure();
    while ((msr = msr->nextMeasure())) {
        Segment* seg = msr->findSegment(SegmentType::EndBarLine, msr->endTick());
        ASSERT_TRUE(seg);

        BarLine* bar = static_cast<BarLine*>(seg->element(0));
        ASSERT_TRUE(bar);

        // bar line should be generated if NORMAL, except the END one at the end
        bool test = bar->generated();
        EXPECT_TRUE(test);
    }

    delete score;
}

//---------------------------------------------------------
//   barline03
//   Sets a staff bar line span involving spanFrom and spanTo and
//   check that it is properly applied to start-repeat
//
//   NO REFERENCE SCORE IS USED.
//---------------------------------------------------------
TEST_F(Engraving_BarlineTests, barline03)
{
    Score* score = ScoreRW::readScore(BARLINE_DATA_DIR + "barline03.mscx");
    ASSERT_TRUE(score);

    score->startCmd(TranslatableString::untranslatable("Engraving barline tests"));
    score->undo(new ChangeProperty(score->staff(0), Pid::STAFF_BARLINE_SPAN, true));
    score->undo(new ChangeProperty(score->staff(0), Pid::STAFF_BARLINE_SPAN_FROM, 2));
    score->undo(new ChangeProperty(score->staff(0), Pid::STAFF_BARLINE_SPAN_TO, -2));
    score->endCmd();

    // 'go' to 5th measure
    Measure* msr = score->firstMeasure();
    for (int i=0; i < 4; i++) {
        msr = msr->nextMeasure();
    }
    // check span data of measure-initial start-repeat bar line
    Segment* seg = msr->findSegment(SegmentType::StartRepeatBarLine, msr->tick());
    ASSERT_TRUE(seg);

    BarLine* bar = toBarLine(seg->element(0));
    ASSERT_TRUE(bar);

    delete score;
}

//---------------------------------------------------------
//   barline04
//   Sets custom span parameters to a system-initial start-repeat bar line and
//   check that it is properly applied to it and to the start-repeat bar lines of staves below.
//
//   NO REFERENCE SCORE IS USED.
//---------------------------------------------------------
TEST_F(Engraving_BarlineTests, barline04)
{
    Score* score = ScoreRW::readScore(BARLINE_DATA_DIR + "barline04.mscx");
    ASSERT_TRUE(score);

    score->doLayout();

    score->startCmd(TranslatableString::untranslatable("Engraving barline tests"));
    // 'go' to 5th measure
    Measure* msr = score->firstMeasure();
    for (int i=0; i < 4; i++) {
        msr = msr->nextMeasure();
    }
    // check span data of measure-initial start-repeat bar line
    Segment* seg = msr->findSegment(SegmentType::StartRepeatBarLine, msr->tick());
    ASSERT_TRUE(seg);

    BarLine* bar = static_cast<BarLine*>(seg->element(0));
    ASSERT_TRUE(bar);

    bar->undoChangeProperty(Pid::BARLINE_SPAN, true);
    bar->undoChangeProperty(Pid::BARLINE_SPAN_FROM, 2);
    bar->undoChangeProperty(Pid::BARLINE_SPAN_TO, 6);
    score->endCmd();

    EXPECT_TRUE(bar->spanStaff());
    EXPECT_EQ(bar->spanFrom(), 2);
    EXPECT_EQ(bar->spanTo(), 6);

    // check start-repeat bar ine in second staff is gone
    EXPECT_EQ(seg->element(1), nullptr);

    delete score;
}

//---------------------------------------------------------
//   barline05
//   Adds a line break in the middle of a end-start-repeat bar line and then checks the two resulting
//   bar lines (an end-repeat and a start-repeat) are not marked as generated.
//
//   NO REFERENCE SCORE IS USED.
//---------------------------------------------------------
TEST_F(Engraving_BarlineTests, barline05)
{
    Score* score = ScoreRW::readScore(BARLINE_DATA_DIR + "barline05.mscx");
    ASSERT_TRUE(score);

    score->doLayout();

    // 'go' to 4th measure
    Measure* msr = score->firstMeasure();
    for (int i=0; i < 3; i++) {
        msr = msr->nextMeasure();
    }
    // create and add a LineBreak element
    LayoutBreak* lb = Factory::createLayoutBreak(msr);
    lb->setLayoutBreakType(LayoutBreakType::LINE);
    lb->setTrack(0);
    score->undoAddElement(lb);
    score->doLayout();

    // check an end-repeat bar line has been created at the end of this measure and it is generated
    Segment* seg = msr->findSegment(SegmentType::EndBarLine, msr->tick() + msr->ticks());
    ASSERT_TRUE(seg);

    BarLine* bar = static_cast<BarLine*>(seg->element(0));
    ASSERT_TRUE(bar);

    EXPECT_EQ(bar->barLineType(), BarLineType::END_REPEAT);
    EXPECT_TRUE(bar->generated());

    // // check an end-repeat bar line has been created at the beginning of the next measure and it is not generated
    // check an end-repeat bar line has been created at the beginning of the next measure and it is generated
    msr = msr->nextMeasure();
    seg = msr->findSegment(SegmentType::StartRepeatBarLine, msr->tick());
    ASSERT_TRUE(seg);

    bar = static_cast<BarLine*>(seg->element(0));
    ASSERT_TRUE(bar);
    EXPECT_TRUE(bar->generated());

    delete score;
}

//---------------------------------------------------------
//   barline06
//   Read a score with 3 staves and custom bar line sub-types for staff i-th at measure i-th
//   and check the custom syb-types are applied only to their respective bar lines,
//   rather than to whole measures.
//
//   NO REFERENCE SCORE IS USED.
//---------------------------------------------------------
TEST_F(Engraving_BarlineTests, barline06)
{
    Score* score = ScoreRW::readScore(BARLINE_DATA_DIR + "barline06.mscx");
    ASSERT_TRUE(score);

    score->doLayout();

    // scan each measure
    Measure* msr   = score->firstMeasure();
    for (int i=0; i < 3; i++) {
        // locate end-measure bar line segment
        Segment* seg = msr->findSegment(SegmentType::EndBarLine, msr->tick() + msr->ticks());
        ASSERT_TRUE(seg);

        // check only i-th staff has custom bar line type
        for (int j=0; j < 3; j++) {
            BarLine* bar = static_cast<BarLine*>(seg->element(j * VOICES));
            ASSERT_TRUE(bar);

            // if not the i-th staff, bar should be normal and not custom
            if (j != i) {
                EXPECT_EQ(bar->barLineType(), BarLineType::NORMAL);
            }
            // in the i-th staff, the bar line should be of type DOUBLE and custom type should be true
            else {
                EXPECT_EQ(bar->barLineType(), BarLineType::DOUBLE);
            }
        }

        msr = msr->nextMeasure();
    }

    delete score;
}

//---------------------------------------------------------
//   dropNormalBarline
//   helper for barline179726()
//---------------------------------------------------------

void dropNormalBarline(EngravingItem* e)
{
    EditData dropData(0);
    BarLine* barLine = Factory::createBarLine(e->score()->dummy());
    barLine->setBarLineType(BarLineType::NORMAL);
    dropData.dropElement = barLine;
    dropData.track = 0;

    e->score()->transactionManager()->transaction(TranslatableString::untranslatable("Drop normal barline test"), [&](Transaction& tx) {
        e->drop(tx, dropData);
    });
}

//---------------------------------------------------------
//   barline179726
//   Drop a normal barline onto measures and barlines of each type of barline
//
//    NO REFERENCE SCORE IS USED.
//---------------------------------------------------------
TEST_F(Engraving_BarlineTests, barline179726)
{
    Score* score = ScoreRW::readScore(BARLINE_DATA_DIR + "barline179726.mscx");
    ASSERT_TRUE(score);

    score->doLayout();

    Measure* m = score->firstMeasure();

    // drop NORMAL onto initial START_REPEAT barline will remove that START_REPEAT
    dropNormalBarline(m->findSegment(SegmentType::StartRepeatBarLine, m->tick())->element(0));
    EXPECT_EQ(m->findSegment(SegmentType::StartRepeatBarLine, Fraction(0, 1)), nullptr);

    // drop NORMAL onto END_START_REPEAT will turn into NORMAL
    dropNormalBarline(m->findSegment(SegmentType::EndBarLine, m->endTick())->element(0));
    BarLine* bar = static_cast<BarLine*>(m->findSegment(SegmentType::EndBarLine, m->endTick())->element(0));
    ASSERT_TRUE(bar);
    EXPECT_EQ(bar->barLineType(), BarLineType::NORMAL);

    m = m->nextMeasure();

    // drop NORMAL onto the END_REPEAT part of an END_START_REPEAT straddling a newline will turn into NORMAL at the end of this meas
    dropNormalBarline(m->findSegment(SegmentType::EndBarLine, m->endTick())->element(0));
    bar = static_cast<BarLine*>(m->findSegment(SegmentType::EndBarLine, m->endTick())->element(0));
    ASSERT_TRUE(bar);
    EXPECT_EQ(bar->barLineType(), BarLineType::NORMAL);

    m = m->nextMeasure();

    // but leave START_REPEAT at the beginning of the newline
    bar = static_cast<BarLine*>(m->findSegment(SegmentType::StartRepeatBarLine, m->tick())->element(0));
    ASSERT_TRUE(bar);

    // drop NORMAL onto the meas ending with an END_START_REPEAT straddling a newline will turn into NORMAL at the end of this meas
    // but note I'm not verifying what happens to the START_REPEAT at the beginning of the newline...I'm not sure that behavior is well-defined yet
    dropNormalBarline(m);
    bar = static_cast<BarLine*>(m->findSegment(SegmentType::EndBarLine, m->endTick())->element(0));
    ASSERT_TRUE(bar);
    EXPECT_EQ(bar->barLineType(), BarLineType::NORMAL);

    m = m->nextMeasure();
    m = m->nextMeasure();

    // drop NORMAL onto the START_REPEAT part of an END_START_REPEAT straddling a newline will remove the START_REPEAT at the beginning of this measure
    dropNormalBarline(m->findSegment(SegmentType::StartRepeatBarLine, m->tick())->element(0));
    EXPECT_EQ(m->findSegment(SegmentType::StartRepeatBarLine, m->tick()), nullptr);

    // but leave END_REPEAT at the end of previous line
    bar = static_cast<BarLine*>(m->prevMeasure()->findSegment(SegmentType::EndBarLine, m->tick())->element(0));
    ASSERT_TRUE(bar);
    EXPECT_EQ(bar->barLineType(), BarLineType::END_REPEAT);

    for (int i = 0; i < 4; i++, m = m->nextMeasure()) {
        // drop NORMAL onto END_REPEAT, BROKEN, DOTTED, DOUBLE at the end of this meas will turn into NORMAL
        dropNormalBarline(m->findSegment(SegmentType::EndBarLine, m->endTick())->element(0));
        bar = static_cast<BarLine*>(m->findSegment(SegmentType::EndBarLine, m->endTick())->element(0));
        ASSERT_TRUE(bar);
        EXPECT_EQ(bar->barLineType(), BarLineType::NORMAL);
    }

    m = m->nextMeasure();

    // drop NORMAL onto a START_REPEAT in middle of a line will remove the START_REPEAT at the beginning of this measure
    dropNormalBarline(m->findSegment(SegmentType::StartRepeatBarLine, m->tick())->element(0));
    EXPECT_EQ(m->findSegment(SegmentType::StartRepeatBarLine, m->tick()), nullptr);

    // drop NORMAL onto final END_REPEAT at end of score will turn into NORMAL
    dropNormalBarline(m->findSegment(SegmentType::EndBarLine, m->endTick())->element(0));
    bar = static_cast<BarLine*>(m->findSegment(SegmentType::EndBarLine, m->endTick())->element(0));
    ASSERT_TRUE(bar);
    EXPECT_EQ(bar->barLineType(), BarLineType::NORMAL);

    delete score;
}

//---------------------------------------------------------
//   deleteSkipBarlines
//---------------------------------------------------------
TEST_F(Engraving_BarlineTests, deleteSkipBarlines)
{
    MasterScore* score = ScoreRW::readScore(BARLINE_DATA_DIR + "barlinedelete.mscx");
    ASSERT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    ASSERT_TRUE(m1);

    score->startCmd(TranslatableString::untranslatable("Engraving barline tests"));
    score->cmdSelectAll();
    score->cmdDeleteSelection();
    score->endCmd();

    score->doLayout();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String("barlinedelete.mscx"), BARLINE_DATA_DIR + String("barlinedelete-ref.mscx")));

    delete score;
}

namespace {
BarLine* endBarLine(Measure* measure, staff_idx_t staff)
{
    return toBarLine(measure->findSegment(SegmentType::EndBarLine, measure->endTick())->element(staff * VOICES));
}
}

namespace {
void dragSpanGrip(Score* score, BarLine* bar, bool connect, bool local = false)
{
    EditData ed;
    ed.modifiers = local ? ControlModifier : NoModifier;
    bar->startEdit(ed);
    auto data = std::static_pointer_cast<BarLineEditData>(ed.getData(bar));
    System* system = bar->measure()->system();
    const double targetY = system->pagePos().y()
                           + (connect ? system->staff(bar->staffIdx() + 1)->y()
                              : system->staff(bar->staffIdx())->y() + bar->staff()->staffHeight(bar->tick()));
    data->yoff2 = targetY - bar->pagePos().y() - bar->ldata()->y2;
    score->startCmd(TranslatableString::untranslatable("Drag barline span grip"));
    bar->endDragGrip(ed);
    score->endCmd();
}
}

TEST_F(Engraving_BarlineTests, explicitSpanSurvivesStaffDefaultsAndUndo)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-editing.mscx"));
    ASSERT_TRUE(score);
    Staff* staff = score->staff(0);
    BarLine* inherited = endBarLine(score->firstMeasure(), 0);
    BarLine* local = endBarLine(score->firstMeasure()->nextMeasure(), 0);
    score->startCmd(TranslatableString::untranslatable("Disconnect one barline"));
    local->undoChangeProperty(Pid::BARLINE_SPAN, false);
    score->endCmd();
    ASSERT_FALSE(local->spanStaffInherited());
    for (bool span : { false, true }) {
        score->startCmd(TranslatableString::untranslatable("Change staff span default"));
        staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, span);
        score->endCmd();
        EXPECT_EQ(inherited->spanStaff(), span);
        EXPECT_FALSE(local->spanStaff());
        EXPECT_FALSE(local->spanStaffInherited());
    }
    score->undoRedo(true, nullptr);
    EXPECT_FALSE(inherited->spanStaff());
    EXPECT_FALSE(local->spanStaff());
    score->undoRedo(true, nullptr);
    EXPECT_TRUE(inherited->spanStaff());
    EXPECT_FALSE(local->spanStaff());
    score->undoRedo(true, nullptr);
    EXPECT_TRUE(local->spanStaff());
    EXPECT_TRUE(local->spanStaffInherited());
    score->undoRedo(false, nullptr);
    EXPECT_FALSE(local->spanStaff());
    EXPECT_FALSE(local->spanStaffInherited());
}

TEST_F(Engraving_BarlineTests, legacyExplicitSpanMatchingStaffDefaultStaysLocal)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(u"midi/midirenderer_data/same_string_diff_staves.mscx"));
    ASSERT_TRUE(score);
    Staff* staff = score->staff(0);
    BarLine* bar = endBarLine(score->firstMeasure(), 0);
    ASSERT_TRUE(staff->barLineSpan());
    ASSERT_TRUE(bar->spanStaff());
    EXPECT_FALSE(bar->spanStaffInherited());

    score->startCmd(TranslatableString::untranslatable("Disconnect staff barlines"));
    staff->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, false);
    score->endCmd();
    EXPECT_TRUE(bar->spanStaff());
    EXPECT_FALSE(bar->spanStaffInherited());
}

TEST_F(Engraving_BarlineTests, explicitSpanEqualToDefaultPersists)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-editing.mscx"));
    ASSERT_TRUE(score);
    BarLine* local = endBarLine(score->firstMeasure()->nextMeasure(), 0);
    score->startCmd(TranslatableString::untranslatable("Disconnect one barline"));
    local->undoChangeProperty(Pid::BARLINE_SPAN, false);
    score->staff(0)->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, false);
    score->endCmd();
    ASSERT_FALSE(local->spanStaffInherited());
    local->setSpanStaff(local->spanStaff());
    EXPECT_FALSE(local->spanStaffInherited());
    score->startCmd(TranslatableString::untranslatable("Set selected connections to their current value"));
    endBarLine(score->firstMeasure(), 0)->undoChangeProperty(Pid::BARLINE_SPAN, false, PropertyFlags::UNSTYLED);
    local->undoChangeProperty(Pid::BARLINE_SPAN, false, PropertyFlags::UNSTYLED);
    score->endCmd();
    EXPECT_TRUE(endBarLine(score->firstMeasure(), 0)->spanStaffInherited());
    EXPECT_FALSE(local->spanStaffInherited());
    const String path(u"barline-span-inheritance-roundtrip.mscx");
    ASSERT_TRUE(ScoreRW::saveScore(score.get(), path));
    std::unique_ptr<MasterScore> reloaded(ScoreRW::readScore(path, true));
    ASSERT_TRUE(reloaded);
    BarLine* inherited = endBarLine(reloaded->firstMeasure(), 0);
    local = endBarLine(reloaded->firstMeasure()->nextMeasure(), 0);
    EXPECT_TRUE(inherited->spanStaffInherited());
    EXPECT_FALSE(local->spanStaffInherited());
    reloaded->startCmd(TranslatableString::untranslatable("Change reloaded default"));
    reloaded->staff(0)->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, true);
    reloaded->endCmd();
    EXPECT_TRUE(inherited->spanStaff());
    EXPECT_FALSE(local->spanStaff());
}

TEST_F(Engraving_BarlineTests, resetSpanRestoresInheritanceAndUndoRestoresOverride)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-editing.mscx"));
    ASSERT_TRUE(score);
    BarLine* bar = endBarLine(score->firstMeasure(), 0);
    score->startCmd(TranslatableString::untranslatable("Set explicit connection"));
    bar->undoChangeProperty(Pid::BARLINE_SPAN, false);
    score->staff(0)->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, false);
    score->endCmd();
    std::unique_ptr<BarLine> copy(bar->clone());
    EXPECT_FALSE(copy->spanStaffInherited());
    score->startCmd(TranslatableString::untranslatable("Reset connection override"));
    bar->undoResetProperty(Pid::BARLINE_SPAN);
    score->endCmd();
    EXPECT_TRUE(bar->spanStaffInherited());
    score->undoRedo(true, nullptr);
    EXPECT_FALSE(bar->spanStaff());
    EXPECT_FALSE(bar->spanStaffInherited());
    score->undoRedo(false, nullptr);
    EXPECT_TRUE(bar->spanStaffInherited());
}

TEST_F(Engraving_BarlineTests, gripChangesStaffDefaultAndPreservesLocalConnections)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-editing.mscx"));
    ASSERT_TRUE(score);
    BarLine* inherited = endBarLine(score->firstMeasure(), 0);
    BarLine* local = endBarLine(score->firstMeasure()->nextMeasure(), 0);
    score->startCmd(TranslatableString::untranslatable("Set local disconnection"));
    local->undoChangeProperty(Pid::BARLINE_SPAN, false);
    score->endCmd();
    dragSpanGrip(score.get(), inherited, false);
    EXPECT_FALSE(score->staff(0)->barLineSpan());
    EXPECT_FALSE(inherited->spanStaff());
    EXPECT_FALSE(local->spanStaff());
    score->undoRedo(true, nullptr);
    EXPECT_TRUE(score->staff(0)->barLineSpan());
    EXPECT_TRUE(inherited->spanStaff());
    EXPECT_FALSE(local->spanStaff());
    // Restoring the default with the local grip only changes the edited barline.
    dragSpanGrip(score.get(), local, true);
    EXPECT_TRUE(score->staff(0)->barLineSpan());
    EXPECT_TRUE(inherited->spanStaff());
    EXPECT_TRUE(local->spanStaff());
    EXPECT_TRUE(local->spanStaffInherited());
    ASSERT_TRUE(score->undoStack()->canUndo());
    score->undoRedo(true, nullptr);
    EXPECT_FALSE(local->spanStaff());
    EXPECT_FALSE(local->spanStaffInherited());
    score->undoRedo(false, nullptr);
    EXPECT_TRUE(local->spanStaff());
    EXPECT_TRUE(local->spanStaffInherited());

    dragSpanGrip(score.get(), inherited, false);
    score->startCmd(TranslatableString::untranslatable("Set local connection"));
    local->undoChangeProperty(Pid::BARLINE_SPAN, true);
    score->endCmd();
    dragSpanGrip(score.get(), local, false);
    EXPECT_FALSE(score->staff(0)->barLineSpan());
    EXPECT_FALSE(inherited->spanStaff());
    EXPECT_FALSE(local->spanStaff());
    EXPECT_TRUE(local->spanStaffInherited());
}

TEST_F(Engraving_BarlineTests, controlGripEditsOneConnectionAndUndoRestoresInheritance)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-editing.mscx"));
    ASSERT_TRUE(score);
    BarLine* bar = endBarLine(score->firstMeasure(), 0);
    BarLine* neighbor = endBarLine(score->firstMeasure()->nextMeasure(), 0);
    dragSpanGrip(score.get(), bar, false, true);
    EXPECT_TRUE(score->staff(0)->barLineSpan());
    EXPECT_FALSE(bar->spanStaff());
    EXPECT_FALSE(bar->spanStaffInherited());
    EXPECT_TRUE(neighbor->spanStaff());
    score->undoRedo(true, nullptr);
    EXPECT_TRUE(bar->spanStaff());
    EXPECT_TRUE(bar->spanStaffInherited());
    score->undoRedo(false, nullptr);
    EXPECT_FALSE(bar->spanStaff());
    EXPECT_FALSE(bar->spanStaffInherited());
}

TEST_F(Engraving_BarlineTests, midMeasureGripKeepsLocalScope)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-editing.mscx"));
    ASSERT_TRUE(score);
    Measure* measure = score->firstMeasure();
    BarLine* middle = toBarLine(measure->findSegment(SegmentType::BarLine, Fraction(1, 2))->element(0));
    ASSERT_TRUE(middle);
    ASSERT_TRUE(middle->spanStaff());
    dragSpanGrip(score.get(), middle, false);
    EXPECT_TRUE(score->staff(0)->barLineSpan());
    EXPECT_FALSE(middle->spanStaff());
    EXPECT_TRUE(endBarLine(measure, 0)->spanStaff());
    score->undoRedo(true, nullptr);
    EXPECT_TRUE(middle->spanStaff());
}

TEST_F(Engraving_BarlineTests, gripUndoPreservesMixedConnectionPattern)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-editing.mscx"));
    ASSERT_TRUE(score);
    score->staff(0)->setProperty(Pid::STAFF_BARLINE_SPAN, false);
    std::vector<BarLine*> bars;
    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        BarLine* bar = endBarLine(measure, 0);
        if (bars.size() < 2) {
            bar->setProperty(Pid::BARLINE_SPAN, true);
        } else {
            bar->setPropertyFlags(Pid::BARLINE_SPAN, PropertyFlags::STYLED);
        }
        bars.push_back(bar);
    }
    score->doLayout();
    auto checkPattern = [&](bool defaultSpan) {
        EXPECT_EQ(score->staff(0)->barLineSpan(), defaultSpan);
        for (size_t i = 0; i < bars.size(); ++i) {
            EXPECT_EQ(bars[i]->spanStaff(), i < 2 || defaultSpan);
            EXPECT_EQ(bars[i]->spanStaffInherited(), i >= 2);
        }
    };
    dragSpanGrip(score.get(), bars.back(), true);
    checkPattern(true);
    score->undoRedo(true, nullptr);
    checkPattern(false);
    score->undoRedo(false, nullptr);
    checkPattern(true);
}

TEST_F(Engraving_BarlineTests, midMeasureConnectionPersistsAcrossStaffDefaultChange)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-editing.mscx"));
    ASSERT_TRUE(score);
    BarLine* middle = toBarLine(score->firstMeasure()->findSegment(SegmentType::BarLine, Fraction(1, 2))->element(0));
    ASSERT_TRUE(middle);
    // Older files can omit the span tag on a mid-measure barline.
    middle->setPropertyFlags(Pid::BARLINE_SPAN, PropertyFlags::STYLED);
    score->staff(0)->setProperty(Pid::STAFF_BARLINE_SPAN, false);
    EXPECT_TRUE(middle->spanStaff());
    const String path(u"barline-middle-span-roundtrip.mscx");
    ASSERT_TRUE(ScoreRW::saveScore(score.get(), path));
    std::unique_ptr<MasterScore> reloaded(ScoreRW::readScore(path, true));
    ASSERT_TRUE(reloaded);
    middle = toBarLine(reloaded->firstMeasure()->findSegment(SegmentType::BarLine, Fraction(1, 2))->element(0));
    ASSERT_TRUE(middle);
    EXPECT_TRUE(middle->spanStaff());
    EXPECT_FALSE(middle->spanStaffInherited());
    EXPECT_FALSE(endBarLine(reloaded->firstMeasure(), 0)->spanStaff());
}

TEST_F(Engraving_BarlineTests, rewriteMeasuresPreservesBarlineConnection)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-editing.mscx"));
    ASSERT_TRUE(score);
    BarLine* bar = endBarLine(score->firstMeasure(), 0);
    bar->setProperty(Pid::BARLINE_SPAN, false);
    for (staff_idx_t staff = 1; staff < score->nstaves(); ++staff) {
        BarLine* other = endBarLine(score->firstMeasure(), staff);
        ASSERT_EQ(other->barLineType(), bar->barLineType());
        other->setGenerated(false);
    }
    TimeSig* signature = Factory::createTimeSig(score->firstMeasure()->first());
    signature->setSig(Fraction(2, 4), TimeSigType::NORMAL);
    score->startCmd(TranslatableString::untranslatable("Rewrite measures"));
    EditTimeSig::addTimeSig(score->transactionManager()->currentOrDummyTransaction(), score.get(),
                            score->firstMeasure(), 0, signature, false);
    score->endCmd();
    bool found = false;
    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        if (measure->endTick() == Fraction(1, 1)) {
            bar = endBarLine(measure, 0);
            ASSERT_TRUE(bar);
            EXPECT_FALSE(bar->spanStaff());
            EXPECT_FALSE(bar->spanStaffInherited());
            found = true;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(Engraving_BarlineTests, editingSpanBackToDefaultRestoresInheritance)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-editing.mscx"));
    ASSERT_TRUE(score);
    BarLine* bar = endBarLine(score->firstMeasure(), 0);
    score->startCmd(TranslatableString::untranslatable("Change span through generic property command"));
    bar->undoChangeProperty(Pid::BARLINE_SPAN, false, PropertyFlags::NOSTYLE);
    score->endCmd();
    EXPECT_FALSE(bar->spanStaff());
    score->startCmd(TranslatableString::untranslatable("Set connection to match staff default"));
    bar->undoChangeProperty(Pid::BARLINE_SPAN, true, PropertyFlags::UNSTYLED);
    score->endCmd();
    EXPECT_TRUE(bar->spanStaff());
    EXPECT_TRUE(bar->spanStaffInherited());
    score->undoRedo(true, nullptr);
    EXPECT_FALSE(bar->spanStaff());
    EXPECT_FALSE(bar->spanStaffInherited());
    score->undoRedo(false, nullptr);
    EXPECT_TRUE(bar->spanStaffInherited());
    score->startCmd(TranslatableString::untranslatable("Change staff connection"));
    score->staff(0)->undoChangeProperty(Pid::STAFF_BARLINE_SPAN, false);
    score->endCmd();
    EXPECT_FALSE(bar->spanStaff());
}

TEST_F(Engraving_BarlineTests, copiedBarlineInheritsDestinationStaffConnection)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-editing.mscx"));
    ASSERT_TRUE(score);
    Measure* measure = score->firstMeasure();
    BarLine* source = endBarLine(measure, 0);
    BarLine* destination = endBarLine(measure, 2);
    const bool destinationSpan = destination->spanStaff();
    const bool destinationInherited = destination->spanStaffInherited();
    ASSERT_TRUE(source->spanStaffInherited());
    ASSERT_TRUE(source->spanStaff());
    ASSERT_FALSE(score->staff(2)->barLineSpan());
    BarLine* copy = source->clone();
    copy->setTrack(2 * VOICES);
    copy->setOwnershipParent(destination->segment());
    score->startCmd(TranslatableString::untranslatable("Copy barline to another staff"));
    score->undoRemoveElement(destination);
    score->undoAddElement(copy);
    score->endCmd();
    EXPECT_TRUE(copy->spanStaffInherited());
    EXPECT_FALSE(copy->spanStaff());
    score->undoRedo(true, nullptr);
    BarLine* restored = endBarLine(measure, 2);
    ASSERT_TRUE(restored);
    EXPECT_EQ(restored->spanStaff(), destinationSpan);
    EXPECT_EQ(restored->spanStaffInherited(), destinationInherited);
}

TEST_F(Engraving_BarlineTests, keyboardSpanEditingUsesGripScope)
{
    for (bool defaultSpan : { false, true }) {
        SCOPED_TRACE(defaultSpan);
        std::unique_ptr<MasterScore> score(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-editing.mscx"));
        ASSERT_TRUE(score);
        Staff* staff = score->staff(0);
        staff->setProperty(Pid::STAFF_BARLINE_SPAN, defaultSpan);
        BarLine* local = endBarLine(score->firstMeasure(), 0);
        BarLine* inherited = endBarLine(score->firstMeasure()->nextMeasure(), 0);
        local->setProperty(Pid::BARLINE_SPAN, !defaultSpan);
        EditData ed;
        ed.key = defaultSpan ? Key_Down : Key_Up;
        score->startCmd(TranslatableString::untranslatable("Change local connection with keyboard"));
        EXPECT_TRUE(local->edit(ed));
        score->endCmd();
        EXPECT_EQ(staff->barLineSpan(), defaultSpan);
        EXPECT_EQ(inherited->spanStaff(), defaultSpan);
        EXPECT_EQ(local->spanStaff(), defaultSpan);
        EXPECT_TRUE(local->spanStaffInherited());
        ASSERT_TRUE(score->undoStack()->canUndo());
        score->undoRedo(true, nullptr);
        EXPECT_EQ(local->spanStaff(), !defaultSpan);
        EXPECT_FALSE(local->spanStaffInherited());

        ed.key = defaultSpan ? Key_Up : Key_Down;
        score->startCmd(TranslatableString::untranslatable("Change staff connection with keyboard"));
        EXPECT_TRUE(inherited->edit(ed));
        score->endCmd();
        EXPECT_EQ(staff->barLineSpan(), !defaultSpan);
        EXPECT_EQ(inherited->spanStaff(), !defaultSpan);
        EXPECT_TRUE(inherited->spanStaffInherited());
        EXPECT_EQ(local->spanStaff(), !defaultSpan);
        EXPECT_FALSE(local->spanStaffInherited());
        ASSERT_TRUE(score->undoStack()->canUndo());
        score->undoRedo(true, nullptr);

        ed.modifiers = ControlModifier;
        score->startCmd(TranslatableString::untranslatable("Force a local connection with keyboard"));
        EXPECT_TRUE(inherited->edit(ed));
        score->endCmd();
        EXPECT_EQ(staff->barLineSpan(), defaultSpan);
        EXPECT_EQ(inherited->spanStaff(), !defaultSpan);
        EXPECT_FALSE(inherited->spanStaffInherited());
    }
}

TEST_F(Engraving_BarlineTests, rangeRestoreUndoesExistingStartRepeatConnection)
{
    std::unique_ptr<MasterScore> source(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-editing.mscx"));
    std::unique_ptr<MasterScore> destination(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-editing.mscx"));
    ASSERT_TRUE(source);
    ASSERT_TRUE(destination);
    BarLine* saved = endBarLine(source->firstMeasure(), 0);
    saved->setProperty(Pid::BARLINE_SPAN, false);
    saved->setBarLineType(BarLineType::END_START_REPEAT);
    ScoreRange range;
    range.read(source->firstMeasure()->first(), source->lastMeasure()->last(), false);
    Measure* next = destination->firstMeasure()->nextMeasure();
    next->setRepeatStart(true);
    destination->doLayout();
    Segment* startRepeat = next->findSegment(SegmentType::StartRepeatBarLine, next->tick());
    ASSERT_TRUE(startRepeat);
    BarLine* existing = toBarLine(startRepeat->element(0));
    ASSERT_TRUE(existing);
    ASSERT_TRUE(existing->spanStaff());
    ASSERT_TRUE(existing->spanStaffInherited());
    ASSERT_TRUE(existing->generated());
    for (Measure* measure = destination->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (Segment* segment = measure->first(); segment; segment = segment->next()) {
            for (track_idx_t track = 0; track < destination->nstaves() * VOICES; ++track) {
                EngravingItem* item = segment->element(track);
                if (item && item->isChordRest()) {
                    destination->removeElement(item);
                    delete item;
                }
            }
        }
    }
    destination->startCmd(TranslatableString::untranslatable("Restore range into existing measures"));
    const bool written = range.write(destination.get(), Fraction(0, 1));
    destination->endCmd();
    ASSERT_TRUE(written);
    ASSERT_EQ(startRepeat->element(0), existing);
    EXPECT_FALSE(existing->spanStaff());
    EXPECT_FALSE(existing->spanStaffInherited());
    EXPECT_FALSE(existing->generated());

    ASSERT_TRUE(destination->undoStack()->canUndo());
    destination->undoRedo(true, nullptr);
    ASSERT_EQ(startRepeat->element(0), existing);
    EXPECT_TRUE(existing->spanStaff());
    EXPECT_TRUE(existing->spanStaffInherited());
    EXPECT_TRUE(existing->generated());

    destination->undoRedo(false, nullptr);
    ASSERT_EQ(startRepeat->element(0), existing);
    EXPECT_FALSE(existing->spanStaff());
    EXPECT_FALSE(existing->spanStaffInherited());
    EXPECT_FALSE(existing->generated());
}

TEST_F(Engraving_BarlineTests, explicitSpanSurvivesForcedRepeat)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-editing.mscx"));
    ASSERT_TRUE(score);
    Measure* measure = score->firstMeasure();
    BarLine* bar = endBarLine(measure, 0);
    bar->setProperty(Pid::BARLINE_SPAN, false);
    measure->setRepeatEnd(true);
    score->doLayout();
    ASSERT_EQ(bar->barLineType(), BarLineType::END_REPEAT);
    measure->setRepeatEnd(false);
    score->doLayout();
    EXPECT_EQ(bar->barLineType(), BarLineType::NORMAL);
    EXPECT_FALSE(bar->spanStaff());
    EXPECT_FALSE(bar->spanStaffInherited());
    EXPECT_FALSE(bar->generated());
}
