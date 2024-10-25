/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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

#include "engraving/compat/scoreaccess.h"
#include "dom/factory.h"
#include "dom/harppedaldiagram.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/part.h"
#include "dom/segment.h"
#include "dom/undo.h"
#include "dom/pitchspelling.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

#include "log.h"

using namespace mu::engraving;

static const String HARPDIAGRAM_DATA_DIR(u"harpdiagrams_data/");

class Engraving_HarpDiagramTests : public ::testing::Test
{
};

TEST_F(Engraving_HarpDiagramTests, harpdiagram)
{
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    HarpPedalDiagram* diagram = Factory::createHarpPedalDiagram(score->dummy()->segment());

    HarpPedalDiagram* d;

    d = toHarpPedalDiagram(ScoreRW::writeReadElement(diagram));
    EXPECT_EQ(d->placement(), PlacementV::ABOVE);
    EXPECT_EQ(d->textStyleType(), TextStyleType::HARP_PEDAL_DIAGRAM);
    EXPECT_TRUE(d->isDiagram());
    delete d;

    diagram->setIsDiagram(false);
    d = toHarpPedalDiagram(ScoreRW::writeReadElement(diagram));
    EXPECT_EQ(d->placement(), PlacementV::BELOW);
    EXPECT_EQ(d->textStyleType(), TextStyleType::HARP_PEDAL_TEXT_DIAGRAM);
    EXPECT_FALSE(d->isDiagram());
    delete d;

    diagram->setPedal(HarpStringType::C, PedalPosition::SHARP);

    d = toHarpPedalDiagram((ScoreRW::writeReadElement(diagram)));
    EXPECT_FALSE(d->isTpcPlayable(Tpc::TPC_C));
    EXPECT_TRUE(d->isTpcPlayable(Tpc::TPC_C_S));
    EXPECT_FALSE(d->isTpcPlayable(Tpc::TPC_D_B));
    delete d;
}

TEST_F(Engraving_HarpDiagramTests, textdiagrams)
{
    const String initFile(HARPDIAGRAM_DATA_DIR + u"harpdiagram-blank.mscx");
    const String ref(HARPDIAGRAM_DATA_DIR + u"textdiagram01-ref.mscx");
    const String write(u"textdiagram-test01.mscx");

    MasterScore* score = ScoreRW::readScore(initFile);
    EXPECT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    Segment* s1 = m1->first(SegmentType::ChordRest);
    EXPECT_TRUE(s1);

    Measure* m2 = m1->nextMeasure();
    Segment* s2 = m2->first(SegmentType::ChordRest);
    EXPECT_TRUE(s2);

    Measure* m3 = m2->nextMeasure();
    Segment* s3 = m3->first(SegmentType::ChordRest);
    EXPECT_TRUE(s3);

    Measure* m4 = m3->nextMeasure();
    Segment* s4 = m4->first(SegmentType::ChordRest);
    EXPECT_TRUE(s4);

    // set first diagram all natural
    HarpPedalDiagram* diagram1 = Factory::createHarpPedalDiagram(s1);
    std::array<PedalPosition, HARP_STRING_NO> pedalState
        = { PedalPosition::NATURAL, PedalPosition::NATURAL, PedalPosition::NATURAL, PedalPosition::NATURAL, PedalPosition::NATURAL,
            PedalPosition::NATURAL, PedalPosition::NATURAL };
    diagram1->setIsDiagram(false);

    score->startCmd(TranslatableString::untranslatable("Harp diagram tests"));
    EditData dd1(0);
    dd1.dropElement = diagram1;
    EngravingItem* e1 = s1->firstElementForNavigation(0);
    e1->drop(dd1);
    score->undo(new ChangeHarpPedalState(diagram1, pedalState));
    score->endCmd();

    String diagramText = diagram1->xmlText();
    String expText
        =
            u"E<sym>csymAccidentalNatural</sym> F<sym>csymAccidentalNatural</sym> G<sym>csymAccidentalNatural</sym> A<sym>csymAccidentalNatural</sym> \nD<sym>csymAccidentalNatural</sym> C<sym>csymAccidentalNatural</sym> B<sym>csymAccidentalNatural</sym> ";
    EXPECT_EQ(expText, diagramText);

    // set C#, F#.  Text should be F#, C#
    HarpPedalDiagram* diagram2 = Factory::createHarpPedalDiagram(s2);
    diagram2->setIsDiagram(false);

    score->startCmd(TranslatableString::untranslatable("Harp diagram tests"));
    EditData dd2(0);
    dd2.dropElement = diagram2;
    EngravingItem* e2 = s2->firstElementForNavigation(0);
    e2->drop(dd2);
    score->undo(new ChangeSingleHarpPedal(diagram2, HarpStringType::C, PedalPosition::SHARP));
    score->undo(new ChangeSingleHarpPedal(diagram2, HarpStringType::F, PedalPosition::SHARP));
    score->endCmd();

    diagramText = diagram2->xmlText();
    expText = u"F<sym>csymAccidentalSharp</sym> \nC<sym>csymAccidentalSharp</sym> ";
    EXPECT_EQ(expText, diagramText);

    // set C#, F# again.  Text should be F#, C#
    HarpPedalDiagram* diagram3 = Factory::createHarpPedalDiagram(s3);
    diagram3->setIsDiagram(false);

    score->startCmd(TranslatableString::untranslatable("Harp diagram tests"));
    EditData dd3(0);
    dd3.dropElement = diagram3;
    EngravingItem* e3 = s3->firstElementForNavigation(0);
    e3->drop(dd3);
    score->endCmd();

    diagramText = diagram3->xmlText();
    expText = u"F<sym>csymAccidentalSharp</sym> \nC<sym>csymAccidentalSharp</sym> ";
    EXPECT_EQ(expText, diagramText);

    // set C#, F#, G#.  Text should be G#
    HarpPedalDiagram* diagram4 = Factory::createHarpPedalDiagram(s4);
    diagram4->setIsDiagram(false);

    score->startCmd(TranslatableString::untranslatable("Harp diagram tests"));
    EditData dd4(0);
    dd4.dropElement = diagram4;
    EngravingItem* e4 = s4->firstElementForNavigation(0);
    e4->drop(dd4);
    score->undo(new ChangeSingleHarpPedal(diagram4, HarpStringType::G, PedalPosition::SHARP));
    score->endCmd();

    diagramText = diagram4->xmlText();
    expText = u"G<sym>csymAccidentalSharp</sym> \n";
    EXPECT_EQ(expText, diagramText);

    // Finally check file is what's expected
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, write, ref));

    delete score;
}

