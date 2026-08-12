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

#include "engraving/dom/lyrics.h"
#include "engraving/dom/utils.h"
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

    EXPECT_TRUE(m2);

    ChordRest* endCR = m2->lastChordRest(0);

    EXPECT_TRUE(endCR);

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

    EXPECT_TRUE(l1);
    EXPECT_TRUE(l2);

    PartialLyricsLine* pll = findPrevPartialLyricsLineDash(l1);

    EXPECT_TRUE(pll);
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
