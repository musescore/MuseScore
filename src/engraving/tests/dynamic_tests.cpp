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
#include <memory>

#include "engraving/dom/dynamic.h"
#include "engraving/dom/masterscore.h"

#include "engraving/compat/scoreaccess.h"
#include "utils/scorerw.h"

using namespace mu::engraving;

class Engraving_DynamicTests : public ::testing::Test
{
};

/** Checks compound styling, standard-dynamic priority, and preservation of ordinary text. */
TEST_F(Engraving_DynamicTests, compoundDynamics)
{
    std::unique_ptr<MasterScore> score(compat::ScoreAccess::createMasterScore(nullptr));
    Dynamic dynamic(score->dummy()->segment());
    for (const char* token : { "sfzp", "sffzp", "sfzpp", "sfzppp", "sffzmp", "sffzmf" }) {
        String expected;
        for (const char* letter = token; *letter; ++letter) {
            switch (*letter) {
            case 's': expected += u"<sym>dynamicSforzando</sym>";
                break;
            case 'f': expected += u"<sym>dynamicForte</sym>";
                break;
            case 'z': expected += u"<sym>dynamicZ</sym>";
                break;
            case 'm': expected += u"<sym>dynamicMezzo</sym>";
                break;
            case 'p': expected += u"<sym>dynamicPiano</sym>";
                break;
            }
        }
        dynamic.setDynamicType(String::fromUtf8(token));
        EXPECT_EQ(dynamic.dynamicType(), DynamicType::OTHER);
        EXPECT_EQ(dynamic.xmlText(), expected);
        dynamic.setDynamicType(dynamic.xmlText());
        EXPECT_EQ(dynamic.xmlText(), expected);
    }
    dynamic.setDynamicType(u"sfp");
    EXPECT_EQ(dynamic.translatedSubtypeUserName(), u"sfp (sforzando piano)");
    EXPECT_EQ(dynamic.dynamicType(), DynamicType::SFP);
    dynamic.setDynamicType(u"sfpp");
    EXPECT_EQ(dynamic.dynamicType(), DynamicType::SFPP);
    dynamic.setDynamicType(u"sffz");
    EXPECT_EQ(dynamic.dynamicType(), DynamicType::SFFZ);
    dynamic.setDynamicType(u"sfzpp");
    EXPECT_EQ(dynamic.translatedSubtypeUserName(), u"sfzpp (sforzando pianissimo)");
    EXPECT_TRUE(dynamic.screenReaderInfo().contains(u"sfzpp (sforzando pianissimo)"));
    dynamic.setDynamicType(u"sffzmp");
    EXPECT_EQ(dynamic.translatedSubtypeUserName(), u"sffzmp (sforzando fortissimo, then mezzo piano)");
    EXPECT_TRUE(dynamic.screenReaderInfo().contains(u"sforzando fortissimo, then mezzo piano"));
    dynamic.setDynamicType(u"sfzmp");
    EXPECT_EQ(dynamic.translatedSubtypeUserName(), u"sfzmp (sforzando mezzo piano)");
    dynamic.setDynamicType(u"sfffzpp");
    EXPECT_EQ(dynamic.translatedSubtypeUserName(), u"sfffzpp (sforzando forte fortissimo, then pianissimo)");
    dynamic.setDynamicType(u"subito sfzp!");
    EXPECT_EQ(dynamic.xmlText(), u"subito <sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicZ</sym><sym>dynamicPiano</sym>!");
    for (const char* text : { "sempre", "sfzmpmore", "sfzm", "sffzmm", "<font face=\"sfzp\"/>dolce" }) {
        dynamic.setDynamicType(String::fromUtf8(text));
        EXPECT_EQ(dynamic.xmlText(), String::fromUtf8(text));
    }
}

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
