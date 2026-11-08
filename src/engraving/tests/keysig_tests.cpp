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
#include <memory>

#include "engraving/dom/factory.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/editing/editkeysig.h"
#include "engraving/editing/transaction/transaction.h"
#include "engraving/editing/transpose.h"
#include "engraving/types/typesconv.h"

#include "utils/scorecomp.h"
#include "utils/scorerw.h"
#include "utils/testutils.h"

using namespace mu::engraving;

static const String KEYSIG_DATA_DIR("keysig_data/");

class Engraving_KeySigTests : public ::testing::Test
{
protected:
    void checkInstrumentChangeKey(Score* score, Key concertKey, Key writtenKey)
    {
        const Fraction tick = score->firstMeasure()->nextMeasure()->tick();
        const KeySigEvent event = score->staff(0)->keySigEvent(tick);
        EXPECT_TRUE(event.forInstrumentChange());
        EXPECT_EQ(event.concertKey(), concertKey);
        EXPECT_EQ(event.key(), writtenKey);
        Segment* segment = score->tick2measure(tick)->findSegment(SegmentType::KeySig, tick);
        ASSERT_TRUE(segment);
        EngravingItem* item = segment->element(0);
        ASSERT_TRUE(item);
        ASSERT_TRUE(item->isKeySig());
        const KeySigEvent actual = toKeySig(item)->keySigEvent();
        EXPECT_TRUE(actual.forInstrumentChange());
        EXPECT_EQ(actual.concertKey(), concertKey);
        EXPECT_EQ(actual.key(), writtenKey);
    }
};

TEST_F(Engraving_KeySigTests, keysig)
{
    String writeFile1("keysig01-test.mscx");
    String reference1(KEYSIG_DATA_DIR + "keysig01-ref.mscx");     // with D maj
    String writeFile2("keysig02-test.mscx");
    String reference2(KEYSIG_DATA_DIR + "keysig02-ref.mscx");     // with Eb maj
    String writeFile3("keysig03-test.mscx");
    String reference3(KEYSIG_DATA_DIR + "keysig03bis-ref.mscx");           // orig
    String writeFile4("keysig04-test.mscx");
    String reference4(KEYSIG_DATA_DIR + "keysig02-ref.mscx");     // with Eb maj
    String writeFile5("keysig05-test.mscx");
    String reference5(KEYSIG_DATA_DIR + "keysig01-ref.mscx");     // with D maj
    String writeFile6("keysig06-test.mscx");
    String reference6(KEYSIG_DATA_DIR + "keysig.mscx");           // orig

    // read file
    MasterScore* score = ScoreRW::readScore(KEYSIG_DATA_DIR + "keysig.mscx");
    ASSERT_TRUE(score);
    Measure* m2 = score->firstMeasure()->nextMeasure();
    ASSERT_TRUE(m2);

    // add a key signature (D major) in measure 2
    KeySigEvent ke2;
    ke2.setConcertKey(Key::D);
    score->startCmd(TranslatableString::untranslatable("Key signature tests"));
    EditKeySig::undoChangeKeySig(score->transactionManager()->currentOrDummyTransaction(), score, score->staff(0), m2->tick(), ke2);
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // change key signature in measure 2 to E flat major
    KeySigEvent ke_3;
    ke_3.setConcertKey(Key(-3));
    score->startCmd(TranslatableString::untranslatable("Key signature tests"));
    EditKeySig::undoChangeKeySig(score->transactionManager()->currentOrDummyTransaction(), score, score->staff(0), m2->tick(), ke_3);
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

    // remove key signature in measure 2
    Segment* s = m2->first(SegmentType::KeySig);
    EngravingItem* e = s->element(0);
    score->startCmd(TranslatableString::untranslatable("Key signature tests"));
    score->undoRemoveElement(e);
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile3, reference3));

    // undo remove
    EditData ed;
    score->transactionManager()->undoRedo(true, &ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile4, reference4));

    // undo change
    score->transactionManager()->undoRedo(true, &ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile5, reference5));

    // undo add
    score->transactionManager()->undoRedo(true, &ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile6, reference6));

    delete score;
}

