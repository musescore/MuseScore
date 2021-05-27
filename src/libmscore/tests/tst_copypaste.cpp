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

#include <QClipboard>

#include "testbase.h"
#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/chordrest.h"
#include "libmscore/chord.h"
#include "libmscore/xml.h"
#include "libmscore/durationtype.h"

static const QString COPYPASTE_DATA_DIR("copypaste_data/");

using namespace Ms;

//---------------------------------------------------------
//   TestCopyPaste
//---------------------------------------------------------

class TestCopyPaste : public QObject, public MTest
{
    Q_OBJECT

    void copypaste(const char*);
    void copypastestaff(const char*);
    void copypastevoice(const char*, int);
    void copypastetuplet(const char*);
    void copypastetremolo();

private slots:
    void initTestCase();
    void copypaste01() { copypaste("01"); }         // start slur
    void copypaste02() { copypaste("02"); }         // end slur
    void copypaste03() { copypaste("03"); }         // slur
    void copypaste04() { copypaste("04"); }         // start tie
    void copypaste05() { copypaste("05"); }         // end tie
    void copypaste06() { copypaste("06"); }         // tie
//      void copypaste07() { copypaste("07"); }       // start ottava
    void copypaste08() { copypaste("08"); }         // end ottava
    void copypaste09() { copypaste("09"); }         // ottava
    void copypaste10() { copypaste("10"); }         // two slurs
    void copypaste11() { copypaste("11"); }         // grace notes
    void copypaste12() { copypaste("12"); }         // voices
    void copyPaste2Voice();                         // voices-partial
    //void copyPaste2Voice2() { copypastevoice("14", 0); }
    //void copyPaste2Voice3() { copypastevoice("15", 1); }
    //void copyPaste2Voice4() { copypastevoice("16", 1); }   // shorten last cr
    void copyPaste2Voice5();                              // cut and move
    void copypaste2Voice6();
    void copyPasteOnlySecondVoice();
    void copypaste19() { copypaste("19"); }         // chord symbols
    //void copyPasteShortTremolo() { copypastevoice("21", 1); }   // remove tremolo on shorten note #30411
    void copypaste22() { copypaste("22"); }         // cross-staff slur
    void copypaste23() { copypaste("23"); }         // full measure tuplet 10/8
    void copypaste24() { copypaste("24"); }         // more complex non reduced tuplet
    void copypaste25() { copypaste("25"); }         // copy full measure rest
    void copypaste26() { copypaste("26"); }         // Copy chords (#298541)

    void copypastestaff50() { copypastestaff("50"); }         // staff & slurs

    void copyPastePartial();

    void copyPasteTuplet01() { copypastetuplet("01"); }
    void copyPasteTuplet02() { copypastetuplet("02"); }

