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
#include "dom/excerpt.h"
#include "dom/factory.h"
#include "dom/glissando.h"
#include "dom/layoutbreak.h"
#include "dom/line.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/part.h"
#include "dom/staff.h"
#include "dom/system.h"
#include "dom/undo.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String SPANNERS_DATA_DIR("spanners_data/");

class Engraving_SpannersTests : public ::testing::Test
{
};

//---------------------------------------------------------
//   Adds glissandi in several contexts.
//---------------------------------------------------------

TEST_F(Engraving_SpannersTests, spanners01)
{
    EditData dropData(0);
    Glissando* gliss;

    MasterScore* score = ScoreRW::readScore(SPANNERS_DATA_DIR + "glissando01.mscx");
    EXPECT_TRUE(score);

    // SIMPLE CASE: GLISSANDO FROM A NOTE TO THE FOLLOWING
    // go to top note of first chord
    Measure* msr   = score->firstMeasure();
    EXPECT_TRUE(msr);
    Segment* seg   = msr->findSegment(SegmentType::ChordRest, Fraction(0, 1));
    EXPECT_TRUE(seg);
    Chord* chord = static_cast<Chord*>(seg->element(0));
    EXPECT_TRUE(chord);
    EXPECT_EQ(chord->type(), ElementType::CHORD);
    Note* note  = chord->upNote();
    EXPECT_TRUE(note);
    // drop a glissando on note
    gliss             = new Glissando(score->dummy());   // create a new element each time, as drop() will eventually delete it
    dropData.pos      = note->pagePos();
    dropData.dropElement  = gliss;
    note->drop(dropData);

    // GLISSANDO FROM TOP STAFF TO BOTTOM STAFF
    // go to top note of first chord of next measure
    msr   = msr->nextMeasure();
    EXPECT_TRUE(msr);
    seg   = msr->first();
    EXPECT_TRUE(seg);
    chord = static_cast<Chord*>(seg->element(0));     // voice 0 of staff 0
    EXPECT_TRUE(chord);
    EXPECT_EQ(chord->type(), ElementType::CHORD);
    note  = chord->upNote();
    EXPECT_TRUE(note);
    // drop a glissando on note
    gliss             = new Glissando(score->dummy());
    dropData.pos      = note->pagePos();
    dropData.dropElement  = gliss;
    note->drop(dropData);

    // GLISSANDO FROM BOTTOM STAFF TO TOP STAFF
    // go to bottom note of first chord of next measure
    msr   = msr->nextMeasure();
    EXPECT_TRUE(msr);
    seg   = msr->first();
    EXPECT_TRUE(seg);
    chord = static_cast<Chord*>(seg->element(4));     // voice 0 of staff 1
    EXPECT_TRUE(chord);
    EXPECT_EQ(chord->type(), ElementType::CHORD);
    note  = chord->upNote();
    EXPECT_TRUE(note);
    // drop a glissando on note
    gliss             = new Glissando(score->dummy());
    dropData.pos      = note->pagePos();
    dropData.dropElement  = gliss;
    note->drop(dropData);

    // GLISSANDO OVER INTERVENING NOTES IN ANOTHER VOICE
    // go to top note of first chord of next measure
    msr   = msr->nextMeasure();
    EXPECT_TRUE(msr);
    seg   = msr->first();
    EXPECT_TRUE(seg);
    chord = static_cast<Chord*>(seg->element(0));     // voice 0 of staff 0
    EXPECT_TRUE(chord);
    EXPECT_EQ(chord->type(), ElementType::CHORD);
    note  = chord->upNote();
    EXPECT_TRUE(note);
    // drop a glissando on note
    gliss             = new Glissando(score->dummy());
    dropData.pos      = note->pagePos();
    dropData.dropElement  = gliss;
    note->drop(dropData);

    // GLISSANDO OVER INTERVENING NOTES IN ANOTHER STAFF
    // go to top note of first chord of next measure
    msr   = msr->nextMeasure()->nextMeasure();
    EXPECT_TRUE(msr);
    seg   = msr->first();
    EXPECT_TRUE(seg);
    chord = static_cast<Chord*>(seg->element(0));     // voice 0 of staff 0
    EXPECT_TRUE(chord);
    EXPECT_EQ(chord->type(), ElementType::CHORD);
    note  = chord->upNote();
    EXPECT_TRUE(note);
    // drop a glissando on note
    gliss             = new Glissando(score->dummy());
    dropData.pos      = note->pagePos();
    dropData.dropElement  = gliss;
    note->drop(dropData);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"glissando01.mscx", SPANNERS_DATA_DIR + u"glissando01-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//   Check loading of score with a glissando from a lower to a higher staff:
//    A score with:
//          grand staff,
//          glissando from a bass staff note to a treble staff note
//    is loaded and laid out and saved: should be round-trip safe.
//---------------------------------------------------------
TEST_F(Engraving_SpannersTests, spanners02)
{
    MasterScore* score = ScoreRW::readScore(SPANNERS_DATA_DIR + u"glissando-crossstaff01.mscx");
    EXPECT_TRUE(score);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"glissando-crossstaff01.mscx", SPANNERS_DATA_DIR + u"glissando-crossstaff01-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//   Loads a score with before- and after-grace notes and adds several glissandi from/to them.
//---------------------------------------------------------
TEST_F(Engraving_SpannersTests, spanners03)
{
    EditData dropData(0);
    Glissando* gliss;

    MasterScore* score = ScoreRW::readScore(SPANNERS_DATA_DIR + u"glissando-graces01.mscx");
    EXPECT_TRUE(score);

    // GLISSANDO FROM MAIN NOTE TO AFTER-GRACE
    // go to top note of first chord
    Measure* msr   = score->firstMeasure();
    EXPECT_TRUE(msr);
    Segment* seg   = msr->findSegment(SegmentType::ChordRest, Fraction(0, 1));
    EXPECT_TRUE(seg);
    Chord* chord = static_cast<Chord*>(seg->element(0));
    EXPECT_TRUE(chord);
    EXPECT_EQ(chord->type(), ElementType::CHORD);
    Note* note  = chord->upNote();
    EXPECT_TRUE(note);
    // drop a glissando on note
    gliss             = new Glissando(score->dummy());   // create a new element each time, as drop() will eventually delete it
    dropData.pos      = note->pagePos();
    dropData.dropElement  = gliss;
    note->drop(dropData);

    // GLISSANDO FROM AFTER-GRACE TO BEFORE-GRACE OF NEXT CHORD
    // go to last after-grace of chord and drop a glissando on it
    Chord* grace = chord->graceNotesAfter().back();
    EXPECT_TRUE(grace);
    EXPECT_EQ(grace->type(), ElementType::CHORD);
    note              = grace->upNote();
    EXPECT_TRUE(note);
    gliss             = new Glissando(score->dummy());
    dropData.pos      = note->pagePos();
    dropData.dropElement  = gliss;
    note->drop(dropData);

    // GLISSANDO FROM MAIN NOTE TO BEFORE-GRACE OF NEXT CHORD
    // go to next chord
    seg               = seg->nextCR(0);
    EXPECT_TRUE(seg);
    chord             = static_cast<Chord*>(seg->element(0));
    EXPECT_TRUE(chord);
    EXPECT_EQ(chord->type(), ElementType::CHORD);
    note              = chord->upNote();
    EXPECT_TRUE(note);
    gliss             = new Glissando(score->dummy());
    dropData.pos      = note->pagePos();
    dropData.dropElement  = gliss;
    note->drop(dropData);

    // GLISSANDO FROM BEFORE-GRACE TO MAIN NOTE
    // go to next chord
    seg               = seg->nextCR(0);
    EXPECT_TRUE(seg);
    chord             = static_cast<Chord*>(seg->element(0));
    EXPECT_TRUE(chord && chord->type() == ElementType::CHORD);
    // go to its last before-grace note
    grace             = chord->graceNotesBefore().back();
    EXPECT_TRUE(grace && grace->type() == ElementType::CHORD);
    note              = grace->upNote();
    EXPECT_TRUE(note);
    gliss             = new Glissando(score->dummy());
    dropData.pos      = note->pagePos();
    dropData.dropElement  = gliss;
    note->drop(dropData);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"glissando-graces01.mscx", SPANNERS_DATA_DIR + u"glissando-graces01-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//   Linking a staff to an existing staff containing a glissando
//---------------------------------------------------------

TEST_F(Engraving_SpannersTests, spanners04)
{
    MasterScore* score = ScoreRW::readScore(SPANNERS_DATA_DIR + u"glissando-cloning01.mscx");
    EXPECT_TRUE(score);

    // add a linked staff to the existing staff
    // (copied and adapted from void MuseScore::editInstrList() in mscore/instrdialog.cpp)
    Staff* oldStaff   = score->staff(0);
    Staff* newStaff   = Factory::createStaff(oldStaff->part());
    newStaff->setPart(oldStaff->part());
    newStaff->initFromStaffType(oldStaff->staffType(Fraction(0, 1)));
    newStaff->setDefaultClefType(ClefTypeList(ClefType::G));

    KeySigEvent ke;
    ke.setConcertKey(Key::C);
    newStaff->setKey(Fraction(0, 1), ke);

    score->undoInsertStaff(newStaff, 1, false);
    Excerpt::cloneStaff(oldStaff, newStaff);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"glissando-cloning01.mscx", SPANNERS_DATA_DIR + u"glissando-cloning01-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
///  spanners05
///   Creating part from an existing staff containing a glissando
//---------------------------------------------------------

TEST_F(Engraving_SpannersTests, DISABLED_spanners05)
{
    MasterScore* score = ScoreRW::readScore(SPANNERS_DATA_DIR + u"glissando-cloning02.mscx");
    EXPECT_TRUE(score);

    // create parts
    // (copied and adapted from void TestParts::createParts() in mtest/libmscore/parts/tst_parts.cpp)
    std::vector<Part*> parts;
    parts.push_back(score->parts().at(0));

    Excerpt* ex = new Excerpt(score);
    score->initAndAddExcerpt(ex, false);
    ex->setName(parts.front()->longName());
    ex->setParts(parts);

//      nscore->setName(parts.front()->partName());

    score->Score::undo(new AddExcerpt(ex));

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"glissando-cloning02.mscx", SPANNERS_DATA_DIR + u"glissando-cloning02-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
///  spanners06
///   Drop a glissando on a staff with a linked staff
//---------------------------------------------------------

TEST_F(Engraving_SpannersTests, spanners06)
{
    EditData dropData(0);
    Glissando* gliss;

    MasterScore* score = ScoreRW::readScore(SPANNERS_DATA_DIR + "glissando-cloning03.mscx");
    EXPECT_TRUE(score);

    // DROP A GLISSANDO ON FIRST NOTE
    Measure* msr   = score->firstMeasure();
    EXPECT_TRUE(msr);
    Segment* seg   = msr->findSegment(SegmentType::ChordRest, Fraction(0, 1));
    EXPECT_TRUE(seg);
    Chord* chord = static_cast<Chord*>(seg->element(0));
    EXPECT_TRUE(chord && chord->type() == ElementType::CHORD);
    Note* note  = chord->upNote();
    EXPECT_TRUE(note);
    // drop a glissando on note
    gliss             = new Glissando(score->dummy());
    dropData.pos      = note->pagePos();
    dropData.dropElement  = gliss;
    note->drop(dropData);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"glissando-cloning03.mscx", SPANNERS_DATA_DIR + u"glissando-cloning03-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
///  spanners07
///   Drop a glissando on a staff with an excerpt
//---------------------------------------------------------

TEST_F(Engraving_SpannersTests, spanners07)
{
    EditData dropData(0);
    Glissando* gliss;

    MasterScore* score = ScoreRW::readScore(SPANNERS_DATA_DIR + u"glissando-cloning04.mscx");
    EXPECT_TRUE(score);

    // DROP A GLISSANDO ON FIRST NOTE
    Measure* msr   = score->firstMeasure();
    EXPECT_TRUE(msr);
    Segment* seg   = msr->findSegment(SegmentType::ChordRest, Fraction(0, 1));
    EXPECT_TRUE(seg);
    Chord* chord = static_cast<Chord*>(seg->element(0));
    EXPECT_TRUE(chord && chord->type() == ElementType::CHORD);
    Note* note  = chord->upNote();
    EXPECT_TRUE(note);
    // drop a glissando on note
    gliss             = new Glissando(score->dummy());
    dropData.pos      = note->pagePos();
    dropData.dropElement  = gliss;
    note->drop(dropData);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"glissando-cloning04.mscx", SPANNERS_DATA_DIR + u"glissando-cloning04-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
///  spanners09
///   Remove a measure containing the end point of a LyricsLine and undo
//
//  +---spanner---+
//         +---remove----+
//
//---------------------------------------------------------

TEST_F(Engraving_SpannersTests, spanners09)
{
    MasterScore* score = ScoreRW::readScore(SPANNERS_DATA_DIR + u"lyricsline02.mscx");
    EXPECT_TRUE(score);

    // DELETE SECOND MEASURE AND VERIFY
    Measure* msr   = score->firstMeasure();
    EXPECT_TRUE(msr);
    msr = msr->nextMeasure();
    EXPECT_TRUE(msr);
    score->startCmd(TranslatableString::untranslatable("Engraving spanners tests"));
    score->select(msr);
    score->cmdTimeDelete();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"lyricsline02.mscx", SPANNERS_DATA_DIR + u"lyricsline02-ref.mscx"));

    // UNDO AND VERIFY
    EditData ed;
    score->undoStack()->undo(&ed);
    score->doLayout();   // measure needs to be renumbered
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"lyricsline02.mscx", SPANNERS_DATA_DIR + u"lyricsline02.mscx"));
    delete score;
}

//---------------------------------------------------------
///  spanners10
///   Remove a measure containing the start point of a LyricsLine and undo
//
//         +---spanner---+
//  +---remove----+
//
//---------------------------------------------------------

TEST_F(Engraving_SpannersTests, spanners10)
{
    MasterScore* score = ScoreRW::readScore(SPANNERS_DATA_DIR + u"lyricsline03.mscx");
    EXPECT_TRUE(score);

    // DELETE SECOND MEASURE AND VERIFY
    Measure* msr   = score->firstMeasure();
    EXPECT_TRUE(msr);
    msr = msr->nextMeasure();
    EXPECT_TRUE(msr);
    score->startCmd(TranslatableString::untranslatable("Engraving spanners tests"));
    score->select(msr);
    score->cmdTimeDelete();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"lyricsline03.mscx", SPANNERS_DATA_DIR + u"lyricsline03-ref.mscx"));

    // UNDO AND VERIFY
    EditData ed;
    score->undoStack()->undo(&ed);
    score->doLayout();   // measure needs to be renumbered
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"lyricsline03.mscx", SPANNERS_DATA_DIR + u"lyricsline03.mscx"));
    delete score;
}

