/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <memory>

#include <gtest/gtest.h>

#include "engraving/api/v1/apistructs.h"
#include "engraving/api/v1/cursor.h"
#include "engraving/api/v1/elements.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/textbase.h"

#include "engraving/editing/edithairpin.h"
#include "engraving/editing/editspanner.h"
#include "engraving/editing/transaction/transaction.h"
#include "engraving/editing/transaction/undostack.h"

#include "engraving/tests/utils/scorerw.h"

using namespace muse;
using namespace mu::engraving;

static const String SCOREELEMENT_DATA_DIR("scoreelement_data/");

// plugins set size_t properties (track, voice, spannerTrack2) with plain numbers;
// a slur, as only spanners that can cross voices have their own end track
TEST(Engraving_ApiScoreElementTests, setSizeTProperties)
{
    // one staff: tracks 0 to 3
    MasterScore* score = ScoreRW::readScore(SCOREELEMENT_DATA_DIR + u"empty.mscx");
    ASSERT_TRUE(score);

    std::unique_ptr<apiv1::EngravingItem> item(apiv1::wrap(Factory::createItem(ElementType::SLUR, score->dummy()),
                                                           apiv1::Ownership::PLUGIN));
    Spanner* slur = toSpanner(item->element());

    item->setProperty("track", 1);
    EXPECT_EQ(slur->track(), 1u);
    EXPECT_EQ(item->property("track").toInt(), 1);

    item->setProperty("voice", 2);
    EXPECT_EQ(slur->voice(), 2u);
    EXPECT_EQ(slur->track(), 2u);

    item->setProperty("spannerTrack2", 3);
    EXPECT_EQ(slur->track2(), 3u);
    EXPECT_EQ(item->property("spannerTrack2").toInt(), 3);

    // invalid values are refused
    item->setProperty("track", -1);
    EXPECT_EQ(slur->track(), 2u);
    item->setProperty("voice", 4);
    EXPECT_EQ(slur->track(), 2u);
    item->setProperty("spannerTrack2", -1);
    EXPECT_EQ(slur->track2(), 3u);

    item.reset();
    delete score;
}

// m1: four quarters in voice 1
static MasterScore* readScoreWithQuarters()
{
    MasterScore* score = ScoreRW::readScore(SCOREELEMENT_DATA_DIR + u"empty.mscx");
    if (!score) {
        return nullptr;
    }

    apiv1::Cursor cursor(score);
    cursor.rewind(apiv1::Cursor::SCORE_START);
    cursor.setDuration(1, 4);
    score->startCmd(TranslatableString::untranslatable("Engraving API score element tests"));
    for (int i = 0; i < 4; ++i) {
        cursor.addNote(60 + i);
    }
    score->endCmd();

    return score;
}

static Chord* chordAt(Score* score, const Fraction& tick)
{
    Segment* segment = score->tick2segment(tick, true, SegmentType::ChordRest);
    EngravingItem* item = segment ? segment->element(0) : nullptr;
    return item && item->isChord() ? toChord(item) : nullptr;
}

// as a plugin does it: one command
static void setInCommand(Score* score, apiv1::EngravingItem* item, const char* name, int value)
{
    score->startCmd(TranslatableString::untranslatable("Engraving API score element tests"));
    item->setProperty(name, value);
    score->endCmd();
}

// moving notes takes the voice commands; changing the number alone would leave them in voice 1's place
TEST(Engraving_ApiScoreElementTests, notesAndChordsKeepTheirVoice)
{
    MasterScore* score = readScoreWithQuarters();
    ASSERT_TRUE(score);
    Chord* chord = chordAt(score, Fraction(0, 1));
    ASSERT_TRUE(chord);

    std::unique_ptr<apiv1::EngravingItem> apiNote(apiv1::wrap(chord->upNote()));
    std::unique_ptr<apiv1::EngravingItem> apiChord(apiv1::wrap(chord));

    setInCommand(score, apiNote.get(), "voice", 1);
    setInCommand(score, apiChord.get(), "voice", 1);
    setInCommand(score, apiChord.get(), "track", 1);

    EXPECT_EQ(chord->track(), 0u);
    EXPECT_EQ(chord->upNote()->track(), 0u);

    apiNote.reset();
    apiChord.reset();
    delete score;
}

