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
#include "libmscore/chord.h"

static const QString JOIN_DATA_DIR("join_data/");

using namespace Ms;

//---------------------------------------------------------
//   TestJoin
//---------------------------------------------------------

class TestJoin : public QObject, public MTest
{
    Q_OBJECT

    void join(const char* p1, const char* p2);
    void join(const char* p1, const char* p2, int);
    void join1(const char* p1);

private slots:
    void initTestCase();
    void join01() { join("join01.mscx", "join01-ref.mscx"); }
    void join02() { join("join02.mscx", "join02-ref.mscx"); }
    void join03() { join("join03.mscx", "join03-ref.mscx"); }
    void join04() { join("join04.mscx", "join04-ref.mscx"); }
    void join05() { join("join05.mscx", "join05-ref.mscx"); }
    void join06() { join("join06.mscx", "join06-ref.mscx", 1); }
    void join07() { join("join07.mscx", "join07-ref.mscx"); }
    void join08() { join1("join08.mscx"); }
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestJoin::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   join
//---------------------------------------------------------

void TestJoin::join(const char* p1, const char* p2)
{
    MasterScore* score = readScore(JOIN_DATA_DIR + p1);
    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();

    QVERIFY(m1 != 0);
    QVERIFY(m2 != 0);
    QVERIFY(m1 != m2);

    score->cmdJoinMeasure(m1, m2);

    QVERIFY(saveCompareScore(score, p1, JOIN_DATA_DIR + p2));
    delete score;
}

void TestJoin::join(const char* p1, const char* p2, int index)
{
    MasterScore* score = readScore(JOIN_DATA_DIR + p1);
    Measure* m1 = score->firstMeasure();
    for (int i = 0; i < index; ++i) {
        m1 = m1->nextMeasure();
    }
    Measure* m2 = m1->nextMeasure();

    QVERIFY(m1 != 0);
    QVERIFY(m2 != 0);
    QVERIFY(m1 != m2);

    score->cmdJoinMeasure(m1, m2);

    QVERIFY(saveCompareScore(score, p1, JOIN_DATA_DIR + p2));
    delete score;
}

void TestJoin::join1(const char* p1)
{
    MasterScore* score = readScore(JOIN_DATA_DIR + p1);
    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();

    QVERIFY(m1 != 0);
    QVERIFY(m2 != 0);
    QVERIFY(m1 != m2);

    score->cmdJoinMeasure(m1, m2);

    // check if notes are still on line 6
    Segment* s = score->firstSegment(SegmentType::ChordRest);

    for (int i = 0; i < 8; ++i) {
        Note* note = static_cast<Ms::Chord*>(s->element(0))->upNote();
        QVERIFY(note->line() == 6);
        s = s->next1(SegmentType::ChordRest);
    }

    delete score;
}

QTEST_MAIN(TestJoin)
#include "tst_join.moc"