//---------------------------------------------------------
//   keysig_78216
//    input score has section breaks on non-measure MeasureBase objects.
//    should not display courtesy keysig at the end of final measure of each section (meas 1, 2, & 3), even if section break occurs on subsequent non-measure frame.
//---------------------------------------------------------
TEST_F(Engraving_KeySigTests, keysig_78216)
{
    MasterScore* score = ScoreRW::readScore(KEYSIG_DATA_DIR + "keysig_78216.mscx");
    ASSERT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    ASSERT_TRUE(m1);
    Measure* m2 = m1->nextMeasure();
    ASSERT_TRUE(m2);
    Measure* m3 = m2->nextMeasure();
    ASSERT_TRUE(m3);

    // verify no keysig exists in segment of final tick of m1, m2, m3
    EXPECT_EQ(m1->findSegment(SegmentType::KeySig, m1->endTick()), nullptr) << "Should be no keysig at end of measure 1.";
    EXPECT_EQ(m2->findSegment(SegmentType::KeySig, m2->endTick()), nullptr) << "Should be no keysig at end of measure 2.";
    EXPECT_EQ(m3->findSegment(SegmentType::KeySig, m3->endTick()), nullptr) << "Should be no keysig at end of measure 3.";

    delete score;
}

TEST_F(Engraving_KeySigTests, concertPitch)
{
    MasterScore* score = ScoreRW::readScore(KEYSIG_DATA_DIR + "concert-pitch.mscx");
    ASSERT_TRUE(score);

    score->cmdConcertPitchChanged(true);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"concert-pitch-01-test.mscx", KEYSIG_DATA_DIR + u"concert-pitch-01-ref.mscx"));
    score->cmdConcertPitchChanged(false);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"concert-pitch-02-test.mscx", KEYSIG_DATA_DIR + u"concert-pitch-02-ref.mscx"));

    delete score;
}

TEST_F(Engraving_KeySigTests, preferSharpFlat)
{
    MasterScore* score1 = ScoreRW::readScore(KEYSIG_DATA_DIR + u"preferSharpFlat-1.mscx");
    ASSERT_TRUE(score1);
    auto parts = score1->parts();
    Part* part1 = parts[0];
    part1->setPreferSharpFlat(PreferSharpFlat::FLATS);
    score1->transactionManager()->transaction(TranslatableString::untranslatable("Key signature tests"), [&](auto& tx) {
        Transpose::transpositionChanged(tx, score1, part1, part1->instrument(Fraction(0, 1))->transpose(), Fraction(0, 1), Fraction(16, 4));
    });
    EXPECT_TRUE(ScoreComp::saveCompareScore(score1, u"preferSharpFlat-1-test.mscx", KEYSIG_DATA_DIR + u"preferSharpFlat-1-ref.mscx"));
    delete score1;

    MasterScore* score2 = ScoreRW::readScore(KEYSIG_DATA_DIR + u"preferSharpFlat-2.mscx");
    ASSERT_TRUE(score2);
    score2->cmdSelectAll();
    score2->transactionManager()->transaction(TranslatableString::untranslatable("Key signature tests"), [&](auto& tx) {
        // transpose augmented unison up
        Transpose::transpose(tx, score2, TransposeMode::BY_INTERVAL, TransposeDirection::UP, Key::C, 1, true, true, true);
    });
    EXPECT_TRUE(ScoreComp::saveCompareScore(score2, u"preferSharpFlat-2-test.mscx", KEYSIG_DATA_DIR + u"preferSharpFlat-2-ref.mscx"));
    delete score2;
}

TEST_F(Engraving_KeySigTests, keysigMode)
{
    MasterScore* score = ScoreRW::readScore(KEYSIG_DATA_DIR + u"keysigMode.mscx");
    ASSERT_TRUE(score);
    Measure* m1 = score->firstMeasure();
    KeySig* ke = toKeySig(m1->findSegment(SegmentType::KeySig, m1->tick())->element(0));
    ke->setProperty(Pid::KEYSIG_MODE, KeyMode::DORIAN);
    score->update();
    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"keysig03.mscx", KEYSIG_DATA_DIR + u"keysig03-ref.mscx"));
    delete score;
}

