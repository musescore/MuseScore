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
#include "engraving/editing/editdynamic.h"
#include "engraving/editing/transaction/transaction.h"
#include "engraving/editing/transaction/undostack.h"

#include "engraving/compat/scoreaccess.h"
#include "utils/scorerw.h"

using namespace mu::engraving;

static const String DYNAMIC_DATA_DIR(u"dynamic_data/");

class Engraving_DynamicTests : public ::testing::Test
{
};

//---------------------------------------------------------
//    read write test
//---------------------------------------------------------

TEST_F(Engraving_DynamicTests, test1)
{
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Dynamic* dynamic = new Dynamic(score->dummy());
    dynamic->setDynamicType(DynamicType(1));

    Dynamic* d;

    dynamic->setDirection(DirectionV::UP);
    dynamic->setPropertyFlags(Pid::DIRECTION, PropertyFlags::UNSTYLED);
    d = toDynamic(ScoreRW::writeReadElement(dynamic));
    EXPECT_EQ(d->direction(), DirectionV::UP);
    delete d;

    dynamic->setDirection(DirectionV::DOWN);
    dynamic->setPropertyFlags(Pid::DIRECTION, PropertyFlags::UNSTYLED);
    d = toDynamic(ScoreRW::writeReadElement(dynamic));
    EXPECT_EQ(d->direction(), DirectionV::DOWN);
    delete d;

    dynamic->setVelocity(23);
    d = toDynamic(ScoreRW::writeReadElement(dynamic));
    EXPECT_EQ(d->velocity(), 23);
    delete d;

    dynamic->setVelocity(57);
    d = toDynamic(ScoreRW::writeReadElement(dynamic));
    EXPECT_EQ(d->velocity(), 57);
    delete d;

    dynamic->setProperty(Pid::VELOCITY, 23);
    d = toDynamic(ScoreRW::writeReadElement(dynamic));
    EXPECT_EQ(d->velocity(), 23);
    delete d;

    dynamic->setProperty(Pid::VELOCITY, 57);
    d = toDynamic(ScoreRW::writeReadElement(dynamic));
    EXPECT_EQ(d->velocity(), 57);
    delete d;

    dynamic->setProperty(Pid::AVOID_BARLINES, false);
    dynamic->setPropertyFlags(Pid::AVOID_BARLINES, PropertyFlags::UNSTYLED);
    d = toDynamic(ScoreRW::writeReadElement(dynamic));
    EXPECT_EQ(d->avoidBarLines(), false);

    dynamic->setProperty(Pid::MUSICAL_SYMBOLS_SCALE, 0.5);
    dynamic->setPropertyFlags(Pid::MUSICAL_SYMBOLS_SCALE, PropertyFlags::UNSTYLED);
    d = toDynamic(ScoreRW::writeReadElement(dynamic));
    EXPECT_EQ(d->symbolScale(), 0.5);

    dynamic->setProperty(Pid::CENTER_ON_NOTEHEAD, true);
    dynamic->setPropertyFlags(Pid::CENTER_ON_NOTEHEAD, PropertyFlags::UNSTYLED);
    d = toDynamic(ScoreRW::writeReadElement(dynamic));
    EXPECT_EQ(d->centerOnNotehead(), true);

    delete d;
}

TEST_F(Engraving_DynamicTests, increaseDecreaseSelectedDynamicsValues)
{
    // [GIVEN] p > f > fffff, with both hairpins initially diminuendos
    MasterScore* score = ScoreRW::readScore(DYNAMIC_DATA_DIR + u"increaseDecreaseDynamics.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    std::vector<Dynamic*> dynamics;
    for (const Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
        const Segment* seg = m->first(SegmentType::ChordRest);
        ASSERT_TRUE(seg);
        Dynamic* dynamic = toDynamic(seg->findAnnotation(ElementType::DYNAMIC, 0, 0));
        ASSERT_TRUE(dynamic);
        dynamics.push_back(dynamic);
    }
    ASSERT_EQ(dynamics.size(), 3);

    dynamics[1]->findAdjacentHairpins();
    Hairpin* leftHp = dynamics[1]->leftHairpin();
    Hairpin* rightHp = dynamics[1]->rightHairpin();
    ASSERT_TRUE(leftHp);
    ASSERT_TRUE(rightHp);
    ASSERT_EQ(leftHp->hairpinType(), HairpinType::DIM_HAIRPIN);
    ASSERT_EQ(rightHp->hairpinType(), HairpinType::DIM_HAIRPIN);

    // [WHEN] Increasing all selected dynamics
    score->cmdSelectAll();
    score->transactionManager()->transaction(TranslatableString::untranslatable("Increase dynamics test"), [&](Transaction& tx) {
        EditDynamic::increaseDecreaseSelectedDynamicsValues(tx, score, 1);
    });

    // [THEN] mp < ff < fffff (fffff can't go any higher)
    EXPECT_EQ(dynamics[0]->dynamicType(), DynamicType::MP);
    EXPECT_EQ(dynamics[1]->dynamicType(), DynamicType::FF);
    EXPECT_EQ(dynamics[2]->dynamicType(), DynamicType::FFFFF);
    EXPECT_EQ(dynamics[1]->xmlText(), Dynamic::dynamicText(DynamicType::FF));
    EXPECT_EQ(leftHp->hairpinType(), HairpinType::CRESC_HAIRPIN);
    EXPECT_EQ(rightHp->hairpinType(), HairpinType::CRESC_HAIRPIN);

    // [WHEN] Decreasing only the middle dynamic
    score->select(dynamics[1], SelectType::SINGLE);
    score->transactionManager()->transaction(TranslatableString::untranslatable("Decrease dynamics test"), [&](Transaction& tx) {
        EditDynamic::increaseDecreaseSelectedDynamicsValues(tx, score, -4);
    });

    // [THEN] mp > p < fffff
    EXPECT_EQ(dynamics[0]->dynamicType(), DynamicType::MP);
    EXPECT_EQ(dynamics[1]->dynamicType(), DynamicType::P);
    EXPECT_EQ(dynamics[2]->dynamicType(), DynamicType::FFFFF);
    EXPECT_EQ(leftHp->hairpinType(), HairpinType::DIM_HAIRPIN);
    EXPECT_EQ(rightHp->hairpinType(), HairpinType::CRESC_HAIRPIN);

    // [WHEN] Undoing both changes
    score->undoStack()->undo(nullptr);
    score->undoStack()->undo(nullptr);

    // [THEN] The original dynamics and hairpin types are restored
    EXPECT_EQ(dynamics[0]->dynamicType(), DynamicType::P);
    EXPECT_EQ(dynamics[1]->dynamicType(), DynamicType::F);
    EXPECT_EQ(dynamics[2]->dynamicType(), DynamicType::FFFFF);
    EXPECT_EQ(leftHp->hairpinType(), HairpinType::DIM_HAIRPIN);
    EXPECT_EQ(rightHp->hairpinType(), HairpinType::DIM_HAIRPIN);

    delete score;
}
