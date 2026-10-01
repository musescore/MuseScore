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

#include "draw/bufferedpaintprovider.h"
#include "draw/painter.h"
#include "modularity/ioc.h"

#include "engraving/dom/barline.h"
#include "engraving/dom/bracket.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafflines.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/system.h"
#include "engraving/dom/timesig.h"
#include "engraving/rendering/iscorerenderer.h"
#include "engraving/rendering/paintoptions.h"

#include "engraving/editing/editproperty.h"
#include "engraving/editing/edittimesig.h"
#include "engraving/editing/transaction/transaction.h"

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
    EXPECT_TRUE(score);

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
            EXPECT_TRUE(seg);

            BarLine* bar = toBarLine(seg->element(0));
            EXPECT_TRUE(bar);
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
    EXPECT_TRUE(score);

    Measure* msr = score->firstMeasure()->nextMeasure();
    TimeSig* ts  = Factory::createTimeSig(score->dummy()->segment());
    ts->setSig(Fraction(3, 4), TimeSigType::NORMAL);

    EditTimeSig::addTimeSig(score->transactionManager()->currentOrDummyTransaction(), score, msr, 0, ts, false);
    score->doLayout();

    msr = score->firstMeasure();
    while ((msr = msr->nextMeasure())) {
        Segment* seg = msr->findSegment(SegmentType::EndBarLine, msr->endTick());
        EXPECT_TRUE(seg);

        BarLine* bar = static_cast<BarLine*>(seg->element(0));
        EXPECT_TRUE(bar);

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
    EXPECT_TRUE(score);

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
    EXPECT_TRUE(seg);

    BarLine* bar = toBarLine(seg->element(0));
    EXPECT_TRUE(bar);

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
    EXPECT_TRUE(score);

    score->doLayout();

    score->startCmd(TranslatableString::untranslatable("Engraving barline tests"));
    // 'go' to 5th measure
    Measure* msr = score->firstMeasure();
    for (int i=0; i < 4; i++) {
        msr = msr->nextMeasure();
    }
    // check span data of measure-initial start-repeat bar line
    Segment* seg = msr->findSegment(SegmentType::StartRepeatBarLine, msr->tick());
    EXPECT_TRUE(seg);

    BarLine* bar = static_cast<BarLine*>(seg->element(0));
    EXPECT_TRUE(bar);

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
    EXPECT_TRUE(score);

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
    lb->setOwnershipParent(msr);
    score->undoAddElement(lb);
    score->doLayout();

    // check an end-repeat bar line has been created at the end of this measure and it is generated
    Segment* seg = msr->findSegment(SegmentType::EndBarLine, msr->tick() + msr->ticks());
    EXPECT_TRUE(seg);

    BarLine* bar = static_cast<BarLine*>(seg->element(0));
    EXPECT_TRUE(bar);

    EXPECT_EQ(bar->barLineType(), BarLineType::END_REPEAT);
    EXPECT_TRUE(bar->generated());

    // // check an end-repeat bar line has been created at the beginning of the next measure and it is not generated
    // check an end-repeat bar line has been created at the beginning of the next measure and it is generated
    msr = msr->nextMeasure();
    seg = msr->findSegment(SegmentType::StartRepeatBarLine, msr->tick());
    EXPECT_TRUE(seg);

    bar = static_cast<BarLine*>(seg->element(0));
    EXPECT_TRUE(bar);
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
    EXPECT_TRUE(score);

    score->doLayout();

    // scan each measure
    Measure* msr   = score->firstMeasure();
    for (int i=0; i < 3; i++) {
        // locate end-measure bar line segment
        Segment* seg = msr->findSegment(SegmentType::EndBarLine, msr->tick() + msr->ticks());
        EXPECT_TRUE(seg);

        // check only i-th staff has custom bar line type
        for (int j=0; j < 3; j++) {
            BarLine* bar = static_cast<BarLine*>(seg->element(j * VOICES));
            EXPECT_TRUE(bar);

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
    BarLine* barLine = Factory::createBarLine(e->score()->dummy()->segment());
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
    EXPECT_TRUE(score);

    score->doLayout();

    Measure* m = score->firstMeasure();

    // drop NORMAL onto initial START_REPEAT barline will remove that START_REPEAT
    dropNormalBarline(m->findSegment(SegmentType::StartRepeatBarLine, m->tick())->element(0));
    EXPECT_EQ(m->findSegment(SegmentType::StartRepeatBarLine, Fraction(0, 1)), nullptr);

    // drop NORMAL onto END_START_REPEAT will turn into NORMAL
    dropNormalBarline(m->findSegment(SegmentType::EndBarLine, m->endTick())->element(0));
    BarLine* bar = static_cast<BarLine*>(m->findSegment(SegmentType::EndBarLine, m->endTick())->element(0));
    EXPECT_TRUE(bar);
    EXPECT_EQ(bar->barLineType(), BarLineType::NORMAL);

    m = m->nextMeasure();

    // drop NORMAL onto the END_REPEAT part of an END_START_REPEAT straddling a newline will turn into NORMAL at the end of this meas
    dropNormalBarline(m->findSegment(SegmentType::EndBarLine, m->endTick())->element(0));
    bar = static_cast<BarLine*>(m->findSegment(SegmentType::EndBarLine, m->endTick())->element(0));
    EXPECT_TRUE(bar);
    EXPECT_EQ(bar->barLineType(), BarLineType::NORMAL);

    m = m->nextMeasure();

    // but leave START_REPEAT at the beginning of the newline
    bar = static_cast<BarLine*>(m->findSegment(SegmentType::StartRepeatBarLine, m->tick())->element(0));
    EXPECT_TRUE(bar);

    // drop NORMAL onto the meas ending with an END_START_REPEAT straddling a newline will turn into NORMAL at the end of this meas
    // but note I'm not verifying what happens to the START_REPEAT at the beginning of the newline...I'm not sure that behavior is well-defined yet
    dropNormalBarline(m);
    bar = static_cast<BarLine*>(m->findSegment(SegmentType::EndBarLine, m->endTick())->element(0));
    EXPECT_TRUE(bar);
    EXPECT_EQ(bar->barLineType(), BarLineType::NORMAL);

    m = m->nextMeasure();
    m = m->nextMeasure();

    // drop NORMAL onto the START_REPEAT part of an END_START_REPEAT straddling a newline will remove the START_REPEAT at the beginning of this measure
    dropNormalBarline(m->findSegment(SegmentType::StartRepeatBarLine, m->tick())->element(0));
    EXPECT_EQ(m->findSegment(SegmentType::StartRepeatBarLine, m->tick()), nullptr);

    // but leave END_REPEAT at the end of previous line
    bar = static_cast<BarLine*>(m->prevMeasure()->findSegment(SegmentType::EndBarLine, m->tick())->element(0));
    EXPECT_TRUE(bar);
    EXPECT_EQ(bar->barLineType(), BarLineType::END_REPEAT);

    for (int i = 0; i < 4; i++, m = m->nextMeasure()) {
        // drop NORMAL onto END_REPEAT, BROKEN, DOTTED, DOUBLE at the end of this meas will turn into NORMAL
        dropNormalBarline(m->findSegment(SegmentType::EndBarLine, m->endTick())->element(0));
        bar = static_cast<BarLine*>(m->findSegment(SegmentType::EndBarLine, m->endTick())->element(0));
        EXPECT_TRUE(bar);
        EXPECT_EQ(bar->barLineType(), BarLineType::NORMAL);
    }

    m = m->nextMeasure();

    // drop NORMAL onto a START_REPEAT in middle of a line will remove the START_REPEAT at the beginning of this measure
    dropNormalBarline(m->findSegment(SegmentType::StartRepeatBarLine, m->tick())->element(0));
    EXPECT_EQ(m->findSegment(SegmentType::StartRepeatBarLine, m->tick()), nullptr);

    // drop NORMAL onto final END_REPEAT at end of score will turn into NORMAL
    dropNormalBarline(m->findSegment(SegmentType::EndBarLine, m->endTick())->element(0));
    bar = static_cast<BarLine*>(m->findSegment(SegmentType::EndBarLine, m->endTick())->element(0));
    EXPECT_TRUE(bar);
    EXPECT_EQ(bar->barLineType(), BarLineType::NORMAL);

    delete score;
}

//---------------------------------------------------------
//   deleteSkipBarlines
//---------------------------------------------------------
TEST_F(Engraving_BarlineTests, deleteSkipBarlines)
{
    MasterScore* score = ScoreRW::readScore(BARLINE_DATA_DIR + "barlinedelete.mscx");
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    EXPECT_TRUE(m1);

    score->startCmd(TranslatableString::untranslatable("Engraving barline tests"));
    score->cmdSelectAll();
    score->cmdDeleteSelection();
    score->endCmd();

    score->doLayout();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String("barlinedelete.mscx"), BARLINE_DATA_DIR + String("barlinedelete-ref.mscx")));

    delete score;
}

