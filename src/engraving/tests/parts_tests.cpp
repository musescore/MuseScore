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

#include "dom/breath.h"
#include "dom/chord.h"
#include "dom/chordline.h"
#include "dom/dynamic.h"
#include "dom/engravingitem.h"
#include "dom/excerpt.h"
#include "dom/factory.h"
#include "dom/fingering.h"
#include "dom/image.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/measurerepeat.h"
#include "dom/note.h"
#include "dom/part.h"
#include "dom/segment.h"
#include "dom/spanner.h"
#include "dom/staff.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"
#include "utils/testutils.h"

using namespace mu;
using namespace mu::engraving;

static const String PARTS_DATA_DIR("parts_data/");

class Engraving_PartsTests : public ::testing::Test
{
public:
    void testPartCreation(const String& test);
    void createLinkedStaff(MasterScore* score);

    MasterScore* doAddBreath();
    MasterScore* doRemoveBreath();
    MasterScore* doAddFingering();
    MasterScore* doRemoveFingering();
    MasterScore* doAddSymbol();
    MasterScore* doRemoveSymbol();
    MasterScore* doAddChordline();
    MasterScore* doRemoveChordline();
    MasterScore* doAddMeasureRepeat();
    MasterScore* doRemoveMeasureRepeat();
    MasterScore* doAddImage();
    MasterScore* doRemoveImage();
};

void Engraving_PartsTests::createLinkedStaff(MasterScore* masterScore)
{
    masterScore->startCmd(TranslatableString::untranslatable("Engraving parts tests"));
    Staff* sourceStaff = masterScore->staff(0);
    EXPECT_TRUE(sourceStaff);
    Staff* linkedStaff = Factory::createStaff(sourceStaff->part());
    linkedStaff->setPart(sourceStaff->part());
    masterScore->undoInsertStaff(linkedStaff, 1, false);
    Excerpt::cloneStaff(sourceStaff, linkedStaff);
    masterScore->endCmd();
    EXPECT_TRUE(masterScore->staff(1));
}

//---------------------------------------------------------
//   voicesExcerpt
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, voicesExcerpt)
{
    MasterScore* masterScore = ScoreRW::readScore(PARTS_DATA_DIR + u"voices.mscx");
    ASSERT_TRUE(masterScore);

    //
    // create first part
    //
    std::vector<Part*> parts;
    std::multimap<track_idx_t, track_idx_t> trackList;
    parts.push_back(masterScore->parts().at(0));
    Score* nscore = masterScore->createScore();

    trackList.insert({ 1, 0 });
    trackList.insert({ 2, 1 });
    trackList.insert({ 4, 4 });

    Excerpt* ex = new Excerpt(masterScore);
    ex->setExcerptScore(nscore);
    nscore->setExcerpt(ex);
    masterScore->excerpts().push_back(ex);
    ex->setName(parts.front()->longName());
    ex->setParts(parts);
    ex->setTracksMapping(trackList);
    Excerpt::createExcerpt(ex);
    EXPECT_TRUE(nscore);

    //nscore->setName(parts.front()->partName());

    //
    // create second part
    //
    parts.clear();
    parts.push_back(masterScore->parts().at(1));
    nscore = masterScore->createScore();

    trackList.clear();
    trackList.insert({ 11, 0 });

    ex = new Excerpt(masterScore);
    ex->setExcerptScore(nscore);
    nscore->setExcerpt(ex);
    masterScore->excerpts().push_back(ex);
    ex->setName(parts.front()->longName());
    ex->setParts(parts);
    ex->setTracksMapping(trackList);
    Excerpt::createExcerpt(ex);
    EXPECT_TRUE(nscore);

    //
    // create second part
    //
    parts.clear();
    parts.push_back(masterScore->parts().at(1));
    nscore = masterScore->createScore();

    trackList.clear();
    trackList.insert({ 8, 0 });

    ex = new Excerpt(masterScore);
    ex->setExcerptScore(nscore);
    nscore->setExcerpt(ex);
    masterScore->excerpts().push_back(ex);
    ex->setName(parts.front()->longName());
    ex->setParts(parts);
    ex->setTracksMapping(trackList);
    Excerpt::createExcerpt(ex);
    EXPECT_TRUE(nscore);

    //nscore->setName(parts.front()->partName());

    masterScore->setExcerptsChanged(true);

    EXPECT_TRUE(ScoreComp::saveCompareScore(masterScore, u"voices.mscx", PARTS_DATA_DIR + u"voices-ref.mscx"));

    delete masterScore;
}

//---------------------------------------------------------
//   testPartCreation
//---------------------------------------------------------

void Engraving_PartsTests::testPartCreation(const String& test)
{
    MasterScore* score = ScoreRW::readScore(PARTS_DATA_DIR + test + u".mscx");
    ASSERT_TRUE(score);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, test + u"-1.mscx", PARTS_DATA_DIR + test + u".mscx"));
    TestUtils::createParts(score, 2);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, test + u"-parts.mscx", PARTS_DATA_DIR + test + u"-parts.mscx"));
    delete score;
}

