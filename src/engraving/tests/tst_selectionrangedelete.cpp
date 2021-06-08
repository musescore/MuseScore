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
#include "libmscore/measure.h"
#include "libmscore/undo.h"

static const QString SELRANGEDELETE_DATA_DIR("selectionrangedelete_data/");

using namespace Ms;

//---------------------------------------------------------
//   TestSelectionRangeDelete
//---------------------------------------------------------

class TestSelectionRangeDelete : public QObject, public MTest
{
    Q_OBJECT
    void verifyDelete(MasterScore* score, size_t spanners);
    void verifyNoDelete(MasterScore* score, size_t spanners);
    void deleteVoice(int voice, QString idx);

private slots:
    void initTestCase();
    void deleteSegmentWithSlur();
    void deleteSegmentWithSpanner();
    void deleteVoice1() { deleteVoice(0, "03"); }
    void deleteVoice2() { deleteVoice(1, "04"); }
    void deleteSkipAnnotations();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestSelectionRangeDelete::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   verifyDelete
//---------------------------------------------------------

void TestSelectionRangeDelete::verifyDelete(MasterScore* score, size_t spanners)
{
    score->startCmd();
    score->cmdDeleteSelection();
    score->endCmd();

    QVERIFY(score->spanner().size() == spanners - 1);
    score->undoRedo(true, 0);
    QVERIFY(score->spanner().size() == spanners);
}

//---------------------------------------------------------
//   verifyNoDelete
//---------------------------------------------------------

void TestSelectionRangeDelete::verifyNoDelete(MasterScore* score, size_t spanners)
{
    score->startCmd();
    score->cmdDeleteSelection();
    score->endCmd();

    QVERIFY(score->spanner().size() == spanners);
    score->undoRedo(true, 0);
    QVERIFY(score->spanner().size() == spanners);
}

//---------------------------------------------------------
//   chordRestAtBeat
//---------------------------------------------------------

Element* chordRestAtBeat(Score* score, int beat, int half = 0)
{
    qDebug("Chordrest at beat %i,%i", beat, half);
    int division = MScore::division;
    int tick = beat * division + half * division / 2;
    return score->tick2segment(Fraction::fromTicks(tick), false, SegmentType::ChordRest, false)->element(0);
}

//---------------------------------------------------------
//   deleteSegmentWithSlur
//---------------------------------------------------------

void TestSelectionRangeDelete::deleteSegmentWithSlur()
{
    /*
     *  Score looks like this:
     *  ss - start slur, es - end slur, q - quarter note, e - eighth note
     *
     *  ss es ss   es
     *  q  q  q  e e
     */
    MasterScore* score = readScore(SELRANGEDELETE_DATA_DIR + "selectionrangedelete01.mscx");

    score->doLayout();
    QVERIFY(score);
    size_t spanners = score->spanner().size();

    score->select(chordRestAtBeat(score, 0), SelectType::RANGE);
    verifyDelete(score, spanners);
    score->deselectAll();

    score->select(chordRestAtBeat(score, 1), SelectType::RANGE);
    verifyDelete(score, spanners);
    score->deselectAll();

    score->select(chordRestAtBeat(score, 2), SelectType::RANGE);
    verifyDelete(score, spanners);
    score->deselectAll();

    score->select(chordRestAtBeat(score, 3), SelectType::RANGE);
    verifyNoDelete(score, spanners);
    score->deselectAll();

    score->select(chordRestAtBeat(score, 3, 1), SelectType::RANGE);
    verifyDelete(score, spanners);
    score->deselectAll();

    delete score;
}

//---------------------------------------------------------
//   deleteSegmentWithSpanner
//---------------------------------------------------------

void TestSelectionRangeDelete::deleteSegmentWithSpanner()
{
    /*
     *  Score looks like this:
     *  ss - start spanner, es - end spanner, q - quarter note
     *
     *  ss    es
     *  q  q  q
     */
    MasterScore* score = readScore(SELRANGEDELETE_DATA_DIR + "selectionrangedelete02.mscx");

    score->doLayout();
    QVERIFY(score);
    size_t spanners = score->spanner().size();

    score->select(chordRestAtBeat(score, 0), SelectType::RANGE);
    verifyNoDelete(score, spanners);
//      verifyDelete(score,spanners);
    score->deselectAll();

    score->select(chordRestAtBeat(score, 1), SelectType::RANGE);
    verifyNoDelete(score, spanners);
    score->deselectAll();

    score->select(chordRestAtBeat(score, 2), SelectType::RANGE);
    verifyNoDelete(score, spanners);
    score->deselectAll();

    score->select(chordRestAtBeat(score, 0), SelectType::RANGE);
    score->select(chordRestAtBeat(score, 2), SelectType::RANGE);
    verifyDelete(score, spanners);
    score->deselectAll();

    delete score;
}

//---------------------------------------------------------
//   deleteVoice
//---------------------------------------------------------

void TestSelectionRangeDelete::deleteVoice(int voice, QString idx)
{
    MasterScore* score = readScore(SELRANGEDELETE_DATA_DIR + QString("selectionrangedelete%1.mscx").arg(idx));

    Measure* m1 = score->firstMeasure();
    QVERIFY(m1);

    SelectionFilterType voiceFilterType = SelectionFilterType((int)SelectionFilterType::FIRST_VOICE + voice);
    score->selectionFilter().setFiltered(voiceFilterType, false);
    score->select(m1, SelectType::RANGE);

    score->startCmd();
    score->cmdDeleteSelection();
    score->endCmd();

    score->doLayout();

    QVERIFY(saveCompareScore(score, QString("selectionrangedelete%1.mscx").arg(idx),
                             SELRANGEDELETE_DATA_DIR + QString("selectionrangedelete%1-ref.mscx").arg(idx)));
    delete score;
}

//---------------------------------------------------------
//   deleteSkipAnnotations
//---------------------------------------------------------

void TestSelectionRangeDelete::deleteSkipAnnotations()
{
    MasterScore* score = readScore(SELRANGEDELETE_DATA_DIR + QString("selectionrangedelete05.mscx"));

    Measure* m1 = score->firstMeasure();
    QVERIFY(m1);

    SelectionFilterType annotationFilterType = SelectionFilterType((int)SelectionFilterType::CHORD_SYMBOL);
    score->selectionFilter().setFiltered(annotationFilterType, false);

    score->startCmd();
    score->cmdSelectAll();
    score->cmdDeleteSelection();
    score->endCmd();

    score->doLayout();

    QVERIFY(saveCompareScore(score, QString("selectionrangedelete05.mscx"),
                             SELRANGEDELETE_DATA_DIR + QString("selectionrangedelete05-ref.mscx")));
    delete score;
}

QTEST_MAIN(TestSelectionRangeDelete)

#include "tst_selectionrangedelete.moc"
