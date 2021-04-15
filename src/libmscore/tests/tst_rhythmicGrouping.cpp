/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/

#include "testing/qtestsuite.h"
#include "testbase.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"

static const QString RHYTHMICGRP_DATA_DIR("rhythmicGrouping_data/");

using namespace Ms;

//---------------------------------------------------------
//   TestRhythmicGrouping
//---------------------------------------------------------

class TestRhythmicGrouping : public QObject, public MTest
{
    Q_OBJECT

    void group(const char* p1, const char* p2, int staves = 0);

private slots:
    void initTestCase();
    void group8ths44() { group("group8ths4-4.mscx",           "group8ths4-4-ref.mscx"); }
    void group8thsSimple() { group("group8thsSimple.mscx",        "group8thsSimple-ref.mscx"); }
    void group8thsCompound() { group("group8thsCompound.mscx",      "group8thsCompound-ref.mscx"); }
    void groupSubbeats() { group("groupSubbeats.mscx",          "groupSubbeats-ref.mscx"); }
    void groupVoices() { group("groupVoices.mscx",            "groupVoices-ref.mscx"); }
    void groupConflicts() { group("groupConflicts.mscx",         "groupConflicts-ref.mscx", 1); }            // only group 1st staff
    void groupArticulationsTies() { group("groupArticulationsTies.mscx", "groupArticulationsTies-ref.mscx"); }    // test for articulations and forward/backward ties
    void groupShortenNotes() { group("groupShortenNotes.mscx",      "groupShortenNotes-ref.mscx"); }         // test for regrouping rhythms when notes should be shortened
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestRhythmicGrouping::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   group
//---------------------------------------------------------

void TestRhythmicGrouping::group(const char* p1, const char* p2, int staves)
{
    MasterScore* score = readScore(RHYTHMICGRP_DATA_DIR + p1);

    if (!staves) {
        score->cmdSelectAll();
        score->cmdResetNoteAndRestGroupings();
    } else {
        Q_ASSERT(staves < score->nstaves());
        score->startCmd();
        for (int track = 0; track < staves * VOICES; track++) {
            score->regroupNotesAndRests(score->firstSegment(SegmentType::All)->tick(),
                                        score->lastSegment()->tick(), track);
        }
        score->endCmd();
    }

    score->doLayout();
    QVERIFY(saveCompareScore(score, p1, RHYTHMICGRP_DATA_DIR + p2));
    delete score;
}

QTEST_MAIN(TestRhythmicGrouping)
#include "tst_rhythmicGrouping.moc"