//---------------------------------------------------------
//   testAppendMeasure
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, appendMeasure)
{
    bool use302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;

    MasterScore* score = ScoreRW::readScore(PARTS_DATA_DIR + u"part-all.mscx");
    ASSERT_TRUE(score);

    TestUtils::createParts(score, 2);

    score->startCmd(TranslatableString::untranslatable("Engraving parts tests"));
    score->insertMeasure(0);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-all-appendmeasures.mscx", PARTS_DATA_DIR + u"part-all-appendmeasures.mscx"));

    score->undoRedo(true, 0);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-all-uappendmeasures.mscx", PARTS_DATA_DIR + u"part-all-uappendmeasures.mscx"));
    delete score;
    MScore::useRead302InTestMode = use302;
}

//---------------------------------------------------------
//   testInsertMeasure
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, insertMeasure)
{
    bool use302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;

    MasterScore* score = ScoreRW::readScore(PARTS_DATA_DIR + u"part-all.mscx");
    ASSERT_TRUE(score);

    TestUtils::createParts(score, 2);

    score->startCmd(TranslatableString::untranslatable("Engraving parts tests"));
    Measure* m = score->firstMeasure();
    score->insertMeasure(m);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-all-insertmeasures.mscx", PARTS_DATA_DIR + u"part-all-insertmeasures.mscx"));

    score->undoRedo(true, 0);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-all-uinsertmeasures.mscx", PARTS_DATA_DIR + u"part-all-uinsertmeasures.mscx"));
    delete score;

    MScore::useRead302InTestMode = use302;
}

//---------------------------------------------------------
//   styleScore
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, styleScore)
{
    MasterScore* score = ScoreRW::readScore(PARTS_DATA_DIR + u"partStyle.mscx");
    ASSERT_TRUE(score);

    TestUtils::createParts(score, 2);
    score->style().set(Sid::clefLeftMargin, 4.0);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"partStyle-score-test.mscx", PARTS_DATA_DIR + u"partStyle-score-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//   styleScoreReload
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, styleScoreReload)
{
    MasterScore* partScore = ScoreRW::readScore(PARTS_DATA_DIR + u"partStyle-score-reload.mscx");
    ASSERT_TRUE(partScore);

    EXPECT_TRUE(ScoreComp::saveCompareScore(partScore, u"partStyle-score-reload-test.mscx",
                                            PARTS_DATA_DIR + u"partStyle-score-reload-ref.mscx"));
    delete partScore;
}

#if 0
//---------------------------------------------------------
//   stylePartDefault
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, stylePartDefault)
{
    MasterScore* score = ScoreRW::readScore(PARTS_DATA_DIR + u"partStyle.mscx");
    ASSERT_TRUE(score);

    // TODO: set defaultStyleForParts
    MScore::_defaultStyleForParts = new MStyle();
    QFile f(PARTS_DATA_DIR + u"style_test.mss");
    EXPECT_TRUE(f.open(QIODevice::ReadOnly));
    MStyle* s = new MStyle(*defaultStyle());
    EXPECT_TRUE(s->load(&f));
    MScore::_defaultStyleForParts = s;
    createParts(score);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"partStyle-part-default-test.mscx",
                                            PARTS_DATA_DIR + u"partStyle-part-default-ref.mscx"));
}

//---------------------------------------------------------
//   styleScoreDefault
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, styleScoreDefault)
{
    MasterScore* score = ScoreRW::readScore(PARTS_DATA_DIR + u"partStyle.mscx");
    ASSERT_TRUE(score);

    // TODO: set defaultStyle
    createParts(score);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"partStyle-score-default-test.mscx",
                                            PARTS_DATA_DIR + u"partStyle-score-default-ref.mscx"));
}

#endif

//---------------------------------------------------------
//   test part creation
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, createPart1)
{
    testPartCreation(u"part-empty");
}

TEST_F(Engraving_PartsTests, createPart2)
{
    bool use302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;
    testPartCreation(u"part-all");
    MScore::useRead302InTestMode = use302;
}

TEST_F(Engraving_PartsTests, createPart3)
{
    // second part has system text on empty chordrest segment
    testPartCreation(u"part-54346");
}

//---------------------------------------------------------
//    Breath
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, createPartBreath)
{
    testPartCreation(u"part-breath");
}

//---------------------------------------------------------
//    doAddBreath
//---------------------------------------------------------

MasterScore* Engraving_PartsTests::doAddBreath()
{
    MasterScore* score = ScoreRW::readScore(PARTS_DATA_DIR + u"part-empty-parts.mscx");
    /*ASSERT_TRUE*/ EXPECT_TRUE(score);

    Measure* m   = score->firstMeasure();
    Segment* s   = m->tick2segment(Fraction(1, 4));
    Chord* chord = toChord(s->element(0));
    Note* note   = chord->upNote();
    EditData dd(0);
    Breath* b = Factory::createBreath(score->dummy()->segment());
    b->setSymId(SymId::breathMarkComma);
    dd.dropElement = b;

    score->startCmd(TranslatableString::untranslatable("Engraving parts tests"));
    note->drop(dd);
    score->endCmd();          // does layout

    return score;
}

