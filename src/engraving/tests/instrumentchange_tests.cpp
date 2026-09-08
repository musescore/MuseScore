/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "global/defer.h"
#include "global/io/buffer.h"
#include "global/io/file.h"
#include "engraving/rw/mscsaver.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/keysig.h"
#include "engraving/editing/editkeysig.h"
#include "engraving/editing/transaction/transaction.h"

#include "engraving/dom/chordrest.h"
#include "engraving/dom/instrchange.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/part.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/editing/editinstrumentchange.h"
#include "engraving/editing/editpart.h"

#include "engraving/compat/midi/midipatch.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu::engraving;

static const String INSTRUMENTCHANGE_DATA_DIR("instrumentchange_data/");

class Engraving_InstrumentChangeTests : public ::testing::Test
{
public:
    MasterScore* test_pre(const char16_t* p);
    void test_post(MasterScore* score, const char16_t* p);
    void checkLinkedKeys(bool remove);
};

MasterScore* Engraving_InstrumentChangeTests::test_pre(const char16_t* p)
{
    String p1 = INSTRUMENTCHANGE_DATA_DIR + p + u".mscx";
    MasterScore* score = ScoreRW::readScore(p1);
    EXPECT_TRUE(score);
    return score;
}

void Engraving_InstrumentChangeTests::test_post(MasterScore* score, const char16_t* p)
{
    String p1 = p;
    p1 += u"-test.mscx";
    String p2 = INSTRUMENTCHANGE_DATA_DIR + p + u"-ref.mscx";
    EXPECT_TRUE(ScoreComp:: saveCompareScore(score, p1, p2));
    delete score;
}

TEST_F(Engraving_InstrumentChangeTests, testAdd)
{
    MasterScore* score = test_pre(u"add");
    Measure* m = score->firstMeasure()->nextMeasure();
    Segment* s = m->first(SegmentType::ChordRest);
    InstrumentChange* ic = new InstrumentChange(s);
    ic->setOwnershipParent(s);
    ic->setTrack(0);
    ic->setXmlText("Instrument");
    score->startCmd(TranslatableString::untranslatable("Instrument change tests"));
    score->undoAddElement(ic);
    score->endCmd();
    test_post(score, u"add");
}

TEST_F(Engraving_InstrumentChangeTests, testDelete)
{
    MasterScore* score = test_pre(u"delete");
    Measure* m = score->firstMeasure()->nextMeasure();
    Segment* s = m->first(SegmentType::ChordRest);
    InstrumentChange* ic = toInstrumentChange(s->annotations()[0]);
    score->deleteItem(ic);
    score->doLayout();
    test_post(score, u"delete");
}

TEST_F(Engraving_InstrumentChangeTests, testChange)
{
    MasterScore* score   = test_pre(u"change");
    Measure* m           = score->firstMeasure()->nextMeasure();
    Segment* s           = m->first(SegmentType::ChordRest);
    InstrumentChange* ic = toInstrumentChange(s->annotations()[0]);
    Instrument* ni       = score->staff(1)->part()->instrument();
    ic->setInstrument(new Instrument(*ni));
    score->startCmd(TranslatableString::untranslatable("Instrument change tests"));
    ic->setXmlText("Instrument Oboe");
    score->undo(new ChangeInstrument(ic, ic->instrument()));
    score->endCmd();
    score->doLayout();
    test_post(score, u"change");
}

TEST_F(Engraving_InstrumentChangeTests, testMixer)
{
    MasterScore* score = test_pre(u"mixer");
    Measure* m = score->firstMeasure()->nextMeasure();
    Segment* s = m->first(SegmentType::ChordRest);
    InstrumentChange* ic = static_cast<InstrumentChange*>(s->annotations()[0]);
    int idx = score->staff(0)->channel(s->tick(), 0);
    InstrChannel* c = score->staff(0)->part()->instrument(s->tick())->channel(idx);
    MidiPatch mp;
    mp.bank = 0;
    mp.drum = false;
    mp.name = "Viola";
    mp.prog = 41;
    mp.synti = "Fluid";
    score->startCmd(TranslatableString::untranslatable("Instrument change tests"));
    ic->setXmlText("Mixer Viola");
    score->undo(new ChangePatch(score, c, mp));
    score->endCmd();
    score->doLayout();
    test_post(score, u"mixer");
}

TEST_F(Engraving_InstrumentChangeTests, testCopy)
{
    MasterScore* score = test_pre(u"copy");
    Measure* m = score->firstMeasure()->nextMeasure();
    Segment* s = m->first(SegmentType::ChordRest);
    InstrumentChange* ic = static_cast<InstrumentChange*>(s->annotations()[0]);
    m = m->nextMeasure();
    s = m->first(SegmentType::ChordRest);
    InstrumentChange* nic = new InstrumentChange(*ic);
    nic->setOwnershipParent(s);
    nic->setTrack(4);
    score->undoAddElement(nic);
    score->doLayout();
    test_post(score, u"copy");
}