    //void copyPasteTremolo01() { copypastetremolo(); }
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestCopyPaste::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   copypaste
//    copy measure 2, paste into measure 4
//---------------------------------------------------------

void TestCopyPaste::copypaste(const char* idx)
{
    MasterScore* score = readScore(COPYPASTE_DATA_DIR + QString("copypaste%1.mscx").arg(idx));
    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();      // src
    Measure* m3 = m2->nextMeasure();
    Measure* m4 = m3->nextMeasure();      // dst

    QVERIFY(m1 != 0);
    QVERIFY(m2 != 0);
    QVERIFY(m3 != 0);
    QVERIFY(m4 != 0);

    score->select(m2);
    if (score->nstaves() > 1) {
        score->select(m2, SelectType::RANGE, score->nstaves() - 1);
    }
    QVERIFY(score->selection().canCopy());
    QString mimeType = score->selection().mimeType();
    QVERIFY(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    QByteArray ba = score->selection().mimeData();
    mimeData->setData(mimeType, ba);
    QApplication::clipboard()->setMimeData(mimeData);
    QVERIFY(m4->first()->element(0) != 0);
    score->select(m4->first()->element(0));

    score->startCmd();
    score->cmdPaste(mimeData, 0);
    score->endCmd();

    QVERIFY(saveCompareScore(score, QString("copypaste%1.mscx").arg(idx),
                             COPYPASTE_DATA_DIR + QString("copypaste%1-ref.mscx").arg(idx)));
    delete score;
}

//---------------------------------------------------------
//   copypaste
//    copy measure 2 from first staff, paste into staff 2
//---------------------------------------------------------

void TestCopyPaste::copypastestaff(const char* idx)
{
    MasterScore* score = readScore(COPYPASTE_DATA_DIR + QString("copypaste%1.mscx").arg(idx));
    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();      // src

    QVERIFY(m1 != 0);
    QVERIFY(m2 != 0);

    score->select(m2, SelectType::RANGE, 0);
    QVERIFY(score->selection().canCopy());
    QString mimeType = score->selection().mimeType();
    QVERIFY(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData());
    QApplication::clipboard()->setMimeData(mimeData);

    score->deselectAll();

    score->select(m2, SelectType::RANGE, 1);

    score->startCmd();
    score->cmdPaste(mimeData, 0);
    score->endCmd();

    QVERIFY(saveCompareScore(score, QString("copypaste%1.mscx").arg(idx),
                             COPYPASTE_DATA_DIR + QString("copypaste%1-ref.mscx").arg(idx)));
    delete score;
}

void TestCopyPaste::copyPastePartial()
{
    MasterScore* score = readScore(COPYPASTE_DATA_DIR + QString("copypaste_partial_01.mscx"));

    Measure* m1 = score->firstMeasure();

    Segment* s = m1->first(SegmentType::ChordRest);
    s = s->next(SegmentType::ChordRest);
    score->select(s->element(0));
    s = s->next(SegmentType::ChordRest);
    score->select(s->element(4), SelectType::RANGE);

    QVERIFY(score->selection().canCopy());
    QString mimeType = score->selection().mimeType();
    QVERIFY(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData());
    QApplication::clipboard()->setMimeData(mimeData);

    score->select(m1->first(SegmentType::ChordRest)->element(0));

    score->startCmd();
    score->cmdPaste(mimeData, 0);
    score->endCmd();

    QVERIFY(saveCompareScore(score, QString("copypaste_partial_01.mscx"),
                             COPYPASTE_DATA_DIR + QString("copypaste_partial_01-ref.mscx")));
    delete score;
}

void TestCopyPaste::copyPaste2Voice()
{
    MasterScore* score = readScore(COPYPASTE_DATA_DIR + QString("copypaste13.mscx"));
    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();

    QVERIFY(m1 != 0);
    QVERIFY(m2 != 0);

    // select 2 chord rests at the start of the first measure
    Segment* s = m1->first(SegmentType::ChordRest);
    score->select(static_cast<Ms::Chord*>(s->element(0))->notes().at(0));
    s = s->next(SegmentType::ChordRest);
    score->select(s->element(0), SelectType::RANGE);

    QVERIFY(score->selection().canCopy());
    QString mimeType = score->selection().mimeType();
    QVERIFY(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData());
    QApplication::clipboard()->setMimeData(mimeData);

    // paste into the second CR of second measure
    Segment* secondCRSeg = m2->first()->next1(SegmentType::ChordRest);
    score->select(secondCRSeg->element(0));

    score->startCmd();
    score->cmdPaste(mimeData, 0);
    score->endCmd();

    QVERIFY(saveCompareScore(score, QString("copypaste13.mscx"),
                             COPYPASTE_DATA_DIR + QString("copypaste13-ref.mscx")));
    delete score;
}

//---------------------------------------------------------
//   copypaste
//    copy measure 2 from first staff, paste into staff 2
//---------------------------------------------------------

void TestCopyPaste::copypastevoice(const char* idx, int voice)
{
    MasterScore* score = readScore(COPYPASTE_DATA_DIR + QString("copypaste%1.mscx").arg(idx));
    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();

    QVERIFY(m1 != 0);
    QVERIFY(m2 != 0);

    // create a range selection on 2 and 3 beat of first measure
    SegmentType segTypeCR = SegmentType::ChordRest;
    Segment* s = m1->first(segTypeCR)->next1(segTypeCR);
    score->select(static_cast<Ms::Chord*>(s->element(voice))->notes().at(0));
    s = s->next(SegmentType::ChordRest);
    score->select(s->element(voice), SelectType::RANGE);

    QVERIFY(score->selection().canCopy());
    QString mimeType = score->selection().mimeType();
    QVERIFY(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData());
    QApplication::clipboard()->setMimeData(mimeData);

    //paste to second measure
    score->select(m2->first()->element(0));

    score->startCmd();
    score->cmdPaste(mimeData, 0);
    score->endCmd();

    QVERIFY(saveCompareScore(score, QString("copypaste%1.mscx").arg(idx),
                             COPYPASTE_DATA_DIR + QString("copypaste%1-ref.mscx").arg(idx)));
    delete score;
}

void TestCopyPaste::copyPaste2Voice5()
{
    MasterScore* score = readScore(COPYPASTE_DATA_DIR + QString("copypaste17.mscx"));
    Measure* m1 = score->firstMeasure();

    QVERIFY(m1 != 0);

    // create a range selection from 2 eighth note to the end of first measure
    SegmentType segTypeCR = SegmentType::ChordRest;
    Segment* s = m1->first(segTypeCR)->next1(segTypeCR);
    score->select(static_cast<Ms::Chord*>(s->element(0))->notes().at(0));

    s = m1->last()->prev(SegmentType::ChordRest);
    score->select(s->element(0), SelectType::RANGE);

    QVERIFY(score->selection().canCopy());
    QString mimeType = score->selection().mimeType();
    QVERIFY(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData());
    QApplication::clipboard()->setMimeData(mimeData);

    score->cmdDeleteSelection();   //cut

    //paste to quarter rest
    Element* dest = m1->first()->next(segTypeCR)->next(segTypeCR)->next(segTypeCR)->element(0);
    QVERIFY(dest->isRest() && static_cast<ChordRest*>(dest)->durationType() == TDuration::DurationType::V_QUARTER);
    score->select(dest);

    score->startCmd();
    score->cmdPaste(mimeData, 0);
    score->endCmd();

    QVERIFY(saveCompareScore(score, QString("copypaste17.mscx"),
                             COPYPASTE_DATA_DIR + QString("copypaste17-ref.mscx")));
    delete score;
}

void TestCopyPaste::copyPasteOnlySecondVoice()
{
    MasterScore* score = readScore(COPYPASTE_DATA_DIR + QString("copypaste18.mscx"));
    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();

    QVERIFY(m1 != 0);
    QVERIFY(m2 != 0);

    score->select(m1, SelectType::RANGE, 0);

    score->selectionFilter().setFiltered(SelectionFilterType::FIRST_VOICE, false);

    QVERIFY(score->selection().canCopy());
    QString mimeType = score->selection().mimeType();
    QVERIFY(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData());
    QApplication::clipboard()->setMimeData(mimeData);

    //paste to second measure
    score->deselectAll();
    score->selectionFilter().setFiltered(SelectionFilterType::FIRST_VOICE, true);
    score->select(m2, SelectType::RANGE);

    score->startCmd();
    score->cmdPaste(mimeData, 0);
    score->endCmd();

    QVERIFY(saveCompareScore(score, QString("copypaste18.mscx"),
                             COPYPASTE_DATA_DIR + QString("copypaste18-ref.mscx")));
    delete score;
}

void TestCopyPaste::copypaste2Voice6()
{
    MasterScore* score = readScore(COPYPASTE_DATA_DIR + QString("copypaste20.mscx"));
    Measure* m1 = score->firstMeasure();

    QVERIFY(m1 != 0);

    // create a range selection from 2nd eighth note to the end of first measure
    SegmentType segTypeCR = SegmentType::ChordRest;
    Segment* s = m1->first(segTypeCR)->next1(segTypeCR);
    score->select(static_cast<Ms::Chord*>(s->element(0))->notes().at(0));

    s = m1->last()->prev(SegmentType::ChordRest);
    score->select(s->element(1), SelectType::RANGE);

    QVERIFY(score->selection().canCopy());
    QString mimeType = score->selection().mimeType();
    QVERIFY(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData());
    QApplication::clipboard()->setMimeData(mimeData);

    //paste to 16th rest
    Element* dest
        = m1->first(segTypeCR)->next(segTypeCR)->next(segTypeCR)->next(segTypeCR)->next(segTypeCR)->element(0);
    qDebug() << int(dest->type());
    QVERIFY(dest->isRest()
            && static_cast<ChordRest*>(dest)->durationType() == TDuration::DurationType::V_16TH);
    score->select(dest);

    score->startCmd();
    score->cmdPaste(mimeData, 0);
    score->endCmd();

    QVERIFY(saveCompareScore(score, QString("copypaste20.mscx"),
                             COPYPASTE_DATA_DIR + QString("copypaste20-ref.mscx")));
    delete score;
}

//---------------------------------------------------------
//   copypastetuplet
//---------------------------------------------------------

void TestCopyPaste::copypastetuplet(const char* idx)
{
    MasterScore* score = readScore(COPYPASTE_DATA_DIR + QString("copypaste_tuplet_%1.mscx").arg(idx));

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();

    Segment* s = m1->first(SegmentType::ChordRest);
    score->select(toChord(s->element(0))->notes().at(0));
    s = s->next(SegmentType::ChordRest);
    score->select(s->element(0), SelectType::RANGE);

    QVERIFY(score->selection().canCopy());
    QString mimeType = score->selection().mimeType();
    QVERIFY(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData());
    QApplication::clipboard()->setMimeData(mimeData);

    Element* dest = m2->first(SegmentType::ChordRest)->element(0);
    score->select(dest);
    score->startCmd();
    score->cmdPaste(mimeData, 0);
    score->endCmd();

    QVERIFY(saveCompareScore(score, QString("copypaste_tuplet_%1.mscx").arg(idx),
                             COPYPASTE_DATA_DIR + QString("copypaste_tuplet_%1-ref.mscx").arg(idx)));
    delete score;
}

//---------------------------------------------------------
//   copypastetremolo
//   copy-paste of tremolo between two notes
//---------------------------------------------------------

void TestCopyPaste::copypastetremolo()
{
    MasterScore* score = readScore(COPYPASTE_DATA_DIR + QString("copypaste_tremolo.mscx"));
    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();
    Measure* m3 = m2->nextMeasure();

    QVERIFY(m1 != 0);
    QVERIFY(m2 != 0);
    QVERIFY(m3 != 0);

    // create a range selection on 2nd to 3rd beat (voice 1) of first measure
    SegmentType segTypeCR = SegmentType::ChordRest;
    Segment* s = m1->first(segTypeCR)->next1(segTypeCR);
    score->select(static_cast<Ms::Chord*>(s->element(1))->notes().at(0));
    s = s->next1(segTypeCR);
    score->select(s->element(1), SelectType::RANGE);

    QVERIFY(score->selection().canCopy());
    QString mimeType = score->selection().mimeType();
    QVERIFY(!mimeType.isEmpty());
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(mimeType, score->selection().mimeData());
    QApplication::clipboard()->setMimeData(mimeData);

    //paste to second measure
    score->select(m2->first()->element(0));

    score->startCmd();
    score->cmdPaste(mimeData, 0);
    score->endCmd();

    // create a range selection on 2nd to 4th beat (voice 0) of first measure
    s = m1->first(segTypeCR)->next1(segTypeCR);
    score->select(static_cast<Ms::Chord*>(s->element(0))->notes().at(0));
    s = s->next1(segTypeCR)->next1(segTypeCR);
    score->select(s->element(0), SelectType::RANGE);

    QVERIFY(score->selection().canCopy());
    mimeType = score->selection().mimeType();
    QVERIFY(!mimeType.isEmpty());
    mimeData->setData(mimeType, score->selection().mimeData());
    QApplication::clipboard()->setMimeData(mimeData);

    //paste to third measure
    score->select(m3->first()->element(0));

    score->startCmd();
    score->cmdPaste(mimeData, 0);
    score->endCmd();

    QVERIFY(saveCompareScore(score, QString("copypaste_tremolo.mscx"),
                             COPYPASTE_DATA_DIR + QString("copypaste_tremolo-ref.mscx")));
    delete score;
}

QTEST_MAIN(TestCopyPaste)
#include "tst_copypaste.moc"