namespace {
struct PaintedBarLine {
    muse::draw::Pen pen;
    muse::PointF from;
    muse::PointF to;
};

void collectBarLines(const muse::draw::DrawData& data, const muse::draw::DrawData::Item& item,
                     std::vector<PaintedBarLine>& lines)
{
    for (const auto& batch : item.datas) {
        for (const auto& polygon : batch.polygons) {
            if (polygon.mode == muse::draw::PolygonMode::Polyline && polygon.polygon.size() == 2) {
                lines.push_back({ data.states.at(batch.state).pen, polygon.polygon.at(0), polygon.polygon.at(1) });
            }
        }
    }
    for (const auto& child : item.chilren) {
        collectBarLines(data, child, lines);
    }
}

std::vector<PaintedBarLine> paintedBarLines(const BarLine* bar)
{
    auto provider = std::make_shared<muse::draw::BufferedPaintProvider>();
    muse::draw::Painter painter(provider, "barline-span-style");
    auto renderer = muse::modularity::globalIoc()->resolve<rendering::IScoreRenderer>("barline_tests");
    renderer->paintItem(painter, bar, rendering::PaintOptions());
    painter.endDraw();
    std::vector<PaintedBarLine> lines;
    auto data = provider->drawData();
    collectBarLines(*data, data->item, lines);
    return lines;
}

BarLine* endBarLine(Measure* measure, staff_idx_t staff)
{
    return toBarLine(measure->findSegment(SegmentType::EndBarLine, measure->endTick())->element(staff * VOICES));
}
}