// Add text diagram between two, see if text updates.  Then test undo.
TEST_F(Engraving_HarpDiagramTests, textdiagrams2)
{
    const String initFile(HARPDIAGRAM_DATA_DIR + u"textdiagram02.mscx");
    const String writeFile(u"textdiagram-test02.mscx");

    MasterScore* score = ScoreRW::readScore(initFile);
    EXPECT_TRUE(score);
    Measure* m2 = score->firstMeasure()->nextMeasure();
    Segment* s2 = m2->first(SegmentType::ChordRest);
    EXPECT_TRUE(s2);
    Part* p = score->partById(1);

    // check final diagram is Ab, Db
    HarpPedalDiagram* diagram1 = p->nextHarpDiagram(s2->tick());
    String expText("A<sym>csymAccidentalFlat</sym> \nD<sym>csymAccidentalFlat</sym> ");
    EXPECT_EQ(diagram1->xmlText(), expText);

    // insert diagram - all naturals
    HarpPedalDiagram* diagram2 = Factory::createHarpPedalDiagram(s2);
    diagram2->setIsDiagram(false);

    score->startCmd(TranslatableString::untranslatable("Harp diagram tests"));
    EditData dd(0);
    dd.dropElement = diagram2;
    EngravingItem* e = s2->firstElementForNavigation(0);
    e->drop(dd);
    score->undo(new ChangeHarpPedalState(diagram2,
                                         { PedalPosition::NATURAL, PedalPosition::NATURAL, PedalPosition::NATURAL, PedalPosition::NATURAL,
                                           PedalPosition::NATURAL,
                                           PedalPosition::NATURAL, PedalPosition::NATURAL }));
    score->endCmd();

    // Check last diagram has updated
    expText
        = u"E<sym>csymAccidentalFlat</sym> A<sym>csymAccidentalFlat</sym> \nD<sym>csymAccidentalFlat</sym> B<sym>csymAccidentalFlat</sym> ";

    EXPECT_EQ(diagram1->xmlText(), expText);

    // Test undo
    score->startCmd(TranslatableString::untranslatable("Harp diagram tests"));
    score->undoRedo(true, &dd);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile, initFile));
}

TEST_F(Engraving_HarpDiagramTests, testmap)
{
    const String initFile(HARPDIAGRAM_DATA_DIR + u"textdiagram02.mscx");

    MasterScore* score = ScoreRW::readScore(initFile);
    EXPECT_TRUE(score);
    Measure* m2 = score->firstMeasure()->nextMeasure();
    Segment* s2 = m2->first();
    EXPECT_TRUE(s2);

    Part* p = score->partById(1);

    HarpPedalDiagram* h1 = p->currentHarpDiagram(s2->tick());

    std::array<PedalPosition,
               HARP_STRING_NO> expState
        = { PedalPosition::NATURAL, PedalPosition::NATURAL, PedalPosition::FLAT, PedalPosition::FLAT, PedalPosition::NATURAL,
            PedalPosition::NATURAL, PedalPosition::NATURAL };
    EXPECT_EQ(expState, h1->getPedalState());

    HarpPedalDiagram* h2 = p->nextHarpDiagram(s2->tick());

    expState
        = { PedalPosition::FLAT, PedalPosition::NATURAL, PedalPosition::FLAT, PedalPosition::FLAT, PedalPosition::NATURAL,
            PedalPosition::NATURAL, PedalPosition::FLAT };
    EXPECT_EQ(expState, h2->getPedalState());

    HarpPedalDiagram* h3 = p->prevHarpDiagram(h2->tick());

    EXPECT_EQ(h3, h1);
}
