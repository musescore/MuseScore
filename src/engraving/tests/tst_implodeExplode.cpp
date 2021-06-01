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
#include "libmscore/measure.h"
#include "libmscore/chord.h"

static const QString IMPLODEEXP_DATA_DIR("implode_explode_data/");

using namespace Ms;

//---------------------------------------------------------
//   TestImplodeExplode
//---------------------------------------------------------

class TestImplodeExplode : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase();
    void undoExplode();
    void undoExplodeVoices();
    void undoExplode1();
    void undoImplode();
    void undoImplodeVoice();
    void implode1();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestImplodeExplode::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   undoExplode
//---------------------------------------------------------

void TestImplodeExplode::undoExplode()
{
    QString readFile(IMPLODEEXP_DATA_DIR + "undoExplode.mscx");
    QString writeFile1("undoExplode01-test.mscx");
    QString reference1(IMPLODEEXP_DATA_DIR + "undoExplode01-ref.mscx");
    QString writeFile2("undoExplode02-test.mscx");
    QString reference2(IMPLODEEXP_DATA_DIR + "undoExplode02-ref.mscx");

    MasterScore* score = readScore(readFile);
    score->doLayout();

    // select all
    score->startCmd();
    score->cmdSelectAll();
    score->endCmd();

    // do
    score->startCmd();
    score->cmdExplode();
    score->endCmd();
    QVERIFY(saveCompareScore(score, writeFile1, reference1));

    // undo
    score->undoStack()->undo(&ed);
    QVERIFY(saveCompareScore(score, writeFile2, reference2));

    delete score;
}

//---------------------------------------------------------
//   undoExplodeVoices
//---------------------------------------------------------

void TestImplodeExplode::undoExplodeVoices()
{
    QString readFile(IMPLODEEXP_DATA_DIR + "undoExplode.mscx");
    QString writeFile1("undoExplode01-test.mscx");
    QString reference1(IMPLODEEXP_DATA_DIR + "undoExplode01-ref.mscx");
    QString writeFile2("undoExplode02-test.mscx");
    QString reference2(IMPLODEEXP_DATA_DIR + "undoExplode02-ref.mscx");
}

//---------------------------------------------------------
//   undoImplode
//---------------------------------------------------------

void TestImplodeExplode::undoImplode()
{
    QString readFile(IMPLODEEXP_DATA_DIR + "undoImplode.mscx");
    QString writeFile1("undoImplode01-test.mscx");
    QString reference1(IMPLODEEXP_DATA_DIR + "undoImplode01-ref.mscx");
    QString writeFile2("undoImplode02-test.mscx");
    QString reference2(IMPLODEEXP_DATA_DIR + "undoImplode02-ref.mscx");

    MasterScore* score = readScore(readFile);
    score->doLayout();

    // select all
    score->startCmd();
    score->cmdSelectAll();
    score->endCmd();

    // do
    score->startCmd();
    score->cmdImplode();
    score->endCmd();
    QVERIFY(saveCompareScore(score, writeFile1, reference1));

    // undo
    score->undoStack()->undo(&ed);
    QVERIFY(saveCompareScore(score, writeFile2, reference2));

    delete score;
}

//---------------------------------------------------------
//   undoExplode1
//---------------------------------------------------------

void TestImplodeExplode::undoExplode1()
{
    QString readFile(IMPLODEEXP_DATA_DIR + "explode1.mscx");
    QString writeFile1("explode1-test.mscx");
    QString reference1(IMPLODEEXP_DATA_DIR + "explode1-ref.mscx");
    QString writeFile2("explode1-test2.mscx");
    QString reference2(IMPLODEEXP_DATA_DIR + "explode1-ref2.mscx");
}

//---------------------------------------------------------
//   undoImplodeVoice
//---------------------------------------------------------

void TestImplodeExplode::undoImplodeVoice()
{
    QString readFile(IMPLODEEXP_DATA_DIR + "undoImplodeVoice.mscx");
    QString writeFile1("undoImplodeVoice01-test.mscx");
    QString reference1(IMPLODEEXP_DATA_DIR + "undoImplodeVoice01-ref.mscx");
    QString writeFile2("undoImplodeVoice02-test.mscx");
    QString reference2(IMPLODEEXP_DATA_DIR + "undoImplodeVoice02-ref.mscx");

    MasterScore* score = readScore(readFile);
    score->doLayout();

    // select all
    score->startCmd();
    score->cmdSelectAll();
    score->endCmd();

    // do
    score->startCmd();
    score->cmdImplode();
    score->endCmd();
    QVERIFY(saveCompareScore(score, writeFile1, reference1));

    // undo
    score->undoStack()->undo(&ed);
    QVERIFY(saveCompareScore(score, writeFile2, reference2));

    delete score;
}

//---------------------------------------------------------
//   implode1
//---------------------------------------------------------

void TestImplodeExplode::implode1()
{
    QString readFile(IMPLODEEXP_DATA_DIR + "implode1.mscx");
    QString writeFile1("implode1-test1.mscx");
    QString writeFile2("implode1-test2.mscx");
    QString reference(IMPLODEEXP_DATA_DIR + "implode1-ref.mscx");

    MasterScore* score = readScore(readFile);
    score->doLayout();

    // select all
    score->startCmd();
    score->cmdSelectAll();
    score->endCmd();

    // do
    score->startCmd();
    score->cmdImplode();
    score->endCmd();
    QVERIFY(saveCompareScore(score, writeFile1, reference));

    // undo
    score->undoStack()->undo(&ed);
    QVERIFY(saveCompareScore(score, writeFile2, readFile));

    delete score;
}

QTEST_MAIN(TestImplodeExplode)

#include "tst_implodeExplode.moc"
