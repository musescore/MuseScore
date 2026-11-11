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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "engraving/dom/factory.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/utils.h"
#include "engraving/editing/editdata.h"
#include "engraving/editing/editlyrics.h"
#include "engraving/editing/transaction/transaction.h"
#include "utils/scorerw.h"

using namespace mu::engraving;

static const String LYRICS_DATA_DIR("lyrics_data/");

class Engraving_LyricsTests : public ::testing::Test
{
};

TEST_F(Engraving_LyricsTests, PartialLyricsLineVerse)
{
    // Check partial lyrics dash verses are linked to their ending lyric
    MasterScore* score = ScoreRW::readScore(LYRICS_DATA_DIR + u"partialLyricsLineVerse.mscx");

    Measure* m2 = score->first()->nextMeasure();

    ASSERT_TRUE(m2);

    ChordRest* endCR = m2->lastChordRest(0);

    ASSERT_TRUE(endCR);

    Lyrics* l1 = nullptr;
    Lyrics* l2 = nullptr;

    for (Lyrics* l : endCR->lyrics()) {
        if (l->verse() == 0) {
            l1 = l;
        }
        if (l->verse() == 1) {
            l2 = l;
        }
    }

    ASSERT_TRUE(l1);
    ASSERT_TRUE(l2);

    PartialLyricsLine* pll = findPrevPartialLyricsLineDash(l1);

    ASSERT_TRUE(pll);
    EXPECT_EQ(pll->verse(), 0);

    score->startCmd(TranslatableString::untranslatable("Lyrics tests"));
    pll->undoChangeProperty(Pid::VERSE, 1, PropertyFlags::UNSTYLED);
    score->endCmd();

    EXPECT_EQ(pll->verse(), 1);
    EXPECT_EQ(l1->verse(), 1);
    EXPECT_EQ(l2->verse(), 0);

    score->startCmd(TranslatableString::untranslatable("Lyrics tests"));
    l1->undoChangeProperty(Pid::VERSE, 0, PropertyFlags::UNSTYLED);
    score->endCmd();

    EXPECT_EQ(pll->verse(), 0);
    EXPECT_EQ(l1->verse(), 0);
    EXPECT_EQ(l2->verse(), 1);
}

// editLyrics.mscx, in quarters: m1 "a" "b" - -, m2 (start repeat) - - - -, m3 (1st volta, end repeat) - - - "c",
// m4 (2nd volta) - - - "e". All lyrics are verse 1, single, below

static Measure* measureAt(Score* score, size_t index)
{
    Measure* m = score->firstMeasure();
    for (size_t i = 0; m && i < index; ++i) {
        m = m->nextMeasure();
    }
    return m;
}

static ChordRest* chordRestAt(Score* score, size_t measureIndex, size_t index)
{
    size_t i = 0;
    for (Segment* s = measureAt(score, measureIndex)->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        if (i++ == index) {
            return toChordRest(s->element(0));
        }
    }
    return nullptr;
}

static Lyrics* lyricsAt(Score* score, size_t measureIndex, size_t index)
{
    return chordRestAt(score, measureIndex, index)->lyrics(0, PlacementV::BELOW);
}

static std::vector<PartialLyricsLine*> partialLyricsLines(Score* score)
{
    std::vector<PartialLyricsLine*> result;
    for (const auto& it : score->spanner()) {
        if (it.second->isPartialLyricsLine()) {
            result.push_back(toPartialLyricsLine(it.second));
        }
    }
    return result;
}

static Lyrics* addEmptyLyrics(Score* score, ChordRest* cr)
{
    Lyrics* lyrics = nullptr;
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving lyrics tests"), [&](Transaction&) {
        lyrics = Factory::createLyrics(cr);
        lyrics->setTrack(cr->track());
        score->undoAddElement(lyrics);
    });
    return lyrics;
}

// As when ending the edit of empty lyrics: returns their state, then removes them
static EditLyrics::InitialState removedEmptyLyricsState(Score* score, Lyrics* lyrics)
{
    const EditLyrics::InitialState state = EditLyrics::initialState(lyrics);
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving lyrics tests"), [&](Transaction&) {
        score->undoRemoveElement(lyrics);
    });
    return state;
}

