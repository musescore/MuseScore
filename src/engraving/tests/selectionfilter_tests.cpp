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
    void testNotesInChordsAction(void (*action)(Score*, MeasureBase*), const String& reference);
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
    bool use302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;
    testFilter(5, ElementsSelectionFilterTypes::CHORD_SYMBOL);
    MScore::useRead302InTestMode = use302;
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
 * @brief Engraving_SelectionFilterTests::testNotesInChordsAction
 * @details This method modifies the selection filter, makes a range selection over five measures, then performs a given action. These steps are
 *          repeated every five measures (until measure 25), and the result is compared with the given reference. The "action" method can take
 *          an auxiliary Measure - for copy/pastes this is used as the paste destination...
 */
void Engraving_SelectionFilterTests::testNotesInChordsAction(void (*action)(Score*, MeasureBase* auxMeasure), const String& reference)
{
    //! [GIVEN] A score with a mixture of chords, single notes, and tuplets...
    MasterScore* score = ScoreRW::readScore(SELECTIONFILTER_DATA_DIR + String(u"selectionfilter_notesinchords.mscx"));
    EXPECT_TRUE(score);

    const auto prepareSelectionRange = [score](MeasureBase* rangeStart, MeasureBase* rangeEnd){
        score->deselectAll();
        score->select(rangeStart, SelectType::SINGLE);
        score->select(rangeEnd, SelectType::RANGE);
    };

    score->selectionFilter().setFiltered(engraving::ElementsSelectionFilterTypes::ALL, false);

    // Measures 1 to 5 - deselect bottom notes
    //! [WHEN] Editing the selection filter and making a range selection over the first five measures...
    MeasureBase* rangeStart = score->measure(0);
    MeasureBase* rangeEnd = score->measure(4);
    MeasureBase* auxMeasure = score->measure(25);
    EXPECT_TRUE(rangeStart && rangeEnd && auxMeasure);

    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::ALL, true);
    score->selectionFilter().setIncludeSingleNotes(true);
    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::BOTTOM_NOTE, false);
    prepareSelectionRange(rangeStart, rangeEnd);

    //! [WHEN] Performing the given action over the selected measures...
    action(score, auxMeasure);

    // Measures 6 to 10 - deselect top notes and thirds
    //! [WHEN] Editing the selection filter and making a range selection over the next five measures...
    rangeStart = score->measure(5);
    rangeEnd = score->measure(9);
    auxMeasure = score->measure(30);
    EXPECT_TRUE(rangeStart && rangeEnd && auxMeasure);

    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::ALL, true);
    score->selectionFilter().setIncludeSingleNotes(true);
    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::TOP_NOTE, false);
    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::THIRD_NOTE, false);
    prepareSelectionRange(rangeStart, rangeEnd);

    //! [WHEN] Performing the given action over the selected measures...
    action(score, auxMeasure);

    // Measures 11 to 15 - deselect single notes
    //! [WHEN] Editing the selection filter and making a range selection over the next five measures...
    rangeStart = score->measure(10);
    rangeEnd = score->measure(14);
    auxMeasure = score->measure(35);
    EXPECT_TRUE(rangeStart && rangeEnd && auxMeasure);

    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::ALL, true);
    score->selectionFilter().setIncludeSingleNotes(false);
    prepareSelectionRange(rangeStart, rangeEnd);

    //! [WHEN] Performing the given action over the selected measures...
    action(score, auxMeasure);

    // Measures 16 to 20 - select seconds and sixths
    //! [WHEN] Editing the selection filter and making a range selection over the next five measures...
    rangeStart = score->measure(15);
    rangeEnd = score->measure(19);
    auxMeasure = score->measure(40);
    EXPECT_TRUE(rangeStart && rangeEnd && auxMeasure);

    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::NONE, true);
    score->selectionFilter().setIncludeSingleNotes(false);
    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::SECOND_NOTE, true);
    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::SIXTH_NOTE, true);
    prepareSelectionRange(rangeStart, rangeEnd);

    //! [WHEN] Performing the given action over the selected measures...
    action(score, auxMeasure);

    // Measures 21 to 25 - select bottom notes, thirds, fourths, and top notes
    //! [WHEN] Editing the selection filter and making a range selection over the next five measures...
    rangeStart = score->measure(20);
    rangeEnd = score->measure(24);
    auxMeasure = score->measure(45);
    EXPECT_TRUE(rangeStart && rangeEnd && auxMeasure);

    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::NONE, true);
    score->selectionFilter().setIncludeSingleNotes(false);
    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::BOTTOM_NOTE, true);
    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::THIRD_NOTE, true);
    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::FOURTH_NOTE, true);
    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::TOP_NOTE, true);
    prepareSelectionRange(rangeStart, rangeEnd);

    //! [WHEN] Performing the given action over the selected measures...
    action(score, auxMeasure);

    //! [EXPECT] The result matches the given reference...
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String(u"selectionfilter_notesinchords.mscx"), reference));
}

