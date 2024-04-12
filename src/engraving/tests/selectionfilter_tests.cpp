/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "dom/masterscore.h"
#include "dom/measure.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String SELECTIONFILTER_DATA_DIR("selectionfilter_data/");

class Engraving_SelectionFilterTests : public ::testing::Test
{
public:
    void testFilter(int idx, SelectionFilterType filter);
    void testFilterSpanner(int idx, SelectionFilterType filter);
};

void Engraving_SelectionFilterTests::testFilter(int idx, SelectionFilterType filter)
{
    Score* score = ScoreRW::readScore(SELECTIONFILTER_DATA_DIR + String(u"selectionfilter%1.mscx").arg(idx));
    EXPECT_TRUE(score);
    score->doLayout();

    Measure* m1 = score->firstMeasure();
    EXPECT_TRUE(m1);

    score->select(m1);

    EXPECT_TRUE(score->selection().canCopy());
    EXPECT_EQ(score->selection().mimeType(), mimeStaffListFormat);

    EXPECT_TRUE(ScoreComp::saveCompareMimeData(score->selection().mimeData(), String("selectionfilter%1-base.xml").arg(idx),
                                               SELECTIONFILTER_DATA_DIR + String("selectionfilter%1-base-ref.xml").arg(idx)));

    score->selectionFilter().setFiltered(filter, false);

    EXPECT_TRUE(score->selection().canCopy());
    EXPECT_EQ(score->selection().mimeType(), mimeStaffListFormat);

    EXPECT_TRUE(ScoreComp::saveCompareMimeData(score->selection().mimeData(), String("selectionfilter%1.xml").arg(idx),
                                               SELECTIONFILTER_DATA_DIR + String("selectionfilter%1-ref.xml").arg(idx)));

    delete score;
}

void Engraving_SelectionFilterTests::testFilterSpanner(int idx, SelectionFilterType filter)
{
    Score* score = ScoreRW::readScore(SELECTIONFILTER_DATA_DIR + String("selectionfilter%1.mscx").arg(idx));
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

    EXPECT_TRUE(ScoreComp::saveCompareMimeData(score->selection().mimeData(), String("selectionfilter%1-base.xml").arg(idx),
                                               SELECTIONFILTER_DATA_DIR + String("selectionfilter%1-base-ref.xml").arg(idx)));

    score->selectionFilter().setFiltered(filter, false);

    EXPECT_TRUE(score->selection().canCopy());
    EXPECT_EQ(score->selection().mimeType(), mimeStaffListFormat);

    EXPECT_TRUE(ScoreComp::saveCompareMimeData(score->selection().mimeData(), String("selectionfilter%1.xml").arg(idx),
                                               SELECTIONFILTER_DATA_DIR + String("selectionfilter%1-ref.xml").arg(idx)));

    delete score;
}

TEST_F(Engraving_SelectionFilterTests, filterDynamic)
{
    testFilter(1, SelectionFilterType::DYNAMIC);
}

TEST_F(Engraving_SelectionFilterTests, filterArticulation)
{
    testFilter(2, SelectionFilterType::ARTICULATION);
}

TEST_F(Engraving_SelectionFilterTests, filterLyrics)
{
    testFilter(3, SelectionFilterType::LYRICS);
}

TEST_F(Engraving_SelectionFilterTests, filterFingering)
{
    testFilter(4, SelectionFilterType::FINGERING);
}

TEST_F(Engraving_SelectionFilterTests, filterChordSymbol)
{
    testFilter(5, SelectionFilterType::CHORD_SYMBOL);
}

TEST_F(Engraving_SelectionFilterTests, filterSlur)
{
    testFilter(6, SelectionFilterType::SLUR);
}

TEST_F(Engraving_SelectionFilterTests, filterFiguredBass)
{
    testFilter(7, SelectionFilterType::FIGURED_BASS);
}

TEST_F(Engraving_SelectionFilterTests, filterOttava)
{
    testFilter(8, SelectionFilterType::OTTAVA);
}

TEST_F(Engraving_SelectionFilterTests, filterPedalLine)
{
    testFilter(9, SelectionFilterType::PEDAL_LINE);
}

TEST_F(Engraving_SelectionFilterTests, filterArpeggio)
{
    testFilter(10, SelectionFilterType::ARPEGGIO);
}

TEST_F(Engraving_SelectionFilterTests, filterFretDiagram)
{
    testFilter(11, SelectionFilterType::FRET_DIAGRAM);
}

TEST_F(Engraving_SelectionFilterTests, filterGlissando)
{
    testFilter(12, SelectionFilterType::GLISSANDO);
}

TEST_F(Engraving_SelectionFilterTests, filterBreath)
{
    testFilter(13, SelectionFilterType::BREATH);
}

TEST_F(Engraving_SelectionFilterTests, filterOtherText)
{
    testFilter(14, SelectionFilterType::OTHER_TEXT);
}

TEST_F(Engraving_SelectionFilterTests, filterOtherLine)
{
    testFilterSpanner(15, SelectionFilterType::OTHER_LINE);
}

TEST_F(Engraving_SelectionFilterTests, filterTremolo)
{
    testFilter(16, SelectionFilterType::TREMOLO);
}

TEST_F(Engraving_SelectionFilterTests, filterVoice1)
{
    testFilter(17, SelectionFilterType::FIRST_VOICE);
}

TEST_F(Engraving_SelectionFilterTests, filterVoice2)
{
    testFilter(18, SelectionFilterType::SECOND_VOICE);
}

TEST_F(Engraving_SelectionFilterTests, filterVoice3)
{
    testFilter(19, SelectionFilterType::THIRD_VOICE);
}

TEST_F(Engraving_SelectionFilterTests, filterVoice4)
{
    testFilter(20, SelectionFilterType::FOURTH_VOICE);
}

TEST_F(Engraving_SelectionFilterTests, filterHairpin)
{
    testFilter(22, SelectionFilterType::HAIRPIN);
}

TEST_F(Engraving_SelectionFilterTests, filterOrnament)
{
    testFilter(23, SelectionFilterType::ORNAMENT);
}
