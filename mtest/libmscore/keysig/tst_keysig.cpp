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
#include "libmscore/measure.h"
#include "libmscore/keysig.h"
#include "libmscore/undo.h"
#include "libmscore/fraction.h"
#include "libmscore/part.h"

#define DIR QString("libmscore/keysig/")

using namespace Ms;

//---------------------------------------------------------
//   TestKeySig
//---------------------------------------------------------

class TestKeySig : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase();
    void keysig();
    void concertPitch();
    void keysig_78216();
    void preferSharpFlat();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestKeySig::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   keysig
//---------------------------------------------------------

void TestKeySig::keysig()
{
    QString writeFile1("keysig01-test.mscx");
    QString reference1(DIR + "keysig01-ref.mscx");     // with D maj
    QString writeFile2("keysig02-test.mscx");
    QString reference2(DIR + "keysig02-ref.mscx");     // with Eb maj
    QString writeFile3("keysig03-test.mscx");
    QString reference3(DIR + "keysig.mscx");           // orig
    QString writeFile4("keysig04-test.mscx");
    QString reference4(DIR + "keysig02-ref.mscx");     // with Eb maj
    QString writeFile5("keysig05-test.mscx");
    QString reference5(DIR + "keysig01-ref.mscx");     // with D maj
    QString writeFile6("keysig06-test.mscx");
    QString reference6(DIR + "keysig.mscx");           // orig

    // read file
    MasterScore* score = readScore(DIR + "keysig.mscx");
    Measure* m2 = score->firstMeasure()->nextMeasure();

    // add a key signature (D major) in measure 2
    KeySigEvent ke2;
    ke2.setKey(Key::D);
    score->startCmd();
    score->undoChangeKeySig(score->staff(0), m2->tick(), ke2);
    score->endCmd();
    QVERIFY(saveCompareScore(score, writeFile1, reference1));

    // change key signature in measure 2 to E flat major
    KeySigEvent ke_3;
    ke_3.setKey(Key(-3));
    score->startCmd();
    score->undoChangeKeySig(score->staff(0), m2->tick(), ke_3);
    score->endCmd();
    QVERIFY(saveCompareScore(score, writeFile2, reference2));

    // remove key signature in measure 2
    Segment* s = m2->first();
    while (!(s->isKeySigType())) {
        s = s->next();
    }
    Element* e = s->element(0);
    score->startCmd();
    score->undoRemoveElement(e);
    score->endCmd();
    QVERIFY(saveCompareScore(score, writeFile3, reference3));

    // undo remove
    score->undoStack()->undo(&ed);
    QVERIFY(saveCompareScore(score, writeFile4, reference4));

    // undo change
    score->undoStack()->undo(&ed);
    QVERIFY(saveCompareScore(score, writeFile5, reference5));

    // undo add
    score->undoStack()->undo(&ed);
    QVERIFY(saveCompareScore(score, writeFile6, reference6));

    delete score;
}

//---------------------------------------------------------
//   keysig_78216
//    input score has section breaks on non-measure MeasureBase objects.
//    should not display courtesy keysig at the end of final measure of each section (meas 1, 2, & 3), even if section break occurs on subsequent non-measure frame.
//---------------------------------------------------------

void TestKeySig::keysig_78216()
{
    MasterScore* score = readScore(DIR + "keysig_78216.mscx");

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();
    Measure* m3 = m2->nextMeasure();

    // verify no keysig exists in segment of final tick of m1, m2, m3
    QVERIFY2(m1->findSegment(SegmentType::KeySig, m1->endTick()) == nullptr,
             "Should be no keysig at end of measure 1.");
    QVERIFY2(m2->findSegment(SegmentType::KeySig, m2->endTick()) == nullptr,
             "Should be no keysig at end of measure 2.");
    QVERIFY2(m3->findSegment(SegmentType::KeySig, m3->endTick()) == nullptr,
             "Should be no keysig at end of measure 3.");

    delete score;
}

//---------------------------------------------------------
//   concertPitch
//---------------------------------------------------------

void TestKeySig::concertPitch()
{
    MasterScore* score = readScore(DIR + "concert-pitch.mscx");
    score->cmdConcertPitchChanged(true);
    QVERIFY(saveCompareScore(score, "concert-pitch-01-test.mscx", DIR + "concert-pitch-01-ref.mscx"));
    score->cmdConcertPitchChanged(false);
    QVERIFY(saveCompareScore(score, "concert-pitch-02-test.mscx", DIR + "concert-pitch-02-ref.mscx"));

    delete score;
}

//---------------------------------------------------------
//   preferSharpFlat
//---------------------------------------------------------

void TestKeySig::preferSharpFlat()
{
    MasterScore* score1 = readScore(DIR + "preferSharpFlat-1.mscx");
    auto parts = score1->parts();
    Part* part1 = parts[0];
    part1->setPreferSharpFlat(PreferSharpFlat::FLATS);
    score1->transpositionChanged(part1, part1->instrument(Fraction(0, 1))->transpose(), Fraction(0, 1),
                                 Fraction(16, 4));
    score1->update();
    score1->doLayout();
    QVERIFY(saveCompareScore(score1, "preferSharpFlat-1-test.mscx", DIR + "preferSharpFlat-1-ref.mscx"));
    delete score1;

    MasterScore* score2 = readScore(DIR + "preferSharpFlat-2.mscx");
    score2->cmdSelectAll();
    score2->startCmd();
    // transpose augmented unison up
    score2->transpose(TransposeMode::BY_INTERVAL, TransposeDirection::UP, Key::C, 1, true, true, true);
    score2->endCmd();
    QVERIFY(saveCompareScore(score2, "preferSharpFlat-2-test.mscx", DIR + "preferSharpFlat-2-ref.mscx"));
    delete score2;
}

QTEST_MAIN(TestKeySig)
#include "tst_keysig.moc"