/**
 * @brief Engraving_SelectionFilterTests_notesInChordsDeleteRange
 * @details See Engraving_SelectionFilterTests::testNotesInChordsAction
 */
TEST_F(Engraving_SelectionFilterTests, notesInChordsDeleteRange)
{
    const auto deleteAction = [](Score* score, MeasureBase*){
        score->startCmd(TranslatableString::untranslatable("Selection filter - delete range test"));
        score->cmdDeleteSelection();
        score->endCmd();
    };

    testNotesInChordsAction(deleteAction, SELECTIONFILTER_DATA_DIR + String(u"selectionfilter_notesinchords_deleterange-ref.mscx"));
}

/**
 * @brief Engraving_SelectionFilterTests_notesInChordsCopyPaste
 * @details See Engraving_SelectionFilterTests::testNotesInChordsAction
 */
TEST_F(Engraving_SelectionFilterTests, notesInChordsCopyPaste)
{
    const auto copyPasteAction = [](Score* score, MeasureBase* auxMeasure) {
        muse::ByteArray mimeData = score->selection().mimeData();
        EXPECT_TRUE(!mimeData.empty());

        score->deselectAll();
        score->select(auxMeasure, SelectType::SINGLE);

        score->startCmd(TranslatableString::untranslatable("Selection filter - copy/paste test"));
        score->cmdPasteStaffList(mimeData);
        score->endCmd();
    };

    testNotesInChordsAction(copyPasteAction, SELECTIONFILTER_DATA_DIR + String(u"selectionfilter_notesinchords_copypaste-ref.mscx"));
}

/**
 * @brief Engraving_VoiceSwitchingTests_gracesAndSlurs
 * @details Start with a score containing a mixture of grace notes and slurs. Iterate through each measure, modify the selection
 *          filter, and perform a range delete over that measure. Check the result against the reference.
 */
TEST_F(Engraving_SelectionFilterTests, gracesAndSlurs)
{
    //! [GIVEN] A score with a grace notes and slurs
    MasterScore* score = ScoreRW::readScore(SELECTIONFILTER_DATA_DIR + String(u"selectionfilter_gracesandslurs.mscx"));
    EXPECT_TRUE(score);

    // Clear selection filter...
    score->selectionFilter().setFiltered(engraving::ElementsSelectionFilterTypes::ALL, false);
    score->selectionFilter().setFiltered(engraving::NotesInChordSelectionFilterTypes::ALL, false);
    score->selectionFilter().setIncludeSingleNotes(false);

    // Measure 1
    //! [WHEN] Performing a delete with nothing filtered...
    MeasureBase* currentMeasure = score->measure(0);
    EXPECT_TRUE(currentMeasure);
    score->deselectAll();
    score->select(currentMeasure, SelectType::SINGLE);

    score->startCmd(TranslatableString::untranslatable("Selection filter - delete range test"));
    score->cmdDeleteSelection();
    score->endCmd();

    // Measure 2
    //! [WHEN] Performing a delete with graces & slurs filtered...
    score->selectionFilter().setFiltered(engraving::ElementsSelectionFilterTypes::SLUR, true);
    score->selectionFilter().setFiltered(engraving::ElementsSelectionFilterTypes::GRACE_NOTE, true);

    currentMeasure = score->measure(1);
    EXPECT_TRUE(currentMeasure);
    score->deselectAll();
    score->select(currentMeasure, SelectType::SINGLE);

    score->startCmd(TranslatableString::untranslatable("Selection filter - delete range test"));
    score->cmdDeleteSelection();
    score->endCmd();

    // Measure 3
    //! [WHEN] Performing a delete with graces only filtered...
    score->selectionFilter().setFiltered(engraving::ElementsSelectionFilterTypes::SLUR, false);

    currentMeasure = score->measure(2);
    EXPECT_TRUE(currentMeasure);
    score->deselectAll();
    score->select(currentMeasure, SelectType::SINGLE);

    score->startCmd(TranslatableString::untranslatable("Selection filter - delete range test"));
    score->cmdDeleteSelection();
    score->endCmd();

    // Measure 4
    //! [WHEN] Performing a delete with slurs only filtered...
    score->selectionFilter().setFiltered(engraving::ElementsSelectionFilterTypes::GRACE_NOTE, false);
    score->selectionFilter().setFiltered(engraving::ElementsSelectionFilterTypes::SLUR, true);

    currentMeasure = score->measure(3);
    EXPECT_TRUE(currentMeasure);
    score->deselectAll();
    score->select(currentMeasure, SelectType::SINGLE);

    score->startCmd(TranslatableString::untranslatable("Selection filter - delete range test"));
    score->cmdDeleteSelection();
    score->endCmd();

    //! [EXPECT] The result matches the given reference...
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String(u"selectionfilter_gracesandslurs.mscx.mscx"),
                                            SELECTIONFILTER_DATA_DIR + String("selectionfilter_gracesandslurs-ref.mscx")));
}
