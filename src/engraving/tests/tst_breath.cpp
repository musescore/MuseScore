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

#include "libmscore/masterscore.h"
#include "libmscore/undo.h"
#include "libmscore/measure.h"
#include "libmscore/breath.h"
#include "libmscore/sym.h"

static const QString BREATH_DATA_DIR("breath_data/");

using namespace Ms;

//---------------------------------------------------------
//   TestBreath
//---------------------------------------------------------

class TestBreath : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase();
    void breath();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestBreath::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   breath
//---------------------------------------------------------

void TestBreath::breath()
{
    QString readFile(BREATH_DATA_DIR + "breath.mscx");
    QString writeFile1("breath01-test.mscx");
    QString reference1(BREATH_DATA_DIR + "breath01-ref.mscx");
    QString writeFile2("breath02-test.mscx");
    QString reference2(BREATH_DATA_DIR + "breath02-ref.mscx");

    MasterScore* score = readScore(readFile);
    score->doLayout();

    // do
    score->startCmd();
    score->cmdSelectAll();
    for (EngravingItem* e : score->selection().elements()) {
        EditData dd(0);
        Breath* b = new Breath(score->dummy()->segment());
        b->setSymId(SymId::breathMarkComma);
        dd.dropElement = b;
        if (e->acceptDrop(dd)) {
            e->drop(dd);
        }
    }
    score->endCmd();
    QVERIFY(saveCompareScore(score, writeFile1, reference1));

    // undo
    score->undoStack()->undo(0);
    QVERIFY(saveCompareScore(score, writeFile2, reference2));

    delete score;
}

QTEST_MAIN(TestBreath)
#include "tst_breath.moc"
