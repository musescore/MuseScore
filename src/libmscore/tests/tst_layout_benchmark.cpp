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

static const QString LAYOUT_DATA_DIR("layout_data/");

using namespace Ms;

//namespace Ms {
//extern void dumpTags();
//};

//---------------------------------------------------------
//   TestBechmark
//---------------------------------------------------------

class TestLayoutBenchmark : public QObject, public MTest
{
    Q_OBJECT

    MasterScore * score;
    void beam(const char* path);

private slots:
    void initTestCase();
    void benchmark3();
    void benchmark1();
    void benchmark2();
    void benchmark4();              // incremental layout (one page)
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestLayoutBenchmark::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   benchmark
//---------------------------------------------------------

void TestLayoutBenchmark::benchmark3()
{
    QString path = root + "/" + LAYOUT_DATA_DIR + "goldberg.mscx";
    score = new MasterScore(mscore->baseStyle());
    score->setName(path);
    MScore::testMode = true;
    QBENCHMARK {
        score->loadMsc(path, false);
    }
}

void TestLayoutBenchmark::benchmark1()
{
    // score = readScore(LAYOUT_DATA_DIR + "goldberg.mscx");
    QBENCHMARK {                          // cold run
        score->doLayout();
    }
}

void TestLayoutBenchmark::benchmark2()
{
    score->doLayout();
    QBENCHMARK {                          // warm run
        score->doLayout();
    }
}

void TestLayoutBenchmark::benchmark4()
{
    QBENCHMARK {
        score->startCmd();
        score->setLayout(Fraction(1,4), -1);
        score->endCmd();
    }
}

QTEST_MAIN(TestLayoutBenchmark)
#include "tst_layout_benchmark.moc"
