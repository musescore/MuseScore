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
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/chordrest.h"
#include "libmscore/instrchange.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/undo.h"

#include "engraving/compat/midi/midipatch.h"

static const QString INSTRUMENTCHANGE_DATA_DIR("instrumentchange_data/");

using namespace Ms;

//---------------------------------------------------------
//   TestInstrumentChange
//---------------------------------------------------------

class TestInstrumentChange : public QObject, public MTest
{
    Q_OBJECT

    MasterScore* test_pre(const char* p);
    void test_post(MasterScore* score, const char* p);

private slots:
    void initTestCase();
    void testAdd();
    void testDelete();
    void testChange();
    void testMixer();
    void testCopy();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestInstrumentChange::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   chordsymbol
//---------------------------------------------------------

MasterScore* TestInstrumentChange::test_pre(const char* p)
{
    QString p1 = INSTRUMENTCHANGE_DATA_DIR + p + ".mscx";
    MasterScore* score = readScore(p1);
    return score;
}

void TestInstrumentChange::test_post(MasterScore* score, const char* p)
{
    QString p1 = p;
    p1 += "-test.mscx";
    QString p2 = INSTRUMENTCHANGE_DATA_DIR + p + "-ref.mscx";
    QVERIFY(saveCompareScore(score, p1, p2));
    delete score;
}

void TestInstrumentChange::testAdd()
{
    MasterScore* score = test_pre("add");
    Measure* m = score->firstMeasure()->nextMeasure();
    Segment* s = m->first(SegmentType::ChordRest);
    InstrumentChange* ic = new InstrumentChange(score);
    ic->setParent(s);
    ic->setTrack(0);
    ic->setXmlText("Instrument");
    score->startCmd();
    score->undoAddElement(ic);
    score->endCmd();
    test_post(score, "add");
}

void TestInstrumentChange::testDelete()
{
    MasterScore* score = test_pre("delete");
    Measure* m = score->firstMeasure()->nextMeasure();
    Segment* s = m->first(SegmentType::ChordRest);
    InstrumentChange* ic = toInstrumentChange(s->annotations()[0]);
    score->deleteItem(ic);
    score->doLayout();
    test_post(score, "delete");
}

void TestInstrumentChange::testChange()
{
    MasterScore* score   = test_pre("change");
    Measure* m           = score->firstMeasure()->nextMeasure();
    Segment* s           = m->first(SegmentType::ChordRest);
    InstrumentChange* ic = toInstrumentChange(s->annotations()[0]);
    Instrument* ni       = score->staff(1)->part()->instrument();
    ic->setInstrument(new Instrument(*ni));
    score->startCmd();
    ic->setXmlText("Instrument Oboe");
    score->undo(new ChangeInstrument(ic, ic->instrument()));
    score->endCmd();
    score->doLayout();
    test_post(score, "change");
}

void TestInstrumentChange::testMixer()
{
    MasterScore* score = test_pre("mixer");
    Measure* m = score->firstMeasure()->nextMeasure();
    Segment* s = m->first(SegmentType::ChordRest);
    InstrumentChange* ic = static_cast<InstrumentChange*>(s->annotations()[0]);
    int idx = score->staff(0)->channel(s->tick(), 0);
    Channel* c = score->staff(0)->part()->instrument(s->tick())->channel(idx);
    MidiPatch* mp = new MidiPatch;
    mp->bank = 0;
    mp->drum = false;
    mp->name = "Viola";
    mp->prog = 41;
    mp->synti = "Fluid";
    score->startCmd();
    ic->setXmlText("Mixer Viola");
    score->undo(new ChangePatch(score, c, mp));
    score->endCmd();
    score->doLayout();
    test_post(score, "mixer");
}

void TestInstrumentChange::testCopy()
{
    MasterScore* score = test_pre("copy");
    Measure* m = score->firstMeasure()->nextMeasure();
    Segment* s = m->first(SegmentType::ChordRest);
    InstrumentChange* ic = static_cast<InstrumentChange*>(s->annotations()[0]);
    m = m->nextMeasure();
    s = m->first(SegmentType::ChordRest);
    InstrumentChange* nic = new InstrumentChange(*ic);
    nic->setParent(s);
    nic->setTrack(4);
    score->undoAddElement(nic);
    score->doLayout();
    test_post(score, "copy");
}

QTEST_MAIN(TestInstrumentChange)
#include "tst_instrumentchange.moc"
