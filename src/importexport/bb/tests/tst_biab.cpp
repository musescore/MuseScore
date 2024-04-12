/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include <QString>

#include "testing/qtestsuite.h"
#include "testbase.h"
#include "engraving/dom/masterscore.h"

static const QString BIAB_DIR("data/");

using namespace Ms;

//---------------------------------------------------------
//   TestBiab
//---------------------------------------------------------

class TestBiab : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase();
    void biab_data();
    void biab();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestBiab::initTestCase()
{
    setRootDir(QString(iex_bb_tests_DATA_ROOT));
}

//---------------------------------------------------------
//   biab_data
//    every "xxx" test requires a *.SGU file and a *.mscx file:
//          xxx.SGU      is the SGU file
//          xxx-ref.mscx is the corresponding (correct)
//                       mscore 2.0 file
//---------------------------------------------------------

void TestBiab::biab_data()
{
    QTest::addColumn<QString>("file");

    QTest::newRow("chords") << "chords";          // notes.SGU notes-ref.mscx
}

//---------------------------------------------------------
//   biab
//---------------------------------------------------------

void TestBiab::biab()
{
    QFETCH(QString, file);

    QString readFile(BIAB_DIR + file + ".SGU");
    QString writeFile(file + "-test.mscx");
    QString reference(BIAB_DIR + file + "-ref.mscx");

    MasterScore* score = readScore(readFile);
    QVERIFY(score);
    score->doLayout();
    QVERIFY(saveCompareScore(score, writeFile, reference));
}

QTEST_MAIN(TestBiab)
#include "tst_biab.moc"
