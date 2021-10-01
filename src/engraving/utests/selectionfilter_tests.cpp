/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "libmscore/masterscore.h"
#include "libmscore/measure.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

static const QString SELECTIONFILTER_DATA_DIR("selectionfilter_data/");

using namespace Ms;
using namespace mu::engraving;

class SelectionFilterTests : public ::testing::Test
{
public:
    void testFilter(int idx, SelectionFilterType filter);
    void testFilterSpanner(int idx, SelectionFilterType filter);
};

void SelectionFilterTests::testFilter(int idx, SelectionFilterType filter)
{
    Score* score = ScoreRW::readScore(SELECTIONFILTER_DATA_DIR + QString("selectionfilter%1.mscx").arg(idx));
    EXPECT_TRUE(score);
    score->doLayout();

    Measure* m1 = score->firstMeasure();
    EXPECT_TRUE(m1);

    score->select(m1);

    EXPECT_TRUE(score->selection().canCopy());
    EXPECT_EQ(score->selection().mimeType(), mimeStaffListFormat);

    EXPECT_TRUE(ScoreComp::saveCompareMimeData(score->selection().mimeData(), QString("selectionfilter%1-base.xml").arg(idx),
                                               SELECTIONFILTER_DATA_DIR + QString("selectionfilter%1-base-ref.xml").arg(idx)));

    score->selectionFilter().setFiltered(filter, false);

    EXPECT_TRUE(score->selection().canCopy());
    EXPECT_EQ(score->selection().mimeType(), mimeStaffListFormat);

    EXPECT_TRUE(ScoreComp::saveCompareMimeData(score->selection().mimeData(), QString("selectionfilter%1.xml").arg(idx),
                                               SELECTIONFILTER_DATA_DIR + QString("selectionfilter%1-ref.xml").arg(idx)));

    delete score;
}

void SelectionFilterTests::testFilterSpanner(int idx, SelectionFilterType filter)
{
    Score* score = ScoreRW::readScore(SELECTIONFILTER_DATA_DIR + QString("selectionfilter%1.mscx").arg(idx));
    EXPECT_TRUE(score);
    score->doLayout();

    Measure* m1 = score->firstMeasure();
    Measure* m2 = score->firstMeasure()->nextMeasure();

    EXPECT_TRUE(m1);
    EXPECT_TRUE(m2);

    score->select(m1);
    score->select(m2, SelectType::RANGE);

    EXPECT_TRUE(score->selection().canCopy());
    EXPECT_EQ(score->selection().mimeType(), mimeStaffListFormat);

    EXPECT_TRUE(ScoreComp::saveCompareMimeData(score->selection().mimeData(), QString("selectionfilter%1-base.xml").arg(idx),
                                               SELECTIONFILTER_DATA_DIR + QString("selectionfilter%1-base-ref.xml").arg(idx)));

    score->selectionFilter().setFiltered(filter, false);

    EXPECT_TRUE(score->selection().canCopy());
    EXPECT_EQ(score->selection().mimeType(), mimeStaffListFormat);

    EXPECT_TRUE(ScoreComp::saveCompareMimeData(score->selection().mimeData(), QString("selectionfilter%1.xml").arg(idx),
                                               SELECTIONFILTER_DATA_DIR + QString("selectionfilter%1-ref.xml").arg(idx)));

    delete score;
}

TEST_F(SelectionFilterTests, filterDynamic)
{
    testFilter(1, SelectionFilterType::DYNAMIC);
}

TEST_F(SelectionFilterTests, filterArticulation)
{
    testFilter(2, SelectionFilterType::ARTICULATION);
}

TEST_F(SelectionFilterTests, filterLyrics)
{
    testFilter(3, SelectionFilterType::LYRICS);
}

TEST_F(SelectionFilterTests, filterFingering)
{
    testFilter(4, SelectionFilterType::FINGERING);
}

TEST_F(SelectionFilterTests, filterChordSymbol)
{
    testFilter(5, SelectionFilterType::CHORD_SYMBOL);
}

TEST_F(SelectionFilterTests, filterSlur)
{
    testFilter(6, SelectionFilterType::SLUR);
}

TEST_F(SelectionFilterTests, filterFiguredBass)
{
    testFilter(7, SelectionFilterType::FIGURED_BASS);
}

TEST_F(SelectionFilterTests, filterOttava)
{
    testFilter(8, SelectionFilterType::OTTAVA);
}

TEST_F(SelectionFilterTests, filterPedalLine)
{
    testFilter(9, SelectionFilterType::PEDAL_LINE);
}

TEST_F(SelectionFilterTests, filterArpeggio)
{
    testFilter(10, SelectionFilterType::ARPEGGIO);
}

TEST_F(SelectionFilterTests, filterFretDiagram)
{
    testFilter(11, SelectionFilterType::FRET_DIAGRAM);
}

TEST_F(SelectionFilterTests, filterGlissando)
{
    testFilter(12, SelectionFilterType::GLISSANDO);
}

TEST_F(SelectionFilterTests, filterBreath)
{
    testFilter(13, SelectionFilterType::BREATH);
}

TEST_F(SelectionFilterTests, filterOtherText)
{
    testFilter(14, SelectionFilterType::OTHER_TEXT);
}

TEST_F(SelectionFilterTests, filterOtherLine)
{
    testFilterSpanner(15, SelectionFilterType::OTHER_LINE);
}

TEST_F(SelectionFilterTests, filterTremolo)
{
    testFilter(16, SelectionFilterType::TREMOLO);
}

TEST_F(SelectionFilterTests, filterVoice1)
{
    testFilter(17, SelectionFilterType::FIRST_VOICE);
}

TEST_F(SelectionFilterTests, filterVoice2)
{
    testFilter(18, SelectionFilterType::SECOND_VOICE);
}

TEST_F(SelectionFilterTests, filterVoice3)
{
    testFilter(19, SelectionFilterType::THIRD_VOICE);
}

TEST_F(SelectionFilterTests, filterVoice4)
{
    testFilter(20, SelectionFilterType::FOURTH_VOICE);
}

TEST_F(SelectionFilterTests, filterHairpin)
{
    testFilter(22, SelectionFilterType::HAIRPIN);
}

TEST_F(SelectionFilterTests, filterOrnament)
{
    testFilter(23, SelectionFilterType::ORNAMENT);
}
