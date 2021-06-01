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
#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/chordrest.h"

static const QString SPLITSTAFF_DATA_DIR("splitstaff_data/");

using namespace Ms;

//---------------------------------------------------------
//   TestSplitStaff
//---------------------------------------------------------

class TestSplitStaff : public QObject, public MTest
{
    Q_OBJECT

    void splitstaff(int, int);

private slots:
    void initTestCase();
    void splitstaff01() { splitstaff(1, 0); }   //single notes
    void splitstaff02() { splitstaff(2, 0); }   //chord
    void splitstaff03() { splitstaff(3, 1); }   //non-top staff
    void splitstaff04() { splitstaff(4, 0); }   //slur up
    void splitstaff05() { splitstaff(5, 0); }   //slur down
    void splitstaff06() { splitstaff(6, 0); }   //tuplet
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestSplitStaff::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
///   splitstaff
//---------------------------------------------------------

void TestSplitStaff::splitstaff(int idx, int staffIdx)
{
    MasterScore* score = readScore(SPLITSTAFF_DATA_DIR + QString("splitstaff0%1.mscx").arg(idx));
    score->startCmd();
    score->splitStaff(staffIdx, 60);
    score->endCmd();

    QVERIFY(saveCompareScore(score, QString("splitstaff0%1.mscx").arg(idx),
                             SPLITSTAFF_DATA_DIR + QString("splitstaff0%1-ref.mscx").arg(idx)));
    delete score;
}

QTEST_MAIN(TestSplitStaff)
#include "tst_splitstaff.moc"