// As NotationInteraction, adding new next lyrics in a separate transaction
static EditLyrics::MelismaEnds addMelisma(Score* score, const EditLyrics::InitialState& state)
{
    EditLyrics::MelismaEnds ends;
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving lyrics tests"), [&](Transaction& tx) {
        ends = EditLyrics::addMelisma(tx, score, state);
    });
    if (ends.isNewNextLyrics) {
        score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving lyrics tests"), [&](Transaction&) {
            score->undoAddElement(ends.nextLyrics);
        });
    }
    return ends;
}

TEST_F(Engraving_LyricsTests, CanNavigateToNextSyllable)
{
    MasterScore* score = ScoreRW::readScore(LYRICS_DATA_DIR + u"editLyrics.mscx");
    ASSERT_TRUE(score);

    EXPECT_TRUE(EditLyrics::canNavigateToNextSyllable(EditLyrics::initialState(lyricsAt(score, 0, 0))));
    // Across the end repeat
    EXPECT_TRUE(EditLyrics::canNavigateToNextSyllable(EditLyrics::initialState(lyricsAt(score, 2, 3))));
    // At the end of the score
    EXPECT_FALSE(EditLyrics::canNavigateToNextSyllable(EditLyrics::initialState(lyricsAt(score, 3, 3))));

    delete score;
}

TEST_F(Engraving_LyricsTests, NavigateToNextSyllableDashesToExistingLyrics)
{
    MasterScore* score = ScoreRW::readScore(LYRICS_DATA_DIR + u"editLyrics.mscx");
    ASSERT_TRUE(score);

    Lyrics* a = lyricsAt(score, 0, 0);
    Lyrics* b = lyricsAt(score, 0, 1);
    const EditLyrics::InitialState state = EditLyrics::initialState(a);

    Lyrics* toLyrics = nullptr;
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving lyrics tests"), [&](Transaction& tx) {
        toLyrics = EditLyrics::navigateToNextSyllable(tx, score, state);
    });

    EXPECT_EQ(toLyrics, b);
    EXPECT_EQ(a->syllabic(), LyricsSyllabic::BEGIN);
    EXPECT_EQ(b->syllabic(), LyricsSyllabic::END);

    delete score;
}

TEST_F(Engraving_LyricsTests, NavigateToNextSyllableCreatesLyrics)
{
    MasterScore* score = ScoreRW::readScore(LYRICS_DATA_DIR + u"editLyrics.mscx");
    ASSERT_TRUE(score);

    Lyrics* b = lyricsAt(score, 0, 1);
    const EditLyrics::InitialState state = EditLyrics::initialState(b);

    Lyrics* toLyrics = nullptr;
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving lyrics tests"), [&](Transaction& tx) {
        toLyrics = EditLyrics::navigateToNextSyllable(tx, score, state);
    });

    ASSERT_TRUE(toLyrics);
    EXPECT_EQ(toLyrics, lyricsAt(score, 0, 2));
    EXPECT_EQ(toLyrics->syllabic(), LyricsSyllabic::END);
    EXPECT_EQ(toLyrics->verse(), 0);
    EXPECT_EQ(toLyrics->textStyleType(), TextStyleType::LYRICS_ODD);
    EXPECT_EQ(b->syllabic(), LyricsSyllabic::BEGIN);

    // One undo step reverts it all
    EditData ed;
    score->undoRedo(true, &ed);
    EXPECT_FALSE(lyricsAt(score, 0, 2));
    EXPECT_EQ(b->syllabic(), LyricsSyllabic::SINGLE);

    delete score;
}

TEST_F(Engraving_LyricsTests, NavigateToNextSyllableBeforeRepeat)
{
    MasterScore* score = ScoreRW::readScore(LYRICS_DATA_DIR + u"editLyrics.mscx");
    ASSERT_TRUE(score);

    // The dash from the end of the 1st volta has no end syllable
    Lyrics* c = lyricsAt(score, 2, 3);
    const EditLyrics::InitialState state = EditLyrics::initialState(c);

    Lyrics* toLyrics = nullptr;
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving lyrics tests"), [&](Transaction& tx) {
        toLyrics = EditLyrics::navigateToNextSyllable(tx, score, state);
    });

    EXPECT_FALSE(toLyrics);
    EXPECT_EQ(c->syllabic(), LyricsSyllabic::BEGIN);
    EXPECT_FALSE(lyricsAt(score, 3, 0));

    delete score;
}

