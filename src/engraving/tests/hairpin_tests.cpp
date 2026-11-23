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

#include "engraving/dom/chord.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/mcursor.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/segment.h"

#include "engraving/compat/dummyelement.h"
#include "engraving/compat/scoreaccess.h"

#include "utils/scorerw.h"
#include "utils/testutils.h"

using namespace mu::engraving;

class Engraving_HairpinTests : public ::testing::Test
{
};

/// A laid-out score with one flute staff, holding a whole note in each of two measures
static MasterScore* createScore()
{
    MCursor c;
    c.setTimeSig(Fraction(4, 4));
    c.createScore(nullptr, u"test");
    c.addPart(u"flute");
    c.move(0, Fraction(0, 1));
    c.addTimeSig(Fraction(4, 4));
    c.addChord(72, TDuration(DurationType::V_WHOLE));
    c.addChord(74, TDuration(DurationType::V_WHOLE));

    MasterScore* score = c.score();
    score->doLayout();

    return score;
}

static void expectOneHairpinParentedInOwnScore(Score* score)
{
    std::vector<Hairpin*> hairpins;
    for (const auto& [tick, spanner] : score->spanner()) {
        if (spanner->isHairpin()) {
            hairpins.push_back(toHairpin(spanner));
        }
    }
    ASSERT_EQ(hairpins.size(), 1);
    EXPECT_EQ(hairpins.front()->parent()->score(), score);
}

TEST_F(Engraving_HairpinTests, hairpin)
{
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);
    Hairpin* hp = new Hairpin(score->dummy()->segment());

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

TEST_F(Engraving_HairpinTests, addHairpinWithPart)
{
    MasterScore* score = createScore();
    Score* part = TestUtils::createPart(score, 0);
    ASSERT_TRUE(part);

    ChordRest* cr = score->firstMeasure()->first(SegmentType::ChordRest)->cr(0);
    ASSERT_TRUE(cr && cr->isChord());
    score->select(toChord(cr)->upNote());

    score->startCmd(TranslatableString::untranslatable("Engraving hairpin tests"));
    score->addHairpins(HairpinType::CRESC_HAIRPIN);
    score->endCmd();

    expectOneHairpinParentedInOwnScore(score);
    expectOneHairpinParentedInOwnScore(part);

    delete score;
}

// A hairpin created on one of the dummy's stand-ins (here its segment) must still give each part's
// linked clone a parent in that part's own score. A clone that hangs on the main score's dummy
// outlives its part and crashes when the main score is deleted.
TEST_F(Engraving_HairpinTests, addHairpinOnDummySegmentWithPart)
{
    MasterScore* score = createScore();
    Score* part = TestUtils::createPart(score, 0);
    ASSERT_TRUE(part);

    ChordRest* cr = score->firstMeasure()->first(SegmentType::ChordRest)->cr(0);
    ASSERT_TRUE(cr);

    Hairpin* hairpin = Factory::createHairpin(score->dummy()->segment());
    hairpin->setHairpinType(HairpinType::CRESC_HAIRPIN);
    score->startCmd(TranslatableString::untranslatable("Engraving hairpin tests"));
    score->addHairpin(hairpin, cr);
    score->endCmd();

    expectOneHairpinParentedInOwnScore(score);
    expectOneHairpinParentedInOwnScore(part);

    // Keep the removed hairpins in the undo stack, as deleting them before closing the score does
    score->startCmd(TranslatableString::untranslatable("Engraving hairpin tests"));
    score->undoRemoveElement(hairpin);
    score->endCmd();
    EXPECT_TRUE(part->spanner().empty());

    delete score;
}

// Undoing the addition of a hairpin in the main score removes its linked clone from a part, where it can
// still be selected. Discarding the redo stack then deletes that clone, but not its segments, so the part's
// selection must not keep them (see Score::isLostWith()).
TEST_F(Engraving_HairpinTests, hairpinSelectedInPartIsDeselectedWhenDeleted)
{
    MasterScore* score = createScore();
    Score* part = TestUtils::createPart(score, 0);
    ASSERT_TRUE(part);

    const auto addHairpin = [score]() {
        ChordRest* cr = score->firstMeasure()->first(SegmentType::ChordRest)->cr(0);
        ASSERT_TRUE(cr && cr->isChord());
        score->select(toChord(cr)->upNote());
        score->startCmd(TranslatableString::untranslatable("Engraving hairpin tests"));
        score->addHairpins(HairpinType::CRESC_HAIRPIN);
        score->endCmd();
    };

    addHairpin();
    part->doLayout();

    Hairpin* partHairpin = nullptr;
    for (const auto& [tick, spanner] : part->spanner()) {
        if (spanner->isHairpin()) {
            partHairpin = toHairpin(spanner);
        }
    }
    ASSERT_TRUE(partHairpin);
    ASSERT_FALSE(partHairpin->spannerSegments().empty());
    part->select(partHairpin->frontSegment());
    ASSERT_EQ(part->selection().elements().size(), 1);

    score->undoRedo(true, nullptr);

    // Discards the redo stack, deleting the undone hairpin
    addHairpin();
    EXPECT_TRUE(part->selection().elements().empty());

    delete score;
}
