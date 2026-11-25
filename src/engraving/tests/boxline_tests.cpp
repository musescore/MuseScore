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

#include <gtest/gtest.h>

#include "engraving/api/v1/elements.h"
#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/boxline.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/mcursor.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/page.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/system.h"
#include "engraving/editing/editboxline.h"
#include "engraving/editing/editdata.h"
#include "engraving/editing/transaction/transaction.h"
#include "engraving/editing/transaction/undostack.h"

#include "utils/scorerw.h"

using namespace mu;
using namespace mu::engraving;

namespace {
constexpr double EPS = 0.01;

void expectNear(const PointF& actual, const PointF& expected)
{
    EXPECT_NEAR(actual.x(), expected.x(), EPS);
    EXPECT_NEAR(actual.y(), expected.y(), EPS);
}
}

//---------------------------------------------------------
//   Model tests: a box that isn't part of a score
//---------------------------------------------------------

class Engraving_BoxLineModelTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_score = compat::ScoreAccess::createMasterScore(nullptr);
        m_box = Factory::createBoxLine(m_score->dummy());
    }

    void TearDown() override
    {
        delete m_box;
        delete m_score;
    }

    MasterScore* m_score = nullptr;
    BoxLine* m_box = nullptr;
};

TEST_F(Engraving_BoxLineModelTests, defaults)
{
    EXPECT_EQ(m_box->boxHeight(), 2.0_sp);
    EXPECT_EQ(m_box->propertyDefault(Pid::BOXLINE_HEIGHT).value<Spatium>(), 2.0_sp);
    EXPECT_TRUE(m_box->placeAbove());
    EXPECT_EQ(m_box->propertyDefault(Pid::PLACEMENT).value<PlacementV>(), PlacementV::ABOVE);
    EXPECT_FALSE(m_box->diagonal());
    EXPECT_FALSE(m_box->centerOnStaff());
    EXPECT_FALSE(m_box->propertyDefault(Pid::BOXLINE_CENTER_ON_STAFF).toBool());
}

TEST_F(Engraving_BoxLineModelTests, heightIsClamped)
{
    m_box->setBoxHeight(0.0_sp);
    EXPECT_EQ(m_box->boxHeight(), BoxLine::MIN_HEIGHT);

    m_box->setBoxHeight(Spatium(BoxLine::MAX_HEIGHT.val() + 1.0));
    EXPECT_EQ(m_box->boxHeight(), BoxLine::MAX_HEIGHT);

    EXPECT_TRUE(m_box->setProperty(Pid::BOXLINE_HEIGHT, -5.0_sp));
    EXPECT_EQ(m_box->getProperty(Pid::BOXLINE_HEIGHT).value<Spatium>(), BoxLine::MIN_HEIGHT);
}

TEST_F(Engraving_BoxLineModelTests, cannotBeMadeDiagonal)
{
    EXPECT_FALSE(m_box->setProperty(Pid::DIAGONAL, true));
    EXPECT_FALSE(m_box->diagonal());
}

TEST_F(Engraving_BoxLineModelTests, cloneCopiesProperties)
{
    m_box->setBoxHeight(4.5_sp);
    m_box->setCenterOnStaff(true);
    BoxLine* copy = m_box->clone();
    EXPECT_EQ(copy->boxHeight(), 4.5_sp);
    EXPECT_TRUE(copy->centerOnStaff());
    delete copy;
}

TEST_F(Engraving_BoxLineModelTests, writeReadRoundTrip)
{
    m_box->setBoxHeight(3.25_sp);
    m_box->setCenterOnStaff(true);
    m_box->setPlacement(PlacementV::BELOW);
    m_box->setLineStyle(LineType::DASHED);
    m_box->setDashLineLen(3.0);
    m_box->setDashGapLen(1.5);

    BoxLine* copy = toBoxLine(ScoreRW::writeReadElement(m_box));
    ASSERT_TRUE(copy);
    EXPECT_EQ(copy->boxHeight(), 3.25_sp);
    EXPECT_TRUE(copy->placeBelow());
    EXPECT_EQ(copy->lineStyle(), LineType::DASHED);
    EXPECT_DOUBLE_EQ(copy->dashLineLen(), 3.0);
    EXPECT_DOUBLE_EQ(copy->dashGapLen(), 1.5);
    EXPECT_TRUE(copy->centerOnStaff());
    delete copy;
}

//---------------------------------------------------------
//   Score tests: a box in a laid-out score
//---------------------------------------------------------

