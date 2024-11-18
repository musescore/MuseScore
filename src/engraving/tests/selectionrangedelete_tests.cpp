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

#include "dom/chord.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/note.h"
#include "dom/segment.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String SELRANGEDELETE_DATA_DIR("selectionrangedelete_data/");

class Engraving_SelectionRangeDeleteTests : public ::testing::Test
{
public:
    void verifyDelete(MasterScore* score, size_t spanners);
    void verifyNoDelete(MasterScore* score, size_t spanners);
    void deleteVoice(int voice, String idx);
};

void Engraving_SelectionRangeDeleteTests::verifyDelete(MasterScore* score, size_t spanners)
{
    score->startCmd(TranslatableString::untranslatable("Selection range delete tests"));
    score->cmdDeleteSelection();
    score->endCmd();

    EXPECT_EQ(score->spanner().size(), spanners - 1);
    score->undoRedo(true, 0);
    EXPECT_EQ(score->spanner().size(), spanners);
}

void Engraving_SelectionRangeDeleteTests::verifyNoDelete(MasterScore* score, size_t spanners)
{
    score->startCmd(TranslatableString::untranslatable("Selection range delete tests"));
    score->cmdDeleteSelection();
    score->endCmd();

    EXPECT_EQ(score->spanner().size(), spanners);
    score->undoRedo(true, 0);
    EXPECT_EQ(score->spanner().size(), spanners);
}

static EngravingItem* chordRestAtBeat(Score* score, int beat, int half = 0)
{
    int division = Constants::DIVISION;
    int tick = beat * division + half * division / 2;
    return score->tick2segment(Fraction::fromTicks(tick), false, SegmentType::ChordRest, false)->element(0);
}

TEST_F(Engraving_SelectionRangeDeleteTests, deleteSegmentWithSlur)
{
    /*
     *  Score looks like this:
     *  ss - start slur, es - end slur, q - quarter note, e - eighth note
     *
     *  ss es ss   es
     *  q  q  q  e e
     */
    MasterScore* score = ScoreRW::readScore(SELRANGEDELETE_DATA_DIR + "selectionrangedelete01.mscx");
    EXPECT_TRUE(score);

    score->doLayout();
    size_t spanners = score->spanner().size();

    score->select(chordRestAtBeat(score, 0), SelectType::RANGE);
    verifyDelete(score, spanners);
    score->deselectAll();

    score->select(chordRestAtBeat(score, 1), SelectType::RANGE);
    verifyDelete(score, spanners);
    score->deselectAll();

    score->select(chordRestAtBeat(score, 2), SelectType::RANGE);
    verifyDelete(score, spanners);
    score->deselectAll();

    score->select(chordRestAtBeat(score, 3), SelectType::RANGE);
    verifyNoDelete(score, spanners);
    score->deselectAll();

    score->select(chordRestAtBeat(score, 3, 1), SelectType::RANGE);
    verifyDelete(score, spanners);
    score->deselectAll();

    delete score;
}

TEST_F(Engraving_SelectionRangeDeleteTests, deleteSegmentWithSpanner)
{
    /*
     *  Score looks like this:
     *  ss - start spanner, es - end spanner, q - quarter note
     *
     *  ss    es
     *  q  q  q
     */
    MasterScore* score = ScoreRW::readScore(SELRANGEDELETE_DATA_DIR + "selectionrangedelete02.mscx");
    EXPECT_TRUE(score);

    score->doLayout();

    size_t spanners = score->spanner().size();

    score->select(chordRestAtBeat(score, 0), SelectType::RANGE);
    verifyNoDelete(score, spanners);
//      verifyDelete(score,spanners);
    score->deselectAll();

    score->select(chordRestAtBeat(score, 1), SelectType::RANGE);
    verifyNoDelete(score, spanners);
    score->deselectAll();

    score->select(chordRestAtBeat(score, 2), SelectType::RANGE);
    verifyNoDelete(score, spanners);
    score->deselectAll();

    score->select(chordRestAtBeat(score, 0), SelectType::RANGE);
    score->select(chordRestAtBeat(score, 2), SelectType::RANGE);
    verifyDelete(score, spanners);
    score->deselectAll();

    delete score;
}

