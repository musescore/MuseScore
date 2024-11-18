/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "dom/chordrest.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/segment.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String SPLIT_DATA_DIR("split_data/");

class Engraving_SplitTests : public ::testing::Test
{
public:
    void split(const char* file, const char* reference, int index = 2);
};

void Engraving_SplitTests::split(const char* f1, const char* ref, int index)
{
    MasterScore* score = ScoreRW::readScore(SPLIT_DATA_DIR + String::fromUtf8(f1));
    EXPECT_TRUE(score);
    Measure* m = score->firstMeasure();
    Segment* s = m->first(SegmentType::ChordRest);
    for (int i = 0; i < index; ++i) {
        s = s->next1(SegmentType::ChordRest);
    }
    ChordRest* cr = toChordRest(s->element(0));

    score->startCmd(TranslatableString::untranslatable("Engraving split tests"));
    score->cmdSplitMeasure(cr);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String::fromUtf8(f1), SPLIT_DATA_DIR + String::fromUtf8(ref)));
    delete score;
}

TEST_F(Engraving_SplitTests, split01)
{
    split("split01.mscx", "split01-ref.mscx");
}

TEST_F(Engraving_SplitTests, split02)
{
    split("split02.mscx", "split02-ref.mscx");
}

TEST_F(Engraving_SplitTests, split03)
{
    split("split03.mscx", "split03-ref.mscx");
}

TEST_F(Engraving_SplitTests, split04)
{
    split("split04.mscx", "split04-ref.mscx");
}

TEST_F(Engraving_SplitTests, split05)
{
    split("split05.mscx", "split05-ref.mscx");
}

TEST_F(Engraving_SplitTests, split06)
{
    split("split06.mscx", "split06-ref.mscx", 6);
}

TEST_F(Engraving_SplitTests, split07)
{
    split("split07.mscx", "split07-ref.mscx");
}

TEST_F(Engraving_SplitTests, split08)
{
    split("split08.mscx", "split08-ref.mscx");
}

TEST_F(Engraving_SplitTests, DISABLED_split183846) //  determine why pageWidth/pageHeight are missing!
{
    split("split183846-irregular-qn-qn-wn.mscx",          "split183846-irregular-qn-qn-wn-ref.mscx", 1);
    split("split183846-irregular-wn-wn.mscx",             "split183846-irregular-wn-wn-ref.mscx", 1);
    split("split183846-irregular-wn-wr-wn-hr-qr.mscx",    "split183846-irregular-wn-wr-wn-hr-qr-ref.mscx", 2);
    split("split183846-irregular-wr-wn-wr-hn-qn.mscx",    "split183846-irregular-wr-wn-wr-hn-qn-ref.mscx", 3);
    split("split183846-irregular-hn-hn-qn-qn-hn-hn.mscx", "split183846-irregular-hn-hn-qn-qn-hn-hn-ref.mscx", 5);
    split("split183846-irregular-verylong.mscx",          "split183846-irregular-verylong-ref.mscx", 7);
}

TEST_F(Engraving_SplitTests, split184061)
{
    split("split184061-no-tie.mscx", "split184061-no-tie-ref.mscx", 3);       // splitting on 11/16th the way though measure, but voice 2 has whole note which can't be divided into two durations
    split("split184061-keep-tie.mscx", "split184061-keep-tie-ref.mscx", 3);     // same, but this split-up whole note has a tie to the next measure...
    split("split184061-keep-tie-before-break-voice-4.mscx", "split184061-keep-tie-before-break-voice-4-ref.mscx",
          2);                                                                                                             // splitting 1/64th after middle of measure...voice 4 already has a tie that need to be preserved after splitting, and voice 2 has whole note that must be split up with triple-dotted
    split("split184061-other-inst-only-one-tie.mscx", "split184061-other-inst-only-one-tie-ref.mscx", 2);     // only the one tied note of the chord in the flute should still be tied over
}

TEST_F(Engraving_SplitTests, split295207)
{
    split("split295207.mscx", "split295207-ref.mscx", 5);
}
