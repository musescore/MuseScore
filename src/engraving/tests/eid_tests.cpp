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
#include "engraving/dom/engravingitem.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"

#include "engraving/compat/scoreaccess.h"
#include "utils/scorerw.h"

using namespace mu::engraving;

static const String DATA_DIR("all_elements_data/");

class Engraving_EIDTests : public ::testing::Test
{
};

TEST_F(Engraving_EIDTests, testRegisteredItems)
{
    MasterScore* score = ScoreRW::readScore(DATA_DIR + u"random_elements.mscx");
    EXPECT_TRUE(score);

    auto checkRegister = [&](EngravingItem* item) {
        EID eid = item->eid();
        if (eid.isValid()) {
            EngravingObject* registeredItem = item->masterScore()->eidRegister()->itemFromEID(item->eid());
            EXPECT_TRUE(registeredItem);
            EXPECT_EQ(registeredItem, item);
        }
    };

    score->scanElements(checkRegister);
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        checkRegister(mb);
    }

    delete score;
}

TEST_F(Engraving_EIDTests, deletedItemIsUnregistered)
{
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    EngravingItem* item = new Dynamic(score->dummy()->segment());
    EID eid = item->assignNewEID();
    EXPECT_TRUE(eid.isValid());
    EXPECT_EQ(score->eidRegister()->itemFromEID(eid), item);

    delete item;

    // The register must not keep a stale entry for the deleted item; a later
    // allocation could reuse the same address and collide with the stale entry
    EXPECT_FALSE(score->eidRegister()->EIDFromItem(item).isValid());

    delete score;
}

TEST_F(Engraving_EIDTests, deletedExcerptScoreItemsAreUnregistered)
{
    MasterScore* master = compat::ScoreAccess::createMasterScore(nullptr);
    Score* partScore = master->createScore();

    Measure* measure = Factory::createMeasure(partScore->dummy()->system());
    partScore->measures()->append(measure);

    EID eid = measure->assignNewEID();
    EXPECT_TRUE(eid.isValid());
    EXPECT_EQ(master->eidRegister()->itemFromEID(eid), measure);

    // Deleting the part score (as when an excerpt is removed) must unregister
    // its items from the master score's register, which stays alive
    delete partScore;

    EXPECT_FALSE(master->eidRegister()->EIDFromItem(measure).isValid());

    delete master;
}

TEST_F(Engraving_EIDTests, writeReadElementDoesNotLeakRegisterEntries)
{
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Dynamic* dynamic = new Dynamic(score->dummy()->segment());
    dynamic->setDynamicType(DynamicType(1));

    // writeReadElement round trips through clipboard write and paste read,
    // which assigns a new EID to the newly created element
    Dynamic* d = toDynamic(ScoreRW::writeReadElement(dynamic));
    EID eid = d->eid();
    EXPECT_TRUE(eid.isValid());
    EXPECT_EQ(score->eidRegister()->itemFromEID(eid), d);

    delete d;

    EXPECT_FALSE(score->eidRegister()->EIDFromItem(d).isValid());

    delete dynamic;
    delete score;
}
