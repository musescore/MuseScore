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

#include "engraving/compat/dummyelement.h"
#include "engraving/compat/scoreaccess.h"

#include "engraving/dom/dynamic.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/system.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

static const String STAVECENTERING_DATA_DIR("stavecentering_data/");

class Engraving_StaveCenteringTests : public ::testing::Test
{
public:
    void SetUp() override
    {
        /* The mscx holds five parts, in this order:
         *
         *   staff 0, 1   Piano     (one part, two staves)
         *   staff 2      Soprano
         *   staff 3      Alto
         *   staff 4      Tenor
         *   staff 5      Flute
         *
         * so that every case is reachable in one score: two staves of one instrument, two adjacent
         * vocal parts, a vocal part next to a non-vocal one, and a single-staff non-vocal part.
         * There is one dynamic on every staff. */
        m_score = ScoreRW::readScore(STAVECENTERING_DATA_DIR + u"staveCentering.mscx");
        ASSERT_TRUE(m_score);
    }

    void TearDown() override
    {
        delete m_score;
        m_score = nullptr;
    }

    Staff* staffOf(size_t partIdx, size_t staffInPartIdx) const
    {
        return m_score->parts().at(partIdx)->staves().at(staffInPartIdx);
    }

    // The one hairpin in the score, on the soprano staff.
    Hairpin* hairpin() const
    {
        for (const auto& pair : m_score->spanner()) {
            if (pair.second->isHairpin()) {
                return toHairpin(pair.second);
            }
        }

        return nullptr;
    }

    // The one dynamic on the given staff.
    Dynamic* dynamicOnStaff(staff_idx_t staffIdx) const
    {
        for (Segment* seg = m_score->firstSegment(SegmentType::ChordRest); seg; seg = seg->next1(SegmentType::ChordRest)) {
            for (EngravingItem* annotation : seg->annotations()) {
                if (annotation->isDynamic() && annotation->staffIdx() == staffIdx) {
                    return toDynamic(annotation);
                }
            }
        }

        return nullptr;
    }

    // Turns centering on for the dynamic on the given staff and lays the score out again,
    // which is what makes DynamicsLayout re-decide its placement.
    void setCenterBetweenStaves(EngravingItem* item, AutoOnOff value)
    {
        m_score->startCmd(TranslatableString::untranslatable("Stave centering tests"));
        item->undoChangeProperty(Pid::CENTER_BETWEEN_STAVES, value);
        m_score->setLayoutAll();
        m_score->endCmd();
    }

    void hideStaff(Staff* staff)
    {
        m_score->startCmd(TranslatableString::untranslatable("Stave centering tests"));
        staff->undoChangeProperty(Pid::VISIBLE, false);
        m_score->setLayoutAll();
        m_score->endCmd();
    }

    MasterScore* m_score = nullptr;
};

// ---------------------------------------------------------------------------
// The fixture itself
//
// Vocal instruments are recognised by their family, which is resolved by looking the
// instrument id up in instruments.xml. A fixture whose <Instrument id="..."> attributes went
// missing would resolve to an empty family and make the tests below pass for the wrong
// reason, so assert what the fixture is before relying on it.
// ---------------------------------------------------------------------------

TEST_F(Engraving_StaveCenteringTests, fixtureInstruments)
{
    ASSERT_EQ(m_score->parts().size(), 5u);
    ASSERT_EQ(m_score->staves().size(), 6u);

    const Instrument* piano = m_score->parts().at(0)->instrument();
    EXPECT_FALSE(piano->isVocalInstrument());
    EXPECT_TRUE(piano->isNormallyMultiStaveInstrument());
    EXPECT_EQ(m_score->parts().at(0)->nstaves(), 2u);

    for (size_t partIdx : { 1, 2, 3 }) {
        const Part* part = m_score->parts().at(partIdx);
        EXPECT_TRUE(part->instrument()->isVocalInstrument()) << part->partName().toStdString();
        EXPECT_EQ(part->nstaves(), 1u);
    }

    const Instrument* flute = m_score->parts().at(4)->instrument();
    EXPECT_FALSE(flute->isVocalInstrument());
    EXPECT_FALSE(flute->isNormallyMultiStaveInstrument());

    for (staff_idx_t staffIdx = 0; staffIdx < m_score->staves().size(); ++staffIdx) {
        EXPECT_TRUE(dynamicOnStaff(staffIdx)) << "no dynamic on staff " << staffIdx;
    }
}

