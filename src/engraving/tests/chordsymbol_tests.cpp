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
#include "dom/durationtype.h"
#include "dom/excerpt.h"
#include "dom/harmony.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/part.h"
#include "dom/segment.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String CHORDSYMBOL_DATA_DIR("chordsymbol_data/");

class Engraving_ChordSymbolTests : public ::testing::Test
{
public:
    MasterScore* test_pre(const char16_t* p);
    void test_post(MasterScore* score, const char16_t* p);

    void selectAllChordSymbols(MasterScore* score);
    void realizeSelectionVoiced(MasterScore* score, Voicing voicing);
};

MasterScore* Engraving_ChordSymbolTests::test_pre(const char16_t* p)
{
    String p1 = CHORDSYMBOL_DATA_DIR + p + ".mscx";
    MasterScore* score = ScoreRW::readScore(p1);
    EXPECT_TRUE(score);
    score->doLayout();
    return score;
}

void Engraving_ChordSymbolTests::test_post(MasterScore* score, const char16_t* p)
{
    String p1 = p;
    p1 += u"-test.mscx";
    String p2 = CHORDSYMBOL_DATA_DIR + p + u"-ref.mscx";
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, p1, p2));
    delete score;
}

//---------------------------------------------------------
//   select all chord symbols within the specified score
//---------------------------------------------------------
void Engraving_ChordSymbolTests::selectAllChordSymbols(MasterScore* score)
{
    //find a chord symbol
    Segment* seg = score->firstSegment(SegmentType::ChordRest);
    EngravingItem* e = 0;
    while (seg) {
        e = seg->findAnnotation(ElementType::HARMONY,
                                0, score->ntracks());
        if (e) {
            break;
        }
        seg = seg->next1();
    }
    score->selectSimilar(e, false);
}

//---------------------------------------------------------
//   realize the current selection of the score using
//   the specified voicing
//---------------------------------------------------------
void Engraving_ChordSymbolTests::realizeSelectionVoiced(MasterScore* score, Voicing voicing)
{
    for (EngravingItem* e : score->selection().elements()) {
        if (e->isHarmony()) {
            e->setProperty(Pid::HARMONY_VOICING, int(voicing));
        }
    }
    score->startCmd(TranslatableString::untranslatable("Realize selection voiced"));
    score->cmdRealizeChordSymbols();
    score->endCmd();
}

TEST_F(Engraving_ChordSymbolTests, testExtend)
{
    MasterScore* score = test_pre(u"extend");
    Measure* m = score->firstMeasure();
    Segment* s = m->first(SegmentType::ChordRest);
    ChordRest* cr = s->cr(0);
    score->changeCRlen(cr, DurationType::V_WHOLE);
    score->doLayout();
    test_post(score, u"extend");
}

TEST_F(Engraving_ChordSymbolTests, testClear)
{
    MasterScore* score = test_pre(u"clear");
    Measure* m = score->firstMeasure();
    score->select(m, SelectType::SINGLE, 0);
    score->cmdDeleteSelection();
    score->doLayout();
    test_post(score, u"clear");
}

TEST_F(Engraving_ChordSymbolTests, testAddLink)
{
    MasterScore* score = test_pre(u"add-link");
    Segment* seg = score->firstSegment(SegmentType::ChordRest);
    ChordRest* cr = seg->cr(0);
    Harmony* harmony = new Harmony(cr->segment());
    harmony->setHarmony(u"C7");
    harmony->setTrack(cr->track());
    harmony->setParent(cr->segment());
    score->undoAddElement(harmony);
    score->doLayout();
    test_post(score, u"add-link");
}

TEST_F(Engraving_ChordSymbolTests, testAddPart)
{
    MasterScore* score = test_pre(u"add-part");
    Segment* seg = score->firstSegment(SegmentType::ChordRest);
    ChordRest* cr = seg->cr(0);
    Harmony* harmony = new Harmony(cr->segment());
    harmony->setHarmony(u"C7");
    harmony->setTrack(cr->track());
    harmony->setParent(cr->segment());
    score->undoAddElement(harmony);
    score->doLayout();
    test_post(score, u"add-part");
}