class Engraving_BoxLineTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        MCursor c;
        c.setTimeSig(Fraction(4, 4));
        c.createScore(nullptr, u"test");
        c.addPart(u"voice");
        c.move(0, Fraction(0, 1));
        c.addTimeSig(Fraction(4, 4));
        for (int i = 0; i < 4; ++i) {
            c.addChord(60, TDuration(DurationType::V_WHOLE));
        }
        m_score = c.score();
        m_score->doLayout();
        m_sp = m_score->style().spatium();
    }

    void TearDown() override
    {
        delete m_score;
    }

    // Box over `measures` measures starting at the first one
    BoxLine* addCenteredBox(int measures)
    {
        BoxLine* box = addBox(measures);
        box->setCenterOnStaff(true);
        m_score->doLayout();
        return box;
    }

    BoxLine* addBox(int measures, PlacementV placement = PlacementV::ABOVE)
    {
        BoxLine* box = Factory::createBoxLine(m_score->dummy());
        box->setTrack(0);
        box->setTrack2(0);
        box->setTick(Fraction(0, 1));
        box->setTicks(Fraction(measures, 1));
        box->setPlacement(placement);
        m_score->addSpanner(box);
        m_score->doLayout();
        return box;
    }

    static BoxLineSegment* front(BoxLine* box) { return toBoxLineSegment(box->frontSegment()); }

    // Grip position in canvas coordinates
    static PointF gripPos(BoxLineSegment* seg, Grip grip)
    {
        return seg->gripsPositions().at(size_t(grip)) + seg->system()->page()->pos();
    }

    // Drags `grip` by `delta` (in canvas units) like the mouse would, as one undoable command
    void drag(BoxLineSegment* seg, Grip grip, const PointF& delta)
    {
        EngravingItem* item = seg;
        EditData ed(nullptr);
        ed.element = item;
        ed.curGrip = grip;
        ed.grips = seg->gripsCount();
        ed.startMove = gripPos(seg, grip);
        ed.pos = ed.startMove + delta;
        ed.delta = ed.evtDelta = ed.moveDelta = delta;

        m_score->startCmd(TranslatableString::untranslatable("Box line tests"));
        item->startEdit(ed);
        item->startDragGrip(ed);
        item->dragGrip(ed);
        item->endDragGrip(ed);
        item->endEdit(ed);
        m_score->endCmd();
        m_score->doLayout();
    }

    MasterScore* m_score = nullptr;
    double m_sp = 0.0;
};

TEST_F(Engraving_BoxLineTests, segmentDelegatesHeightToBox)
{
    BoxLine* box = addBox(2);
    box->setBoxHeight(3.0_sp);
    EXPECT_EQ(front(box)->getProperty(Pid::BOXLINE_HEIGHT).value<Spatium>(), 3.0_sp);
}

TEST_F(Engraving_BoxLineTests, engravingApiProperties)
{
    BoxLine* box = addBox(2);
    apiv1::EngravingItem* apiBox = apiv1::wrap(box, apiv1::Ownership::SCORE);
    EXPECT_DOUBLE_EQ(apiBox->property("boxLineHeight").toDouble(), 2.0);
    EXPECT_FALSE(apiBox->property("boxLineCenterOnStaff").toBool());

    m_score->startCmd(TranslatableString::untranslatable("Box line tests"));
    EXPECT_TRUE(apiBox->setProperty("boxLineHeight", 4.5));
    EXPECT_TRUE(apiBox->setProperty("boxLineCenterOnStaff", true));
    m_score->endCmd();
    EXPECT_EQ(box->boxHeight(), 4.5_sp);
    EXPECT_TRUE(box->centerOnStaff());
}

TEST_F(Engraving_BoxLineTests, heightGripChangesOnlyHeight)
{
    for (PlacementV placement : { PlacementV::ABOVE, PlacementV::BELOW }) {
        BoxLine* box = addBox(2, placement);
        BoxLineSegment* seg = front(box);
        const PointF offset = seg->offset();
        const PointF offset2 = seg->userOff2();

        // [WHEN] Dragging the height grip 1sp away from the staff
        const double away = placement == PlacementV::ABOVE ? -m_sp : m_sp;
        drag(seg, BoxLineSegment::HEIGHT, PointF(m_sp, away));

        // [THEN] The box is 1sp taller, and nothing else changes
        EXPECT_EQ(box->boxHeight(), 3.0_sp);
        EXPECT_EQ(box->tick(), Fraction(0, 1));
        EXPECT_EQ(box->ticks(), Fraction(2, 1));
        expectNear(seg->offset(), offset);
        expectNear(seg->userOff2(), offset2);

        // [WHEN] Dragging it far towards the staff
        drag(seg, BoxLineSegment::HEIGHT, PointF(0.0, -10.0 * away));

        // [THEN] The height stops at the minimum
        EXPECT_EQ(box->boxHeight(), BoxLine::MIN_HEIGHT);

        m_score->removeSpanner(box);
        delete box;
    }
}