// ---------------------------------------------------------------------------
// staffToCenterAgainst()
// ---------------------------------------------------------------------------

TEST_F(Engraving_StaveCenteringTests, staffToCenterAgainst_grandStaff)
{
    Dynamic* topStaffDynamic = dynamicOnStaff(0);
    Dynamic* bottomStaffDynamic = dynamicOnStaff(1);
    ASSERT_TRUE(topStaffDynamic && bottomStaffDynamic);

    // Nothing above the topmost staff of the score
    EXPECT_EQ(topStaffDynamic->staffToCenterAgainst(true), nullptr);
    EXPECT_EQ(topStaffDynamic->staffToCenterAgainst(false), staffOf(0, 1));

    EXPECT_EQ(bottomStaffDynamic->staffToCenterAgainst(true), staffOf(0, 0));
    // The staff below belongs to another part, and the piano is not a vocal instrument
    EXPECT_EQ(bottomStaffDynamic->staffToCenterAgainst(false), nullptr);
}

TEST_F(Engraving_StaveCenteringTests, staffToCenterAgainst_vocalStaves)
{
    Dynamic* sopranoDynamic = dynamicOnStaff(2);
    Dynamic* altoDynamic = dynamicOnStaff(3);
    Dynamic* tenorDynamic = dynamicOnStaff(4);
    ASSERT_TRUE(sopranoDynamic && altoDynamic && tenorDynamic);

    // The staff above the soprano is the piano's, which is not vocal
    EXPECT_EQ(sopranoDynamic->staffToCenterAgainst(true), nullptr);
    EXPECT_EQ(sopranoDynamic->staffToCenterAgainst(false), staffOf(2, 0));

    EXPECT_EQ(altoDynamic->staffToCenterAgainst(true), staffOf(1, 0));
    EXPECT_EQ(altoDynamic->staffToCenterAgainst(false), staffOf(3, 0));

    EXPECT_EQ(tenorDynamic->staffToCenterAgainst(true), staffOf(2, 0));
    // The flute below is not vocal
    EXPECT_EQ(tenorDynamic->staffToCenterAgainst(false), nullptr);
}

TEST_F(Engraving_StaveCenteringTests, staffToCenterAgainst_singleStaffNonVocalPart)
{
    Dynamic* fluteDynamic = dynamicOnStaff(5);
    ASSERT_TRUE(fluteDynamic);

    EXPECT_EQ(fluteDynamic->staffToCenterAgainst(true), nullptr);
    EXPECT_EQ(fluteDynamic->staffToCenterAgainst(false), nullptr);
}

TEST_F(Engraving_StaveCenteringTests, staffToCenterAgainst_spanner)
{
    Hairpin* sopranoHairpin = hairpin();
    ASSERT_TRUE(sopranoHairpin);
    ASSERT_EQ(sopranoHairpin->staffIdx(), static_cast<staff_idx_t>(2));
    ASSERT_EQ(sopranoHairpin->findAncestor(ElementType::SYSTEM), nullptr);

    EXPECT_EQ(sopranoHairpin->staffToCenterAgainst(true), nullptr);
    EXPECT_EQ(sopranoHairpin->staffToCenterAgainst(false), staffOf(2, 0));
}

TEST_F(Engraving_StaveCenteringTests, staffToCenterAgainst_explicitSystem)
{
    ASSERT_FALSE(m_score->systems().empty());
    const System* system = m_score->systems().front();

    for (staff_idx_t staffIdx = 0; staffIdx < m_score->staves().size(); ++staffIdx) {
        Dynamic* dynamic = dynamicOnStaff(staffIdx);
        ASSERT_TRUE(dynamic);
        for (bool above : { true, false }) {
            // Passing the system explicitly must agree with letting the item find its own:
            EXPECT_EQ(dynamic->staffToCenterAgainst(above, system), dynamic->staffToCenterAgainst(above))
                << "staff " << staffIdx << (above ? " above" : " below");
        }
    }
}