TEST_F(Engraving_LyricsTests, NavigateToNextSyllableAfterRepeatAddsPartialDash)
{
    MasterScore* score = ScoreRW::readScore(LYRICS_DATA_DIR + u"editLyrics.mscx");
    ASSERT_TRUE(score);

    // Dashing from empty lyrics at the start of the 2nd volta, which has no syllable before it to dash from
    ChordRest* initialCR = chordRestAt(score, 3, 0);
    const EditLyrics::InitialState state = removedEmptyLyricsState(score, addEmptyLyrics(score, initialCR));

    Lyrics* toLyrics = nullptr;
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving lyrics tests"), [&](Transaction& tx) {
        toLyrics = EditLyrics::navigateToNextSyllable(tx, score, state);
    });

    // The cursor doesn't advance
    ASSERT_TRUE(toLyrics);
    EXPECT_EQ(toLyrics->chordRest(), initialCR);
    EXPECT_EQ(toLyrics->syllabic(), LyricsSyllabic::END);

    const std::vector<PartialLyricsLine*> lines = partialLyricsLines(score);
    ASSERT_EQ(lines.size(), 1);
    EXPECT_FALSE(lines.front()->isEndMelisma());
    EXPECT_EQ(lines.front()->tick(), initialCR->tick());
    EXPECT_EQ(lines.front()->verse(), 0);

    delete score;
}

TEST_F(Engraving_LyricsTests, AddMelismaToExistingLyrics)
{
    MasterScore* score = ScoreRW::readScore(LYRICS_DATA_DIR + u"editLyrics.mscx");
    ASSERT_TRUE(score);

    Lyrics* a = lyricsAt(score, 0, 0);
    Lyrics* b = lyricsAt(score, 0, 1);
    const EditLyrics::InitialState state = EditLyrics::initialState(a);

    EditLyrics::MelismaEnds ends = addMelisma(score, state);

    EXPECT_EQ(ends.nextLyrics, b);
    EXPECT_EQ(ends.fromLyrics, a);
    // A one-chord melisma
    EXPECT_EQ(a->ticks(), Lyrics::TEMP_MELISMA_TICKS);
    EXPECT_EQ(a->syllabic(), LyricsSyllabic::SINGLE);
    EXPECT_EQ(b->syllabic(), LyricsSyllabic::SINGLE);

    delete score;
}

TEST_F(Engraving_LyricsTests, AddMelismaCreatesAndExtends)
{
    MasterScore* score = ScoreRW::readScore(LYRICS_DATA_DIR + u"editLyrics.mscx");
    ASSERT_TRUE(score);

    Lyrics* b = lyricsAt(score, 0, 1);

    const EditLyrics::InitialState bState = EditLyrics::initialState(b);
    EditLyrics::MelismaEnds ends = addMelisma(score, bState);

    ASSERT_TRUE(ends.nextLyrics);
    EXPECT_EQ(ends.nextLyrics, lyricsAt(score, 0, 2));
    EXPECT_EQ(ends.nextLyrics->syllabic(), LyricsSyllabic::SINGLE);
    EXPECT_EQ(ends.nextLyrics->textStyleType(), TextStyleType::LYRICS_ODD);
    EXPECT_EQ(ends.fromLyrics, b);

    // Extending again from the new lyrics, left empty, takes the melisma from b over its chord
    const EditLyrics::InitialState emptyState = removedEmptyLyricsState(score, ends.nextLyrics);
    ends = addMelisma(score, emptyState);

    ASSERT_TRUE(ends.nextLyrics);
    EXPECT_EQ(ends.nextLyrics, lyricsAt(score, 0, 3));
    EXPECT_EQ(ends.fromLyrics, b);
    EXPECT_EQ(b->ticks(), Fraction(1, 4));

    // Ending the edit of the new lyrics left empty rolls back only adding them
    EditData ed;
    score->undoRedo(true, &ed);
    EXPECT_FALSE(lyricsAt(score, 0, 3));
    EXPECT_EQ(b->ticks(), Fraction(1, 4));

    delete score;
}

TEST_F(Engraving_LyricsTests, AddMelismaBeforeRepeat)
{
    MasterScore* score = ScoreRW::readScore(LYRICS_DATA_DIR + u"editLyrics.mscx");
    ASSERT_TRUE(score);

    Lyrics* c = lyricsAt(score, 2, 3);
    const EditLyrics::InitialState state = EditLyrics::initialState(c);

    EditLyrics::MelismaEnds ends = addMelisma(score, state);

    // The melisma extends past the repeat, with nothing to edit next
    EXPECT_FALSE(ends.nextLyrics);
    EXPECT_EQ(ends.fromLyrics, c);
    EXPECT_EQ(c->ticks(), Lyrics::TEMP_MELISMA_TICKS);
    EXPECT_FALSE(lyricsAt(score, 3, 0));

    delete score;
}