//---------------------------------------------------------
//   addBreath
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, addBreath)
{
    MasterScore* score = doAddBreath();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-breath-add.mscx", PARTS_DATA_DIR + u"part-breath-add.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoAddBreath
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoAddBreath)
{
    MasterScore* score = doAddBreath();

    score->undoRedo(true, 0);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-breath-uadd.mscx", PARTS_DATA_DIR + u"part-breath-uadd.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoRedoAddBreath
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoRedoAddBreath)
{
    MasterScore* score = doAddBreath();

    score->undoRedo(true, 0);
    score->undoRedo(false, 0);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-breath-uradd.mscx", PARTS_DATA_DIR + u"part-breath-uradd.mscx"));
    delete score;
}

//---------------------------------------------------------
//   doRemoveBreath
//---------------------------------------------------------

MasterScore* Engraving_PartsTests::doRemoveBreath()
{
    MasterScore* score = ScoreRW::readScore(PARTS_DATA_DIR + u"part-breath-add.mscx");
    /*ASSERT_TRUE*/ EXPECT_TRUE(score);

    Measure* m   = score->firstMeasure();
    Segment* s   = m->first()->next(SegmentType::Breath);
    Breath* b    = toBreath(s->element(0));

    score->select(b);
    score->startCmd(TranslatableString::untranslatable("Engraving parts tests"));
    score->cmdDeleteSelection();
    score->setLayoutAll();
    score->endCmd();
    return score;
}

//---------------------------------------------------------
//   removeBreath
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, removeBreath)
{
    MasterScore* score = doRemoveBreath();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-breath-del.mscx", PARTS_DATA_DIR + u"part-breath-del.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoRemoveBreath
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoRemoveBreath)
{
    MasterScore* score = doRemoveBreath();
    score->undoRedo(true, 0);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-breath-udel.mscx", PARTS_DATA_DIR + u"part-breath-udel.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoRedoRemoveBreath
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoRedoRemoveBreath)
{
    MasterScore* score = doRemoveBreath();
    score->undoRedo(true, 0);
    score->undoRedo(false, 0);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-breath-urdel.mscx", PARTS_DATA_DIR + u"part-breath-urdel.mscx"));
    delete score;
}

//---------------------------------------------------------
//   Fingering
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, createPartFingering)
{
    testPartCreation(u"part-fingering");
}

//---------------------------------------------------------
//   doAddFingering
//---------------------------------------------------------

MasterScore* Engraving_PartsTests::doAddFingering()
{
    MasterScore* score = ScoreRW::readScore(PARTS_DATA_DIR + u"part-empty-parts.mscx");
    /*ASSERT_TRUE*/ EXPECT_TRUE(score);

    Measure* m   = score->firstMeasure();
    Segment* s   = m->tick2segment(Fraction(1, 4));
    Chord* chord = toChord(s->element(0));
    Note* note   = chord->upNote();
    EditData dd(0);
    Fingering* b = Factory::createFingering(note);
    b->setXmlText("3");
    dd.dropElement = b;

    score->startCmd(TranslatableString::untranslatable("Engraving parts tests"));
    note->drop(dd);
    score->endCmd();          // does layout
    return score;
}

//---------------------------------------------------------
//   addFingering
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, addFingering)
{
    MasterScore* score = doAddFingering();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-fingering-add.mscx", PARTS_DATA_DIR + u"part-fingering-add.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoAddFingering
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoAddFingering)
{
    MasterScore* score = doAddFingering();
    score->undoRedo(true, 0);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-fingering-uadd.mscx", PARTS_DATA_DIR + u"part-fingering-uadd.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoRedoAddFingering
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoRedoAddFingering)
{
    MasterScore* score = doAddFingering();
    score->undoRedo(true, 0);
    score->undoRedo(false, 0);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-fingering-uradd.mscx", PARTS_DATA_DIR + u"part-fingering-uradd.mscx"));
    delete score;
}

//---------------------------------------------------------
//   doRemoveFingering
//---------------------------------------------------------

MasterScore* Engraving_PartsTests::doRemoveFingering()
{
    MasterScore* score = ScoreRW::readScore(PARTS_DATA_DIR + u"part-fingering-parts.mscx");
    /*ASSERT_TRUE*/ EXPECT_TRUE(score);

    Measure* m   = score->firstMeasure();
    Segment* s   = m->first()->next(SegmentType::ChordRest);
    Chord* chord = toChord(s->element(0));
    Note* note   = chord->upNote();
    EngravingItem* fingering = 0;
    for (EngravingItem* e : note->el()) {
        if (e->type() == ElementType::FINGERING) {
            fingering = e;
            break;
        }
    }
    score->select(fingering);

    score->startCmd(TranslatableString::untranslatable("Engraving parts tests"));
    score->cmdDeleteSelection();
    score->setLayoutAll();
    score->endCmd();
    return score;
}

//---------------------------------------------------------
//   removeFingering
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, removeFingering)
{
    MasterScore* score = doRemoveFingering();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-fingering-del.mscx", PARTS_DATA_DIR + u"part-fingering-del.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoRemoveFingering
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoRemoveFingering)
{
    MasterScore* score = doRemoveFingering();
    score->undoRedo(true, 0);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-fingering-udel.mscx", PARTS_DATA_DIR + u"part-fingering-udel.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoRedoRemoveFingering
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoRedoRemoveFingering)
{
    MasterScore* score = doRemoveFingering();
    score->undoRedo(true, 0);
    score->undoRedo(false, 0);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-fingering-urdel.mscx", PARTS_DATA_DIR + u"part-fingering-urdel.mscx"));
    delete score;
}

