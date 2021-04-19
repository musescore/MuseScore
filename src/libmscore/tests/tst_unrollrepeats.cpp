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

static const QString UNROLLREPEATS_DATA_DIR("unrollrepeats_data/");

using namespace Ms;

//---------------------------------------------------------
//   TestUnrollRepeats
//---------------------------------------------------------

class TestUnrollRepeats : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase();
    void clefKeyTs();
    void pickupMeasure();
};

QTEST_MAIN(TestUnrollRepeats)
#include "tst_unrollrepeats.moc"

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestUnrollRepeats::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
///   clefKeyTs
///   unroll a score with a complex system of
///   clef, key, time signature changes.
//---------------------------------------------------------

void TestUnrollRepeats::clefKeyTs()
{
    MasterScore* score = readScore(UNROLLREPEATS_DATA_DIR + "clef-key-ts-test.mscx");

    MasterScore* unrolled = score->unrollRepeats();

    QVERIFY(saveCompareScore(unrolled, "clef-key-ts-test.mscx", UNROLLREPEATS_DATA_DIR + "clef-key-ts-ref.mscx"));
}

//---------------------------------------------------------
///   pickupMeasure
///   unroll a score with a pickup measure.
///   pickup measure should get merged to a full bar on repeat
//---------------------------------------------------------

void TestUnrollRepeats::pickupMeasure()
{
    MasterScore* score = readScore(UNROLLREPEATS_DATA_DIR + "pickup-measure-test.mscx");

    MasterScore* unrolled = score->unrollRepeats();

    QVERIFY(saveCompareScore(unrolled, "pickup-measure-test.mscx", UNROLLREPEATS_DATA_DIR + "pickup-measure-ref.mscx"));
}