TEST_F(Engraving_StaveCenteringTests, staffToCenterAgainst_skipsHiddenStaff)
{
    Dynamic* sopranoDynamic = dynamicOnStaff(2);
    ASSERT_TRUE(sopranoDynamic);
    ASSERT_EQ(sopranoDynamic->staffToCenterAgainst(false), staffOf(2, 0));

    hideStaff(staffOf(2, 0));

    // A hidden staff is skipped: with the alto hidden, the soprano is centered against the tenor
    EXPECT_EQ(sopranoDynamic->staffToCenterAgainst(false), staffOf(3, 0));
}

// ---------------------------------------------------------------------------
// Placement of automatically placed items: setPlacementBasedOnVoiceAssignment() is reached
// only by items which have voice assignment properties: dynamics, expressions and hairpins.
// A lyric has no voice assignment and simply keeps its styled placement.
// ---------------------------------------------------------------------------

// On a grand staff, centered items go to the inner side of their own staff
TEST_F(Engraving_StaveCenteringTests, placement_grandStaff)
{
    Dynamic* topStaffDynamic = dynamicOnStaff(0);
    Dynamic* bottomStaffDynamic = dynamicOnStaff(1);
    ASSERT_TRUE(topStaffDynamic && bottomStaffDynamic);

    setCenterBetweenStaves(topStaffDynamic, AutoOnOff::ON);
    EXPECT_EQ(topStaffDynamic->placement(), PlacementV::BELOW);

    setCenterBetweenStaves(bottomStaffDynamic, AutoOnOff::ON);
    EXPECT_EQ(bottomStaffDynamic->placement(), PlacementV::ABOVE);
}

// An instrument still counts as having two staves when one of them is hidden, but there is
// no longer a gap to center in, so the item must not be moved to the inner side
TEST_F(Engraving_StaveCenteringTests, placement_grandStaffWithOneStaffHidden)
{
    Dynamic* bottomStaffDynamic = dynamicOnStaff(1);
    ASSERT_TRUE(bottomStaffDynamic);

    setCenterBetweenStaves(bottomStaffDynamic, AutoOnOff::ON);
    ASSERT_EQ(bottomStaffDynamic->placement(), PlacementV::ABOVE);

    hideStaff(staffOf(0, 0));

    ASSERT_EQ(m_score->parts().at(0)->nstaves(), 2);
    ASSERT_EQ(bottomStaffDynamic->staffToCenterAgainst(true), nullptr);
    ASSERT_EQ(bottomStaffDynamic->staffToCenterAgainst(false), nullptr);
    EXPECT_EQ(bottomStaffDynamic->placement(), PlacementV::BELOW);
}

// On vocal staves the item is moved to whichever side has a staff to center against
TEST_F(Engraving_StaveCenteringTests, placement_vocalStaves)
{
    Dynamic* sopranoDynamic = dynamicOnStaff(2);
    Dynamic* altoDynamic = dynamicOnStaff(3);
    Hairpin* sopranoHairpin = hairpin(); // hairpins are placed the same as dynamics
    ASSERT_TRUE(sopranoDynamic && altoDynamic && sopranoHairpin);

    // Dynamics on vocal staves go above the staff by default:
    ASSERT_EQ(sopranoDynamic->placement(), PlacementV::ABOVE);
    ASSERT_EQ(sopranoHairpin->placement(), PlacementV::ABOVE);

    setCenterBetweenStaves(sopranoDynamic, AutoOnOff::ON);
    EXPECT_EQ(sopranoDynamic->placement(), PlacementV::BELOW);

    setCenterBetweenStaves(sopranoHairpin, AutoOnOff::ON);
    EXPECT_EQ(sopranoHairpin->placement(), PlacementV::BELOW);

    // The alto does have a staff above it:
    setCenterBetweenStaves(altoDynamic, AutoOnOff::ON);
    EXPECT_EQ(altoDynamic->placement(), PlacementV::ABOVE);
}

// With no staff to center against on either side, the placement is unchanged
TEST_F(Engraving_StaveCenteringTests, placement_noStaffToCenterAgainst)
{
    Dynamic* sopranoDynamic = dynamicOnStaff(2);
    ASSERT_TRUE(sopranoDynamic);

    // Leaves the soprano between the piano and the flute, neither of which is vocal
    hideStaff(staffOf(2, 0));
    hideStaff(staffOf(3, 0));
    ASSERT_EQ(sopranoDynamic->staffToCenterAgainst(true), nullptr);
    ASSERT_EQ(sopranoDynamic->staffToCenterAgainst(false), nullptr);

    const PlacementV placementBefore = sopranoDynamic->placement();
    setCenterBetweenStaves(sopranoDynamic, AutoOnOff::ON);
    EXPECT_EQ(sopranoDynamic->placement(), placementBefore);
}