TEST_F(Engraving_KeySigTests, linkedPrecedingKeyChange)
{
    for (bool fromExcerpt : { false, true }) {
        SCOPED_TRACE(fromExcerpt);
        std::unique_ptr<MasterScore> score(ScoreRW::readScore(KEYSIG_DATA_DIR + u"linked-instrument-change.mscx"));
        ASSERT_TRUE(score);
        Score* excerpt = TestUtils::createPart(score.get());
        ASSERT_TRUE(excerpt);
        Score* owner = fromExcerpt ? excerpt : score.get();
        for (Score* target : { static_cast<Score*>(score.get()), excerpt }) {
            ASSERT_NO_FATAL_FAILURE(checkInstrumentChangeKey(target, Key::G, Key::A));
        }
        KeySigEvent key;
        key.setConcertKey(Key::D);
        owner->startCmd(TranslatableString::untranslatable("Change preceding key"));
        EditKeySig::undoChangeKeySig(owner->transactionManager()->currentOrDummyTransaction(), owner,
                                     owner->staff(0), Fraction(1, 2), key);
        owner->endCmd();
        for (Score* target : { static_cast<Score*>(score.get()), excerpt }) {
            ASSERT_NO_FATAL_FAILURE(checkInstrumentChangeKey(target, Key::D, Key::E));
        }
        owner->undoRedo(true, nullptr);
        for (Score* target : { static_cast<Score*>(score.get()), excerpt }) {
            ASSERT_NO_FATAL_FAILURE(checkInstrumentChangeKey(target, Key::G, Key::A));
        }
        owner->undoRedo(false, nullptr);
        for (Score* target : { static_cast<Score*>(score.get()), excerpt }) {
            ASSERT_NO_FATAL_FAILURE(checkInstrumentChangeKey(target, Key::D, Key::E));
        }
    }
}

TEST_F(Engraving_KeySigTests, linkedPrecedingKeyDeletion)
{
    for (bool fromExcerpt : { false, true }) {
        SCOPED_TRACE(fromExcerpt);
        std::unique_ptr<MasterScore> score(ScoreRW::readScore(KEYSIG_DATA_DIR + u"linked-instrument-change.mscx"));
        ASSERT_TRUE(score);
        Score* excerpt = TestUtils::createPart(score.get());
        ASSERT_TRUE(excerpt);
        Score* owner = fromExcerpt ? excerpt : score.get();
        for (Score* target : { static_cast<Score*>(score.get()), excerpt }) {
            ASSERT_NO_FATAL_FAILURE(checkInstrumentChangeKey(target, Key::G, Key::A));
        }
        Segment* segment = owner->firstMeasure()->findSegment(SegmentType::KeySig, Fraction(1, 2));
        ASSERT_TRUE(segment);
        EngravingItem* item = segment->element(0);
        ASSERT_TRUE(item);
        ASSERT_TRUE(item->isKeySig());
        ASSERT_FALSE(toKeySig(item)->forInstrumentChange());
        ASSERT_EQ(item->linkList().size(), 2u);
        owner->select(item, SelectType::SINGLE, 0);
        owner->startCmd(TranslatableString::untranslatable("Delete preceding key"));
        owner->cmdDeleteSelection();
        owner->endCmd();
        for (Score* target : { static_cast<Score*>(score.get()), excerpt }) {
            ASSERT_NO_FATAL_FAILURE(checkInstrumentChangeKey(target, Key::C, Key::D));
        }
        owner->undoRedo(true, nullptr);
        for (Score* target : { static_cast<Score*>(score.get()), excerpt }) {
            ASSERT_NO_FATAL_FAILURE(checkInstrumentChangeKey(target, Key::G, Key::A));
        }
        owner->undoRedo(false, nullptr);
        for (Score* target : { static_cast<Score*>(score.get()), excerpt }) {
            ASSERT_NO_FATAL_FAILURE(checkInstrumentChangeKey(target, Key::C, Key::D));
        }
    }
}

//---------------------------------------------------------
//   KeySigEvent equality must take the per-keysignature Jianpu tonic
//   override fields into account, since they are independent of key/mode/custom.
//---------------------------------------------------------
TEST_F(Engraving_KeySigTests, jianpuKeySigEventEquality)
{
    KeySigEvent a;
    a.setConcertKey(Key::G);
    KeySigEvent b = a;
    EXPECT_TRUE(a == b);

    b.setJianpuNumbering(JianpuNumbering::CUSTOM);
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);

    KeySigEvent c = a;
    c.setJianpuTonicKey(Key::D);
    EXPECT_FALSE(a == c);

    KeySigEvent d = a;
    d.setJianpuTonicMode(KeyMode::DORIAN);
    EXPECT_FALSE(a == d);
}