TEST_F(Engraving_BarlineTests, spanStyleIndependentConnections)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-style.mscx"));
    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 3);
    for (Measure* measure = score->firstMeasure(); measure != score->lastMeasure()->prevMeasure(); measure = measure->nextMeasure()) {
        for (staff_idx_t staff = 0; staff < 2; ++staff) {
            BarLine* bar = endBarLine(measure, staff);
            ASSERT_TRUE(bar);
            EXPECT_EQ(bar->barLineType(), BarLineType::NORMAL);
            auto lines = paintedBarLines(bar);
            ASSERT_EQ(lines.size(), 2);
            EXPECT_EQ(lines[0].pen.style(), muse::draw::PenStyle::SolidLine);
            EXPECT_EQ(lines[1].pen.style(), staff == 0 ? muse::draw::PenStyle::CustomDashLine : muse::draw::PenStyle::DotLine);
            EXPECT_DOUBLE_EQ(lines[0].to.y(), lines[1].from.y());
            EXPECT_DOUBLE_EQ(lines[0].from.x(), lines[1].from.x());
            EXPECT_GT(lines[1].to.y(), lines[1].from.y());
            EXPECT_EQ(endBarLine(measure, 2)->ldata()->spanStyle, BarLineSpanStyle::DEFAULT);
        }
    }
    score->staff(0)->setProperty(Pid::STAFF_BARLINE_SPAN_STYLE, static_cast<int>(BarLineSpanStyle::SOLID));
    score->doLayout();
    EXPECT_EQ(paintedBarLines(endBarLine(score->firstMeasure(), 0))[1].pen.style(), muse::draw::PenStyle::SolidLine);
    EXPECT_EQ(paintedBarLines(endBarLine(score->firstMeasure(), 1))[1].pen.style(), muse::draw::PenStyle::DotLine);
}