//---------------------------------------------------------
///  spanners11
///   Remove a measure entirely containing a LyricsLine and undo
//
//         +---spanner---+
//  +-----------remove------------+
//
//---------------------------------------------------------

TEST_F(Engraving_SpannersTests, spanners11)
{
    MasterScore* score = ScoreRW::readScore(SPANNERS_DATA_DIR + u"lyricsline04.mscx");
    EXPECT_TRUE(score);

    // DELETE SECOND MEASURE AND VERIFY
    Measure* msr   = score->firstMeasure();
    EXPECT_TRUE(msr);
    msr = msr->nextMeasure();
    EXPECT_TRUE(msr);
    score->startCmd(TranslatableString::untranslatable("Engraving spanners tests"));
    score->select(msr);
    score->cmdTimeDelete();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"lyricsline04.mscx", SPANNERS_DATA_DIR + u"lyricsline04-ref.mscx"));

    // UNDO AND VERIFY
    EditData ed;
    score->undoStack()->undo(&ed);
    score->doLayout();   // measure needs to be renumbered
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"lyricsline04.mscx", SPANNERS_DATA_DIR + u"lyricsline04.mscx"));
    delete score;
}

//---------------------------------------------------------
///  spanners12
///   Remove a measure containing the middle portion of a LyricsLine and undo
//
//  +-----------spanner-----------+
//          +---remove----+
//
//---------------------------------------------------------