//---------------------------------------------------------
//   jianpuKeyMapping should mirror the staff's key/mode when FOLLOW_SCORE_KEY
//   is selected, and use the explicit override fields when CUSTOM is selected.
//---------------------------------------------------------
TEST_F(Engraving_KeySigTests, jianpuKeyMapping)
{
    KeySigEvent ks;
    ks.setConcertKey(Key::D);
    ks.setMode(KeyMode::DORIAN);

    KeyMode mode;
    int tonicTpc;

    jianpuKeyMapping(ks, mode, tonicTpc);
    EXPECT_EQ(mode, KeyMode::DORIAN);
    EXPECT_EQ(tonicTpc, jianpuTonicTpc(Key::D, KeyMode::DORIAN));

    ks.setJianpuNumbering(JianpuNumbering::CUSTOM);
    ks.setJianpuTonicKey(Key::A_B);
    ks.setJianpuTonicMode(KeyMode::LYDIAN);

    jianpuKeyMapping(ks, mode, tonicTpc);
    EXPECT_EQ(mode, KeyMode::LYDIAN);
    EXPECT_EQ(tonicTpc, key2Tpc(Key::A_B));
}

//---------------------------------------------------------
//   jianpuKeyLabel should produce the "<degree>=<tonic>" label used for the
//   Jianpu key signature, falling back to the major mapping when the mode
//   is UNKNOWN or NONE (mode is never guessed from the music).
//---------------------------------------------------------
TEST_F(Engraving_KeySigTests, jianpuKeyLabel)
{
    // Follow mode, unmarked major/minor (UNKNOWN): falls back to major
    KeySigEvent unknown;
    unknown.setConcertKey(Key::C);
    EXPECT_EQ(jianpuKeyLabel(unknown), u"1=C");

    // Follow mode, unmarked, one sharp (UNKNOWN): falls back to G major
    KeySigEvent oneSharp;
    oneSharp.setConcertKey(Key::G);
    EXPECT_EQ(jianpuKeyLabel(oneSharp), u"1=G");

    // Follow mode, atonal (NONE): falls back to major (fixed at C)
    KeySigEvent atonal;
    atonal.setConcertKey(Key::C);
    atonal.setMode(KeyMode::NONE);
    EXPECT_EQ(jianpuKeyLabel(atonal), u"1=C");

    // Follow mode, C major
    KeySigEvent cMajor;
    cMajor.setConcertKey(Key::C);
    cMajor.setMode(KeyMode::MAJOR);
    EXPECT_EQ(jianpuKeyLabel(cMajor), u"1=C");

    // Follow mode, A minor (same key signature as C major)
    KeySigEvent aMinor;
    aMinor.setConcertKey(Key::C);
    aMinor.setMode(KeyMode::MINOR);
    EXPECT_EQ(jianpuKeyLabel(aMinor), u"6=A");

    // Follow mode, C Dorian (relative major is Bb major)
    KeySigEvent cDorian;
    cDorian.setConcertKey(Key::B_B);
    cDorian.setMode(KeyMode::DORIAN);
    EXPECT_EQ(jianpuKeyLabel(cDorian), u"2=C");

    // Custom numbering always shows a label, regardless of the score's mode
    KeySigEvent custom;
    custom.setConcertKey(Key::G);
    custom.setJianpuNumbering(JianpuNumbering::CUSTOM);
    custom.setJianpuTonicKey(Key::C);
    custom.setJianpuTonicMode(KeyMode::MAJOR);
    EXPECT_EQ(jianpuKeyLabel(custom), u"1=C");
}

TEST_F(Engraving_KeySigTests, jianpuEnumXmlConversions)
{
    for (JianpuNumbering numbering : { JianpuNumbering::FOLLOW_SCORE_KEY, JianpuNumbering::CUSTOM }) {
        EXPECT_EQ(TConv::fromXml(TConv::toXml(numbering), JianpuNumbering::FOLLOW_SCORE_KEY), numbering);
    }
    for (int key = int(Key::MIN); key <= int(Key::MAX); ++key) {
        EXPECT_EQ(TConv::fromXml(TConv::toXml(Key(key)), Key::INVALID), Key(key));
    }
    EXPECT_EQ(TConv::toXml(JianpuNumbering::CUSTOM), "custom");
    EXPECT_EQ(TConv::toXml(Key::E_B), "Eb");
    EXPECT_EQ(TConv::fromXml("unrecognized", JianpuNumbering::FOLLOW_SCORE_KEY), JianpuNumbering::FOLLOW_SCORE_KEY);
    EXPECT_EQ(TConv::fromXml("unrecognized", Key::C), Key::C);
}

