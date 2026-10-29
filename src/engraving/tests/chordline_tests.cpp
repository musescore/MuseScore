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

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordline.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/segment.h"
#include "engraving/editing/transaction/undostack.h"
#include "engraving/rendering/score/tlayout.h"
#include "engraving/rw/compat/compatutils.h"
#include "engraving/rw/xmlreader.h"

#include "utils/scorerw.h"
#include "utils/testutils.h"

using namespace mu;
using namespace mu::engraving;
using muse::draw::PainterPath;

namespace {
void expectNear(const PointF& actual, const PointF& expected)
{
    EXPECT_NEAR(actual.x(), expected.x(), 1e-6);
    EXPECT_NEAR(actual.y(), expected.y(), 1e-6);
}

void expectPoints(const ChordLine::Points& actual, const ChordLine::Points& expected)
{
    for (size_t i = 0; i < actual.size(); ++i) {
        expectNear(actual[i], expected[i]);
    }
}

void expectPath(const PainterPath& path, const std::vector<std::pair<PainterPath::ElementType, PointF> >& expected)
{
    ASSERT_EQ(path.elementCount(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        const PainterPath::Element& e = path.elementAt(i);
        EXPECT_EQ(e.type, expected[i].first);
        expectNear(PointF(e.x, e.y), expected[i].second);
    }
}

PointF pathEnd(const ChordLine* chordLine)
{
    const PainterPath& path = chordLine->ldata()->path;
    const PainterPath::Element& e = path.elementAt(path.elementCount() - 1);
    return PointF(e.x, e.y);
}
}

//---------------------------------------------------------
//   Unit tests: a chord line on a chord that isn't part of a score
//---------------------------------------------------------

class Engraving_ChordLineTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_score = compat::ScoreAccess::createMasterScore(nullptr);
        m_chord = Factory::createChord(m_score->dummy());
        m_chordLine = Factory::createChordLine(m_chord);
        m_chordLine->setChordLineType(ChordLineType::FALL);
    }

    void TearDown() override
    {
        delete m_chordLine;
        delete m_chord;
        delete m_score;
    }

    void drag(Grip grip, const PointF& delta)
    {
        EditData ed(nullptr);
        ed.curGrip = grip;
        ed.delta = delta;
        m_chordLine->dragGrip(ed);
    }

    // A legacy <Path> of a fall whose end point was moved by (1, 0.5) spaces, saved with the given scale
    void readLegacyPath(double scale)
    {
        auto element = [](int type, double x, double y) {
            return "<Element type=\"" + std::to_string(type) + "\" x=\"" + std::to_string(x) + "\" y=\"" + std::to_string(y) + "\"/>";
        };
        const std::string xml = "<Path>"
                                + element(0, 0.0, 0.0)
                                + element(2, 0.6 * scale, 0.0)
                                + element(3, 1.2 * scale, 0.5 * scale)
                                + element(3, 2.2 * scale, 1.5 * scale)
                                + "</Path>";
        XmlReader e(muse::ByteArray(xml.c_str()));
        e.readNextStartElement();
        compat::CompatUtils::readChordLinePath(m_chordLine, e);
    }

    MasterScore* m_score = nullptr;
    Chord* m_chord = nullptr;
    ChordLine* m_chordLine = nullptr;
};

TEST_F(Engraving_ChordLineTests, defaultPoints)
{
    m_chordLine->setChordLineType(ChordLineType::FALL);
    expectPoints(m_chordLine->defaultPoints(), { PointF(0, 0), PointF(0.6, 0), PointF(1.2, 0.5), PointF(1.2, 1) });

    m_chordLine->setChordLineType(ChordLineType::DOIT);
    expectPoints(m_chordLine->defaultPoints(), { PointF(0, 0), PointF(0.6, 0), PointF(1.2, -0.5), PointF(1.2, -1) });

    m_chordLine->setChordLineType(ChordLineType::SCOOP);
    expectPoints(m_chordLine->defaultPoints(), { PointF(0, 0), PointF(0, 0.5), PointF(-0.6, 1), PointF(-1.2, 1) });

    m_chordLine->setChordLineType(ChordLineType::PLOP);
    expectPoints(m_chordLine->defaultPoints(), { PointF(0, 0), PointF(0, -0.5), PointF(-0.6, -1), PointF(-1.2, -1) });
}

