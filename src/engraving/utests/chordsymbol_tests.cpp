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
#include "libmscore/undo.h"
#include "libmscore/excerpt.h"
#include "libmscore/part.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/chordrest.h"
#include "libmscore/harmony.h"
#include "libmscore/duration.h"
#include "libmscore/durationtype.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

static const QString CHORDSYMBOL_DATA_DIR("chordsymbol_data/");

using namespace mu::engraving;
using namespace Ms;

class ChordSymbolTests : public ::testing::Test
{
public:
    MasterScore* test_pre(const char* p);
    void test_post(MasterScore* score, const char* p);

    void selectAllChordSymbols(MasterScore* score);
    void realizeSelectionVoiced(MasterScore* score, Voicing voicing);
};

MasterScore* ChordSymbolTests::test_pre(const char* p)
{
    QString p1 = CHORDSYMBOL_DATA_DIR + p + ".mscx";
    MasterScore* score = ScoreRW::readScore(p1);
    EXPECT_TRUE(score);
    score->doLayout();
    return score;
}

void ChordSymbolTests::test_post(MasterScore* score, const char* p)
{
    QString p1 = p;
    p1 += "-test.mscx";
    QString p2 = CHORDSYMBOL_DATA_DIR + p + "-ref.mscx";
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, p1, p2));
    delete score;
}

//---------------------------------------------------------
//   select all chord symbols within the specified score
//---------------------------------------------------------
void ChordSymbolTests::selectAllChordSymbols(MasterScore* score)
{
    //find a chord symbol
    Segment* seg = score->firstSegment(SegmentType::ChordRest);
    EngravingItem* e = 0;
    while (seg) {
        e = seg->findAnnotation(ElementType::HARMONY,
                                0, score->ntracks());
        if (e) {
            break;
        }
        seg = seg->next1();
    }
    score->selectSimilar(e, false);
}

//---------------------------------------------------------
//   realize the current selection of the score using
//   the specified voicing
//---------------------------------------------------------
void ChordSymbolTests::realizeSelectionVoiced(MasterScore* score, Voicing voicing)
{
    for (EngravingItem* e : score->selection().elements()) {
        if (e->isHarmony()) {
            e->setProperty(Pid::HARMONY_VOICING, int(voicing));
        }
    }
    score->startCmd();
    score->cmdRealizeChordSymbols();
    score->endCmd();
}

TEST_F(ChordSymbolTests, testExtend)
{
    MasterScore* score = test_pre("extend");
    Measure* m = score->firstMeasure();
    Segment* s = m->first(SegmentType::ChordRest);
    ChordRest* cr = s->cr(0);
    score->changeCRlen(cr, DurationType::V_WHOLE);
    score->doLayout();
    test_post(score, "extend");
}

TEST_F(ChordSymbolTests, testClear)
{
    MasterScore* score = test_pre("clear");
    Measure* m = score->firstMeasure();
    score->select(m, SelectType::SINGLE, 0);
    score->cmdDeleteSelection();
    score->doLayout();
    test_post(score, "clear");
}

TEST_F(ChordSymbolTests, testAddLink)
{
    MasterScore* score = test_pre("add-link");
    Segment* seg = score->firstSegment(SegmentType::ChordRest);
    ChordRest* cr = seg->cr(0);
    Harmony* harmony = new Harmony(cr->segment());
    harmony->setHarmony("C7");
    harmony->setTrack(cr->track());
    harmony->setParent(cr->segment());
    score->undoAddElement(harmony);
    score->doLayout();
    test_post(score, "add-link");
}

TEST_F(ChordSymbolTests, testAddPart)
{
    MasterScore* score = test_pre("add-part");
    Segment* seg = score->firstSegment(SegmentType::ChordRest);
    ChordRest* cr = seg->cr(0);
    Harmony* harmony = new Harmony(cr->segment());
    harmony->setHarmony("C7");
    harmony->setTrack(cr->track());
    harmony->setParent(cr->segment());
    score->undoAddElement(harmony);
    score->doLayout();
    test_post(score, "add-part");
}

TEST_F(ChordSymbolTests, testNoSystem)
{
    MasterScore* score = test_pre("no-system");

    //
    // create first part
    //
    QList<Part*> parts;
    parts.append(score->parts().at(0));
    Score* nscore = score->createScore();

    Excerpt* ex = new Excerpt(score);
    ex->setExcerptScore(nscore);
    nscore->setExcerpt(ex);
    score->excerpts().append(ex);
    ex->setName(parts.front()->longName());
    ex->setParts(parts);
    Excerpt::createExcerpt(ex);

//      nscore->setTitle(parts.front()->partName());
    nscore->style().set(Sid::createMultiMeasureRests, true);

    //
    // create second part
    //
    parts.clear();
    parts.append(score->parts().at(1));
    nscore = score->createScore();

    ex = new Excerpt(score);
    ex->setExcerptScore(nscore);
    nscore->setExcerpt(ex);
    score->excerpts().append(ex);
    ex->setName(parts.front()->longName());
    ex->setParts(parts);
    Excerpt::createExcerpt(ex);

//      nscore->setTitle(parts.front()->partName());
    nscore->style().set(Sid::createMultiMeasureRests, true);

    score->setExcerptsChanged(true);
    score->doLayout();
    test_post(score, "no-system");
}