//---------------------------------------------------------
//   Symbol
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, createPartSymbol)
{
    testPartCreation(u"part-symbol");
}

//---------------------------------------------------------
//   doAddSymbol
//---------------------------------------------------------

MasterScore* Engraving_PartsTests::doAddSymbol()
{
    MasterScore* score = ScoreRW::readScore(PARTS_DATA_DIR + u"part-empty-parts.mscx");
    /*ASSERT_TRUE*/ EXPECT_TRUE(score);

    Measure* m   = score->firstMeasure();
    Segment* s   = m->tick2segment(Fraction(1, 4));
    Chord* chord = toChord(s->element(0));
    Note* note   = chord->upNote();
    EditData dd(0);
    Symbol* b  = new Symbol(note);
    b->setSym(SymId::gClef);
    dd.dropElement = b;

    score->startCmd(TranslatableString::untranslatable("Engraving parts tests"));
    note->drop(dd);
    score->endCmd();          // does layout
    return score;
}

//---------------------------------------------------------
//   addSymbol
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, addSymbol)
{
    MasterScore* score = doAddSymbol();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-symbol-add.mscx", PARTS_DATA_DIR + u"part-symbol-add.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoAddSymbol
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoAddSymbol)
{
    MasterScore* score = doAddSymbol();
    score->undoRedo(true, 0);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-symbol-uadd.mscx", PARTS_DATA_DIR + u"part-symbol-uadd.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoRedoAddSymbol
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoRedoAddSymbol)
{
    MasterScore* score = doAddSymbol();
    score->undoRedo(true, 0);
    score->undoRedo(false, 0);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-symbol-uradd.mscx", PARTS_DATA_DIR + u"part-symbol-uradd.mscx"));
    delete score;
}

//---------------------------------------------------------
//   doRemoveSymbol
//---------------------------------------------------------

MasterScore* Engraving_PartsTests::doRemoveSymbol()
{
    MasterScore* score = ScoreRW::readScore(PARTS_DATA_DIR + u"part-symbol-parts.mscx");
    /*ASSERT_TRUE*/ EXPECT_TRUE(score);

    Measure* m   = score->firstMeasure();
    Segment* s   = m->first()->next(SegmentType::ChordRest);
    Chord* chord = toChord(s->element(0));
    Note* note   = chord->upNote();
    EngravingItem* se = 0;
    for (EngravingItem* e : note->el()) {
        if (e->type() == ElementType::SYMBOL) {
            se = e;
            break;
        }
    }
    score->select(se);

    score->startCmd(TranslatableString::untranslatable("Engraving parts tests"));
    score->cmdDeleteSelection();
    score->setLayoutAll();
    score->endCmd();
    return score;
}

//---------------------------------------------------------
//   removeSymbol
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, removeSymbol)
{
    MasterScore* score = doRemoveSymbol();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-symbol-del.mscx", PARTS_DATA_DIR + u"part-symbol-del.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoRemoveSymbol
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoRemoveSymbol)
{
    MasterScore* score = doRemoveSymbol();
    score->undoRedo(true, 0);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-symbol-udel.mscx", PARTS_DATA_DIR + u"part-symbol-udel.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoRedoRemoveSymbol
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoRedoRemoveSymbol)
{
    MasterScore* score = doRemoveSymbol();
    score->undoRedo(true, 0);
    score->undoRedo(false, 0);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-symbol-urdel.mscx", PARTS_DATA_DIR + u"part-symbol-urdel.mscx"));
    delete score;
}

//---------------------------------------------------------
//   Chordline
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, createPartChordline)
{
    testPartCreation(u"part-chordline");
}

//---------------------------------------------------------
//   doAddChordline
//---------------------------------------------------------

MasterScore* Engraving_PartsTests::doAddChordline()
{
    MasterScore* score = ScoreRW::readScore(PARTS_DATA_DIR + u"part-empty-parts.mscx");
    /*ASSERT_TRUE*/ EXPECT_TRUE(score);

    Measure* m   = score->firstMeasure();
    Segment* s   = m->tick2segment(Fraction(1, 4));
    Chord* chord = toChord(s->element(0));
    Note* note   = chord->upNote();
    EditData dd(0);
    ChordLine* b  = Factory::createChordLine(chord);
    b->setChordLineType(ChordLineType::FALL);
    dd.dropElement = b;

    score->startCmd(TranslatableString::untranslatable("Engraving parts tests"));
    note->drop(dd);
    score->endCmd();          // does layout
    return score;
}

//---------------------------------------------------------
//   addChordline
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, addChordline)
{
    MasterScore* score = doAddChordline();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-chordline-add.mscx", PARTS_DATA_DIR + u"part-chordline-add.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoAddChordline
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoAddChordline)
{
    MasterScore* score = doAddChordline();
    score->undoRedo(true, 0);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-chordline-uadd.mscx", PARTS_DATA_DIR + u"part-chordline-uadd.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoRedoAddChordline
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoRedoAddChordline)
{
    MasterScore* score = doAddChordline();
    score->undoRedo(true, 0);
    score->undoRedo(false, 0);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-chordline-uradd.mscx", PARTS_DATA_DIR + u"part-chordline-uradd.mscx"));
    delete score;
}