TEST_F(Engraving_ChordLineTests, bezierOffsetProperties)
{
    const std::array<ChordLine::Point, 4> points { ChordLine::Point::START, ChordLine::Point::BEZIER1, ChordLine::Point::BEZIER2,
                                                   ChordLine::Point::END };

    for (ChordLine::Point point : points) {
        EXPECT_EQ(m_chordLine->propertyDefault(ChordLine::bezierOffsetPid(point)).value<PointF>(), PointF());
    }

    // [WHEN] Setting a different offset on each point through its property
    for (size_t i = 0; i < points.size(); ++i) {
        m_chordLine->setProperty(ChordLine::bezierOffsetPid(points[i]), PointF(double(i + 1), -double(i + 1)));
    }

    // [THEN] Each property maps to its own point
    for (size_t i = 0; i < points.size(); ++i) {
        const PointF expected(double(i + 1), -double(i + 1));
        expectNear(m_chordLine->bezierOffset(points[i]), expected);
        expectNear(m_chordLine->getProperty(ChordLine::bezierOffsetPid(points[i])).value<PointF>(), expected);
    }
}

TEST_F(Engraving_ChordLineTests, gripsCount)
{
    EXPECT_EQ(m_chordLine->gripsCount(), 4);

    m_chordLine->setStraight(true);
    EXPECT_EQ(m_chordLine->gripsCount(), 1);

    m_chordLine->setStraight(false);
    m_chordLine->setWavy(true);
    EXPECT_EQ(m_chordLine->gripsCount(), 0);

    m_chordLine->setWavy(false);
    m_chordLine->setChordLineType(ChordLineType::NOTYPE);
    EXPECT_EQ(m_chordLine->gripsCount(), 0);
}

TEST_F(Engraving_ChordLineTests, cloneKeepsEdits)
{
    // [GIVEN] An edited chord line
    m_chordLine->setBezierOffset(ChordLine::Point::BEZIER1, PointF(0.25, -0.5));
    m_chordLine->setBezierOffset(ChordLine::Point::END, PointF(1.0, 0.5));
    m_chordLine->setPlayChordLine(false);

    // [WHEN] Cloning it (e.g. when creating parts)
    ChordLine* clone = m_chordLine->clone();

    // [THEN] The clone has the same edits
    expectPoints(clone->bezierOffsets(), m_chordLine->bezierOffsets());
    EXPECT_FALSE(clone->playChordLine());

    delete clone;
}

TEST_F(Engraving_ChordLineTests, layoutPath)
{
    ChordLine::LayoutData* ldata = m_chordLine->mutldata();
    m_chordLine->setBezierOffset(ChordLine::Point::END, PointF(10.0, 5.0));

    // [THEN] A curved line is the default bezier, in spatium units, plus the offsets
    const double spatium = 10.0;
    rendering::score::TLayout::layoutChordLinePath(m_chordLine, ldata, spatium, 1.0);
    expectPath(ldata->path, { { PainterPath::ElementType::MoveToElement, PointF(0, 0) },    // (0 * 10, 0 * 10)
                   { PainterPath::ElementType::CurveToElement, PointF(6, 0) },              // (0.6 * 10, 0 * 10)
                   { PainterPath::ElementType::CurveToDataElement, PointF(12, 5) },         // (1.2 * 10, 0.5 * 10)
                   { PainterPath::ElementType::CurveToDataElement, PointF(22, 15) } });     // (1.2 * 10 + 10, 1 * 10 + 5)

    // [THEN] The chord's mag scales the whole shape, offsets included
    rendering::score::TLayout::layoutChordLinePath(m_chordLine, ldata, spatium, 0.5);
    expectPath(ldata->path, { { PainterPath::ElementType::MoveToElement, PointF(0, 0) },
                   { PainterPath::ElementType::CurveToElement, PointF(3, 0) },
                   { PainterPath::ElementType::CurveToDataElement, PointF(6, 2.5) },
                   { PainterPath::ElementType::CurveToDataElement, PointF(11, 7.5) } });

    // [THEN] A straight line only uses the start and end points
    m_chordLine->setStraight(true);
    rendering::score::TLayout::layoutChordLinePath(m_chordLine, ldata, spatium, 1.0);
    expectPath(ldata->path, { { PainterPath::ElementType::MoveToElement, PointF(0, 0) },    // (0 * 10, 0 * 10)
                   { PainterPath::ElementType::LineToElement, PointF(22, 15) } });          // (1.2 * 10 + 10, 1 * 10 + 5)

    // [THEN] A wavy line has no path
    m_chordLine->setWavy(true);
    rendering::score::TLayout::layoutChordLinePath(m_chordLine, ldata, spatium, 1.0);
    EXPECT_EQ(ldata->path.elementCount(), 0);
}

