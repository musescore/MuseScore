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

#include "dom/dynamic.h"
#include "dom/masterscore.h"

#include "engraving/compat/scoreaccess.h"
#include "utils/scorerw.h"

using namespace mu::engraving;

class Engraving_DynamicTests : public ::testing::Test
{
};

//---------------------------------------------------------
//    read write test
//---------------------------------------------------------

TEST_F(Engraving_DynamicTests, test1)
{
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Dynamic* dynamic = new Dynamic(score->dummy()->segment());
    dynamic->setDynamicType(DynamicType(1));

    Dynamic* d;

    dynamic->setDirection(DirectionV::UP);
    dynamic->setPropertyFlags(Pid::DIRECTION, PropertyFlags::UNSTYLED);
    d = toDynamic(ScoreRW::writeReadElement(dynamic));
    EXPECT_EQ(d->direction(), DirectionV::UP);
    delete d;

    dynamic->setDirection(DirectionV::DOWN);
    dynamic->setPropertyFlags(Pid::DIRECTION, PropertyFlags::UNSTYLED);
    d = static_cast<Dynamic*>(ScoreRW::writeReadElement(dynamic));
    EXPECT_EQ(d->direction(), DirectionV::DOWN);
    delete d;

    dynamic->setVelocity(23);
    d = static_cast<Dynamic*>(ScoreRW::writeReadElement(dynamic));
    EXPECT_EQ(d->velocity(), 23);
    delete d;

    dynamic->setVelocity(57);
    d = static_cast<Dynamic*>(ScoreRW::writeReadElement(dynamic));
    EXPECT_EQ(d->velocity(), 57);
    delete d;

    dynamic->setProperty(Pid::VELOCITY, 23);
    d = static_cast<Dynamic*>(ScoreRW::writeReadElement(dynamic));
    EXPECT_EQ(d->velocity(), 23);
    delete d;

    dynamic->setProperty(Pid::VELOCITY, 57);
    d = static_cast<Dynamic*>(ScoreRW::writeReadElement(dynamic));
    EXPECT_EQ(d->velocity(), 57);
    delete d;

    dynamic->setProperty(Pid::AVOID_BARLINES, false);
    dynamic->setPropertyFlags(Pid::AVOID_BARLINES, PropertyFlags::UNSTYLED);
    d = static_cast<Dynamic*>(ScoreRW::writeReadElement(dynamic));
    EXPECT_EQ(d->avoidBarLines(), false);

    dynamic->setProperty(Pid::DYNAMICS_SIZE, 0.5);
    dynamic->setPropertyFlags(Pid::DYNAMICS_SIZE, PropertyFlags::UNSTYLED);
    d = static_cast<Dynamic*>(ScoreRW::writeReadElement(dynamic));
    EXPECT_EQ(d->dynamicsSize(), 0.5);

    dynamic->setProperty(Pid::CENTER_ON_NOTEHEAD, true);
    dynamic->setPropertyFlags(Pid::CENTER_ON_NOTEHEAD, PropertyFlags::UNSTYLED);
    d = static_cast<Dynamic*>(ScoreRW::writeReadElement(dynamic));
    EXPECT_EQ(d->centerOnNotehead(), true);

    delete d;
}
