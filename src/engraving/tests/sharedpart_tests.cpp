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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/sharedpart.h"

using namespace mu::engraving;
using namespace muse;

class Engraving_SharedPartTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_score = compat::ScoreAccess::createMasterScore(nullptr);
    }

    void TearDown() override
    {
        delete m_score;
    }

    //! number: the instrument's number (0: none)
    Part* addPart(const String& name, int number = 0)
    {
        Part* part = new Part(m_score);
        part->setLongNameAll(name);
        part->instrument()->setNumber(number);
        m_score->appendPart(part);
        m_score->appendStaff(Factory::createStaff(part));

        return part;
    }

    //! name: the shared instrument's name
    SharedPart* addSharedPart(const std::vector<Part*>& parts, const String& name)
    {
        SharedPart* sharedPart = new SharedPart(m_score);
        sharedPart->setLongNameAll(name);
        m_score->appendPart(sharedPart);
        m_score->appendStaff(Factory::createStaff(sharedPart));

        for (Part* part : parts) {
            sharedPart->addOriginPart(part);
        }

        return sharedPart;
    }

    MasterScore* m_score = nullptr;
};

TEST_F(Engraving_SharedPartTests, PartName_FromInstrumentNumbers)
{
    // [GIVEN] Horns numbered 2 and 4
    const SharedPart* sharedPart = addSharedPart({ addPart(u"Horn", 2), addPart(u"Horn", 4) }, u"Horn");

    // [THEN] The shared part is named after their numbers
    EXPECT_EQ(sharedPart->partName(), u"Horn 2–4");
}

TEST_F(Engraving_SharedPartTests, PartName_FromNumbersInNames)
{
    // [GIVEN] Violins numbered only in their names (as in templates)
    const SharedPart* sharedPart = addSharedPart({ addPart(u"Violins 1"), addPart(u"Violins 2") }, u"Violins");

    // [THEN] The shared part is named after those numbers
    EXPECT_EQ(sharedPart->partName(), u"Violins 1–2");
}

TEST_F(Engraving_SharedPartTests, PartName_WithoutNumbers)
{
    // [GIVEN] Two flutes without numbers
    const SharedPart* sharedPart = addSharedPart({ addPart(u"Flute"), addPart(u"Flute") }, u"Flutes");

    // [THEN] The shared part has the shared instrument's name
    EXPECT_EQ(sharedPart->partName(), u"Flutes");
}

TEST_F(Engraving_SharedPartTests, PartName_SingleNumber)
{
    // [GIVEN] Trumpet 1 and a trumpet without a number
    const SharedPart* sharedPart = addSharedPart({ addPart(u"Trumpet", 1), addPart(u"Trumpet") }, u"Trumpets");

    // [THEN] There's no range: the shared part has the shared instrument's name
    EXPECT_EQ(sharedPart->partName(), u"Trumpets");
}

TEST_F(Engraving_SharedPartTests, PartName_MixedInstruments)
{
    // [GIVEN] A flute and an oboe
    const SharedPart* sharedPart = addSharedPart({ addPart(u"Flute", 1), addPart(u"Oboe", 2) }, u"Woodwinds");

    // [THEN] Their numbers don't make a range: the shared part has the shared instrument's name
    EXPECT_EQ(sharedPart->partName(), u"Woodwinds");
}

TEST_F(Engraving_SharedPartTests, PartName_KeepsTransposition)
{
    // [GIVEN] Two horns without numbers, on a shared horn in F
    SharedPart* sharedPart = addSharedPart({ addPart(u"Horn"), addPart(u"Horn") }, u"Horn");
    sharedPart->instrument()->setTransposition(u"F");

    // [THEN] The shared instrument's name includes its transposition
    EXPECT_EQ(sharedPart->partName(), u"Horn in F");
}
