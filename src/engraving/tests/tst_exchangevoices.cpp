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
#include "libmscore/chord.h"
#include "libmscore/segment.h"

static const QString EXCHVOICES_DATA_DIR("exchangevoices_data/");

using namespace Ms;

//---------------------------------------------------------
//   TestExchangevoices
//---------------------------------------------------------

class TestExchangevoices : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase();

    void slurs();
    void glissandi();
    void undoChangeVoice();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestExchangevoices::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   slurs
//---------------------------------------------------------

void TestExchangevoices::slurs()
{
    QString p1 = EXCHVOICES_DATA_DIR + "exchangevoices-slurs.mscx";
    QVERIFY(score);
    Score* score = readScore(p1);
    score->doLayout();

    // select all
    score->startCmd();
    score->cmdSelectAll();
    score->endCmd();

    // do
    score->startCmd();
    score->cmdExchangeVoice(0, 1);
    score->endCmd();

    // compare
    QVERIFY(saveCompareScore(score, "exchangevoices-slurs.mscx", EXCHVOICES_DATA_DIR + "exchangevoices-slurs-ref.mscx"));
}

//---------------------------------------------------------
//   glissandi
//---------------------------------------------------------

void TestExchangevoices::glissandi()
{
    QString p1 = EXCHVOICES_DATA_DIR + "exchangevoices-gliss.mscx";
    QVERIFY(score);
    Score* score = readScore(p1);
    score->doLayout();

    // select all
    score->startCmd();
    score->cmdSelectAll();
    score->endCmd();

    // do
    score->startCmd();
    score->cmdExchangeVoice(0, 1);
    score->endCmd();

    // compare
    QVERIFY(saveCompareScore(score, "exchangevoices-gliss.mscx", EXCHVOICES_DATA_DIR + "exchangevoices-gliss-ref.mscx"));
}

//---------------------------------------------------------
//   undoChangeVoice
//---------------------------------------------------------

void TestExchangevoices::undoChangeVoice()
{
    QString readFile(EXCHVOICES_DATA_DIR + "undoChangeVoice.mscx");
    QString writeFile1("undoChangeVoice01-test.mscx");
    QString reference1(EXCHVOICES_DATA_DIR + "undoChangeVoice01-ref.mscx");
    QString writeFile2("undoChangeVoice02-test.mscx");
    QString reference2(EXCHVOICES_DATA_DIR + "undoChangeVoice02-ref.mscx");

    MasterScore* score = readScore(readFile);
    score->doLayout();

    // do
    score->deselectAll();
    // select bottom note of all voice 1 chords
    for (Segment* s = score->firstSegment(SegmentType::ChordRest); s; s = s->next1()) {
        ChordRest* cr = static_cast<ChordRest*>(s->element(0));
        if (cr && cr->type() == ElementType::CHORD) {
            Ms::Chord* c = static_cast<Ms::Chord*>(cr);
            score->select(c->downNote(), SelectType::ADD);
        }
    }
    // change voice
    score->startCmd();
    score->changeSelectedNotesVoice(1);
    score->endCmd();
    QVERIFY(saveCompareScore(score, writeFile1, reference1));

    // undo
    score->undoStack()->undo(&ed);
    QVERIFY(saveCompareScore(score, writeFile2, reference2));

    delete score;
}

QTEST_MAIN(TestExchangevoices)
#include "tst_exchangevoices.moc"
