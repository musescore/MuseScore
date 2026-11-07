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

#include <gtest/gtest.h>

#include "engraving/dom/dynamic.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/measure.h"

#include "engraving/editing/editposition.h"
#include "engraving/editing/transaction/transaction.h"

#include "utils/scorerw.h"

using namespace mu;
using namespace mu::engraving;

static const String FREEZE_DATA_DIR(u"freeze_data/");

class Engraving_FreezeTests : public ::testing::Test
{
};

static void freezeArticulationsAndCheckPositions(const Fraction& chordTick, bool freezeAll)
{
    SCOPED_TRACE(chordTick.toString().toStdString());

    MasterScore* score = ScoreRW::readScore(FREEZE_DATA_DIR + u"articulations.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Chord* chord = score->firstMeasure()->findChord(chordTick, 0);
    ASSERT_TRUE(chord);
    const std::vector<Articulation*> articulations = chord->articulations();
    ASSERT_FALSE(articulations.empty());

    std::vector<PointF> positionsBefore;
    std::vector<PointF> offsetsBefore;
    positionsBefore.reserve(articulations.size());
    offsetsBefore.reserve(articulations.size());
    for (Articulation* a : articulations) {
        positionsBefore.push_back(a->pagePos());
        offsetsBefore.push_back(a->offset());
    }

    if (freezeAll) {
        score->select(std::vector<EngravingItem*>(articulations.begin(), articulations.end()), SelectType::ADD);
    } else {
        score->select(articulations.front(), SelectType::SINGLE);
    }

    const std::vector<EngravingItem*> selected = score->selection().elements();
    ASSERT_FALSE(selected.empty());

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving freeze tests"), [&](Transaction& transaction) {
        EditPosition::freezeItemsPositions(transaction, selected);
    });

    for (size_t i = 0; i < articulations.size(); ++i) {
        const Articulation* a = articulations.at(i);
        SCOPED_TRACE("articulation " + a->subtypeUserName().str.toStdString());
        EXPECT_TRUE(muse::RealIsEqual(a->pagePos().y(), positionsBefore.at(i).y()));
    }

    EditData ed;
    score->undoRedo(true, &ed);

    for (size_t i = 0; i < articulations.size(); ++i) {
        const Articulation* a = articulations.at(i);
        SCOPED_TRACE("undo articulation " + a->subtypeUserName().str.toStdString());
        EXPECT_TRUE(a->autoplace());
        EXPECT_EQ(a->offset(), offsetsBefore.at(i));
        EXPECT_EQ(a->pagePos(), positionsBefore.at(i));
    }

    delete score;
}

TEST_F(Engraving_FreezeTests, freezeBottomArticulation)
{
    freezeArticulationsAndCheckPositions(Fraction(0, 1), false);
    freezeArticulationsAndCheckPositions(Fraction(1, 4), false);
}

TEST_F(Engraving_FreezeTests, freezeAllArticulations)
{
    freezeArticulationsAndCheckPositions(Fraction(0, 1), true);
    freezeArticulationsAndCheckPositions(Fraction(1, 4), true);
}

static Dynamic* findFirstDynamic(Score* score)
{
    for (Segment* seg = score->firstSegment(SegmentType::ChordRest); seg; seg = seg->next1(SegmentType::ChordRest)) {
        for (EngravingItem* annotation : seg->annotations()) {
            if (annotation->isDynamic()) {
                return toDynamic(annotation);
            }
        }
    }
    return nullptr;
}

TEST_F(Engraving_FreezeTests, freezeDynamic)
{
    MasterScore* score = ScoreRW::readScore(FREEZE_DATA_DIR + u"dynamic.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Dynamic* dynamic = findFirstDynamic(score);
    ASSERT_TRUE(dynamic);

    const PointF dynamicPosBefore = dynamic->pagePos();
    const PointF dynamicOffsetBefore = dynamic->offset();

    score->select(dynamic, SelectType::SINGLE);

    const std::vector<EngravingItem*> selected = score->selection().elements();
    ASSERT_EQ(selected.size(), 1);

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving freeze tests"), [&](Transaction& transaction) {
        EditPosition::freezeItemsPositions(transaction, selected);
    });

    EXPECT_EQ(dynamic->pagePos(), dynamicPosBefore);

    EditData ed;
    score->undoRedo(true, &ed);

    EXPECT_TRUE(dynamic->autoplace());
    EXPECT_EQ(dynamic->offset(), dynamicOffsetBefore);
    EXPECT_EQ(dynamic->pagePos(), dynamicPosBefore);

    delete score;
}

TEST_F(Engraving_FreezeTests, freezeDynamicAndHairpin)
{
    MasterScore* score = ScoreRW::readScore(FREEZE_DATA_DIR + u"hairpin.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    Dynamic* dynamic = findFirstDynamic(score);
    ASSERT_TRUE(dynamic);

    Hairpin* hairpin = nullptr;
    for (const auto& [tick, spanner] : score->spannerMap().map()) {
        if (spanner->isHairpin()) {
            hairpin = toHairpin(spanner);
            break;
        }
    }
    ASSERT_TRUE(hairpin);
    ASSERT_FALSE(hairpin->spannerSegments().empty());
    HairpinSegment* hairpinSegment = toHairpinSegment(hairpin->frontSegment());

    const PointF dynamicPosBefore = dynamic->pagePos();
    const PointF dynamicOffsetBefore = dynamic->offset();
    const PointF hairpinPosBefore = hairpinSegment->pagePos();
    const PointF hairpinOffsetBefore = hairpinSegment->offset();
    const PointF hairpinOffset2Before = hairpinSegment->userOff2();
    const PointF hairpinPos2Before = hairpinSegment->pos2();

    score->select(dynamic, SelectType::SINGLE);
    score->select(hairpinSegment, SelectType::ADD);

    const std::vector<EngravingItem*> selected = score->selection().elements();
    ASSERT_EQ(selected.size(), 2);

    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving freeze tests"), [&](Transaction& transaction) {
        EditPosition::freezeItemsPositions(transaction, selected);
    });

    ASSERT_FALSE(hairpin->spannerSegments().empty());
    hairpinSegment = toHairpinSegment(hairpin->frontSegment());

    EXPECT_EQ(dynamic->pagePos(), dynamicPosBefore);
    EXPECT_EQ(hairpinSegment->pagePos(), hairpinPosBefore);
    // Unsnapping from the dynamic moves the hairpin's layout end point, so offset2 must compensate
    EXPECT_NE(hairpinSegment->userOff2(), hairpinOffset2Before);
    EXPECT_EQ(hairpinSegment->pos2(), hairpinPos2Before);

    EditData ed;
    score->undoRedo(true, &ed);

    ASSERT_FALSE(hairpin->spannerSegments().empty());
    hairpinSegment = toHairpinSegment(hairpin->frontSegment());

    EXPECT_TRUE(dynamic->autoplace());
    EXPECT_EQ(dynamic->offset(), dynamicOffsetBefore);
    EXPECT_EQ(dynamic->pagePos(), dynamicPosBefore);
    EXPECT_TRUE(hairpinSegment->autoplace());
    EXPECT_EQ(hairpinSegment->offset(), hairpinOffsetBefore);
    EXPECT_EQ(hairpinSegment->userOff2(), hairpinOffset2Before);
    EXPECT_EQ(hairpinSegment->pagePos(), hairpinPosBefore);
    EXPECT_EQ(hairpinSegment->pos2(), hairpinPos2Before);

    delete score;
}