TEST_F(Engraving_ChordLineTests, chordMagFollowsChord)
{
    EXPECT_DOUBLE_EQ(m_chordLine->chordMag(), 1.0);

    m_chord->setSmall(true);
    EXPECT_DOUBLE_EQ(m_chordLine->chordMag(), m_score->style().styleD(Sid::smallNoteMag));
}

TEST_F(Engraving_ChordLineTests, dragGripMovesItsPoint)
{
    const double sp = m_chordLine->spatium();

    // [WHEN] Dragging the first control point of a curved line
    drag(Grip(1), PointF(sp, -sp));

    // [THEN] Only that point is offset
    expectNear(m_chordLine->bezierOffset(ChordLine::Point::START), PointF());
    expectNear(m_chordLine->bezierOffset(ChordLine::Point::BEZIER1), PointF(sp, -sp));
    expectNear(m_chordLine->bezierOffset(ChordLine::Point::BEZIER2), PointF());
    expectNear(m_chordLine->bezierOffset(ChordLine::Point::END), PointF());
}

TEST_F(Engraving_ChordLineTests, dragGripOnSmallChord)
{
    // [GIVEN] A chord line on a small chord
    m_chord->setSmall(true);
    const double mag = m_chordLine->chordMag();
    ASSERT_LT(mag, 1.0);
    const double sp = m_chordLine->spatium();

    // [WHEN] Dragging its end point
    drag(Grip(3), PointF(sp, 0.5 * sp));

    // [THEN] The drawn end point moves by exactly the dragged distance
    ChordLine::LayoutData* ldata = m_chordLine->mutldata();
    rendering::score::TLayout::layoutChordLinePath(m_chordLine, ldata, sp, mag);
    const PointF defaultEnd = m_chordLine->defaultPoints()[size_t(ChordLine::Point::END)] * sp * mag;
    expectNear(pathEnd(m_chordLine), defaultEnd + PointF(sp, 0.5 * sp));
}

TEST_F(Engraving_ChordLineTests, dragGripIgnoredForWavyLines)
{
    m_chordLine->setWavy(true);

    drag(Grip(0), PointF(10.0, 10.0));

    expectPoints(m_chordLine->bezierOffsets(), ChordLine::Points());
}

TEST_F(Engraving_ChordLineTests, writeReadBezierOffsets)
{
    // [GIVEN] An unedited chord line
    // [THEN] No offsets are saved
    const String unedited = String::fromUtf8(ScoreRW::writeElement(m_chordLine));
    for (size_t i = 0; i < size_t(ChordLine::Point::POINTS_COUNT); ++i) {
        const String tag = u"<" + String::fromAscii(propertyName(ChordLine::bezierOffsetPid(ChordLine::Point(i))));
        EXPECT_FALSE(unedited.contains(tag));
    }

    // [GIVEN] An edited chord line
    const double sp = m_chordLine->spatium();
    m_chordLine->setBezierOffset(ChordLine::Point::BEZIER1, PointF(0.25 * sp, -0.5 * sp));
    m_chordLine->setBezierOffset(ChordLine::Point::END, PointF(1.0 * sp, 0.5 * sp));

    // [WHEN] Saving and reading it back
    ChordLine* read = toChordLine(ScoreRW::writeReadElement(m_chordLine));

    // [THEN] It has the edits
    EXPECT_EQ(read->chordLineType(), ChordLineType::FALL);
    expectPoints(read->bezierOffsets(), m_chordLine->bezierOffsets());

    delete read;
}

