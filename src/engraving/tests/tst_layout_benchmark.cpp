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

#include "engraving/compat/mscxcompat.h"
#include "engraving/compat/scoreaccess.h"

using namespace mu::engraving;

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
    score = mu::engraving::compat::ScoreAccess::createMasterScoreWithBaseStyle();
    score->setName(path);
    MScore::testMode = true;
    QBENCHMARK {
        compat::loadMsczOrMscx(score, path);
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
        score->setLayout(Fraction(1, 4), -1);
        score->endCmd();
    }
}

QTEST_MAIN(TestLayoutBenchmark)
#include "tst_layout_benchmark.moc"
