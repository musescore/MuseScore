//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QString>

#include "testing/qtestsuite.h"
#include "testbase.h"
#include "libmscore/score.h"

static const QString BIAB_DIR("data/");

using namespace Ms;

//---------------------------------------------------------
//   TestBiab
//---------------------------------------------------------

class TestBiab : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase();
    void biab_data();
    void biab();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestBiab::initTestCase()
{
    initMTest(QString(iex_bb_tests_DATA_ROOT));
}

//---------------------------------------------------------
//   biab_data
//    every "xxx" test requires a *.SGU file and a *.mscx file:
//          xxx.SGU      is the SGU file
//          xxx-ref.mscx is the corresponding (correct)
//                       mscore 2.0 file
//---------------------------------------------------------

void TestBiab::biab_data()
{
    QTest::addColumn<QString>("file");

    QTest::newRow("chords") << "chords";          // notes.SGU notes-ref.mscx
}

//---------------------------------------------------------
//   biab
//---------------------------------------------------------

void TestBiab::biab()
{
    QFETCH(QString, file);

    QString readFile(BIAB_DIR + file + ".SGU");
    QString writeFile(file + "-test.mscx");
    QString reference(BIAB_DIR + file + "-ref.mscx");

    MasterScore* score = readScore(readFile);
    QVERIFY(score);
    score->doLayout();
    QVERIFY(saveCompareScore(score, writeFile, reference));
}

QTEST_MAIN(TestBiab)
#include "tst_biab.moc"