TEST_F(Engraving_KeySigTests, jianpuKeySigProperties)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(KEYSIG_DATA_DIR + u"keysigMode.mscx"));
    ASSERT_TRUE(score);
    Measure* m1 = score->firstMeasure();
    ASSERT_TRUE(m1);
    Segment* segment = m1->findSegment(SegmentType::KeySig, m1->tick());
    ASSERT_TRUE(segment);
    KeySig* ke = toKeySig(segment->element(0));
    ASSERT_TRUE(ke);
    ASSERT_FALSE(ke->generated());

    EXPECT_EQ(propertyType(Pid::JIANPU_NUMBERING), P_TYPE::JIANPU_NUMBERING);
    EXPECT_EQ(propertyType(Pid::JIANPU_TONIC_KEY), P_TYPE::KEY);
    EXPECT_EQ(ke->propertyDefault(Pid::JIANPU_NUMBERING).value<JianpuNumbering>(), JianpuNumbering::FOLLOW_SCORE_KEY);
    EXPECT_EQ(ke->propertyDefault(Pid::JIANPU_TONIC_KEY).value<Key>(), Key::C);
    EXPECT_EQ(ke->propertyDefault(Pid::JIANPU_TONIC_MODE).value<KeyMode>(), KeyMode::MAJOR);

    EXPECT_EQ(ke->getProperty(Pid::JIANPU_NUMBERING).value<JianpuNumbering>(), JianpuNumbering::FOLLOW_SCORE_KEY);

    EXPECT_TRUE(ke->setProperty(Pid::JIANPU_NUMBERING, JianpuNumbering::CUSTOM));
    EXPECT_TRUE(ke->setProperty(Pid::JIANPU_TONIC_KEY, Key::E_B));
    EXPECT_TRUE(ke->setProperty(Pid::JIANPU_TONIC_MODE, KeyMode::PHRYGIAN));
    EXPECT_EQ(ke->staff()->keySigEvent(ke->tick()), ke->keySigEvent());

    EXPECT_EQ(ke->getProperty(Pid::JIANPU_NUMBERING).type(), P_TYPE::JIANPU_NUMBERING);
    EXPECT_EQ(ke->getProperty(Pid::JIANPU_TONIC_KEY).type(), P_TYPE::KEY);
    EXPECT_EQ(ke->getProperty(Pid::JIANPU_NUMBERING).value<JianpuNumbering>(), JianpuNumbering::CUSTOM);
    EXPECT_EQ(ke->getProperty(Pid::JIANPU_TONIC_KEY).value<Key>(), Key::E_B);
    EXPECT_EQ(ke->getProperty(Pid::JIANPU_TONIC_MODE).value<KeyMode>(), KeyMode::PHRYGIAN);

    for (Pid property : { Pid::JIANPU_NUMBERING, Pid::JIANPU_TONIC_KEY }) {
        const PropertyValue value = ke->getProperty(property);
        EXPECT_EQ(PropertyValue::fromQVariant(value.toQVariant(), propertyType(property)), value);
    }

    std::unique_ptr<KeySig> restored(toKeySig(ScoreRW::writeReadElement(ke)));
    ASSERT_TRUE(restored);
    EXPECT_EQ(restored->jianpuNumbering(), JianpuNumbering::CUSTOM);
    EXPECT_EQ(restored->jianpuTonicKey(), Key::E_B);
    EXPECT_EQ(restored->jianpuTonicMode(), KeyMode::PHRYGIAN);
}

