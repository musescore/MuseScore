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
    const std::pair<const char*, const char*> cases[] = {
        { "sfzp", "<sym>dynamicSforzato</sym><sym>dynamicPiano</sym>" },
        { "sffzp", "<sym>dynamicSforzatoFF</sym><sym>dynamicPiano</sym>" },
        { "sfzpp", "<sym>dynamicSforzato</sym><sym>dynamicPP</sym>" },
        { "sfzppp", "<sym>dynamicSforzato</sym><sym>dynamicPPP</sym>" },
        { "sffzmp", "<sym>dynamicSforzatoFF</sym><sym>dynamicMP</sym>" },
        { "sffzmf", "<sym>dynamicSforzatoFF</sym><sym>dynamicMF</sym>" },
        { "sfpppp", "<sym>dynamicSforzando1</sym><sym>dynamicPPPP</sym>" },
        { "sffffz", "<sym>dynamicSforzando</sym><sym>dynamicFFFF</sym><sym>dynamicZ</sym>" },
        { "sfffffz", "<sym>dynamicSforzando</sym><sym>dynamicFFFFF</sym><sym>dynamicZ</sym>" },
        { "sffffffz", "<sym>dynamicSforzando</sym><sym>dynamicFFFFFF</sym><sym>dynamicZ</sym>" },
        { "sffffffzpp", "<sym>dynamicSforzando</sym><sym>dynamicFFFFFF</sym><sym>dynamicZ</sym><sym>dynamicPP</sym>" }
    };
    for (const auto& entry : cases) {
        const String expected = String::fromUtf8(entry.second);
        dynamic.setDynamicType(String::fromUtf8(entry.first));
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
    EXPECT_EQ(dynamic.xmlText(), Dynamic::dynamicText(DynamicType::SFPP));
    dynamic.setDynamicType(u"sfff");
    EXPECT_EQ(dynamic.dynamicType(), DynamicType::SFFF);
    EXPECT_EQ(dynamic.xmlText(), Dynamic::dynamicText(DynamicType::SFFF));
    dynamic.setDynamicType(u"sfffz");
    EXPECT_EQ(dynamic.dynamicType(), DynamicType::SFFFZ);
    EXPECT_EQ(dynamic.xmlText(), Dynamic::dynamicText(DynamicType::SFFFZ));
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
    dynamic.setDynamicType(u"sffffffzpp");
    EXPECT_EQ(dynamic.translatedSubtypeUserName(), u"sffffffzpp (sforzando ffffff, then pianissimo)");
    dynamic.setDynamicType(u"sffffz");
    EXPECT_EQ(dynamic.translatedSubtypeUserName(), u"sffffz");
    dynamic.setDynamicType(u"subito sfzp!");
    EXPECT_EQ(dynamic.xmlText(), u"subito <sym>dynamicSforzato</sym><sym>dynamicPiano</sym>!");
    for (const char* text : { "sempre", "sfzmpmore", "sfzm", "sffzmm", "sfffffffz", "sfffffffzpp", "sfzppppppp",
                              "<font face=\"sfzp\"/>dolce" }) {
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
