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

#include "engraving/style/style.h"

#include "libmscore/chord.h"
#include "libmscore/measure.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"

static const QString EARLYMUSIC_DATA_DIR("earlymusic_data/");

using namespace Ms;

//---------------------------------------------------------
//   TestEarlymusic
//---------------------------------------------------------

class TestEarlymusic : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase();
    void earlymusic01();              // setting cross-measure value flag and undoing
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestEarlymusic::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
///  earlymusic01
///   Setting cross-measure value flag and undoing.
//---------------------------------------------------------

void TestEarlymusic::earlymusic01()
{
    MasterScore* score = readScore(EARLYMUSIC_DATA_DIR + "mensurstrich01.mscx");
    QVERIFY(score);
    score->doLayout();

    // go to first chord and verify crossMeasure values
    Measure* msr   = score->firstMeasure();
    QVERIFY(msr);
    Segment* seg   = msr->findSegment(SegmentType::ChordRest, Fraction(0, 1));
    QVERIFY(seg);
    Ms::Chord* chord = static_cast<Ms::Chord*>(seg->element(0));
    QVERIFY(chord && chord->type() == ElementType::CHORD);
    QVERIFY(chord->crossMeasure() == CrossMeasure::UNKNOWN);
    TDuration cmDur   = chord->crossMeasureDurationType();
//      QVERIFY(cmDur.type() == TDuration::DurationType::V_INVALID);    // irrelevant if crossMeasure() == UNKNOWN
    TDuration acDur   = chord->actualDurationType();
    QVERIFY(acDur.type() == TDuration::DurationType::V_BREVE);
    TDuration dur     = chord->durationType();
    QVERIFY(dur.type() == TDuration::DurationType::V_BREVE);

    // set crossMeasureValue flag ON: score should not change
    MStyle newStyle = score->style();
    newStyle.set(Sid::crossMeasureValues, true);
    score->startCmd();
    score->deselectAll();
    score->undo(new ChangeStyle(score, newStyle));
    score->update();
    score->endCmd();
    // verify crossMeasureDurationType did change
    QVERIFY(chord->crossMeasure() == CrossMeasure::FIRST);
    cmDur = chord->crossMeasureDurationType();
    QVERIFY(cmDur.type() == TDuration::DurationType::V_LONG);
    acDur = chord->actualDurationType();
    QVERIFY(acDur.type() == TDuration::DurationType::V_BREVE);
    dur   = chord->durationType();
    QVERIFY(dur.type() == TDuration::DurationType::V_LONG);
    // verify score file did not change
    QVERIFY(saveCompareScore(score, "mensurstrich01.mscx", EARLYMUSIC_DATA_DIR + "mensurstrich01-ref.mscx"));

    // UNDO AND VERIFY
    score->undoStack()->undo(&ed);
    score->doLayout();
    QVERIFY(chord->crossMeasure() == CrossMeasure::UNKNOWN);
    cmDur = chord->crossMeasureDurationType();
//      QVERIFY(cmDur.type() == TDuration::DurationType::V_LONG);    // irrelevant if crossMeasure() == UNKNOWN
    acDur = chord->actualDurationType();
    QVERIFY(acDur.type() == TDuration::DurationType::V_BREVE);
    dur   = chord->durationType();
    QVERIFY(dur.type() == TDuration::DurationType::V_BREVE);
    QVERIFY(saveCompareScore(score, "mensurstrich01.mscx", EARLYMUSIC_DATA_DIR + "mensurstrich01.mscx"));
    delete score;
}

QTEST_MAIN(TestEarlymusic)
#include "tst_earlymusic.moc"
