//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"
#include "libmscore/excerpt.h"
#include "libmscore/part.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/chordrest.h"
#include "libmscore/harmony.h"
#include "libmscore/duration.h"
#include "libmscore/durationtype.h"

#define DIR QString("libmscore/chordsymbol/")

using namespace Ms;

//---------------------------------------------------------
//   TestChordSymbol
//---------------------------------------------------------

class TestChordSymbol : public QObject, public MTest
{
    Q_OBJECT

    MasterScore* test_pre(const char* p);
    void test_post(MasterScore* score, const char* p);

    void selectAllChordSymbols(MasterScore* score);
    void realizeSelectionVoiced(MasterScore* score, Voicing voicing);

private slots:
    void initTestCase();
    void testExtend();
    void testClear();
    void testAddLink();
    void testAddPart();
    void testNoSystem();
    void testTranspose();
    void testTransposePart();
    void testRealizeClose();
    void testRealizeDrop2();
    void testRealize3Note();
    void testRealize4Note();
    void testRealize6Note();
    void testRealizeConcertPitch();
    void testRealizeTransposed();
    void testRealizeOverride();
    void testRealizeTriplet();
    void testRealizeDuration();
    void testRealizeJazz();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestChordSymbol::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   chordsymbol
//---------------------------------------------------------

MasterScore* TestChordSymbol::test_pre(const char* p)
{
    QString p1 = DIR + p + ".mscx";
    MasterScore* score = readScore(p1);
    score->doLayout();
    return score;
}

void TestChordSymbol::test_post(MasterScore* score, const char* p)
{
    QString p1 = p;
    p1 += "-test.mscx";
    QString p2 = DIR + p + "-ref.mscx";
    QVERIFY(saveCompareScore(score, p1, p2));
    delete score;
}

//---------------------------------------------------------
//   TestChordSymbol
///   select all chord symbols within the specified score
//---------------------------------------------------------
void TestChordSymbol::selectAllChordSymbols(MasterScore* score)
{
    //find a chord symbol
    Segment* seg = score->firstSegment(SegmentType::ChordRest);
    Element* e = 0;
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
//   realizeSelectionVoiced
///   realize the current selection of the score using
///   the specified voicing
//---------------------------------------------------------
void TestChordSymbol::realizeSelectionVoiced(MasterScore* score, Voicing voicing)
{
    for (Element* e : score->selection().elements()) {
        if (e->isHarmony()) {
            e->setProperty(Pid::HARMONY_VOICING, int(voicing));
        }
    }
    score->startCmd();
    score->cmdRealizeChordSymbols();
    score->endCmd();
}

void TestChordSymbol::testExtend()
{
    MasterScore* score = test_pre("extend");
    Measure* m = score->firstMeasure();
    Segment* s = m->first(SegmentType::ChordRest);
    ChordRest* cr = s->cr(0);
    score->changeCRlen(cr, TDuration::DurationType::V_WHOLE);
    score->doLayout();
    test_post(score, "extend");
}

void TestChordSymbol::testClear()
{
    MasterScore* score = test_pre("clear");
    Measure* m = score->firstMeasure();
    score->select(m, SelectType::SINGLE, 0);
    score->cmdDeleteSelection();
    score->doLayout();
    test_post(score, "clear");
}

void TestChordSymbol::testAddLink()
{
    MasterScore* score = test_pre("add-link");
    Segment* seg = score->firstSegment(SegmentType::ChordRest);
    ChordRest* cr = seg->cr(0);
    Harmony* harmony = new Harmony(score);
    harmony->setHarmony("C7");
    harmony->setTrack(cr->track());
    harmony->setParent(cr->segment());
    score->undoAddElement(harmony);
    score->doLayout();
    test_post(score, "add-link");
}

void TestChordSymbol::testAddPart()
{
    MasterScore* score = test_pre("add-part");
    Segment* seg = score->firstSegment(SegmentType::ChordRest);
    ChordRest* cr = seg->cr(0);
    Harmony* harmony = new Harmony(score);
    harmony->setHarmony("C7");
    harmony->setTrack(cr->track());
    harmony->setParent(cr->segment());
    score->undoAddElement(harmony);
    score->doLayout();
    test_post(score, "add-part");
}

void TestChordSymbol::testNoSystem()
{
    MasterScore* score = test_pre("no-system");

    //
    // create first part
    //
    QList<Part*> parts;
    parts.append(score->parts().at(0));
    Score* nscore = new Score(score);

    Excerpt* ex = new Excerpt(score);
    ex->setPartScore(nscore);
    nscore->setExcerpt(ex);
    score->excerpts().append(ex);
    ex->setTitle(parts.front()->longName());
    ex->setParts(parts);
    Excerpt::createExcerpt(ex);
    QVERIFY(nscore);

//      nscore->setTitle(parts.front()->partName());
    nscore->style().set(Sid::createMultiMeasureRests, true);

    //
    // create second part
    //
    parts.clear();
    parts.append(score->parts().at(1));
    nscore = new Score(score);

    ex = new Excerpt(score);
    ex->setPartScore(nscore);
    nscore->setExcerpt(ex);
    score->excerpts().append(ex);
    ex->setTitle(parts.front()->longName());
    ex->setParts(parts);
    Excerpt::createExcerpt(ex);
    QVERIFY(nscore);

//      nscore->setTitle(parts.front()->partName());
    nscore->style().set(Sid::createMultiMeasureRests, true);

    score->setExcerptsChanged(true);
    score->doLayout();
    test_post(score, "no-system");
}

void TestChordSymbol::testTranspose()
{
    MasterScore* score = test_pre("transpose");
    score->startCmd();
    score->cmdSelectAll();
    score->transpose(TransposeMode::BY_INTERVAL, TransposeDirection::UP, Key::C, 4, false, true, true);
    score->endCmd();
    test_post(score, "transpose");
}

void TestChordSymbol::testTransposePart()
{
    MasterScore* score = test_pre("transpose-part");
    score->startCmd();
    score->cmdSelectAll();
    score->transpose(TransposeMode::BY_INTERVAL, TransposeDirection::UP, Key::C, 4, false, true, true);
    score->endCmd();
    test_post(score, "transpose-part");
}

//---------------------------------------------------------
//   testRealizeClose
///   check close voicing algorithm
//---------------------------------------------------------
void TestChordSymbol::testRealizeClose()
{
    MasterScore* score = test_pre("realize");
    selectAllChordSymbols(score);
    realizeSelectionVoiced(score, Voicing::CLOSE);
    test_post(score, "realize-close");
}

//---------------------------------------------------------
//   testRealizeDrop2
///   check Drop 2 voicing algorithm
//---------------------------------------------------------
void TestChordSymbol::testRealizeDrop2()
{
    MasterScore* score = test_pre("realize");
    selectAllChordSymbols(score);
    realizeSelectionVoiced(score, Voicing::DROP_2);
    test_post(score, "realize-drop2");
}

//---------------------------------------------------------
//   testRealize3Note
///   check 3 note voicing algorithm
//---------------------------------------------------------
void TestChordSymbol::testRealize3Note()
{
    MasterScore* score = test_pre("realize");
    selectAllChordSymbols(score);
    realizeSelectionVoiced(score, Voicing::THREE_NOTE);
    test_post(score, "realize-3note");
}

//---------------------------------------------------------
//   testRealize4Note
///   check 4 note voicing algorithm
//---------------------------------------------------------
void TestChordSymbol::testRealize4Note()
{
    MasterScore* score = test_pre("realize");
    selectAllChordSymbols(score);
    realizeSelectionVoiced(score, Voicing::FOUR_NOTE);
    test_post(score, "realize-4note");
}

//---------------------------------------------------------
//   testRealize6Note
///   check 6 note voicing algorithm
//---------------------------------------------------------
void TestChordSymbol::testRealize6Note()
{
    MasterScore* score = test_pre("realize");
    selectAllChordSymbols(score);
    realizeSelectionVoiced(score, Voicing::SIX_NOTE);
    test_post(score, "realize-6note");
}

//---------------------------------------------------------
//   testRealizeConcertPitch
///   Check if the note pitches and tpcs are correct after realizing
///   chord symbols on transposed instruments.
//---------------------------------------------------------
void TestChordSymbol::testRealizeConcertPitch()
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
//   testRealizeTransposed
///   Check if the note pitches and tpcs are correct after
///   transposing the score
//---------------------------------------------------------
void TestChordSymbol::testRealizeTransposed()
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
//   testRealizeOverride
///   Check for correctness when using the override
///   feature for realizing chord symbols
//---------------------------------------------------------
void TestChordSymbol::testRealizeOverride()
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
//   testRealizeTriplet
///   Check for correctness when realizing chord symbols on triplets
//---------------------------------------------------------
void TestChordSymbol::testRealizeTriplet()
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
//   testRealizeDuration
///   Check for correctness when realizing chord symbols
///   with different durations
//---------------------------------------------------------
void TestChordSymbol::testRealizeDuration()
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
//   testRealizeJazz
///   Check for correctness when realizing chord symbols
///   with jazz mode
//---------------------------------------------------------
void TestChordSymbol::testRealizeJazz()
{
    MasterScore* score = test_pre("realize-jazz");
    //realize all chord symbols
    selectAllChordSymbols(score);
    score->startCmd();
    score->cmdRealizeChordSymbols();
    score->endCmd();
    test_post(score, "realize-jazz");
}

QTEST_MAIN(TestChordSymbol)
#include "tst_chordsymbol.moc"