TEST_F(Engraving_SpannersTests, spanners12)
{
    MasterScore* score = ScoreRW::readScore(SPANNERS_DATA_DIR + u"lyricsline05.mscx");
    EXPECT_TRUE(score);

    // DELETE SECOND MEASURE AND VERIFY
    Measure* msr   = score->firstMeasure();
    EXPECT_TRUE(msr);
    msr = msr->nextMeasure();
    EXPECT_TRUE(msr);
    score->startCmd(TranslatableString::untranslatable("Engraving spanners tests"));
    score->select(msr);
    score->cmdTimeDelete();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"lyricsline05.mscx", SPANNERS_DATA_DIR + u"lyricsline05-ref.mscx"));

    // UNDO AND VERIFY
    EditData ed;
    score->undoStack()->undo(&ed);
    score->doLayout();   // measure needs to be renumbered
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"lyricsline05.mscx", SPANNERS_DATA_DIR + u"lyricsline05.mscx"));
    delete score;
}

//---------------------------------------------------------
///  spanners13
///   Drop a line break at a bar line in the middle of a LyricsLine and check LyricsLineSegments are correct
//
//---------------------------------------------------------

TEST_F(Engraving_SpannersTests, DISABLED_spanners13)
{
    EditData dropData(0);
    LayoutBreak* brk;

    MasterScore* score = ScoreRW::readScore(SPANNERS_DATA_DIR + u"lyricsline06.mscx");
    EXPECT_TRUE(score);

    // DROP A BREAK AT FIRST MEASURE AND VERIFY
    Measure* msr   = score->firstMeasure();
    EXPECT_TRUE(msr);
    brk = Factory::createLayoutBreak(score->dummy()->measure());
    brk->setLayoutBreakType(LayoutBreakType::LINE);
    dropData.pos      = msr->pagePos();
    dropData.dropElement  = brk;
    score->startCmd(TranslatableString::untranslatable("Engraving spanners tests"));
    msr->drop(dropData);
    score->endCmd();
    // VERIFY SEGMENTS IN SYSTEMS AND THEN SCORE
    for (System* sys : score->systems()) {
        EXPECT_TRUE(sys->spannerSegments().size() == 1);
    }
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"lyricsline06.mscx", SPANNERS_DATA_DIR + u"lyricsline06-ref.mscx"));

    // UNDO AND VERIFY
    EditData ed;
    score->undoStack()->undo(&ed);
    score->doLayout();        // systems need to be re-computed
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"lyricsline06.mscx", SPANNERS_DATA_DIR + u"lyricsline06.mscx"));
    delete score;
}

