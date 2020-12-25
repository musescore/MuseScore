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

static const QString CONCERTPITCH_DATA_DIR("concertpitch_data/");

using namespace Ms;

//---------------------------------------------------------
//   TestConcertPitchBenchmark
//---------------------------------------------------------

class TestConcertPitchBenchmark : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase();
    void benchmark();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestConcertPitchBenchmark::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   benchmark
//---------------------------------------------------------

void TestConcertPitchBenchmark::benchmark()
{
    Score* score = readScore(CONCERTPITCH_DATA_DIR + "concertpitchbenchmark.mscx");
    QBENCHMARK {
        // switch to concert pitch
        score->cmdConcertPitchChanged(true);
        score->doLayout();
        // switch back
        // TODO: this should be UNDO, but UNDO doesn't work with transpose!
        score->cmdConcertPitchChanged(false);
        score->doLayout();
    }
}

QTEST_MAIN(TestConcertPitchBenchmark)
#include "tst_concertpitchbenchmark.moc"