TEST(Engraving_ApiScoreElementTests, staffTextVoiceAndTrack)
{
    MasterScore* score = readScoreWithQuarters();
    ASSERT_TRUE(score);

    score->startCmd(TranslatableString::untranslatable("Engraving API score element tests"));
    TextBase* text = score->addText(TextStyleType::STAFF, chordAt(score, Fraction(0, 1)));
    score->endCmd();
    ASSERT_TRUE(text);
    std::unique_ptr<apiv1::EngravingItem> apiText(apiv1::wrap(text));

    setInCommand(score, apiText.get(), "voice", 1);
    EXPECT_EQ(text->track(), 1u);

    EditData ed;
    score->undoRedo(true, &ed);
    EXPECT_EQ(text->track(), 0u);
    score->undoRedo(false, &ed);
    EXPECT_EQ(text->track(), 1u);

    // within its staff only
    setInCommand(score, apiText.get(), "track", 2);
    EXPECT_EQ(text->track(), 2u);
    setInCommand(score, apiText.get(), "track", 4);
    EXPECT_EQ(text->track(), 2u);

    apiText.reset();
    delete score;
}

// system items are on the top staff and copied to the staves that show system objects
TEST(Engraving_ApiScoreElementTests, systemTextKeepsItsVoice)
{
    MasterScore* score = readScoreWithQuarters();
    ASSERT_TRUE(score);

    score->startCmd(TranslatableString::untranslatable("Engraving API score element tests"));
    TextBase* text = score->addText(TextStyleType::SYSTEM, chordAt(score, Fraction(0, 1)));
    score->endCmd();
    ASSERT_TRUE(text);
    std::unique_ptr<apiv1::EngravingItem> apiText(apiv1::wrap(text));

    setInCommand(score, apiText.get(), "voice", 1);
    setInCommand(score, apiText.get(), "track", 1);
    EXPECT_EQ(text->track(), 0u);

    apiText.reset();
    delete score;
}

// as the voice commands do
TEST(Engraving_ApiScoreElementTests, dynamicVoiceIsForThatVoiceOnly)
{
    MasterScore* score = readScoreWithQuarters();
    ASSERT_TRUE(score);

    score->startCmd(TranslatableString::untranslatable("Engraving API score element tests"));
    TextBase* dynamic = score->addText(TextStyleType::DYNAMICS, chordAt(score, Fraction(0, 1)));
    score->endCmd();
    ASSERT_TRUE(dynamic);
    std::unique_ptr<apiv1::EngravingItem> apiDynamic(apiv1::wrap(dynamic));

    setInCommand(score, apiDynamic.get(), "voice", 1);
    EXPECT_EQ(dynamic->track(), 1u);
    EXPECT_EQ(dynamic->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>(), VoiceAssignment::CURRENT_VOICE_ONLY);

    apiDynamic.reset();
    delete score;
}

