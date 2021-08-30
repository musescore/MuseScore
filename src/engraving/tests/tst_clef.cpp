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

#include "testing/qtestsuite.h"

#include "testbase.h"

#include "libmscore/masterscore.h"
#include "libmscore/measure.h"
#include "libmscore/undo.h"

static const QString CLEF_DATA_DIR("clef_data/");

using namespace Ms;

//---------------------------------------------------------
//   TestClef
//---------------------------------------------------------

class TestClef : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase();
    void clef1();
    void clef2();
    void clef3();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestClef::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   clef1
//    two clefs at tick position zero
//---------------------------------------------------------

void TestClef::clef1()
{
    MasterScore* score = readScore(CLEF_DATA_DIR + "clef-1.mscx");
    QVERIFY(saveCompareScore(score, "clef-1.mscx", CLEF_DATA_DIR + "clef-1-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//   clef2
//    change timesig -> rewrite measures ->insertTime
//---------------------------------------------------------

void TestClef::clef2()
{
    MasterScore* score = readScore(CLEF_DATA_DIR + "clef-2.mscx");
    Measure* m = score->firstMeasure();
    m = m->nextMeasure();
    m = m->nextMeasure();
    TimeSig* ts = new TimeSig(score->dummy()->segment());
    ts->setSig(Fraction(2, 4));
    score->cmdAddTimeSig(m, 0, ts, false);

    score->doLayout();
    QVERIFY(saveCompareScore(score, "clef-2.mscx", CLEF_DATA_DIR + "clef-2-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//   clef3
//    change the first clef of a score by changing the first measure's clef
//---------------------------------------------------------

void TestClef::clef3()
{
    MasterScore* score = readScore(CLEF_DATA_DIR + "clef-3.mscx");
    Measure* m = score->firstMeasure();
    score->undoChangeClef(score->staff(0), m, ClefType::F);

    score->doLayout();
    QVERIFY(saveCompareScore(score, "clef-3.mscx", CLEF_DATA_DIR + "clef-3-ref.mscx"));
    delete score;
}

QTEST_MAIN(TestClef)
#include "tst_clef.moc"
