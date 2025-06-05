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

#include "dom/keysig.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/part.h"
#include "dom/undo.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String KEYSIG_DATA_DIR("keysig_data/");

class Engraving_KeySigTests : public ::testing::Test
{
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
    EXPECT_TRUE(score);
    Measure* m2 = score->firstMeasure()->nextMeasure();
    EXPECT_TRUE(m2);

    // add a key signature (D major) in measure 2
    KeySigEvent ke2;
    ke2.setConcertKey(Key::D);
    score->startCmd(TranslatableString::untranslatable("Key signature tests"));
    score->undoChangeKeySig(score->staff(0), m2->tick(), ke2);
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // change key signature in measure 2 to E flat major
    KeySigEvent ke_3;
    ke_3.setConcertKey(Key(-3));
    score->startCmd(TranslatableString::untranslatable("Key signature tests"));
    score->undoChangeKeySig(score->staff(0), m2->tick(), ke_3);
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

    // remove key signature in measure 2
    Segment* s = m2->first();
    while (!(s->isKeySigType())) {
        s = s->next();
    }
    EngravingItem* e = s->element(0);
    score->startCmd(TranslatableString::untranslatable("Key signature tests"));
    score->undoRemoveElement(e);
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile3, reference3));

    // undo remove
    EditData ed;
    score->undoStack()->undo(&ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile4, reference4));

    // undo change
    score->undoStack()->undo(&ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile5, reference5));

    // undo add
    score->undoStack()->undo(&ed);
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
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    EXPECT_TRUE(m1);
    Measure* m2 = m1->nextMeasure();
    EXPECT_TRUE(m2);
    Measure* m3 = m2->nextMeasure();
    EXPECT_TRUE(m3);

    // verify no keysig exists in segment of final tick of m1, m2, m3
    EXPECT_EQ(m1->findSegment(SegmentType::KeySig, m1->endTick()), nullptr) << "Should be no keysig at end of measure 1.";
    EXPECT_EQ(m2->findSegment(SegmentType::KeySig, m2->endTick()), nullptr) << "Should be no keysig at end of measure 2.";
    EXPECT_EQ(m3->findSegment(SegmentType::KeySig, m3->endTick()), nullptr) << "Should be no keysig at end of measure 3.";

    delete score;
}

TEST_F(Engraving_KeySigTests, concertPitch)
{
    MasterScore* score = ScoreRW::readScore(KEYSIG_DATA_DIR + "concert-pitch.mscx");
    EXPECT_TRUE(score);

    score->cmdConcertPitchChanged(true);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"concert-pitch-01-test.mscx", KEYSIG_DATA_DIR + u"concert-pitch-01-ref.mscx"));
    score->cmdConcertPitchChanged(false);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"concert-pitch-02-test.mscx", KEYSIG_DATA_DIR + u"concert-pitch-02-ref.mscx"));

    delete score;
}

TEST_F(Engraving_KeySigTests, preferSharpFlat)
{
    MasterScore* score1 = ScoreRW::readScore(KEYSIG_DATA_DIR + u"preferSharpFlat-1.mscx");
    EXPECT_TRUE(score1);
    auto parts = score1->parts();
    Part* part1 = parts[0];
    part1->setPreferSharpFlat(PreferSharpFlat::FLATS);
    score1->transpositionChanged(part1, part1->instrument(Fraction(0, 1))->transpose(), Fraction(0, 1), Fraction(16, 4));
    score1->update();
    score1->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score1, u"preferSharpFlat-1-test.mscx", KEYSIG_DATA_DIR + u"preferSharpFlat-1-ref.mscx"));
    delete score1;

    MasterScore* score2 = ScoreRW::readScore(KEYSIG_DATA_DIR + u"preferSharpFlat-2.mscx");
    EXPECT_TRUE(score2);
    score2->cmdSelectAll();
    score2->startCmd(TranslatableString::untranslatable("Key signature tests"));
    // transpose augmented unison up
    score2->transpose(TransposeMode::BY_INTERVAL, TransposeDirection::UP, Key::C, 1, true, true, true);
    score2->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score2, u"preferSharpFlat-2-test.mscx", KEYSIG_DATA_DIR + u"preferSharpFlat-2-ref.mscx"));
    delete score2;
}

TEST_F(Engraving_KeySigTests, keysigMode)
{
    MasterScore* score = ScoreRW::readScore(KEYSIG_DATA_DIR + u"keysigMode.mscx");
    EXPECT_TRUE(score);
    Measure* m1 = score->firstMeasure();
    KeySig* ke = toKeySig(m1->findSegment(SegmentType::KeySig, m1->tick())->element(0));
    ke->setProperty(Pid::KEYSIG_MODE, int(KeyMode::DORIAN));
    score->update();
    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"keysig03.mscx", KEYSIG_DATA_DIR + u"keysig03-ref.mscx"));
    delete score;
}