TEST_F(Engraving_LyricsTests, AddMelismaAfterRepeatAddsPartialMelisma)
{
    MasterScore* score = ScoreRW::readScore(LYRICS_DATA_DIR + u"editLyrics.mscx");
    ASSERT_TRUE(score);

    ChordRest* initialCR = chordRestAt(score, 3, 0);
    const EditLyrics::InitialState state = removedEmptyLyricsState(score, addEmptyLyrics(score, initialCR));

    EditLyrics::MelismaEnds ends = addMelisma(score, state);

    ASSERT_TRUE(ends.nextLyrics);
    EXPECT_EQ(ends.nextLyrics, lyricsAt(score, 3, 1));
    EXPECT_FALSE(ends.fromLyrics);

    const std::vector<PartialLyricsLine*> lines = partialLyricsLines(score);
    ASSERT_EQ(lines.size(), 1);
    EXPECT_TRUE(lines.front()->isEndMelisma());
    EXPECT_EQ(lines.front()->tick(), initialCR->tick());
    EXPECT_EQ(lines.front()->ticks(), initialCR->ticks());

    delete score;
}

TEST_F(Engraving_LyricsTests, AddVerse)
{
    MasterScore* score = ScoreRW::readScore(LYRICS_DATA_DIR + u"editLyrics.mscx");
    ASSERT_TRUE(score);

    Lyrics* a = lyricsAt(score, 0, 0);
    const EditLyrics::InitialState state = EditLyrics::initialState(a);

    Lyrics* added = nullptr;
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving lyrics tests"), [&](Transaction& tx) {
        added = EditLyrics::addVerse(tx, score, state);
    });

    ASSERT_TRUE(added);
    EXPECT_EQ(added->chordRest(), a->chordRest());
    EXPECT_EQ(added->verse(), 1);
    EXPECT_EQ(added->placement(), a->placement());
    EXPECT_EQ(added->textStyleType(), TextStyleType::LYRICS_EVEN);
    EXPECT_EQ(a->chordRest()->lyrics(1, PlacementV::BELOW), added);

    delete score;
}

// Dashes from the lyrics to the next syllable
static Lyrics* dash(Score* score, Lyrics* lyrics)
{
    const EditLyrics::InitialState state = EditLyrics::initialState(lyrics);
    Lyrics* toLyrics = nullptr;
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving lyrics tests"), [&](Transaction& tx) {
        toLyrics = EditLyrics::navigateToNextSyllable(tx, score, state);
    });
    return toLyrics;
}

static EditLyrics::NavigatedLyrics navigateToLyrics(Score* score, Lyrics* lyrics, bool back, bool moveOnly)
{
    const EditLyrics::InitialState state = EditLyrics::initialState(lyrics);
    EditLyrics::NavigatedLyrics navigated;
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving lyrics tests"), [&](Transaction& tx) {
        navigated = EditLyrics::navigateToLyrics(tx, score, state, back, moveOnly);
    });
    return navigated;
}

TEST_F(Engraving_LyricsTests, CanNavigateToLyrics)
{
    MasterScore* score = ScoreRW::readScore(LYRICS_DATA_DIR + u"editLyrics.mscx");
    ASSERT_TRUE(score);

    EXPECT_TRUE(EditLyrics::canNavigateToLyrics(EditLyrics::initialState(lyricsAt(score, 0, 0)), false));
    EXPECT_TRUE(EditLyrics::canNavigateToLyrics(EditLyrics::initialState(lyricsAt(score, 0, 1)), true));
    // At the start and end of the score
    EXPECT_FALSE(EditLyrics::canNavigateToLyrics(EditLyrics::initialState(lyricsAt(score, 0, 0)), true));
    EXPECT_FALSE(EditLyrics::canNavigateToLyrics(EditLyrics::initialState(lyricsAt(score, 3, 3)), false));

    delete score;
}

