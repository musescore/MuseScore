/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#include "testutils.h"
#include "dom/score.h"

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
        score->cmdConcertPitchChanged(true, true);
        score->doLayout();
        // switch back
        // TODO: this should be UNDO, but UNDO doesn't work with transpose!
        score->cmdConcertPitchChanged(false, true);
        score->doLayout();
    }
}

QTEST_MAIN(TestConcertPitchBenchmark)
#include "tst_concertpitchbenchmark.moc"