// Only centering that is explicitly turned ON (but not AUTO) moves an item to the other side
TEST_F(Engraving_StaveCenteringTests, placement_notChangedWhenCenteringIsAuto)
{
    Dynamic* sopranoDynamic = dynamicOnStaff(2);
    ASSERT_TRUE(sopranoDynamic);
    ASSERT_EQ(sopranoDynamic->getProperty(Pid::CENTER_BETWEEN_STAVES).value<AutoOnOff>(), AutoOnOff::AUTO);
    ASSERT_EQ(sopranoDynamic->placement(), PlacementV::ABOVE);

    m_score->startCmd(TranslatableString::untranslatable("Stave centering tests"));
    m_score->undoChangeStyleVal(Sid::dynamicsHairpinsAutoCenterOnVocalStaves, true);
    m_score->setLayoutAll();
    m_score->endCmd();
    EXPECT_EQ(sopranoDynamic->placement(), PlacementV::ABOVE);
}

// ---------------------------------------------------------------------------
// Serialization of the centering properties
// ---------------------------------------------------------------------------

TEST_F(Engraving_StaveCenteringTests, centeringPropertiesRoundTrip)
{
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    Lyrics* defaultLyrics = Factory::createLyrics(score->dummy()->chord());
    ASSERT_EQ(defaultLyrics->centerBetweenStaves(), AutoOnOff::AUTO);
    Lyrics* readDefaultLyrics = toLyrics(ScoreRW::writeReadElement(defaultLyrics));
    EXPECT_EQ(readDefaultLyrics->centerBetweenStaves(), AutoOnOff::AUTO);
    delete readDefaultLyrics;
    delete defaultLyrics;

    Lyrics* lyrics = Factory::createLyrics(score->dummy()->chord());
    lyrics->setProperty(Pid::CENTER_BETWEEN_STAVES, AutoOnOff::ON);
    Lyrics* readLyrics = toLyrics(ScoreRW::writeReadElement(lyrics));
    EXPECT_EQ(readLyrics->getProperty(Pid::CENTER_BETWEEN_STAVES).value<AutoOnOff>(), AutoOnOff::ON);
    delete readLyrics;
    delete lyrics;

    PartialLyricsLine* line = Factory::createPartialLyricsLine(score->dummy());
    line->setProperty(Pid::CENTER_BETWEEN_STAVES, AutoOnOff::ON);
    line->setProperty(Pid::PLACEMENT, PlacementV::ABOVE);
    line->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
    PartialLyricsLine* readLine = toPartialLyricsLine(ScoreRW::writeReadElement(line));
    EXPECT_EQ(readLine->getProperty(Pid::CENTER_BETWEEN_STAVES).value<AutoOnOff>(), AutoOnOff::ON);
    EXPECT_EQ(readLine->placement(), PlacementV::ABOVE);
    delete readLine;
    delete line;

    delete score;
}

TEST_F(Engraving_StaveCenteringTests, segmentAndLineShareTheCenteringSetting)
{
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);
    PartialLyricsLine* line = Factory::createPartialLyricsLine(score->dummy());
    LineSegment* segment = line->createLineSegment();
    ASSERT_TRUE(segment->isPartialLyricsLineSegment());

    EXPECT_EQ(segment->propertyDefault(Pid::CENTER_BETWEEN_STAVES).value<AutoOnOff>(), AutoOnOff::AUTO);

    line->setProperty(Pid::CENTER_BETWEEN_STAVES, AutoOnOff::ON);
    EXPECT_EQ(segment->getProperty(Pid::CENTER_BETWEEN_STAVES).value<AutoOnOff>(), AutoOnOff::ON);

    segment->setProperty(Pid::CENTER_BETWEEN_STAVES, AutoOnOff::OFF);
    EXPECT_EQ(line->centerBetweenStaves(), AutoOnOff::OFF);

    delete segment;
    delete line;
    delete score;
}
