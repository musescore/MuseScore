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

#include "testing/qtestsuite.h"
#include "testbase.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"

static const QString RWUNDORESET_DATA_DIR("readwriteundoreset_data/");

using namespace Ms;

//---------------------------------------------------------
//   TestReadWrite
//---------------------------------------------------------

class TestReadWriteUndoReset : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testReadWriteResetPositions_data();
    void testReadWriteResetPositions();

    void testMMRestLinksRecreateMMRest();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestReadWriteUndoReset::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   testReadWriteResetPositions
//---------------------------------------------------------

void TestReadWriteUndoReset::testReadWriteResetPositions_data()
{
    QTest::addColumn<QString>("file");

    QTest::newRow("barlines") << "barlines";
    QTest::newRow("slurs") << "slurs";
    QTest::newRow("mmrestBarlineTextLinks") << "mmrestBarlineTextLinks";   // see issue #296426
}

void TestReadWriteUndoReset::testReadWriteResetPositions()
{
    QFETCH(QString, file);

    QString readFile(RWUNDORESET_DATA_DIR + file + ".mscx");
    QString writeFile(file + "-undoreset-test.mscx");

    MasterScore* score = readScore(readFile);
    QVERIFY(score);
    score->cmdResetAllPositions();
    score->undoRedo(/* undo */ true, nullptr);
    QVERIFY(saveCompareScore(score, writeFile, readFile));

    delete score;
}

//---------------------------------------------------------
//   testMMRestLinksRecreateMMRest
///   For barlines links with MM rests a separate test is
///   needed: in this test score, if creating MM rests from
///   scratch, <linked> tags in BarLines may have appeared
///   before <linkedMain> tags, so they were not able to
///   link and prevented text elements from linking as well.
///
///   See issue #296426
//---------------------------------------------------------

void TestReadWriteUndoReset::testMMRestLinksRecreateMMRest()
{
    const QString file("mmrestBarlineTextLinks");

    QString readFile(RWUNDORESET_DATA_DIR + file + ".mscx");
    QString writeFile(file + "-recreate-mmrest-test.mscx");
    QString disableMMRestRefFile(RWUNDORESET_DATA_DIR + file + "-disable-mmrest-ref.mscx");
    QString recreateMMRestRefFile(RWUNDORESET_DATA_DIR + file + "-recreate-mmrest-ref.mscx");

    MasterScore* score = readScore(readFile);
    QVERIFY(score);

    // Regenerate MM rests from scratch:
    // 1) turn MM rests off
    score->startCmd();
    score->undo(new ChangeStyleVal(score, Sid::createMultiMeasureRests, false));
    score->endCmd();

    // 2) save/close/reopen the score
    QVERIFY(saveCompareScore(score, writeFile, disableMMRestRefFile));
    delete score;
    score = readCreatedScore(writeFile);

    // 3) turn MM rests back on
    score->startCmd();
    score->undo(new ChangeStyleVal(score, Sid::createMultiMeasureRests, true));
    score->endCmd();

    QVERIFY(saveCompareScore(score, writeFile, recreateMMRestRefFile));

    delete score;
}

QTEST_MAIN(TestReadWriteUndoReset)
#include "tst_readwriteundoreset.moc"
