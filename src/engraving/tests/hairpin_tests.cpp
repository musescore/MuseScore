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

#include "engraving/dom/dynamic.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/editing/edithairpin.h"
#include "engraving/editing/transaction/transaction.h"
#include "engraving/editing/transaction/undostack.h"

#include "engraving/compat/scoreaccess.h"
#include "utils/scorerw.h"

using namespace mu::engraving;

static const String HAIRPIN_DATA_DIR(u"hairpin_data/");

class Engraving_HairpinTests : public ::testing::Test
{
};

TEST_F(Engraving_HairpinTests, hairpin)
{
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);
    Hairpin* hp = new Hairpin(score->dummy());

    // subtype
    hp->setHairpinType(HairpinType::DIM_HAIRPIN);
    Hairpin* hp2 = toHairpin(ScoreRW::writeReadElement(hp));
    EXPECT_EQ(hp2->hairpinType(), HairpinType::DIM_HAIRPIN);
    delete hp2;

    hp->setHairpinType(HairpinType::CRESC_HAIRPIN);
    hp2 = toHairpin(ScoreRW::writeReadElement(hp));
    EXPECT_EQ(hp2->hairpinType(), HairpinType::CRESC_HAIRPIN);
    delete hp2;
}

TEST_F(Engraving_HairpinTests, autoFlipHairpinsType)
{
    // [GIVEN] p < f > p, with both hairpins initially crescendos
    MasterScore* score = ScoreRW::readScore(HAIRPIN_DATA_DIR + u"autoFlipHairpins.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    const Measure* m2 = score->firstMeasure()->nextMeasure();
    ASSERT_TRUE(m2);
    Segment* seg = m2->first(SegmentType::ChordRest);
    ASSERT_TRUE(seg);
    Dynamic* dynamic = toDynamic(seg->findAnnotation(ElementType::DYNAMIC, 0, 0));
    ASSERT_TRUE(dynamic);
    ASSERT_EQ(dynamic->dynamicType(), DynamicType::F);

    dynamic->findAdjacentHairpins();
    Hairpin* leftHp = dynamic->leftHairpin();
    Hairpin* rightHp = dynamic->rightHairpin();
    ASSERT_TRUE(leftHp);
    ASSERT_TRUE(rightHp);
    ASSERT_EQ(leftHp->hairpinType(), HairpinType::CRESC_HAIRPIN);
    ASSERT_EQ(rightHp->hairpinType(), HairpinType::CRESC_HAIRPIN);

    // [WHEN] Auto-flipping around the f
    score->transactionManager()->transaction(TranslatableString::untranslatable("Auto flip hairpins test"), [&](Transaction& tx) {
        EditHairpin::autoFlipHairpinsType(tx, dynamic);
    });

    // [THEN] p < f > p
    EXPECT_EQ(leftHp->hairpinType(), HairpinType::CRESC_HAIRPIN);
    EXPECT_EQ(rightHp->hairpinType(), HairpinType::DIM_HAIRPIN);

    // [WHEN] Changing the f to pp and auto-flipping
    score->transactionManager()->transaction(TranslatableString::untranslatable("Auto flip hairpins test"), [&](Transaction& tx) {
        dynamic->undoChangeProperty(Pid::DYNAMIC_TYPE, DynamicType::PP);
        dynamic->undoChangeProperty(Pid::TEXT, Dynamic::dynamicText(DynamicType::PP));
        EditHairpin::autoFlipHairpinsType(tx, dynamic);
    });

    // [THEN] p > pp < p
    EXPECT_EQ(leftHp->hairpinType(), HairpinType::DIM_HAIRPIN);
    EXPECT_EQ(rightHp->hairpinType(), HairpinType::CRESC_HAIRPIN);

    // [WHEN] Undoing both changes
    score->undoStack()->undo(nullptr);
    EXPECT_EQ(dynamic->dynamicType(), DynamicType::F);
    EXPECT_EQ(leftHp->hairpinType(), HairpinType::CRESC_HAIRPIN);
    EXPECT_EQ(rightHp->hairpinType(), HairpinType::DIM_HAIRPIN);

    score->undoStack()->undo(nullptr);

    // [THEN] The original hairpin types are restored
    EXPECT_EQ(leftHp->hairpinType(), HairpinType::CRESC_HAIRPIN);
    EXPECT_EQ(rightHp->hairpinType(), HairpinType::CRESC_HAIRPIN);

    delete score;
}
