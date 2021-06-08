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
#include "libmscore/undo.h"

static const QString TRANSPOSE_DATA_DIR("transpose_data/");

using namespace Ms;

//---------------------------------------------------------
//   TestTranspose
//---------------------------------------------------------

class TestTranspose : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase();
    void undoTranspose();
    void undoDiatonicTranspose();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestTranspose::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   undoTranspose
//---------------------------------------------------------

void TestTranspose::undoTranspose()
{
    QString readFile(TRANSPOSE_DATA_DIR + "undoTranspose.mscx");
    QString writeFile1("undoTranspose01-test.mscx");
    QString reference1(TRANSPOSE_DATA_DIR + "undoTranspose01-ref.mscx");
    QString writeFile2("undoTranspose02-test.mscx");
    QString reference2(TRANSPOSE_DATA_DIR + "undoTranspose02-ref.mscx");

    MasterScore* score = readScore(readFile);

    // select all
    score->cmdSelectAll();

    // transpose major second up
    score->startCmd();
    score->transpose(TransposeMode::BY_INTERVAL, TransposeDirection::UP, Key::C, 4,
                     true, true, true);
    score->endCmd();
    QVERIFY(saveCompareScore(score, writeFile1, reference1));

    // undo
    score->undoStack()->undo(&ed);
    QVERIFY(saveCompareScore(score, writeFile2, reference2));

    delete score;
}

//---------------------------------------------------------
//   undoDiatonicTranspose
//---------------------------------------------------------

void TestTranspose::undoDiatonicTranspose()
{
    QString readFile(TRANSPOSE_DATA_DIR + "undoDiatonicTranspose.mscx");
    QString writeFile1("undoDiatonicTranspose01-test.mscx");
    QString reference1(TRANSPOSE_DATA_DIR + "undoDiatonicTranspose01-ref.mscx");
    QString writeFile2("undoDiatonicTranspose02-test.mscx");
    QString reference2(TRANSPOSE_DATA_DIR + "undoDiatonicTranspose02-ref.mscx");

    MasterScore* score = readScore(readFile);
    score->doLayout();

    // select all
    score->cmdSelectAll();

    // transpose diatonic fourth down
    score->startCmd();
    score->transpose(TransposeMode::DIATONICALLY, TransposeDirection::DOWN, Key::C, 3,
                     true, false, false);
    score->endCmd();
    QVERIFY(saveCompareScore(score, writeFile1, reference1));

    // undo
    score->undoStack()->undo(&ed);
    QVERIFY(saveCompareScore(score, writeFile2, reference2));

    delete score;
}

QTEST_MAIN(TestTranspose)
#include "tst_transpose.moc"