TEST_F(ChordSymbolTests, testTranspose)
{
    MasterScore* score = test_pre("transpose");
    score->startCmd();
    score->cmdSelectAll();
    score->transpose(TransposeMode::BY_INTERVAL, TransposeDirection::UP, Key::C, 4, false, true, true);
    score->endCmd();
    test_post(score, "transpose");
}

TEST_F(ChordSymbolTests, testTransposePart)
{
    MasterScore* score = test_pre("transpose-part");
    score->startCmd();
    score->cmdSelectAll();
    score->transpose(TransposeMode::BY_INTERVAL, TransposeDirection::UP, Key::C, 4, false, true, true);
    score->endCmd();
    test_post(score, "transpose-part");
}

//---------------------------------------------------------
//   check close voicing algorithm
//---------------------------------------------------------
TEST_F(ChordSymbolTests, testRealizeClose)
{
    MasterScore* score = test_pre("realize");
    selectAllChordSymbols(score);
    realizeSelectionVoiced(score, Voicing::CLOSE);
    test_post(score, "realize-close");
}

//---------------------------------------------------------
//   check Drop 2 voicing algorithm
//---------------------------------------------------------
TEST_F(ChordSymbolTests, testRealizeDrop2)
{
    MasterScore* score = test_pre("realize");
    selectAllChordSymbols(score);
    realizeSelectionVoiced(score, Voicing::DROP_2);
    test_post(score, "realize-drop2");
}

//---------------------------------------------------------
//   check 3 note voicing algorithm
//---------------------------------------------------------
TEST_F(ChordSymbolTests, testRealize3Note)
{
    MasterScore* score = test_pre("realize");
    selectAllChordSymbols(score);
    realizeSelectionVoiced(score, Voicing::THREE_NOTE);
    test_post(score, "realize-3note");
}

//---------------------------------------------------------
//   check 4 note voicing algorithm
//---------------------------------------------------------
TEST_F(ChordSymbolTests, testRealize4Note)
{
    MasterScore* score = test_pre("realize");
    selectAllChordSymbols(score);
    realizeSelectionVoiced(score, Voicing::FOUR_NOTE);
    test_post(score, "realize-4note");
}

//---------------------------------------------------------
//   check 6 note voicing algorithm
//---------------------------------------------------------
TEST_F(ChordSymbolTests, testRealize6Note)
{
    MasterScore* score = test_pre("realize");
    selectAllChordSymbols(score);
    realizeSelectionVoiced(score, Voicing::SIX_NOTE);
    test_post(score, "realize-6note");
}

//---------------------------------------------------------
//   Check if the note pitches and tpcs are correct after realizing
//   chord symbols on transposed instruments.
//---------------------------------------------------------
TEST_F(ChordSymbolTests, testRealizeConcertPitch)
{
    MasterScore* score = test_pre("realize-concert-pitch");
    //concert pitch off
    score->startCmd();
    score->cmdConcertPitchChanged(false);
    score->endCmd();

    //realize all chord symbols
    selectAllChordSymbols(score);
    score->startCmd();
    score->cmdRealizeChordSymbols();
    score->endCmd();
    test_post(score, "realize-concert-pitch");
}

//---------------------------------------------------------
//   Check if the note pitches and tpcs are correct after
//   transposing the score
//---------------------------------------------------------
TEST_F(ChordSymbolTests, testRealizeTransposed)
{
    MasterScore* score = test_pre("transpose");
    //transpose
    score->cmdSelectAll();
    score->transpose(TransposeMode::BY_INTERVAL, TransposeDirection::UP, Key::C, 4, false, true, true);

    //realize all chord symbols
    selectAllChordSymbols(score);
    score->startCmd();
    score->cmdRealizeChordSymbols();
    score->endCmd();
    test_post(score, "realize-transpose");
}

//---------------------------------------------------------
//   Check for correctness when using the override
//   feature for realizing chord symbols
//---------------------------------------------------------
TEST_F(ChordSymbolTests, testRealizeOverrides)
{
    MasterScore* score = test_pre("realize-override");
    //realize all chord symbols
    selectAllChordSymbols(score);
    score->startCmd();
    score->cmdRealizeChordSymbols(true, Voicing::ROOT_ONLY, HDuration::SEGMENT_DURATION);
    score->endCmd();
    test_post(score, "realize-override");
}

//---------------------------------------------------------
//   Check for correctness when realizing chord symbols on triplets
//---------------------------------------------------------
TEST_F(ChordSymbolTests, testRealizeTriplet)
{
    MasterScore* score = test_pre("realize-triplet");
    //realize all chord symbols
    selectAllChordSymbols(score);
    score->startCmd();
    score->cmdRealizeChordSymbols();
    score->endCmd();
    test_post(score, "realize-triplet");
}

//---------------------------------------------------------
//   Check for correctness when realizing chord symbols
//   with different durations
//---------------------------------------------------------
TEST_F(ChordSymbolTests, testRealizeDuration)
{
    MasterScore* score = test_pre("realize-duration");
    //realize all chord symbols
    selectAllChordSymbols(score);
    score->startCmd();
    score->cmdRealizeChordSymbols();
    score->endCmd();
    test_post(score, "realize-duration");
}

//---------------------------------------------------------
//   Check for correctness when realizing chord symbols
//   with jazz mode
//---------------------------------------------------------
TEST_F(ChordSymbolTests, testRealizeJazz)
{
    MasterScore* score = test_pre("realize-jazz");
    //realize all chord symbols
    selectAllChordSymbols(score);
    score->startCmd();
    score->cmdRealizeChordSymbols();
    score->endCmd();
    test_post(score, "realize-jazz");
}