void Engraving_SelectionRangeDeleteTests::deleteVoice(int voice, String idx)
{
    MasterScore* score = ScoreRW::readScore(SELRANGEDELETE_DATA_DIR + String("selectionrangedelete%1.mscx").arg(idx));
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    EXPECT_TRUE(m1);

    SelectionFilterType voiceFilterType = SelectionFilterType((int)SelectionFilterType::FIRST_VOICE + voice);
    score->selectionFilter().setFiltered(voiceFilterType, false);
    score->select(m1, SelectType::RANGE);

    score->startCmd(TranslatableString::untranslatable("Selection range delete tests"));
    score->cmdDeleteSelection();
    score->endCmd();

    score->doLayout();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String(u"selectionrangedelete%1.mscx").arg(idx),
                                            SELRANGEDELETE_DATA_DIR + String(u"selectionrangedelete%1-ref.mscx").arg(idx)));
    delete score;
}

TEST_F(Engraving_SelectionRangeDeleteTests, deleteVoice1)
{
    deleteVoice(0, u"03");
}

TEST_F(Engraving_SelectionRangeDeleteTests, deleteVoice2)
{
    deleteVoice(1, u"04");
}

TEST_F(Engraving_SelectionRangeDeleteTests, deleteSkipAnnotations)
{
    MasterScore* score = ScoreRW::readScore(SELRANGEDELETE_DATA_DIR + String(u"selectionrangedelete05.mscx"));
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    EXPECT_TRUE(m1);

    SelectionFilterType annotationFilterType = SelectionFilterType((int)SelectionFilterType::CHORD_SYMBOL);
    score->selectionFilter().setFiltered(annotationFilterType, false);

    score->startCmd(TranslatableString::untranslatable("Selection range delete tests"));
    score->cmdSelectAll();
    score->cmdDeleteSelection();
    score->endCmd();

    score->doLayout();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String(u"selectionrangedelete05.mscx"),
                                            SELRANGEDELETE_DATA_DIR + String(u"selectionrangedelete05-ref.mscx")));
    delete score;
}

TEST_F(Engraving_SelectionRangeDeleteTests, deletePartialNestedTuplets)
{
    // A score where each measure contains a tuplet with nested tuplets.
    // In each measure, some notes have the 'x' notehead.
    // We make a range selection consisting of those 'x' noteheads, and delete the selection.

    MasterScore* score = ScoreRW::readScore(SELRANGEDELETE_DATA_DIR + String(u"selectionrangedelete06_partialnestedtuplets.mscx"));
    ASSERT_TRUE(score);

    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        SCOPED_TRACE("Measure no " + std::to_string(measure->no()));

        score->deselectAll();

        for (Segment* segment = measure->first(SegmentType::ChordRest); segment; segment = segment->next(SegmentType::ChordRest)) {
            mu::engraving::EngravingItem* element = segment->element(0);
            if (!element || !element->isChord()) {
                continue;
            }

            Chord* chord = toChord(element);
            if (chord->notes().empty()) {
                continue;
            }

            Note* note = chord->notes().front();
            if (note->headGroup() == NoteHeadGroup::HEAD_CROSS) {
                score->select(note, SelectType::RANGE);
            }
        }

        if (score->selection().isRange()) {
            score->cmdDeleteSelection();

            EXPECT_TRUE(score->sanityCheck());
        }
    }

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String(u"selectionrangedelete06_partialnestedtuplets.mscx"),
                                            SELRANGEDELETE_DATA_DIR + String(u"selectionrangedelete06_partialnestedtuplets-ref.mscx")));
}
