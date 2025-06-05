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

#include "dom/engravingitem.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"
#include "utils/testutils.h"

using namespace mu;
using namespace mu::engraving;

static const String DATA_DIR("all_elements_data/");

class Engraving_EIDTests : public ::testing::Test
{
};

static void checkRegister(void*, EngravingItem* item)
{
    EID eid = item->eid();
    if (eid.isValid()) {
        EngravingObject* registeredItem = item->masterScore()->eidRegister()->itemFromEID(item->eid());
        EXPECT_TRUE(registeredItem);
        EXPECT_EQ(registeredItem, item);
    }
}

TEST_F(Engraving_EIDTests, testRegisteredItems)
{
    bool useRead302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;

    MasterScore* score = ScoreRW::readScore(DATA_DIR + u"random_elements.mscx");
    EXPECT_TRUE(score);

    score->scanElements(nullptr, checkRegister, true);
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        checkRegister(nullptr, mb);
    }

    delete score;
    MScore::useRead302InTestMode = useRead302;
}
