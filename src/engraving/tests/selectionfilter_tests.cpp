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
    void testFilter(int idx, const SelectionFilterTypesVariant& type);
    void testFilterSpanner(int idx, const SelectionFilterTypesVariant& type);
    void testNotesInChordsAction(void (*action)(Score*), const String& reference);
};

void Engraving_SelectionFilterTests::testFilter(int idx, const SelectionFilterTypesVariant& type)
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

    score->selectionFilter().setFiltered(type, false);

    EXPECT_TRUE(score->selection().canCopy());
    EXPECT_EQ(score->selection().mimeType(), mimeStaffListFormat);

    EXPECT_TRUE(ScoreComp::saveCompareMimeData(score->selection().mimeData(), String("selectionfilter%1.xml").arg(idx),
                                               SELECTIONFILTER_DATA_DIR + String("selectionfilter%1-ref.xml").arg(idx)));

    delete score;
}

void Engraving_SelectionFilterTests::testFilterSpanner(int idx, const SelectionFilterTypesVariant& type)
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

    score->selectionFilter().setFiltered(type, false);

    EXPECT_TRUE(score->selection().canCopy());
    EXPECT_EQ(score->selection().mimeType(), mimeStaffListFormat);

    EXPECT_TRUE(ScoreComp::saveCompareMimeData(score->selection().mimeData(), String("selectionfilter%1.xml").arg(idx),
                                               SELECTIONFILTER_DATA_DIR + String("selectionfilter%1-ref.xml").arg(idx)));

    delete score;
}

TEST_F(Engraving_SelectionFilterTests, filterDynamic)
{
    testFilter(1, ElementsSelectionFilterTypes::DYNAMIC);
}

TEST_F(Engraving_SelectionFilterTests, filterArticulation)
{
    testFilter(2, ElementsSelectionFilterTypes::ARTICULATION);
}

TEST_F(Engraving_SelectionFilterTests, filterLyrics)
{
    testFilter(3, ElementsSelectionFilterTypes::LYRICS);
}

TEST_F(Engraving_SelectionFilterTests, filterFingering)
{
    testFilter(4, ElementsSelectionFilterTypes::FINGERING);
}

TEST_F(Engraving_SelectionFilterTests, filterChordSymbol)
{
    testFilter(5, ElementsSelectionFilterTypes::CHORD_SYMBOL);
}

TEST_F(Engraving_SelectionFilterTests, filterSlur)
{
    testFilter(6, ElementsSelectionFilterTypes::SLUR);
}

TEST_F(Engraving_SelectionFilterTests, filterFiguredBass)
{
    testFilter(7, ElementsSelectionFilterTypes::FIGURED_BASS);
}

TEST_F(Engraving_SelectionFilterTests, filterOttava)
{
    testFilter(8, ElementsSelectionFilterTypes::OTTAVA);
}

TEST_F(Engraving_SelectionFilterTests, filterPedalLine)
{
    testFilter(9, ElementsSelectionFilterTypes::PEDAL_LINE);
}

TEST_F(Engraving_SelectionFilterTests, filterArpeggio)
{
    testFilter(10, ElementsSelectionFilterTypes::ARPEGGIO);
}

TEST_F(Engraving_SelectionFilterTests, filterFretDiagram)
{
    testFilter(11, ElementsSelectionFilterTypes::FRET_DIAGRAM);
}

TEST_F(Engraving_SelectionFilterTests, filterGlissando)
{
    testFilter(12, ElementsSelectionFilterTypes::GLISSANDO);
}

TEST_F(Engraving_SelectionFilterTests, filterBreath)
{
    testFilter(13, ElementsSelectionFilterTypes::BREATH);
}

TEST_F(Engraving_SelectionFilterTests, filterOtherText)
{
    testFilter(14, ElementsSelectionFilterTypes::OTHER_TEXT);
}

TEST_F(Engraving_SelectionFilterTests, filterOtherLine)
{
    testFilterSpanner(15, ElementsSelectionFilterTypes::OTHER_LINE);
}

TEST_F(Engraving_SelectionFilterTests, filterTremolo)
{
    testFilter(16, ElementsSelectionFilterTypes::TREMOLO);
}

TEST_F(Engraving_SelectionFilterTests, filterVoice1)
{
    testFilter(17, VoicesSelectionFilterTypes::FIRST_VOICE);
}

TEST_F(Engraving_SelectionFilterTests, filterVoice2)
{
    testFilter(18, VoicesSelectionFilterTypes::SECOND_VOICE);
}