//---------------------------------------------------------
//   doRemoveChordline
//---------------------------------------------------------

MasterScore* Engraving_PartsTests::doRemoveChordline()
{
    MasterScore* score = ScoreRW::readScore(PARTS_DATA_DIR + u"part-chordline-parts.mscx");
    /*ASSERT_TRUE*/ EXPECT_TRUE(score);

    Measure* m   = score->firstMeasure();
    Segment* s   = m->first()->next(SegmentType::ChordRest);
    Chord* chord = toChord(s->element(0));

    EngravingItem* se = 0;
    for (EngravingItem* e : chord->el()) {
        if (e->type() == ElementType::CHORDLINE) {
            se = e;
            break;
        }
    }
    score->select(se);

    score->startCmd(TranslatableString::untranslatable("Engraving parts tests"));
    score->cmdDeleteSelection();
    score->setLayoutAll();
    score->endCmd();
    return score;
}

//---------------------------------------------------------
//   removeChordline
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, removeChordline)
{
    MasterScore* score = doRemoveChordline();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-chordline-del.mscx", PARTS_DATA_DIR + u"part-chordline-del.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoRemoveChordline
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoRemoveChordline)
{
    MasterScore* score = doRemoveChordline();
    score->undoRedo(true, 0);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-chordline-udel.mscx", PARTS_DATA_DIR + u"part-chordline-udel.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoRedoRemoveChordline
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoRedoRemoveChordline)
{
    MasterScore* score = doRemoveChordline();
    score->undoRedo(true, 0);
    score->undoRedo(false, 0);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-chordline-urdel.mscx", PARTS_DATA_DIR + u"part-chordline-urdel.mscx"));
    delete score;
}

//---------------------------------------------------------
//   MeasureRepeat
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, createPartMeasureRepeat)
{
    testPartCreation(u"part-measure-repeat");
}

//---------------------------------------------------------
//   doAddMeasureRepeat
//---------------------------------------------------------

MasterScore* Engraving_PartsTests::doAddMeasureRepeat()
{
    MasterScore* score = ScoreRW::readScore(PARTS_DATA_DIR + u"part-empty-parts.mscx");
    /*ASSERT_TRUE*/ EXPECT_TRUE(score);

    Measure* m = score->firstMeasure()->nextMeasure();

    score->startCmd(TranslatableString::untranslatable("Engraving parts tests"));
    score->cmdAddMeasureRepeat(m, 4, 0); // test with 4-measure repeat in first staff
    score->setLayoutAll();
    score->endCmd();

    return score;
}

//---------------------------------------------------------
//   addMeasureRepeat
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, addMeasureRepeat)
{
    MasterScore* score = doAddMeasureRepeat();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-measure-repeat-add.mscx", PARTS_DATA_DIR + u"part-measure-repeat-add.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoAddMeasureRepeat
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoAddMeasureRepeat)
{
    MasterScore* score = doAddMeasureRepeat();

    score->undoRedo(true, 0);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-measure-repeat-uadd.mscx", PARTS_DATA_DIR + u"part-measure-repeat-uadd.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoRedoAddMeasureRepeat
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoRedoAddMeasureRepeat)
{
    MasterScore* score = doAddMeasureRepeat();

    score->undoRedo(true, 0);
    score->undoRedo(false, 0);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-measure-repeat-uradd.mscx", PARTS_DATA_DIR + u"part-measure-repeat-uradd.mscx"));
    delete score;
}

//---------------------------------------------------------
//   doRemoveMeasureRepeat
//---------------------------------------------------------

MasterScore* Engraving_PartsTests::doRemoveMeasureRepeat()
{
    MasterScore* score = ScoreRW::readScore(PARTS_DATA_DIR + u"part-measure-repeat-parts.mscx");
    /*ASSERT_TRUE*/ EXPECT_TRUE(score);

    Measure* m = score->firstMeasure()->nextMeasure()->nextMeasure();
    MeasureRepeat* mr = m->measureRepeatElement(0);
    score->select(mr);

    score->startCmd(TranslatableString::untranslatable("Engraving parts tests"));
    score->cmdDeleteSelection();
    score->setLayoutAll();
    score->endCmd();
    return score;
}

//---------------------------------------------------------
//   removeMeasureRepeat
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, removeMeasureRepeat)
{
    MasterScore* score = doRemoveMeasureRepeat();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-measure-repeat-del.mscx", PARTS_DATA_DIR + u"part-measure-repeat-del.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoRemoveMeasureRepeat
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoRemoveMeasureRepeat)
{
    MasterScore* score = doRemoveMeasureRepeat();
    score->undoRedo(true, 0);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-measure-repeat-udel.mscx", PARTS_DATA_DIR + u"part-measure-repeat-udel.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoRedoRemoveMeasureRepeat
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoRedoRemoveMeasureRepeat)
{
    MasterScore* score = doRemoveMeasureRepeat();
    score->undoRedo(true, 0);
    score->undoRedo(false, 0);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-measure-repeat-urdel.mscx", PARTS_DATA_DIR + u"part-measure-repeat-urdel.mscx"));
    delete score;
}

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, createPartImage)
{
    testPartCreation(u"part-image");
}

