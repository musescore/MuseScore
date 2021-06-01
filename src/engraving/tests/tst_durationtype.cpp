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

#include "libmscore/mscore.h"
#include "libmscore/score.h"
#include "libmscore/note.h"
#include "libmscore/accidental.h"
#include "libmscore/chord.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/tremolo.h"
#include "libmscore/articulation.h"
#include "libmscore/sym.h"
#include "libmscore/key.h"
#include "libmscore/pitchspelling.h"

static const QString DURATIONTYPE_DATA_DIR("durationtype_data/");

using namespace Ms;

//---------------------------------------------------------
//   TestDurationType
//---------------------------------------------------------

class TestDurationType : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase();
    void halfDuration();
    void doubleDuration();
    void decDurationDotted();
    void incDurationDotted();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestDurationType::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   halfDuration
//    Simple tests for command "half-duration" (default shortcut "Q").
//    starts with Whole note and repeatedly applies cmdHalfDuration()
//---------------------------------------------------------

void TestDurationType::halfDuration()
{
    MasterScore* score = readScore(DURATIONTYPE_DATA_DIR + "empty.mscx");
    score->inputState().setTrack(0);
    score->inputState().setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    score->inputState().setDuration(TDuration::DurationType::V_WHOLE);
    score->inputState().setNoteEntryMode(true);

    score->startCmd();
    score->cmdAddPitch(42, false, false);
    Ms::Chord* c = score->firstMeasure()->findChord(Fraction(0, 1), 0);
    QVERIFY(c->ticks() == Fraction(1, 1));
    score->endCmd();

    // repeatedly half-duration from V_WHOLE to V_128
    for (int i = 128; i > 1; i /= 2) {
        score->startCmd();
        score->cmdHalfDuration();
        score->endCmd();
        Ms::Chord* c = score->firstMeasure()->findChord(Fraction(0, 1), 0);
        QVERIFY(c->ticks() == Fraction(i / 2, 128));
    }
}

//---------------------------------------------------------
//   halfDuration
//    Simple tests for command "double-duration" (default shortcut "W").
//    Starts with 128th note and repeatedly applies cmdDoubleDuration() up to Whole note.
//---------------------------------------------------------

void TestDurationType::doubleDuration()
{
    MasterScore* score = readScore(DURATIONTYPE_DATA_DIR + "empty.mscx");
    score->inputState().setTrack(0);
    score->inputState().setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    score->inputState().setDuration(TDuration::DurationType::V_128TH);
    score->inputState().setNoteEntryMode(true);

    score->startCmd();
    score->cmdAddPitch(42, false, false);
    QVERIFY(score->firstMeasure()->findChord(Fraction(0, 1), 0)->ticks() == Fraction(1, 128));

    // repeatedly double-duration from V_128 to V_WHOLE
    for (int i = 1; i < 128; i *= 2) {
        score->cmdDoubleDuration();
        Ms::Chord* c = score->firstMeasure()->findChord(Fraction(0, 1), 0);
        QVERIFY(c->ticks() == Fraction(2 * i, 128));
    }
    score->endCmd();
}

//---------------------------------------------------------
//   decDurationDotted
//    Simple tests for command "dec-duration-dotted" (default shortcut "Shift+Q").
//    Starts with Whole note and repeatedly applies cmdDecDurationDotted() down to 128th note.
//---------------------------------------------------------

void TestDurationType::decDurationDotted()
{
    MasterScore* score = readScore(DURATIONTYPE_DATA_DIR + "empty.mscx");
    score->inputState().setTrack(0);
    score->inputState().setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    score->inputState().setDuration(TDuration::DurationType::V_WHOLE);
    score->inputState().setNoteEntryMode(true);

    score->startCmd();
    score->cmdAddPitch(42, false, false);
    Ms::Chord* c = score->firstMeasure()->findChord(Fraction(0, 1), 0);
    QVERIFY(c->ticks() == Fraction(1, 1));

    // repeatedly dec-duration-dotted from V_WHOLE to V_128
    for (int i = 128; i > 1; i /= 2) {
        score->cmdDecDurationDotted();
        Ms::Chord* c = score->firstMeasure()->findChord(Fraction(0, 1), 0);
        QVERIFY(c->ticks() == Fraction(i + i / 2, 256));

        score->cmdDecDurationDotted();
        c = score->firstMeasure()->findChord(Fraction(0, 1), 0);
        QVERIFY(c->ticks() == Fraction(i / 2, 128));
    }
    score->endCmd();
}

//---------------------------------------------------------
//   incDurationDotted
//    Simple tests for command "inc-duration-dotted" (default shortcut "Shift+W").
//    Starts with 128th note and repeatedly applies cmdIncDurationDotted() up to Whole note.
//---------------------------------------------------------

void TestDurationType::incDurationDotted()
{
    MasterScore* score = readScore(DURATIONTYPE_DATA_DIR + "empty.mscx");
    score->inputState().setTrack(0);
    score->inputState().setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    score->inputState().setDuration(TDuration::DurationType::V_128TH);
    score->inputState().setNoteEntryMode(true);

    score->startCmd();
    score->cmdAddPitch(42, false, false);
    QVERIFY(score->firstMeasure()->findChord(Fraction(0, 1), 0)->ticks() == Fraction(1, 128));

    // repeatedly inc-duration-dotted from V_128 to V_WHOLE
    for (int i = 1; i < 128; i *= 2) {
        score->cmdIncDurationDotted();
        QVERIFY(score->firstMeasure()->findChord(Fraction(0, 1), 0)->ticks() == Fraction(3 * i, 256));

        score->cmdIncDurationDotted();
        QVERIFY(score->firstMeasure()->findChord(Fraction(0, 1), 0)->ticks() == Fraction(i, 64));
    }
    score->endCmd();
}

QTEST_MAIN(TestDurationType)

#include "tst_durationtype.moc"