TEST_F(Engraving_SelectionFilterTests, filterVoice3)
{
    testFilter(19, VoicesSelectionFilterTypes::THIRD_VOICE);
}

TEST_F(Engraving_SelectionFilterTests, filterVoice4)
{
    testFilter(20, VoicesSelectionFilterTypes::FOURTH_VOICE);
}

TEST_F(Engraving_SelectionFilterTests, filterHairpin)
{
    testFilter(22, ElementsSelectionFilterTypes::HAIRPIN);
}

TEST_F(Engraving_SelectionFilterTests, filterOrnament)
{
    testFilter(23, ElementsSelectionFilterTypes::ORNAMENT);
}

/**
 * @brief Engraving_VoiceSwitchingTests_notesInChordsDeleteRangeg
 * @details Every five measures in this score, this test changes the "notes in chords" filter and performs
 *          a range delete. The resultant score is checked against the expected reference.
 */
TEST_F(Engraving_SelectionFilterTests, notesInChordsDeleteRange)
{
    //! [GIVEN] A score with a mixture of chords, single notes, and tuplets...
    MasterScore* score = ScoreRW::readScore(SELECTIONFILTER_DATA_DIR + String(u"selectionfilter_notesinchords.mscx"));
    ASSERT_TRUE(score);

    score->selectionFilter().setFiltered(engraving::ElementsSelectionFilterTypes::ALL, false);

    // Measures 1 to 5 - deselect bottom notes
    //! [WHEN] Editing the selection filter and making a range selection over five measures...
    MeasureBase* m1 = score->measure(0);
    MeasureBase* m2 = score->measure(4);

    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::ALL, true);
    score->selectionFilter().setIncludeSingleNotes(true);
    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::BOTTOM_NOTE, false);

    score->deselectAll();
    score->select(m1, SelectType::RANGE);
    score->select(m2, SelectType::RANGE);

    //! [WHEN] Performing a delete action...
    score->cmdDeleteSelection();

    // Measures 6 to 10 - deselect top notes and thirds
    //! [WHEN] Editing the selection filter and making a range selection over five measures...
    m1 = score->measure(5);
    m2 = score->measure(9);

    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::ALL, true);
    score->selectionFilter().setIncludeSingleNotes(true);
    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::TOP_NOTE, false);
    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::THIRD_NOTE, false);

    score->deselectAll();
    score->select(m1, SelectType::RANGE);
    score->select(m2, SelectType::RANGE);

    //! [WHEN] Performing a delete action...
    score->cmdDeleteSelection();

    // Measures 11 to 15 - deselect single notes
    //! [WHEN] Editing the selection filter and making a range selection over five measures...
    m1 = score->measure(10);
    m2 = score->measure(14);

    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::ALL, true);
    score->selectionFilter().setIncludeSingleNotes(false);

    score->deselectAll();
    score->select(m1, SelectType::RANGE);
    score->select(m2, SelectType::RANGE);

    //! [WHEN] Performing a delete action...
    score->cmdDeleteSelection();

    // Measures 16 to 20 - select seconds and sixths
    //! [WHEN] Editing the selection filter and making a range selection over five measures...
    m1 = score->measure(15);
    m2 = score->measure(19);

    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::NONE, true);
    score->selectionFilter().setIncludeSingleNotes(false);
    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::SECOND_NOTE, true);
    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::SIXTH_NOTE, true);

    score->deselectAll();
    score->select(m1, SelectType::RANGE);
    score->select(m2, SelectType::RANGE);

    //! [WHEN] Performing a delete action...
    score->cmdDeleteSelection();

    // Measures 21 to 25 - select bottom notes, thirds, fourths, and top notes
    //! [WHEN] Editing the selection filter and making a range selection over five measures...
    m1 = score->measure(20);
    m2 = score->measure(24);

    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::NONE, true);
    score->selectionFilter().setIncludeSingleNotes(false);
    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::BOTTOM_NOTE, true);
    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::THIRD_NOTE, true);
    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::FOURTH_NOTE, true);
    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::TOP_NOTE, true);

    score->deselectAll();
    score->select(m1, SelectType::RANGE);
    score->select(m2, SelectType::RANGE);

    //! [WHEN] Performing a delete action...
    score->cmdDeleteSelection();

    //! [EXPECT] The result matches our expectations...
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String(u"selectionfilter_notesinchords.mscx"),
                                            SELECTIONFILTER_DATA_DIR + String(u"selectionfilter_notesinchords_deleterange-ref.mscx")));
}