TEST_F(Engraving_ChordLineTests, readLegacyPath)
{
    // [WHEN] Reading a path saved before offsets were stored per point
    readLegacyPath(1.0);

    // [THEN] Only the end point, which differs from the default shape, is offset
    const double sp = m_chordLine->spatium();
    expectPoints(m_chordLine->bezierOffsets(), { PointF(), PointF(), PointF(), PointF(1.0 * sp, 0.5 * sp) });
}

TEST_F(Engraving_ChordLineTests, readLegacyStraightPath)
{
    m_chordLine->setStraight(true);
    XmlReader e(muse::ByteArray("<Path><Element type=\"0\" x=\"0\" y=\"0\"/><Element type=\"1\" x=\"2.2\" y=\"1.5\"/></Path>"));
    e.readNextStartElement();

    compat::CompatUtils::readChordLinePath(m_chordLine, e);

    const double sp = m_chordLine->spatium();
    expectPoints(m_chordLine->bezierOffsets(), { PointF(), PointF(), PointF(), PointF(1.0 * sp, 0.5 * sp) });
}

TEST_F(Engraving_ChordLineTests, readLegacyPathSmallChord)
{
    // [GIVEN] A chord line on a small chord
    m_chord->setSmall(true);
    ASSERT_LT(m_chordLine->chordMag(), 1.0);

    // [WHEN] Reading a legacy path
    readLegacyPath(1.0);

    // [THEN] It's laid out where the saved path was drawn, at the saved points times spatium
    const double sp = m_chordLine->spatium();
    ChordLine::LayoutData* ldata = m_chordLine->mutldata();
    rendering::score::TLayout::layoutChordLinePath(m_chordLine, ldata, sp, m_chordLine->chordMag());
    expectPath(ldata->path, { { PainterPath::ElementType::MoveToElement, PointF(0, 0) },
                   { PainterPath::ElementType::CurveToElement, PointF(0.6 * sp, 0) },
                   { PainterPath::ElementType::CurveToDataElement, PointF(1.2 * sp, 0.5 * sp) },
                   { PainterPath::ElementType::CurveToDataElement, PointF(2.2 * sp, 1.5 * sp) } });
}

//---------------------------------------------------------
//   Integration tests: a chord line in a score with parts and undo
//---------------------------------------------------------

static const String CHORDLINE_DATA_DIR(u"chordline_data/");

class Engraving_ChordLineScoreTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_score = ScoreRW::readScore(CHORDLINE_DATA_DIR + u"chordline.mscx");
        ASSERT_TRUE(m_score);
        m_chordLine = findChordLine(m_score);
        ASSERT_TRUE(m_chordLine);
    }

    void TearDown() override
    {
        delete m_score;
    }

    static ChordLine* findChordLine(Score* score)
    {
        for (Segment* s = score->firstSegment(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
            EngravingItem* e = s->element(0);
            if (!e || !e->isChord()) {
                continue;
            }
            for (EngravingItem* el : toChord(e)->el()) {
                if (el->isChordLine()) {
                    return toChordLine(el);
                }
            }
        }
        return nullptr;
    }

    // Checks that the laid out path ends at the default end point moved by the given absolute offset
    static void expectLaidOutEndOffset(const ChordLine* chordLine, const PointF& endOffset)
    {
        const PointF defaultEnd = chordLine->defaultPoints()[size_t(ChordLine::Point::END)];
        expectNear(pathEnd(chordLine), (defaultEnd * chordLine->spatium() + endOffset) * chordLine->chordMag());
    }

    static void editEnd(ChordLine* chordLine, const PointF& offset)
    {
        Score* score = chordLine->score();
        score->startCmd(TranslatableString::untranslatable("Engraving chord line tests"));
        chordLine->undoChangeProperty(ChordLine::bezierOffsetPid(ChordLine::Point::END), offset);
        score->endCmd();
    }

    static void dragGrip(ChordLine* chordLine, Grip grip, const PointF& delta)
    {
        Score* score = chordLine->score();
        EditData ed(nullptr);
        ed.curGrip = grip;
        ed.delta = delta;

        score->startCmd(TranslatableString::untranslatable("Engraving chord line tests"));
        chordLine->startDragGrip(ed);
        chordLine->dragGrip(ed);
        chordLine->endDragGrip(ed);
        score->endCmd();
    }

    MasterScore* m_score = nullptr;
    ChordLine* m_chordLine = nullptr;
};