void Engraving_InstrumentChangeTests::checkLinkedKeys(bool remove)
{
    for (bool fromExcerpt : { false, true }) {
        SCOPED_TRACE(fromExcerpt);
        const String input = remove ? u"linked-key-delete.mscz" : u"linked-key-change.mscz";
        std::unique_ptr<MasterScore> score(ScoreRW::readScore(INSTRUMENTCHANGE_DATA_DIR + input));
        ASSERT_TRUE(score);
        ASSERT_EQ(score->excerpts().size(), 1u);
        Score* excerpt = score->excerpts().front()->excerptScore();
        Score* owner = fromExcerpt ? excerpt : score.get();
        const Fraction changeTick = score->firstMeasure()->nextMeasure()->tick();
        const Key initial = remove ? Key::G : Key::C;
        const Key expected = remove ? Key::C : Key::G;
        auto check = [changeTick](MasterScore* master, Key expectedKey) {
            for (Score* target : { static_cast<Score*>(master), master->excerpts().front()->excerptScore() }) {
                const KeySigEvent event = target->staff(0)->keySigEvent(changeTick);
                EXPECT_EQ(event.concertKey(), expectedKey);
                EXPECT_TRUE(event.forInstrumentChange());
                Segment* segment = target->tick2measure(changeTick)->findSegment(SegmentType::KeySig, changeTick);
                ASSERT_TRUE(segment);
                EngravingItem* item = segment->element(0);
                ASSERT_TRUE(item && item->isKeySig());
                EXPECT_EQ(toKeySig(item)->keySigEvent().concertKey(), expectedKey);
            }
        };
        check(score.get(), initial);
        if (remove) {
            Segment* segment = owner->firstMeasure()->findSegment(SegmentType::KeySig, Fraction(1, 2));
            ASSERT_TRUE(segment);
            KeySig* key = toKeySig(segment->element(0));
            ASSERT_TRUE(key);
            ASSERT_FALSE(key->forInstrumentChange());
            ASSERT_EQ(key->linkList().size(), 2u);
            owner->select(key, SelectType::SINGLE, 0);
            score->startCmd(TranslatableString::untranslatable("Delete preceding key"));
            owner->cmdDeleteSelection();
            EXPECT_EQ(MScore::_error, MsError::MS_NO_ERROR);
            score->endCmd();
        } else {
            KeySigEvent key;
            key.setConcertKey(Key::G);
            score->startCmd(TranslatableString::untranslatable("Change preceding key"));
            EditKeySig::undoChangeKeySig(score->transactionManager()->currentOrDummyTransaction(), owner, owner->staff(0), Fraction(0,
                                                                                                                                    1),
                                         key);
            score->endCmd();
        }
        check(score.get(), expected);
        score->undoRedo(true, nullptr);
        check(score.get(), initial);
        score->undoRedo(false, nullptr);
        check(score.get(), expected);
        ASSERT_TRUE(score->sanityCheck());

        const String fileName = String(u"linked-key-%1-%2.mscz").arg(remove).arg(fromExcerpt);
        auto buffer = muse::io::Buffer::opened(muse::io::IODevice::WriteOnly);
        MscWriter::Params params;
        params.device = &buffer;
        params.filePath = fileName;
        params.mode = MscIoMode::Zip;
        MscWriter writer(params);
        ASSERT_TRUE(writer.open());
        {
            // Write production MSCZ excerpts separately, without test-mode inline excerpts.
            const bool testMode = MScore::testMode;
            DEFER { MScore::testMode = testMode;
            };
            MScore::testMode = false;
            ASSERT_TRUE(MscSaver(score->iocContext()).writeMscz(score.get(), writer, false));
        }
        writer.close();
        ASSERT_FALSE(writer.hasError());
        ASSERT_TRUE(muse::io::File::writeFile(fileName, buffer.data()));
        score.reset();
        score.reset(ScoreRW::readScore(fileName, true));
        ASSERT_TRUE(score);
        ASSERT_EQ(score->excerpts().size(), 1u);
        EXPECT_TRUE(score->sanityCheck());
        check(score.get(), expected);
    }
}

TEST_F(Engraving_InstrumentChangeTests, linkedPrecedingKeyChange)
{
    checkLinkedKeys(false);
}

TEST_F(Engraving_InstrumentChangeTests, linkedPrecedingKeyDeletion)
{
    checkLinkedKeys(true);
}