TEST_F(Engraving_LyricsTests, NavigateToLyricsEndsWord)
{
    MasterScore* score = ScoreRW::readScore(LYRICS_DATA_DIR + u"editLyrics.mscx");
    ASSERT_TRUE(score);

    Lyrics* a = lyricsAt(score, 0, 0);
    Lyrics* b = lyricsAt(score, 0, 1);
    ASSERT_EQ(dash(score, a), b);

    const EditLyrics::NavigatedLyrics navigated = navigateToLyrics(score, a, false, false);

    EXPECT_EQ(navigated.lyrics, b);
    EXPECT_FALSE(navigated.isNew);
    EXPECT_EQ(a->syllabic(), LyricsSyllabic::SINGLE);
    EXPECT_EQ(b->syllabic(), LyricsSyllabic::SINGLE);

    delete score;
}

TEST_F(Engraving_LyricsTests, NavigateToLyricsMoveOnly)
{
    MasterScore* score = ScoreRW::readScore(LYRICS_DATA_DIR + u"editLyrics.mscx");
    ASSERT_TRUE(score);

    Lyrics* a = lyricsAt(score, 0, 0);
    Lyrics* b = lyricsAt(score, 0, 1);
    ASSERT_EQ(dash(score, a), b);

    const EditLyrics::NavigatedLyrics navigated = navigateToLyrics(score, a, false, true);

    EXPECT_EQ(navigated.lyrics, b);
    EXPECT_EQ(a->syllabic(), LyricsSyllabic::BEGIN);
    EXPECT_EQ(b->syllabic(), LyricsSyllabic::END);

    delete score;
}

TEST_F(Engraving_LyricsTests, NavigateToLyricsCreatesLyrics)
{
    MasterScore* score = ScoreRW::readScore(LYRICS_DATA_DIR + u"editLyrics.mscx");
    ASSERT_TRUE(score);

    const EditLyrics::NavigatedLyrics navigated = navigateToLyrics(score, lyricsAt(score, 0, 1), false, false);

    ASSERT_TRUE(navigated.lyrics);
    EXPECT_TRUE(navigated.isNew);
    EXPECT_EQ(navigated.lyrics, lyricsAt(score, 0, 2));
    EXPECT_EQ(navigated.lyrics->syllabic(), LyricsSyllabic::SINGLE);
    EXPECT_EQ(navigated.lyrics->verse(), 0);

    delete score;
}

TEST_F(Engraving_LyricsTests, NavigateToLyricsBack)
{
    MasterScore* score = ScoreRW::readScore(LYRICS_DATA_DIR + u"editLyrics.mscx");
    ASSERT_TRUE(score);

    Lyrics* a = lyricsAt(score, 0, 0);
    Lyrics* b = lyricsAt(score, 0, 1);
    ASSERT_EQ(dash(score, a), b);

    // Moving back leaves the dash
    const EditLyrics::NavigatedLyrics navigated = navigateToLyrics(score, b, true, false);

    EXPECT_EQ(navigated.lyrics, a);
    EXPECT_FALSE(navigated.isNew);
    EXPECT_EQ(a->syllabic(), LyricsSyllabic::BEGIN);
    EXPECT_EQ(b->syllabic(), LyricsSyllabic::END);

    delete score;
}

TEST_F(Engraving_LyricsTests, NavigateToVerse)
{
    MasterScore* score = ScoreRW::readScore(LYRICS_DATA_DIR + u"editLyrics.mscx");
    ASSERT_TRUE(score);

    Lyrics* a = lyricsAt(score, 0, 0);
    const EditLyrics::InitialState state = EditLyrics::initialState(a);

    Lyrics* verse2 = nullptr;
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving lyrics tests"), [&](Transaction& tx) {
        verse2 = EditLyrics::navigateToVerse(tx, score, state, 1);
    });

    ASSERT_TRUE(verse2);
    EXPECT_EQ(verse2->chordRest(), a->chordRest());
    EXPECT_EQ(verse2->verse(), 1);
    EXPECT_EQ(verse2->placement(), a->placement());
    // The style is copied from the lyrics navigated from
    EXPECT_EQ(verse2->textStyleType(), a->textStyleType());

    // Existing lyrics of the verse aren't duplicated
    Lyrics* again = nullptr;
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving lyrics tests"), [&](Transaction& tx) {
        again = EditLyrics::navigateToVerse(tx, score, state, 1);
    });
    EXPECT_EQ(again, verse2);

    delete score;
}