TEST_F(Engraving_KeySigTests, jianpuLinkedKeySigProperties)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(KEYSIG_DATA_DIR + u"keysigMode.mscx"));
    ASSERT_TRUE(score);
    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);
    Segment* segment = measure->findSegment(SegmentType::KeySig, measure->tick());
    ASSERT_TRUE(segment);
    KeySig* source = toKeySig(segment->element(0));
    KeySig* linked = toKeySig(segment->element(4));
    ASSERT_TRUE(source);
    ASSERT_TRUE(linked);
    ASSERT_FALSE(source->generated());
    linked->linkTo(source);

    const Pid properties[] = { Pid::JIANPU_NUMBERING, Pid::JIANPU_TONIC_KEY, Pid::JIANPU_TONIC_MODE };
    const PropertyValue values[] = { JianpuNumbering::CUSTOM, Key::E_B, KeyMode::PHRYGIAN };
    PropertyValue originalValues[3];
    for (size_t index = 0; index < 3; ++index) {
        originalValues[index] = source->getProperty(properties[index]);
        EXPECT_EQ(linked->getProperty(properties[index]), originalValues[index]);
    }

    score->startCmd(TranslatableString::untranslatable("Key signature tests"));
    for (size_t index = 0; index < 3; ++index) {
        source->undoChangeProperty(properties[index], values[index]);
    }
    score->endCmd();

    for (size_t index = 0; index < 3; ++index) {
        EXPECT_EQ(source->getProperty(properties[index]), values[index]);
        EXPECT_EQ(linked->getProperty(properties[index]), originalValues[index]);
    }
    EXPECT_EQ(source->staff()->keySigEvent(source->tick()), source->keySigEvent());
    EXPECT_EQ(linked->staff()->keySigEvent(linked->tick()), linked->keySigEvent());

    EditData editData;
    score->transactionManager()->undoRedo(true, &editData);
    for (size_t index = 0; index < 3; ++index) {
        EXPECT_EQ(source->getProperty(properties[index]), originalValues[index]);
        EXPECT_EQ(linked->getProperty(properties[index]), originalValues[index]);
    }
    EXPECT_EQ(source->staff()->keySigEvent(source->tick()), source->keySigEvent());

    score->transactionManager()->undoRedo(false, &editData);
    for (size_t index = 0; index < 3; ++index) {
        EXPECT_EQ(source->getProperty(properties[index]), values[index]);
        EXPECT_EQ(linked->getProperty(properties[index]), originalValues[index]);
    }
    EXPECT_EQ(source->staff()->keySigEvent(source->tick()), source->keySigEvent());
}

TEST_F(Engraving_KeySigTests, jianpuGeneratedKeySigProperties)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(u"beam_data/jianpuBeam.mscx"));
    ASSERT_TRUE(score);
    Measure* first = score->firstMeasure();
    ASSERT_TRUE(first);
    Measure* second = first->nextMeasure();
    ASSERT_TRUE(second);
    Staff* staff = score->staff(0);
    ASSERT_TRUE(staff);
    ASSERT_TRUE(staff->isJianpuStaff(first->tick()));

    Transaction& tx = score->transactionManager()->currentOrDummyTransaction();
    EditKeySig::undoChangeKeySig(tx, score.get(), staff, first->tick(), staff->keySigEvent(first->tick()));

    LayoutBreak* lineBreak = Factory::createLayoutBreak(first);
    lineBreak->setLayoutBreakType(LayoutBreakType::LINE);
    lineBreak->setTrack(0);
    score->undoAddElement(lineBreak);
    score->doLayout();

    Segment* firstSegment = first->findSegment(SegmentType::KeySig, first->tick());
    Segment* secondSegment = second->findSegment(SegmentType::KeySig, second->tick());
    ASSERT_TRUE(firstSegment);
    ASSERT_TRUE(secondSegment);
    KeySig* initial = toKeySig(firstSegment->element(0));
    KeySig* continuation = toKeySig(secondSegment->element(0));
    ASSERT_TRUE(initial);
    ASSERT_TRUE(continuation);
    ASSERT_FALSE(initial->generated());
    ASSERT_TRUE(continuation->generated());

    const Pid properties[] = { Pid::KEYSIG_MODE, Pid::JIANPU_NUMBERING, Pid::JIANPU_TONIC_KEY, Pid::JIANPU_TONIC_MODE };
    const PropertyValue values[] = { KeyMode::DORIAN, JianpuNumbering::CUSTOM, Key::E_B, KeyMode::PHRYGIAN };
    const KeySigEvent originalEvent = staff->keySigEvent(second->tick());
    const Fraction originalKeyTick = staff->currentKeyTick(second->tick());
    for (size_t i = 0; i < 4; ++i) {
        SCOPED_TRACE(i);
        const PropertyValue originalValue = continuation->getProperty(properties[i]);
        EXPECT_FALSE(continuation->setProperty(properties[i], values[i]));
        EXPECT_EQ(continuation->getProperty(properties[i]), originalValue);
        EXPECT_EQ(staff->keySigEvent(second->tick()), originalEvent);
        EXPECT_EQ(staff->currentKeyTick(second->tick()), originalKeyTick);
        EXPECT_TRUE(continuation->generated());
    }

    PropertyValue originalValues[4];
    score->startCmd(TranslatableString::untranslatable("Change Jianpu key signature"));
    for (size_t i = 0; i < 4; ++i) {
        SCOPED_TRACE(i);
        originalValues[i] = initial->getProperty(properties[i]);
        initial->undoChangeProperty(properties[i], values[i]);
        EXPECT_EQ(initial->getProperty(properties[i]), values[i]);
        EXPECT_EQ(staff->keySigEvent(first->tick()), initial->keySigEvent());
    }
    score->endCmd();

    EditData editData;
    score->transactionManager()->undoRedo(true, &editData);
    for (size_t i = 0; i < 4; ++i) {
        SCOPED_TRACE(i);
        EXPECT_EQ(initial->getProperty(properties[i]), originalValues[i]);
    }
    EXPECT_EQ(staff->keySigEvent(first->tick()), initial->keySigEvent());

    score->transactionManager()->undoRedo(false, &editData);
    for (size_t i = 0; i < 4; ++i) {
        SCOPED_TRACE(i);
        EXPECT_EQ(initial->getProperty(properties[i]), values[i]);
    }
    EXPECT_EQ(staff->keySigEvent(first->tick()), initial->keySigEvent());
    score->doLayout();
    secondSegment = second->findSegment(SegmentType::KeySig, second->tick());
    ASSERT_TRUE(secondSegment);
    continuation = toKeySig(secondSegment->element(0));
    ASSERT_TRUE(continuation);
    EXPECT_TRUE(continuation->generated());
    EXPECT_EQ(continuation->keySigEvent(), staff->keySigEvent(second->tick()));
}