//---------------------------------------------------------
///  spanners14
///   creating part from an existing grand staff containing a cross staff glissando
//---------------------------------------------------------

TEST_F(Engraving_SpannersTests, DISABLED_spanners14)
{
    MasterScore* score = ScoreRW::readScore(SPANNERS_DATA_DIR + u"glissando-cloning05.mscx");
    EXPECT_TRUE(score);

    // create parts
    // (copied and adapted from void TestParts::createParts() in mtest/libmscore/parts/tst_parts.cpp)
    std::vector<Part*> parts;
    parts.push_back(score->parts().at(0));

    Excerpt* ex = new Excerpt(score);
    score->initAndAddExcerpt(ex, false);

    ex->setName(parts.front()->longName());
    ex->setParts(parts);

//      nscore->setName(parts.front()->partName());

    score->Score::undo(new AddExcerpt(ex));
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"glissando-cloning05.mscx", SPANNERS_DATA_DIR + u"glissando-cloning05-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
///  spanners15
///   set the color of a spanner and save
//---------------------------------------------------------

TEST_F(Engraving_SpannersTests, spanners15)
{
    MasterScore* score = ScoreRW::readScore(SPANNERS_DATA_DIR + u"linecolor01.mscx");
    EXPECT_TRUE(score);

    for (auto it = score->spanner().cbegin(); it != score->spanner().cend(); ++it) {
        Spanner* spanner = (*it).second;
        SLine* sl = static_cast<SLine*>(spanner);
        sl->setProperty(Pid::COLOR, Color(255, 0, 0, 255));
        for (auto ss : sl->spannerSegments()) {
            ss->setProperty(Pid::MIN_DISTANCE, 0.0);
            ss->setPropertyFlags(Pid::MIN_DISTANCE, PropertyFlags::UNSTYLED);
        }
    }

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"linecolor01.mscx", SPANNERS_DATA_DIR + u"linecolor01-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
///  spanners16
///   read manually adjusted lines on a small staff and save
//---------------------------------------------------------

TEST_F(Engraving_SpannersTests, spanners16)
{
    MasterScore* score = ScoreRW::readScore(SPANNERS_DATA_DIR + u"smallstaff01.mscx");
    EXPECT_TRUE(score);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"smallstaff01.mscx", SPANNERS_DATA_DIR + u"smallstaff01-ref.mscx"));
    delete score;
}