TEST_F(Engraving_ChordSymbolTests, testNoSystem)
{
    MasterScore* score = test_pre(u"no-system");

    //
    // create first part
    //
    std::vector<Part*> parts;
    parts.push_back(score->parts().at(0));
    Score* nscore = score->createScore();

    Excerpt* ex = new Excerpt(score);
    ex->setExcerptScore(nscore);
    nscore->setExcerpt(ex);
    score->excerpts().push_back(ex);
    ex->setName(parts.front()->longName());
    ex->setParts(parts);
    Excerpt::createExcerpt(ex);

//      nscore->setTitle(parts.front()->partName());
    nscore->style().set(Sid::createMultiMeasureRests, true);

    //
    // create second part
    //
    parts.clear();
    parts.push_back(score->parts().at(1));
    nscore = score->createScore();

    ex = new Excerpt(score);
    ex->setExcerptScore(nscore);
    nscore->setExcerpt(ex);
    score->excerpts().push_back(ex);
    ex->setName(parts.front()->longName());
    ex->setParts(parts);
    Excerpt::createExcerpt(ex);

//      nscore->setTitle(parts.front()->partName());
    nscore->style().set(Sid::createMultiMeasureRests, true);

    score->setExcerptsChanged(true);
    score->doLayout();
    test_post(score, u"no-system");
}

TEST_F(Engraving_ChordSymbolTests, testTranspose)
{
    MasterScore* score = test_pre(u"transpose");
    score->startCmd(TranslatableString::untranslatable("Engraving chord symbol tests"));
    score->cmdSelectAll();
    score->transpose(TransposeMode::BY_INTERVAL, TransposeDirection::UP, Key::C, 4, false, true, true);
    score->endCmd();
    test_post(score, u"transpose");
}

TEST_F(Engraving_ChordSymbolTests, testTransposePart)
{
    MasterScore* score = test_pre(u"transpose-part");
    score->startCmd(TranslatableString::untranslatable("Engraving chord symbol tests"));
    score->cmdSelectAll();
    score->transpose(TransposeMode::BY_INTERVAL, TransposeDirection::UP, Key::C, 4, false, true, true);
    score->endCmd(false, /*layoutAllParts = */ true);
    test_post(score, u"transpose-part");
}

//---------------------------------------------------------
//   check close voicing algorithm
//---------------------------------------------------------
TEST_F(Engraving_ChordSymbolTests, testRealizeClose)
{
    MasterScore* score = test_pre(u"realize");
    selectAllChordSymbols(score);
    realizeSelectionVoiced(score, Voicing::CLOSE);
    test_post(score, u"realize-close");
}

//---------------------------------------------------------
//   check Drop 2 voicing algorithm
//---------------------------------------------------------
TEST_F(Engraving_ChordSymbolTests, testRealizeDrop2)
{
    MasterScore* score = test_pre(u"realize");
    selectAllChordSymbols(score);
    realizeSelectionVoiced(score, Voicing::DROP_2);
    test_post(score, u"realize-drop2");
}

//---------------------------------------------------------
//   check 3 note voicing algorithm
//---------------------------------------------------------
TEST_F(Engraving_ChordSymbolTests, testRealize3Note)
{
    MasterScore* score = test_pre(u"realize");
    selectAllChordSymbols(score);
    realizeSelectionVoiced(score, Voicing::THREE_NOTE);
    test_post(score, u"realize-3note");
}

//---------------------------------------------------------
//   check 4 note voicing algorithm
//---------------------------------------------------------
TEST_F(Engraving_ChordSymbolTests, testRealize4Note)
{
    MasterScore* score = test_pre(u"realize");
    selectAllChordSymbols(score);
    realizeSelectionVoiced(score, Voicing::FOUR_NOTE);
    test_post(score, u"realize-4note");
}

//---------------------------------------------------------
//   check 6 note voicing algorithm
//---------------------------------------------------------
TEST_F(Engraving_ChordSymbolTests, testRealize6Note)
{
    MasterScore* score = test_pre(u"realize");
    selectAllChordSymbols(score);
    realizeSelectionVoiced(score, Voicing::SIX_NOTE);
    test_post(score, u"realize-6note");
}

//---------------------------------------------------------
//   Check if the note pitches and tpcs are correct after realizing
//   chord symbols on transposed instruments.
//---------------------------------------------------------
TEST_F(Engraving_ChordSymbolTests, testRealizeConcertPitch)
{
    MasterScore* score = test_pre(u"realize-concert-pitch");
    //concert pitch off
    score->startCmd(TranslatableString::untranslatable("Engraving chord symbol tests"));
    score->cmdConcertPitchChanged(false);
    score->endCmd();

    //realize all chord symbols
    selectAllChordSymbols(score);
    score->startCmd(TranslatableString::untranslatable("Engraving chord symbol tests"));
    score->cmdRealizeChordSymbols();
    score->endCmd();
    test_post(score, u"realize-concert-pitch");
}