TEST_F(Engraving_BarlineTests, spanStylePreservesInteriorAndSpecialBars)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-style.mscx"));
    ASSERT_TRUE(score);
    BarLine* bar = endBarLine(score->firstMeasure(), 0);
    score->staff(0)->setBarLineSpanStyle(BarLineSpanStyle::SOLID);
    for (BarLineType type : { BarLineType::BROKEN, BarLineType::DOTTED }) {
        bar->setBarLineType(type);
        bar->calcY();
        auto lines = paintedBarLines(bar);
        ASSERT_EQ(lines.size(), 2);
        EXPECT_EQ(lines[0].pen.style(), type == BarLineType::BROKEN ? muse::draw::PenStyle::CustomDashLine : muse::draw::PenStyle::DotLine);
        EXPECT_EQ(lines[1].pen.style(), muse::draw::PenStyle::SolidLine);
    }
    for (BarLineType type : { BarLineType::DOUBLE, BarLineType::END, BarLineType::REVERSE_END,
                              BarLineType::START_REPEAT, BarLineType::END_REPEAT, BarLineType::END_START_REPEAT,
                              BarLineType::HEAVY, BarLineType::DOUBLE_HEAVY }) {
        bar->setBarLineType(type);
        score->staff(0)->setBarLineSpanStyle(BarLineSpanStyle::DEFAULT);
        bar->calcY();
        auto original = paintedBarLines(bar);
        score->staff(0)->setBarLineSpanStyle(BarLineSpanStyle::DASHED);
        bar->calcY();
        auto custom = paintedBarLines(bar);
        EXPECT_EQ(bar->ldata()->spanStyle, BarLineSpanStyle::DEFAULT);
        ASSERT_EQ(original.size(), custom.size());
        for (size_t i = 0; i < original.size(); ++i) {
            EXPECT_EQ(original[i].pen, custom[i].pen);
            EXPECT_EQ(original[i].from, custom[i].from);
            EXPECT_EQ(original[i].to, custom[i].to);
        }
    }
}

TEST_F(Engraving_BarlineTests, spanStyleRoundTripAndReset)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-style.mscx"));
    ASSERT_TRUE(score);
    const String path(u"barline-span-style-roundtrip.mscx");
    ASSERT_TRUE(ScoreRW::saveScore(score.get(), path));
    std::unique_ptr<MasterScore> reloaded(ScoreRW::readScore(path, true));
    ASSERT_TRUE(reloaded);
    EXPECT_EQ(reloaded->staff(0)->barLineSpanStyle(), BarLineSpanStyle::DASHED);
    EXPECT_EQ(reloaded->staff(1)->barLineSpanStyle(), BarLineSpanStyle::DOTTED);
    EXPECT_EQ(reloaded->staff(2)->barLineSpanStyle(), BarLineSpanStyle::DEFAULT);
    reloaded->staff(0)->setProperty(Pid::STAFF_BARLINE_SPAN_STYLE, reloaded->staff(0)->propertyDefault(Pid::STAFF_BARLINE_SPAN_STYLE));
    ASSERT_TRUE(ScoreRW::saveScore(reloaded.get(), path));
    reloaded.reset(ScoreRW::readScore(path, true));
    ASSERT_TRUE(reloaded);
    EXPECT_EQ(reloaded->staff(0)->barLineSpanStyle(), BarLineSpanStyle::DEFAULT);
    EXPECT_EQ(paintedBarLines(endBarLine(reloaded->firstMeasure(), 0)).size(), 1);
}

TEST_F(Engraving_BarlineTests, spanStyleUndoRedoAndStaffCopy)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-style.mscx"));
    ASSERT_TRUE(score);
    score->startCmd(TranslatableString::untranslatable("Change connection style"));
    score->staff(0)->undoChangeProperty(Pid::STAFF_BARLINE_SPAN_STYLE, static_cast<int>(BarLineSpanStyle::SOLID));
    score->endCmd();
    EXPECT_EQ(score->staff(0)->barLineSpanStyle(), BarLineSpanStyle::SOLID);
    score->undoRedo(true, nullptr);
    EXPECT_EQ(score->staff(0)->barLineSpanStyle(), BarLineSpanStyle::DASHED);
    score->undoRedo(false, nullptr);
    EXPECT_EQ(score->staff(0)->barLineSpanStyle(), BarLineSpanStyle::SOLID);
    std::unique_ptr<Staff> copy(score->staff(0)->clone());
    EXPECT_EQ(copy->barLineSpanStyle(), BarLineSpanStyle::SOLID);
    EXPECT_FALSE(score->staff(0)->setProperty(Pid::STAFF_BARLINE_SPAN_STYLE, -1));
    EXPECT_FALSE(score->staff(0)->setProperty(Pid::STAFF_BARLINE_SPAN_STYLE, 4));
    EXPECT_EQ(score->staff(0)->barLineSpanStyle(), BarLineSpanStyle::SOLID);
}