TEST_F(Engraving_BoxLineTests, heightGripDragIsUndoable)
{
    BoxLine* box = addBox(2);
    drag(front(box), BoxLineSegment::HEIGHT, PointF(0.0, -m_sp));
    ASSERT_EQ(box->boxHeight(), 3.0_sp);

    m_score->undoStack()->undo(nullptr);
    EXPECT_EQ(box->boxHeight(), 2.0_sp);
}

TEST_F(Engraving_BoxLineTests, startEndGripsResizeOnlyHorizontally)
{
    for (Grip grip : { Grip::START, Grip::END }) {
        const Grip other = grip == Grip::START ? Grip::END : Grip::START;

        BoxLine* box = addBox(2);
        BoxLineSegment* seg = front(box);
        const PointF before = gripPos(seg, grip);
        const PointF otherBefore = gripPos(seg, other);

        // [WHEN] Dragging the grip diagonally
        drag(seg, grip, PointF(m_sp, -3.0 * m_sp));

        // [THEN] It follows only the horizontal movement: the height and the time anchors don't change
        EXPECT_EQ(box->boxHeight(), 2.0_sp);
        EXPECT_EQ(box->tick(), Fraction(0, 1));
        EXPECT_EQ(box->ticks(), Fraction(2, 1));
        expectNear(gripPos(front(box), grip), before + PointF(m_sp, 0.0));
        expectNear(gripPos(front(box), other), otherBefore);

        m_score->removeSpanner(box);
        delete box;
    }
}

TEST_F(Engraving_BoxLineTests, centeredBoxDoesNotAutoplace)
{
    BoxLine* box = addCenteredBox(2);
    BoxLineSegment* seg = front(box);

    // [THEN] Autoplace doesn't apply, even if the AUTOPLACE property is kept untouched
    EXPECT_FALSE(seg->autoplace());
    EXPECT_FALSE(box->autoplace());
    EXPECT_TRUE(seg->getProperty(Pid::AUTOPLACE).toBool());

    // [WHEN] It goes back to being placed above the staff
    box->setCenterOnStaff(false);
    m_score->doLayout();

    // [THEN] Autoplace applies again
    EXPECT_TRUE(front(box)->autoplace());
    EXPECT_TRUE(box->autoplace());
}

TEST_F(Engraving_BoxLineTests, centeredBoxHeightGripGrowsBothWays)
{
    BoxLine* box = addCenteredBox(2);
    const PointF heightGrip = gripPos(front(box), BoxLineSegment::HEIGHT);
    const PointF startGrip = gripPos(front(box), Grip::START);

    // [WHEN] Dragging the height grip 1sp up
    drag(front(box), BoxLineSegment::HEIGHT, PointF(0.0, -m_sp));

    // [THEN] The box grows 1sp in each direction, so the grip follows the mouse
    EXPECT_EQ(box->boxHeight(), 4.0_sp);
    expectNear(gripPos(front(box), BoxLineSegment::HEIGHT), heightGrip + PointF(0.0, -m_sp));
    expectNear(gripPos(front(box), Grip::START), startGrip + PointF(0.0, m_sp));
}

//---------------------------------------------------------
//   EditBoxLine tests: adding boxes to the selection
//---------------------------------------------------------

class Engraving_EditBoxLineTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Two staves with four whole-note measures each
        MCursor c;
        c.setTimeSig(Fraction(4, 4));
        c.createScore(nullptr, u"test");
        c.addPart(u"voice");
        c.addPart(u"voice");
        for (int track : { 0, 4 }) {
            c.move(track, Fraction(0, 1));
            if (track == 0) {
                c.addTimeSig(Fraction(4, 4));
            }
            for (int i = 0; i < 4; ++i) {
                c.addChord(60, TDuration(DurationType::V_WHOLE));
            }
        }
        m_score = c.score();
        m_score->doLayout();
    }

    void TearDown() override
    {
        delete m_score;
    }

    Measure* measure(int idx) const
    {
        Measure* m = m_score->firstMeasure();
        for (int i = 0; i < idx; ++i) {
            m = m->nextMeasure();
        }
        return m;
    }

    Chord* chord(int measureIdx, track_idx_t track) const
    {
        Measure* m = measure(measureIdx);
        return m->findChord(m->tick(), track);
    }

    std::vector<BoxLine*> addBoxLines()
    {
        m_score->startCmd(TranslatableString::untranslatable("Edit box line tests"));
        std::vector<BoxLine*> boxes = EditBoxLine::addBoxLines(m_score->transactionManager()->currentOrDummyTransaction(), m_score);
        m_score->endCmd();
        return boxes;
    }

    size_t boxCount() const
    {
        size_t count = 0;
        for (const auto& pair : m_score->spanner()) {
            if (pair.second->isBoxLine()) {
                ++count;
            }
        }
        return count;
    }

    MasterScore* m_score = nullptr;
};