//---------------------------------------------------------
//   Check if the note pitches and tpcs are correct after
//   transposing the score
//---------------------------------------------------------
TEST_F(Engraving_ChordSymbolTests, testRealizeTransposed)
{
    MasterScore* score = test_pre(u"transpose");
    //transpose
    score->cmdSelectAll();
    score->transpose(TransposeMode::BY_INTERVAL, TransposeDirection::UP, Key::C, 4, false, true, true);

    //realize all chord symbols
    selectAllChordSymbols(score);
    score->startCmd(TranslatableString::untranslatable("Engraving chord symbol tests"));
    score->cmdRealizeChordSymbols();
    score->endCmd();
    test_post(score, u"realize-transpose");
}

//---------------------------------------------------------
//   Check for correctness when using the override
//   feature for realizing chord symbols
//---------------------------------------------------------
TEST_F(Engraving_ChordSymbolTests, testRealizeOverrides)
{
    MasterScore* score = test_pre(u"realize-override");
    //realize all chord symbols
    selectAllChordSymbols(score);
    score->startCmd(TranslatableString::untranslatable("Engraving chord symbol tests"));
    score->cmdRealizeChordSymbols(true, Voicing::ROOT_ONLY, HDuration::SEGMENT_DURATION);
    score->endCmd();
    test_post(score, u"realize-override");
}

//---------------------------------------------------------
//   Check for correctness when realizing chord symbols on triplets
//---------------------------------------------------------
TEST_F(Engraving_ChordSymbolTests, testRealizeTriplet)
{
    MasterScore* score = test_pre(u"realize-triplet");
    //realize all chord symbols
    selectAllChordSymbols(score);
    score->startCmd(TranslatableString::untranslatable("Engraving chord symbol tests"));
    score->cmdRealizeChordSymbols();
    score->endCmd();
    test_post(score, u"realize-triplet");
}

//---------------------------------------------------------
//   Check for correctness when realizing chord symbols
//   with different durations
//---------------------------------------------------------
TEST_F(Engraving_ChordSymbolTests, testRealizeDuration)
{
    MasterScore* score = test_pre(u"realize-duration");
    //realize all chord symbols
    selectAllChordSymbols(score);
    score->startCmd(TranslatableString::untranslatable("Engraving chord symbol tests"));
    score->cmdRealizeChordSymbols();
    score->endCmd();
    test_post(score, u"realize-duration");
}

//---------------------------------------------------------
//   Check for correctness when realizing chord symbols
//   with jazz mode
//---------------------------------------------------------
TEST_F(Engraving_ChordSymbolTests, testRealizeJazz)
{
    MasterScore* score = test_pre(u"realize-jazz");
    //realize all chord symbols
    selectAllChordSymbols(score);
    score->startCmd(TranslatableString::untranslatable("Engraving chord symbol tests"));
    score->cmdRealizeChordSymbols();
    score->endCmd();
    test_post(score, u"realize-jazz");
}

TEST_F(Engraving_ChordSymbolTests, testNashvilleNumbers) {
    bool use302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;
    MasterScore* score = test_pre(u"nashville-numbers");
    selectAllChordSymbols(score);

    static const std::array<std::pair<int, String>, 23> tpcAndExtension { {
        { 14, u"" },
        { 16, u"" },
        { 18, u"" },
        { 13, u"" },
        { 16, u"" },
        { 18, u"" },
        { 20, u"" },
        { 15, u"" },
        { 10, u"" },
        { 12, u"" },
        { 14, u"" },
        { 9, u"" },
        { 5, u"" },
        { 7, u"" },
        { 9, u"" },
        { 4, u"" },
        { 12, u"m" },
        { 14, u"o" },
        { 16, u"o7#11" },
        { 14, u"6" },
        { 14, u"69" },
        { 18, u"sus" },
        { 13, u"6" }
    } };

    size_t idx = 0;

    std::vector<EngravingItem*> els = score->selection().elements(ElementType::HARMONY);

    ASSERT_EQ(els.size(), tpcAndExtension.size());

    for (EngravingItem* e : els) {
        Harmony* h = toHarmony(e);

        int tpc = tpcAndExtension.at(idx).first;
        String ext = tpcAndExtension.at(idx).second;

        EXPECT_EQ(tpc, h->rootTpc());
        EXPECT_EQ(ext, h->hTextName());

        idx++;
    }

    MScore::useRead302InTestMode = use302;
}
