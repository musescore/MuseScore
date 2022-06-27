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

#include <gtest/gtest.h>

#include "libmscore/masterscore.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/chordrest.h"
#include "libmscore/instrchange.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/undo.h"

#include "engraving/compat/midi/midipatch.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String INSTRUMENTCHANGE_DATA_DIR("instrumentchange_data/");

class InstrumentChangeTests : public ::testing::Test
{
public:
    MasterScore* test_pre(const char16_t* p);
    void test_post(MasterScore* score, const char16_t* p);
};

MasterScore* InstrumentChangeTests::test_pre(const char16_t* p)
{
    String p1 = INSTRUMENTCHANGE_DATA_DIR + p + u".mscx";
    MasterScore* score = ScoreRW::readScore(p1);
    EXPECT_TRUE(score);
    return score;
}

void InstrumentChangeTests::test_post(MasterScore* score, const char16_t* p)
{
    String p1 = p;
    p1 += u"-test.mscx";
    String p2 = INSTRUMENTCHANGE_DATA_DIR + p + u"-ref.mscx";
    EXPECT_TRUE(ScoreComp:: saveCompareScore(score, p1, p2));
    delete score;
}

TEST_F(InstrumentChangeTests, testAdd)
{
    MasterScore* score = test_pre(u"add");
    Measure* m = score->firstMeasure()->nextMeasure();
    Segment* s = m->first(SegmentType::ChordRest);
    InstrumentChange* ic = new InstrumentChange(s);
    ic->setParent(s);
    ic->setTrack(0);
    ic->setXmlText("Instrument");
    score->startCmd();
    score->undoAddElement(ic);
    score->endCmd();
    test_post(score, u"add");
}

TEST_F(InstrumentChangeTests, testDelete)
{
    MasterScore* score = test_pre(u"delete");
    Measure* m = score->firstMeasure()->nextMeasure();
    Segment* s = m->first(SegmentType::ChordRest);
    InstrumentChange* ic = toInstrumentChange(s->annotations()[0]);
    score->deleteItem(ic);
    score->doLayout();
    test_post(score, u"delete");
}

TEST_F(InstrumentChangeTests, testChange)
{
    MasterScore* score   = test_pre(u"change");
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
    test_post(score, u"change");
}

TEST_F(InstrumentChangeTests, testMixer)
{
    MasterScore* score = test_pre(u"mixer");
    Measure* m = score->firstMeasure()->nextMeasure();
    Segment* s = m->first(SegmentType::ChordRest);
    InstrumentChange* ic = static_cast<InstrumentChange*>(s->annotations()[0]);
    int idx = score->staff(0)->channel(s->tick(), 0);
    InstrChannel* c = score->staff(0)->part()->instrument(s->tick())->channel(idx);
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
    test_post(score, u"mixer");
}

TEST_F(InstrumentChangeTests, testCopy)
{
    MasterScore* score = test_pre(u"copy");
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
    test_post(score, u"copy");
}