TEST_F(Engraving_ChordLineScoreTests, editedChordLineInNewPart)
{
    // [GIVEN] A score with a chord line whose end point was moved by (1, 0.5) spaces
    const double sp = m_chordLine->spatium();
    editEnd(m_chordLine, PointF(1.0 * sp, 0.5 * sp));

    // [WHEN] Creating a part from it
    TestUtils::createParts(m_score, 1);
    ASSERT_EQ(m_score->excerpts().size(), 1);

    // [THEN] The part's chord line keeps the edited shape, scaled to the part's spatium
    ChordLine* partChordLine = findChordLine(m_score->excerpts().front()->excerptScore());
    ASSERT_TRUE(partChordLine);
    const double partSp = partChordLine->spatium();
    expectNear(partChordLine->bezierOffset(ChordLine::Point::END), PointF(1.0 * partSp, 0.5 * partSp));
    expectLaidOutEndOffset(partChordLine, PointF(1.0 * partSp, 0.5 * partSp));
}

TEST_F(Engraving_ChordLineScoreTests, dragGripPropagatesToExistingPart)
{
    // [GIVEN] A score with a chord line and an existing part
    TestUtils::createParts(m_score, 1);

    Score* partScore = m_score->excerpts().front()->excerptScore();
    partScore->setIsOpen(true); // only open parts are laid out on update
    ChordLine* partChordLine = findChordLine(partScore);
    ASSERT_TRUE(partChordLine);

    // [WHEN] Dragging the end grip in the main score
    const double sp = m_chordLine->spatium();
    dragGrip(m_chordLine, Grip(3), PointF(sp, 0.5 * sp)); // curved line: grip 3 moves the end point

    // [THEN] The main score and the part both have the edited shape, the part's scaled to its spatium
    const PointF offset(sp, 0.5 * sp);
    const double partSp = partChordLine->spatium();
    const PointF partOffset(partSp, 0.5 * partSp);
    expectNear(m_chordLine->bezierOffset(ChordLine::Point::END), offset);
    expectNear(partChordLine->bezierOffset(ChordLine::Point::END), partOffset);
    expectLaidOutEndOffset(m_chordLine, offset);
    expectLaidOutEndOffset(partChordLine, partOffset);

    // [WHEN] Undoing
    m_score->undoRedo(true, nullptr);

    // [THEN] Both go back to the default shape
    expectNear(m_chordLine->bezierOffset(ChordLine::Point::END), PointF());
    expectNear(partChordLine->bezierOffset(ChordLine::Point::END), PointF());
    expectLaidOutEndOffset(m_chordLine, PointF());
    expectLaidOutEndOffset(partChordLine, PointF());
}

TEST_F(Engraving_ChordLineScoreTests, resetRestoresDefaultShape)
{
    // [GIVEN] A score with an edited chord line
    const PointF offset(m_chordLine->spatium(), 0.5 * m_chordLine->spatium());
    editEnd(m_chordLine, offset);

    // [WHEN] Resetting it
    m_score->startCmd(TranslatableString::untranslatable("Engraving chord line tests"));
    m_chordLine->reset();
    m_score->endCmd();

    // [THEN] It has the default shape
    expectNear(m_chordLine->bezierOffset(ChordLine::Point::END), PointF());
    expectLaidOutEndOffset(m_chordLine, PointF());

    // [WHEN] Undoing
    m_score->undoRedo(true, nullptr);

    // [THEN] The edit is back
    expectNear(m_chordLine->bezierOffset(ChordLine::Point::END), offset);
    expectLaidOutEndOffset(m_chordLine, offset);
}