// a slur ends on a chord or rest in its end track, and follows its notes' voice
TEST(Engraving_ApiScoreElementTests, slurEndTrack)
{
    MasterScore* score = readScoreWithQuarters();
    ASSERT_TRUE(score);

    score->startCmd(TranslatableString::untranslatable("Engraving API score element tests"));
    Slur* slur = score->addSlur(chordAt(score, Fraction(0, 1)), chordAt(score, Fraction(1, 4)), nullptr);
    score->endCmd();
    ASSERT_TRUE(slur);
    std::unique_ptr<apiv1::EngravingItem> apiSlur(apiv1::wrap(slur));

    // past the last track, and voice 2 with nothing at the end
    setInCommand(score, apiSlur.get(), "spannerTrack2", 400);
    setInCommand(score, apiSlur.get(), "spannerTrack2", 4);
    setInCommand(score, apiSlur.get(), "spannerTrack2", 1);
    EXPECT_EQ(slur->track2(), 0u);

    setInCommand(score, apiSlur.get(), "voice", 1);
    EXPECT_EQ(slur->track(), 0u);

    apiv1::Cursor cursor(score);
    apiv1::Fraction endTick(Fraction(1, 4));
    cursor.rewindToFraction(&endTick);
    cursor.setTrack(1);
    cursor.setDuration(1, 4);
    score->startCmd(TranslatableString::untranslatable("Engraving API score element tests"));
    cursor.addNote(67);
    score->endCmd();

    setInCommand(score, apiSlur.get(), "spannerTrack2", 1);
    EXPECT_EQ(slur->track2(), 1u);
    ASSERT_TRUE(slur->endCR());
    EXPECT_EQ(slur->endCR()->track(), 1u);

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    apiSlur.reset();
    delete score;
}

// lines change voice only where the app puts them in a voice: hairpins, trills and Let Ring
TEST(Engraving_ApiScoreElementTests, lineVoice)
{
    MasterScore* score = readScoreWithQuarters();
    ASSERT_TRUE(score);
    Chord* first = chordAt(score, Fraction(0, 1));
    Chord* second = chordAt(score, Fraction(1, 4));
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);

    score->startCmd(TranslatableString::untranslatable("Engraving API score element tests"));
    Transaction& tx = score->transactionManager()->currentOrDummyTransaction();
    Hairpin* hairpin = EditHairpin::addHairpin(tx, score, HairpinType::CRESC_HAIRPIN, first, second);
    Spanner* textLine = toSpanner(Factory::createItem(ElementType::TEXTLINE, score->dummy()));
    EditSpanner::addSpanner(tx, score, textLine, 0, first->segment(), second->segment());
    score->endCmd();
    ASSERT_TRUE(hairpin);
    std::unique_ptr<apiv1::EngravingItem> apiHairpin(apiv1::wrap(hairpin));
    std::unique_ptr<apiv1::EngravingItem> apiTextLine(apiv1::wrap(textLine));

    setInCommand(score, apiHairpin.get(), "voice", 1);
    EXPECT_EQ(hairpin->track(), 1u);
    EXPECT_EQ(hairpin->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>(), VoiceAssignment::CURRENT_VOICE_ONLY);

    setInCommand(score, apiTextLine.get(), "voice", 1);
    setInCommand(score, apiTextLine.get(), "track", 1);
    EXPECT_EQ(textLine->track(), 0u);

    apiHairpin.reset();
    apiTextLine.reset();
    delete score;
}

// a hairpin ends on its start track: another end track would only add an empty undo step
TEST(Engraving_ApiScoreElementTests, hairpinEndTrack)
{
    MasterScore* score = readScoreWithQuarters();
    ASSERT_TRUE(score);

    score->startCmd(TranslatableString::untranslatable("Engraving API score element tests"));
    Hairpin* hairpin = EditHairpin::addHairpin(score->transactionManager()->currentOrDummyTransaction(), score,
                                               HairpinType::CRESC_HAIRPIN, chordAt(score, Fraction(0, 1)),
                                               chordAt(score, Fraction(1, 4)));
    score->endCmd();
    ASSERT_TRUE(hairpin);
    std::unique_ptr<apiv1::EngravingItem> apiHairpin(apiv1::wrap(hairpin));

    const size_t undoIndex = score->undoStack()->currentIndex();
    setInCommand(score, apiHairpin.get(), "spannerTrack2", 1);
    EXPECT_EQ(score->undoStack()->currentIndex(), undoIndex);

    apiHairpin.reset();
    delete score;
}
