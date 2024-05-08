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

#include "dom/hairpin.h"
#include "dom/masterscore.h"

#include "engraving/compat/scoreaccess.h"
#include "utils/scorerw.h"

using namespace mu;
using namespace mu::engraving;

class Engraving_HairpinTests : public ::testing::Test
{
};

TEST_F(Engraving_HairpinTests, hairpin)
{
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);
    Hairpin* hp = new Hairpin(score->dummy()->segment());

    // subtype
    hp->setHairpinType(HairpinType::DECRESC_HAIRPIN);
    Hairpin* hp2 = static_cast<Hairpin*>(ScoreRW::writeReadElement(hp));
    EXPECT_EQ(hp2->hairpinType(), HairpinType::DECRESC_HAIRPIN);
    delete hp2;

    hp->setHairpinType(HairpinType::CRESC_HAIRPIN);
    hp2 = static_cast<Hairpin*>(ScoreRW::writeReadElement(hp));
    EXPECT_EQ(hp2->hairpinType(), HairpinType::CRESC_HAIRPIN);
    delete hp2;
}