//---------------------------------------------------------
//   doAddImage
//---------------------------------------------------------

MasterScore* Engraving_PartsTests::doAddImage()
{
    MasterScore* score = ScoreRW::readScore(PARTS_DATA_DIR + u"part-empty-parts.mscx");
    /*ASSERT_TRUE*/ EXPECT_TRUE(score);

    Measure* m   = score->firstMeasure();
    Segment* s   = m->tick2segment(Fraction(1, 4));
    Chord* chord = toChord(s->element(0));
    Note* note   = chord->upNote();
    EditData dd(0);
    Image* b = Factory::createImage(note);
    b->load(PARTS_DATA_DIR + u"schnee.png");
    dd.dropElement = b;

    score->startCmd(TranslatableString::untranslatable("Engraving parts tests"));
    note->drop(dd);
    score->endCmd();          // does layout
    return score;
}

//---------------------------------------------------------
//   addImage
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, addImage)
{
    MasterScore* score = doAddImage();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-image-add.mscx", PARTS_DATA_DIR + u"part-image-add.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoAddImage
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoAddImage)
{
    MasterScore* score = doAddImage();
    score->undoRedo(true, 0);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-image-uadd.mscx", PARTS_DATA_DIR + u"part-image-uadd.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoRedoAddImage
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoRedoAddImage)
{
    MasterScore* score = doAddImage();
    score->undoRedo(true, 0);
    score->undoRedo(false, 0);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-image-uradd.mscx", PARTS_DATA_DIR + u"part-image-uradd.mscx"));
    delete score;
}

//---------------------------------------------------------
//   doRemoveImage
//---------------------------------------------------------

MasterScore* Engraving_PartsTests::doRemoveImage()
{
    MasterScore* score = ScoreRW::readScore(PARTS_DATA_DIR + u"part-image-parts.mscx");
    /*ASSERT_TRUE*/ EXPECT_TRUE(score);

    Measure* m   = score->firstMeasure();
    Segment* s   = m->first()->next(SegmentType::ChordRest);
    Chord* chord = toChord(s->element(0));
    Note* note   = chord->upNote();
    EngravingItem* fingering = 0;
    for (EngravingItem* e : note->el()) {
        if (e->type() == ElementType::IMAGE) {
            fingering = e;
            break;
        }
    }
    score->select(fingering);

    score->startCmd(TranslatableString::untranslatable("Engraving parts tests"));
    score->cmdDeleteSelection();
    score->setLayoutAll();
    score->endCmd();
    return score;
}

//---------------------------------------------------------
//   removeImage
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, removeImage)
{
    MasterScore* score = doRemoveImage();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-image-del.mscx", PARTS_DATA_DIR + u"part-image-del.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoRemoveImage
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoRemoveImage)
{
    MasterScore* score = doRemoveImage();
    score->undoRedo(true, 0);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-image-udel.mscx", PARTS_DATA_DIR + u"part-image-udel.mscx"));
    delete score;
}

//---------------------------------------------------------
//   undoRedoRemoveImage
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, undoRedoRemoveImage)
{
    MasterScore* score = doRemoveImage();
    score->undoRedo(true, 0);
    score->undoRedo(false, 0);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-image-urdel.mscx", PARTS_DATA_DIR + u"part-image-urdel.mscx"));
    delete score;
}

//---------------------------------------------------------
//   Stemless
//---------------------------------------------------------

TEST_F(Engraving_PartsTests, createPartStemless)
{
    testPartCreation(u"part-stemless");
}

TEST_F(Engraving_PartsTests, partExclusion)
{
    MasterScore* masterScore = ScoreRW::readScore(PARTS_DATA_DIR + u"partExclusion.mscx");

    EXPECT_TRUE(masterScore);

    Score* partScore = TestUtils::createPart(masterScore);
    EXPECT_TRUE(partScore);

    ScoreRW::saveScore(masterScore, u"partExclusion.mscx");
    EXPECT_TRUE(ScoreComp::saveCompareScore(partScore, u"partExclusion-part-0.mscx", PARTS_DATA_DIR + u"partExclusion-part-0.mscx"));

    // Collect the relevant items
    std::vector<EngravingItem*> itemsToExclude;
    for (MeasureBase* mb = masterScore->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            itemsToExclude.push_back(mb);
            continue;
        }
        for (Segment& segment : toMeasure(mb)->segments()) {
            EngravingItem* item = segment.elementAt(0);
            if (item && item->isClef() && !item->generated()) {
                itemsToExclude.push_back(item);
            }
            for (EngravingItem* annotation : segment.annotations()) {
                if (annotation->isTextBase()) {
                    itemsToExclude.push_back(annotation);
                }
            }
        }
    }
    for (auto pair : masterScore->spanner()) {
        Spanner* spanner = pair.second;
        if (spanner && spanner->isOttava()) {
            itemsToExclude.push_back(spanner);
        }
    }

    // Invert exclusion property
    for (EngravingItem* item : itemsToExclude) {
        masterScore->select(item);
        bool exclude = item->getProperty(Pid::EXCLUDE_FROM_OTHER_PARTS).toBool();
        item->undoChangeProperty(Pid::EXCLUDE_FROM_OTHER_PARTS, !exclude);
    }

    // Excluded items are not in the part anymore and viceversa
    EXPECT_TRUE(ScoreComp::saveCompareScore(partScore, u"partExclusion-part-1.mscx", PARTS_DATA_DIR + u"partExclusion-part-1.mscx"));

    // Revert again and check it is equal to the initial file again
    for (EngravingItem* item : itemsToExclude) {
        masterScore->select(item);
        bool exclude = item->getProperty(Pid::EXCLUDE_FROM_OTHER_PARTS).toBool();
        item->undoChangeProperty(Pid::EXCLUDE_FROM_OTHER_PARTS, !exclude);
    }

    // Not applicable anymore because creating new elements creates new EIDs too
    //EXPECT_TRUE(ScoreComp::saveCompareScore(partScore, u"partExclusion-part-0.mscx", PARTS_DATA_DIR + u"partExclusion-part-0.mscx"));
}