TEST_F(Engraving_BarlineTests, spanStyleStaffGeometry)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-style.mscx"));
    ASSERT_TRUE(score);
    for (int lineCount : { 1, 2, 5 }) {
        StaffType type = *score->staff(0)->staffType(Fraction(0, 1));
        type.setLines(lineCount);
        type.setLineDistance(Spatium(1.3));
        type.setUserMag(.7);
        score->staff(0)->setStaffType(Fraction(0, 1), type);
        score->doLayout();
        BarLine* bar = endBarLine(score->firstMeasure(), 0);
        const StaffType* current = score->staff(0)->staffType(Fraction(0, 1));
        double expected = current->yoffset().val() * current->spatium()
                          + (lineCount == 1 ? 2 : lineCount - 1) * current->lineDistance().val() * current->spatium()
                          + score->style().styleS(Sid::staffLineWidth).val() * current->spatium() * .5;
        EXPECT_NEAR(bar->ldata()->spanStartY, expected, 1e-6);
        auto lines = paintedBarLines(bar);
        ASSERT_EQ(lines.size(), 2);
        EXPECT_NEAR(lines[1].from.y(), expected, 1e-6);
    }
}

TEST_F(Engraving_BarlineTests, spanStyleHiddenAndDisconnectedStaves)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-style.mscx"));
    ASSERT_TRUE(score);
    score->staff(1)->setProperty(Pid::VISIBLE, false);
    score->doLayout();
    BarLine* bar = endBarLine(score->firstMeasure(), 0);
    EXPECT_EQ(bar->ldata()->spanStyle, BarLineSpanStyle::DASHED);
    EXPECT_NEAR(bar->ldata()->y2, score->firstMeasure()->staffLines(2)->y1() - score->firstMeasure()->system()->staff(0)->y(), 1e-6);
    bar->setSpanStaff(false);
    bar->calcY();
    EXPECT_EQ(bar->ldata()->spanStyle, BarLineSpanStyle::DEFAULT);
    EXPECT_EQ(paintedBarLines(bar).size(), 1);
    EXPECT_EQ(score->staff(0)->barLineSpanStyle(), BarLineSpanStyle::DASHED);
}

TEST_F(Engraving_BarlineTests, spanStyleDashWidthAndOffsets)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(BARLINE_DATA_DIR + "barline-span-style.mscx"));
    ASSERT_TRUE(score);
    score->style().set(Sid::dashBarWidth, Spatium(.6));
    score->style().set(Sid::dashBarDash, Spatium(1.2));
    score->style().set(Sid::dashBarGap, Spatium(.8));
    score->doLayout();
    BarLine* bar = endBarLine(score->firstMeasure(), 0);
    auto lines = paintedBarLines(bar);
    ASSERT_EQ(lines.size(), 2);
    EXPECT_NEAR(lines[1].pen.widthF(), score->style().styleAbsolute(Sid::dashBarWidth) * bar->mag(), 1e-6);
    EXPECT_DOUBLE_EQ(lines[0].from.x(), lines[1].from.x());
    ASSERT_EQ(lines[1].pen.dashPattern().size(), 2);
    EXPECT_NEAR(lines[1].pen.dashPattern()[0], 2.0, 1e-6);
    EXPECT_NEAR(lines[1].pen.dashPattern()[1], .8 / .6, 1e-6);
    EXPECT_LE(bar->ldata()->bbox().left(), lines[1].from.x() - lines[1].pen.widthF() * .5);
    EXPECT_GE(bar->ldata()->bbox().right(), lines[1].from.x() + lines[1].pen.widthF() * .5);
    bar->setSpanFrom(10);
    bar->calcY();
    lines = paintedBarLines(bar);
    ASSERT_EQ(lines.size(), 1);
    EXPECT_EQ(lines[0].pen.style(), muse::draw::PenStyle::CustomDashLine);
    EXPECT_GT(lines[0].to.y(), lines[0].from.y());
}