TEST_F(Engraving_EditBoxLineTests, singleChordSelection)
{
    m_score->select(chord(3, 0), SelectType::SINGLE);

    std::vector<BoxLine*> boxes = addBoxLines();

    ASSERT_EQ(boxes.size(), 1);
    EXPECT_EQ(boxes.front()->track(), 0);
    EXPECT_EQ(boxes.front()->tick(), Fraction(3, 1));
    EXPECT_EQ(boxes.front()->tick2(), Fraction(4, 1));
}

TEST_F(Engraving_EditBoxLineTests, listSelectionSpansSelectedChords)
{
    // [GIVEN] The chords of measures 2 and 3 of the second staff are selected
    m_score->select(chord(1, 4), SelectType::SINGLE);
    m_score->select(chord(2, 4), SelectType::ADD);

    // [WHEN] Adding a box
    std::vector<BoxLine*> boxes = addBoxLines();

    // [THEN] One box covers both chords, on the second staff
    ASSERT_EQ(boxes.size(), 1);
    BoxLine* box = boxes.front();
    EXPECT_EQ(box->track(), 4);
    EXPECT_EQ(box->tick(), Fraction(1, 1));
    EXPECT_EQ(box->tick2(), Fraction(3, 1));
    EXPECT_EQ(boxCount(), 1);
}

TEST_F(Engraving_EditBoxLineTests, rangeSelectionAddsOneBoxPerStaff)
{
    // [GIVEN] Measures 1-2 of both staves are range-selected
    m_score->selection().setRangeTicks(Fraction(0, 1), Fraction(2, 1), 0, 2);
    m_score->selection().updateSelectedElements();

    // [WHEN] Adding a box
    std::vector<BoxLine*> boxes = addBoxLines();

    // [THEN] Each staff gets a box covering the range
    ASSERT_EQ(boxes.size(), 2);
    for (size_t i = 0; i < boxes.size(); ++i) {
        EXPECT_EQ(boxes[i]->track(), i * VOICES);
        EXPECT_EQ(boxes[i]->tick(), Fraction(0, 1));
        EXPECT_EQ(boxes[i]->tick2(), Fraction(2, 1));
    }
    EXPECT_EQ(boxCount(), 2);
}

TEST_F(Engraving_EditBoxLineTests, rangeSelectionOverLinkedStavesAddsOneBoxPerStaff)
{
    // [GIVEN] The first staff has a linked staff (as with standard + TAB)
    m_score->startCmd(TranslatableString::untranslatable("Edit box line tests"));
    Staff* staff = m_score->staff(0);
    Staff* linkedStaff = Factory::createStaff(staff->part());
    linkedStaff->setPart(staff->part());
    m_score->undoInsertStaff(linkedStaff, 1, false);
    Excerpt::cloneStaff(staff, linkedStaff);
    m_score->endCmd();
    ASSERT_EQ(m_score->nstaves(), 3);

    // [GIVEN] Measures 1-2 of the linked pair are range-selected
    m_score->selection().setRangeTicks(Fraction(0, 1), Fraction(2, 1), 0, 2);
    m_score->selection().updateSelectedElements();

    // [WHEN] Adding a box
    std::vector<BoxLine*> boxes = addBoxLines();

    // [THEN] One box is added for the linked pair
    ASSERT_EQ(boxes.size(), 1);
    EXPECT_EQ(boxes.front()->tick(), Fraction(0, 1));
    EXPECT_EQ(boxes.front()->tick2(), Fraction(2, 1));
    EXPECT_EQ(boxCount(), 2);
}

TEST_F(Engraving_EditBoxLineTests, nothingAddedWithoutChordRest)
{
    // [GIVEN] A selection without chords or rests (a time signature)
    m_score->select(measure(0)->findSegment(SegmentType::TimeSig, Fraction(0, 1))->element(0), SelectType::SINGLE);

    EXPECT_TRUE(addBoxLines().empty());
    EXPECT_EQ(boxCount(), 0);
}

TEST_F(Engraving_EditBoxLineTests, addIsUndoable)
{
    m_score->select(chord(0, 0), SelectType::SINGLE);
    ASSERT_EQ(addBoxLines().size(), 1);
    ASSERT_EQ(boxCount(), 1);

    m_score->undoStack()->undo(nullptr);
    EXPECT_EQ(boxCount(), 0);
}