TEST_F(Engraving_PartsTests, partPropertyLinking)
{
    MasterScore* masterScore = ScoreRW::readScore(PARTS_DATA_DIR + u"partPropertyLinking.mscx");

    EXPECT_TRUE(masterScore);

    Score* partScore = TestUtils::createPart(masterScore);
    EXPECT_TRUE(partScore);

    ScoreRW::saveScore(masterScore, u"partPropertyLinking.mscx");
    EXPECT_TRUE(ScoreComp::saveCompareScore(partScore, u"partPropertyLinking-part-0.mscx",
                                            PARTS_DATA_DIR + u"partPropertyLinking-part-0.mscx"));

    Dynamic* dynamic = nullptr;
    Measure* measure = masterScore->firstMeasure();
    for (Segment& segment : measure->segments()) {
        for (EngravingItem* annotation : segment.annotations()) {
            if (annotation->isDynamic()) {
                dynamic = toDynamic(annotation);
                break;
            }
        }
    }
    Dynamic* testItem = toDynamic(dynamic->findLinkedInScore(partScore));
    EXPECT_TRUE(testItem);

    testItem->undoChangeProperty(Pid::PLACEMENT, PropertyValue::fromValue(PlacementV::ABOVE), PropertyFlags::NOSTYLE);
    testItem->undoChangeProperty(Pid::DYNAMICS_SIZE, PropertyValue::fromValue(1.2), PropertyFlags::NOSTYLE);
    EXPECT_FALSE(testItem->isPositionLinkedToMaster());
    EXPECT_FALSE(testItem->isAppearanceLinkedToMaster());

    EXPECT_TRUE(ScoreComp::saveCompareScore(partScore, u"partPropertyLinking-part-1.mscx",
                                            PARTS_DATA_DIR + u"partPropertyLinking-part-1.mscx"));

    testItem->undoChangeProperty(Pid::POSITION_LINKED_TO_MASTER, true, PropertyFlags::NOSTYLE);
    testItem->undoChangeProperty(Pid::APPEARANCE_LINKED_TO_MASTER, true, PropertyFlags::NOSTYLE);
    EXPECT_TRUE(ScoreComp::saveCompareScore(partScore, u"partPropertyLinking-part-0.mscx",
                                            PARTS_DATA_DIR + u"partPropertyLinking-part-0.mscx"));
}

TEST_F(Engraving_PartsTests, partSpanners)
{
    bool useRead302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;

    testPartCreation(u"part-spanners");

    MScore::useRead302InTestMode = useRead302;
}

TEST_F(Engraving_PartsTests, partTies) {
    const String test = u"linked-ties";
    MasterScore* score = ScoreRW::readScore(PARTS_DATA_DIR + test + u".mscx");
    ASSERT_TRUE(score);
    createLinkedStaff(score);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, test + u"-1.mscx", PARTS_DATA_DIR + test + u"-1.mscx"));
    TestUtils::createPart(score);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, test + u"-parts.mscx", PARTS_DATA_DIR + test + u"-parts.mscx"));
    delete score;
}

TEST_F(Engraving_PartsTests, partVisibleTracks) {
    Score* score = ScoreRW::readScore(PARTS_DATA_DIR + u"part-visible-tracks.mscx");
    EXPECT_TRUE(score);

    Score* part = nullptr;
    for (Score* s : score->scoreList()) {
        if (!s->isMaster()) {
            part = s;
            break;
        }
    }
    EXPECT_TRUE(part);
    Measure* m = part->firstMeasure();
    EXPECT_TRUE(m);
    Chord* c = m->findChord(Fraction(0, 1), 0);
    EXPECT_TRUE(c);
    Note* n = c->downNote();
    EXPECT_TRUE(n);

    part->startCmd(TranslatableString::untranslatable("Engraving parts tests"));
    part->select(n);
    part->changeSelectedElementsVoice(1);
    part->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"part-visible-tracks-score.mscx",
                                            PARTS_DATA_DIR + u"part-visible-tracks-score-ref.mscx"));

    EXPECT_TRUE(ScoreComp::saveCompareScore(part, u"part-visible-tracks-part.mscx",
                                            PARTS_DATA_DIR + u"part-visible-tracks-part-ref.mscx"));
}