//---------------------------------------------------------
//   End-to-end check that TLayout::layoutKeySig actually populates
//   KeySig::LayoutData::jianpuLabel on a real Jianpu staff: the major
//   fallback label for the untouched (mode == UNKNOWN) header key sig, a
//   label once Mode is set, and a label for Custom numbering.
//---------------------------------------------------------
TEST_F(Engraving_KeySigTests, jianpuKeyLabelLayout)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(u"beam_data/jianpuBeam.mscx"));
    ASSERT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    ASSERT_TRUE(m1);
    Segment* keySigSeg = m1->findSegment(SegmentType::KeySig, m1->tick());
    ASSERT_TRUE(keySigSeg);
    KeySig* ke = toKeySig(keySigSeg->element(0));
    ASSERT_TRUE(ke);
    ASSERT_TRUE(ke->staff()->isJianpuStaff(ke->tick()));

    // Untouched header key sig: mode defaults to UNKNOWN, falls back to major
    EXPECT_EQ(ke->ldata()->jianpuLabel, u"1=C");

    Staff* staff = ke->staff();
    Transaction& tx = score->transactionManager()->currentOrDummyTransaction();
    EditKeySig::undoChangeKeySig(tx, score.get(), staff, m1->tick(), staff->keySigEvent(m1->tick()));
    ASSERT_FALSE(ke->generated());
    EXPECT_TRUE(ke->setProperty(Pid::KEYSIG_MODE, KeyMode::MAJOR));
    score->doLayout();
    keySigSeg = m1->findSegment(SegmentType::KeySig, m1->tick());
    ASSERT_TRUE(keySigSeg);
    ke = toKeySig(keySigSeg->element(0));
    ASSERT_TRUE(ke);
    EXPECT_EQ(ke->ldata()->jianpuLabel, u"1=C");

    EXPECT_TRUE(ke->setProperty(Pid::JIANPU_NUMBERING, JianpuNumbering::CUSTOM));
    EXPECT_TRUE(ke->setProperty(Pid::JIANPU_TONIC_KEY, Key::D));
    EXPECT_TRUE(ke->setProperty(Pid::JIANPU_TONIC_MODE, KeyMode::MAJOR));
    score->doLayout();
    keySigSeg = m1->findSegment(SegmentType::KeySig, m1->tick());
    ASSERT_TRUE(keySigSeg);
    ke = toKeySig(keySigSeg->element(0));
    ASSERT_TRUE(ke);
    EXPECT_EQ(ke->ldata()->jianpuLabel, u"1=D");
}