TEST_F(Engraving_PartsTests, inputFromParts) {
    bool useRead302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;

    // Enter notes *in parts* and check that they are correctly cloned to the score.

    Score* score = ScoreRW::readScore(PARTS_DATA_DIR + u"input-from-parts.mscz");
    EXPECT_TRUE(score);
    staff_idx_t fluteStaff = 0;
    staff_idx_t oboeStaff = 1;
    staff_idx_t clarinetStaff = 2;
    staff_idx_t bassoonStaff = 3;

    Score* flutePart = nullptr;
    Score* oboePart = nullptr;
    Score* clarinetPart = nullptr;
    Score* bassoonPart = nullptr;
    for (Score* part : score->scoreList()) {
        String partName = part->name();
        if (partName == u"Flute") {
            flutePart = part;
        } else if (partName == u"Oboe") {
            oboePart = part;
        } else if (partName == u"Clarinet in B♭") {
            clarinetPart = part;
        } else if (partName == u"Bassoon") {
            bassoonPart = part;
        }
    }
    EXPECT_TRUE(flutePart && oboePart && clarinetPart && bassoonPart);

    track_idx_t voice = 3;
    Segment* partSegment = flutePart->firstMeasure()->findFirstR(SegmentType::ChordRest, Fraction(0, 1));
    EXPECT_TRUE(partSegment);
    flutePart->setNoteRest(partSegment, voice, NoteVal(60), Fraction(1, 1));
    Segment* scoreSegment = score->tick2segment(partSegment->tick(), true, SegmentType::ChordRest);
    EXPECT_TRUE(scoreSegment);
    Chord* chord = toChord(scoreSegment->elementAt(staff2track(fluteStaff) + voice));
    EXPECT_TRUE(chord);

    voice = 2;
    partSegment = oboePart->firstMeasure()->nextMeasure()->findFirstR(SegmentType::ChordRest, Fraction(0, 1));
    EXPECT_TRUE(partSegment);
    oboePart->setNoteRest(partSegment, voice, NoteVal(60), Fraction(1, 1));
    scoreSegment = score->tick2segment(partSegment->tick(), true, SegmentType::ChordRest);
    EXPECT_TRUE(scoreSegment);
    chord = toChord(scoreSegment->elementAt(staff2track(oboeStaff) + voice));
    EXPECT_TRUE(chord);

    voice = 1;
    partSegment = clarinetPart->firstMeasure()->nextMeasure()->nextMeasure()->findFirstR(SegmentType::ChordRest, Fraction(0, 1));
    EXPECT_TRUE(partSegment);
    clarinetPart->setNoteRest(partSegment, voice, NoteVal(60), Fraction(1, 1));
    scoreSegment = score->tick2segment(partSegment->tick(), true, SegmentType::ChordRest);
    EXPECT_TRUE(scoreSegment);
    chord = toChord(scoreSegment->elementAt(staff2track(clarinetStaff) + voice));
    EXPECT_TRUE(chord);

    voice = 0;
    partSegment = bassoonPart->firstMeasure()->nextMeasure()->nextMeasure()->nextMeasure()->findFirstR(SegmentType::ChordRest, Fraction(0,
                                                                                                                                        1));
    EXPECT_TRUE(partSegment);
    bassoonPart->setNoteRest(partSegment, voice, NoteVal(60), Fraction(1, 1));
    scoreSegment = score->tick2segment(partSegment->tick(), true, SegmentType::ChordRest);
    EXPECT_TRUE(scoreSegment);
    chord = toChord(scoreSegment->elementAt(staff2track(bassoonStaff) + voice));
    EXPECT_TRUE(chord);

    MScore::useRead302InTestMode = useRead302;
}

//---------------------------------------------------------
//   staffStyles
//---------------------------------------------------------

#if 0
TEST_F(Engraving_PartsTests, staffStyles)
{
    MasterScore* score = ScoreRW::readScore(PARTS_DATA_DIR + u"part1.mscx");
    /*ASSERT_TRUE*/ EXPECT_TRUE(score);

    //int numOfStaffTypes = score->staffTypes().count();
    createParts(score);
    // check the number of staff styles did not change
    //EXPECT_EQ(numOfStaffTypes, score->staffTypes().count());
    // modify a staff type
    int numOfLines = score->staffType(0)->lines() - 1;
    StaffType* newStaffType = score->staffType(0)->clone();
    newStaffType->setLines(numOfLines);
    score->addStaffType(0, newStaffType);
    // check the number of staff lines is correctly updated in root score and in parts
    EXPECT_EQ(score->staff(0)->lines(), numOfLines);
    Excerpt* part = score->excerpts().at(0);
    EXPECT_EQ(part->excerptScore()->staff(0)->lines(), numOfLines);
    part = score->excerpts().at(1);
    EXPECT_EQ(part->excerptScore()->staff(0)->lines(), numOfLines);
    delete score;
}

#endif
