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

// Hairpins and dynamics: crescendo/diminuendo direction, swell splitting, endpoint/snap-back by xoffset,
// bar-line clamping, dedup, cross-staff displacement, and Latin-1 text decoding.

#include <gtest/gtest.h>

#include "engraving/dom/dynamic.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/stafftext.h"

#include "testbase.h"

static const QString ENC_DIR(QString(iex_encore_tests_DATA_ROOT) + "/data/");

using namespace mu::engraving;

class Tst_Hairpins : public ::testing::Test, public MTest
{
protected:
    void SetUp() override { setRootDir(ENC_DIR); }
};

#define ENC_SANITY_TEST(testName, fileName) \
    TEST_F(Tst_Hairpins, testName) { \
        MasterScore* score = readEncoreScore(fileName); \
        ASSERT_NE(score, nullptr) << "Failed to load " << fileName; \
        EXPECT_GT(score->nmeasures(), 0); \
        muse::Ret ret = score->sanityCheck(); \
        EXPECT_TRUE(ret) << "Corrupted: " << ret.text(); \
        delete score; \
    }

// Regression: speguleco direction lives in bit 0; Encore 5 also sets bit 1 (0x02=cresc, 0x03=dim).
// Old `speguleco==0` check treated every Encore 5 hairpin as diminuendo.
TEST_F(Tst_Hairpins, v0c4_hairpin_speguleco_bit0)
{
    MasterScore* score = readEncoreScore("importer_hairpin_speguleco_bit0.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_hairpin_speguleco_bit0.enc";

    std::vector<HairpinType> seenTypes;
    for (const auto& kv : score->spanner()) {
        Spanner* sp = kv.second;
        if (sp && sp->isHairpin()) {
            seenTypes.push_back(toHairpin(sp)->hairpinType());
        }
    }
    ASSERT_EQ(seenTypes.size(), 2u);
    EXPECT_EQ(seenTypes[0], HairpinType::CRESC_HAIRPIN)
        << "speguleco=0x02 must import as crescendo";
    EXPECT_EQ(seenTypes[1], HairpinType::DIM_HAIRPIN)
        << "speguleco=0x03 must import as diminuendo";
    delete score;
}

// Regression: same-measure CRESC+DIM swell pair must split the measure at the midpoint.
TEST_F(Tst_Hairpins, v0c4_swell_pair_splits_at_measure_midpoint)
{
    MasterScore* score = readEncoreScore("importer_hairpin_speguleco_bit0.enc");
    ASSERT_NE(score, nullptr);

    std::vector<Hairpin*> hairpins;
    for (const auto& kv : score->spanner()) {
        Spanner* sp = kv.second;
        if (sp && sp->isHairpin()) {
            hairpins.push_back(toHairpin(sp));
        }
    }
    std::sort(hairpins.begin(), hairpins.end(),
              [](Hairpin* a, Hairpin* b) { return a->tick() < b->tick(); });
    ASSERT_EQ(hairpins.size(), 2u);

    const Measure* m0 = score->firstMeasure();
    ASSERT_NE(m0, nullptr);
    const Fraction midTick = m0->tick() + m0->ticks() / 2;

    EXPECT_EQ(hairpins[0]->hairpinType(), HairpinType::CRESC_HAIRPIN);
    EXPECT_EQ(hairpins[0]->tick2(), midTick)
        << "CRESC in same-measure swell pair must end exactly at the measure midpoint";

    EXPECT_EQ(hairpins[1]->hairpinType(), HairpinType::DIM_HAIRPIN);
    EXPECT_EQ(hairpins[1]->tick(), midTick)
        << "DIM in same-measure swell pair must start exactly at the measure midpoint";
    EXPECT_EQ(hairpins[1]->tick2(), m0->tick() + m0->ticks())
        << "DIM in same-measure swell pair must end at the barline";
    delete score;
}

// Regression: importer extended hairpins to the end of their alMezuro measure, overlapping adjacent hairpins.
// Fix: scan forward for the first Dynamic within the alMezuro window and stop there.
TEST_F(Tst_Hairpins, v0c4_hairpin_ends_at_next_dynamic)
{
    MasterScore* score = readEncoreScore("importer_hairpin_ends_at_next_dynamic.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_hairpin_ends_at_next_dynamic.enc";

    Hairpin* found = nullptr;
    for (const auto& kv : score->spanner()) {
        Spanner* sp = kv.second;
        if (sp && sp->isHairpin()) {
            found = toHairpin(sp);
            break;
        }
    }
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->tick(), Fraction(1, 4))
        << "hairpin must start at the WEDGESTART tick";
    EXPECT_EQ(found->tick2(), Fraction(3, 4))
        << "hairpin must end at the next Dynamic (tick=720, beat 4), "
        "not at the bar line of its alMezuro target measure";
    delete score;
}

// Regression: ORN xoffset < tagged chord-rest xoffset means the glyph belongs to the preceding chord.
TEST_F(Tst_Hairpins, v0c4_dyn_snap_back_by_xoffset)
{
    MasterScore* score = readEncoreScore("importer_dyn_snap_back_by_xoffset.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_dyn_snap_back_by_xoffset.enc";

    Measure* m1 = score->firstMeasure();
    ASSERT_NE(m1, nullptr);
    Dynamic* found = nullptr;
    Fraction foundTick;
    for (Segment* s = m1->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        for (EngravingItem* ann : s->annotations()) {
            if (ann && ann->isDynamic()) {
                found = toDynamic(ann);
                foundTick = s->tick();
                break;
            }
        }
        if (found) {
            break;
        }
    }
    ASSERT_NE(found, nullptr) << "expected one Dynamic in the measure";
    EXPECT_EQ(foundTick, Fraction(1, 8))
        << "dynamic must snap from the tagged eighth (tick 1/4) back to "
        "the previous eighth (tick 1/8) because its xoffset matches "
        "that note's region";
    delete score;
}

// Regression: same snap-back convention for WEDGESTART.
TEST_F(Tst_Hairpins, v0c4_wedge_snap_back_by_xoffset)
{
    MasterScore* score = readEncoreScore("importer_wedge_snap_back_by_xoffset.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_wedge_snap_back_by_xoffset.enc";

    Hairpin* found = nullptr;
    for (const auto& kv : score->spanner()) {
        Spanner* sp = kv.second;
        if (sp && sp->isHairpin()) {
            found = toHairpin(sp);
            break;
        }
    }
    ASSERT_NE(found, nullptr) << "expected one Hairpin";
    EXPECT_EQ(found->tick(), Fraction(0, 1))
        << "hairpin must snap back from the tagged eighth (tick 1/2) to "
        "the start of the half note (tick 0) because its xoffset is "
        "less than the eighth's xoffset";
    delete score;
}

// Regression: hairpin xoffset2 < first-note xoffset in target measure means Encore ends at the bar line.
TEST_F(Tst_Hairpins, v0c4_hairpin_barline_clamp)
{
    MasterScore* score = readEncoreScore("importer_hairpin_barline_clamp.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_hairpin_barline_clamp.enc";

    std::vector<Hairpin*> hairpins;
    for (const auto& kv : score->spanner()) {
        Spanner* sp = kv.second;
        if (sp && sp->isHairpin()) {
            hairpins.push_back(toHairpin(sp));
        }
    }
    std::sort(hairpins.begin(), hairpins.end(),
              [](Hairpin* a, Hairpin* b) { return a->tick() < b->tick(); });
    ASSERT_GE(hairpins.size(), 2u);
    EXPECT_EQ(hairpins[0]->hairpinType(), HairpinType::DIM_HAIRPIN);
    EXPECT_EQ(hairpins[0]->tick2(), Fraction(1, 1))
        << "dim hairpin with xoffset2 < firstNoteXoff must end at bar line";
    EXPECT_EQ(hairpins[1]->hairpinType(), HairpinType::CRESC_HAIRPIN);
    EXPECT_GE(hairpins[1]->tick(), Fraction(1, 1))
        << "cresc hairpin must start inside m2, not before bar line";
    delete score;
}

// Regression: when xoffset2 falls between two notes in the target measure, the hairpin must end
// at the note with the largest xoffset still <= xoffset2.
TEST_F(Tst_Hairpins, v0c4_hairpin_xoffset2_snap)
{
    MasterScore* score = readEncoreScore("importer_hairpin_xoffset2_snap.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_hairpin_xoffset2_snap.enc";

    std::vector<Hairpin*> hairpins;
    for (const auto& kv : score->spanner()) {
        Spanner* sp = kv.second;
        if (sp && sp->isHairpin()) {
            hairpins.push_back(toHairpin(sp));
        }
    }
    std::sort(hairpins.begin(), hairpins.end(),
              [](Hairpin* a, Hairpin* b) { return a->tick() < b->tick(); });
    ASSERT_GE(hairpins.size(), 1u);
    EXPECT_EQ(hairpins[0]->hairpinType(), HairpinType::DIM_HAIRPIN);
    EXPECT_EQ(hairpins[0]->tick2(), Fraction(5, 4))
        << "hairpin with xoffset2 between two notes must end at the note with largest xoffset <= xoffset2";
    delete score;
}

// Regression: Encore sometimes writes duplicate dynamics on the same (staff, voice, tick).
TEST_F(Tst_Hairpins, v0c4_dyn_dedup)
{
    MasterScore* score = readEncoreScore("importer_dyn_dedup.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_dyn_dedup.enc";

    Measure* m1 = score->firstMeasure();
    ASSERT_NE(m1, nullptr);
    int dynCount = 0;
    for (Segment* s = m1->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        for (EngravingItem* ann : s->annotations()) {
            if (ann && ann->isDynamic()) {
                ++dynCount;
            }
        }
    }
    EXPECT_EQ(dynCount, 1)
        << "two identical MF ORNs at the same tick must collapse to one "
        "Dynamic on the segment";
    delete score;
}

// Regression: dynamic ORN with yoffset > 0 visually belongs to staffIdx-1 (stored on N, rendered on N-1).
TEST_F(Tst_Hairpins, v0c4_dyn_displaced_to_staff_above)
{
    MasterScore* score = readEncoreScore("importer_dyn_displaced_to_staff_above.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_dyn_displaced_to_staff_above.enc";

    const track_idx_t trackStaff0 = 0;
    const track_idx_t trackStaff1 = static_cast<track_idx_t>(VOICES);
    bool foundOnStaff0 = false;
    bool foundOnStaff1 = false;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest); s;
             s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* ann : s->annotations()) {
                if (!ann || !ann->isDynamic()) {
                    continue;
                }
                if (ann->track() == trackStaff0) {
                    foundOnStaff0 = true;
                }
                if (ann->track() == trackStaff1) {
                    foundOnStaff1 = true;
                }
            }
        }
    }
    EXPECT_TRUE(foundOnStaff0)
        << "MF with yoffset > 0 must be rerouted to staff 0 (the staff above)";
    EXPECT_FALSE(foundOnStaff1)
        << "the displaced MF must NOT remain on staff 1";
    delete score;
}

// Regression: WEDGE at tick==durTicks (bar line) had no ChordRest; snap-start returned next measure's tick.
TEST_F(Tst_Hairpins, v0c4_hairpin_snapstart_at_barline)
{
    MasterScore* score = readEncoreScore("importer_hairpin_snapstart_at_barline.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_hairpin_snapstart_at_barline.enc";

    Hairpin* dim = nullptr;
    for (const auto& kv : score->spanner()) {
        Spanner* sp = kv.second;
        if (sp && sp->isHairpin() && toHairpin(sp)->hairpinType() == HairpinType::DIM_HAIRPIN) {
            dim = toHairpin(sp);
            break;
        }
    }
    ASSERT_NE(dim, nullptr) << "dim hairpin starting at end of m1 must not be dropped";
    EXPECT_LT(dim->tick(), Fraction(1, 1))
        << "hairpin start must be inside m1 (snap from bar-line tick to last "
        "note with xoff <= ornament.xoffset)";
    EXPECT_EQ(dim->tick(), Fraction(1, 2))
        << "start must snap to tick=480 (xoff=90, latest note with xoff<=110)";
    EXPECT_EQ(dim->tick2(), Fraction(1, 1) + Fraction(1, 4))
        << "dim must end at the MF dynamic in m2 (next-dynamic endpoint)";
    delete score;
}

// Regression: bar-line clamp must yield to next-Dynamic endpoint when one exists.
TEST_F(Tst_Hairpins, v0c4_hairpin_endpoint_dynamic_wins)
{
    MasterScore* score = readEncoreScore("importer_hairpin_endpoint_dynamic_wins.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_hairpin_endpoint_dynamic_wins.enc";

    Hairpin* dim = nullptr;
    for (const auto& kv : score->spanner()) {
        Spanner* sp = kv.second;
        if (sp && sp->isHairpin() && toHairpin(sp)->hairpinType() == HairpinType::DIM_HAIRPIN) {
            dim = toHairpin(sp);
            break;
        }
    }
    ASSERT_NE(dim, nullptr) << "dim hairpin must be present";
    const Fraction m2tick = Fraction(1, 1);
    EXPECT_GT(dim->tick2(), m2tick)
        << "hairpin must end at MF dynamic in m2 (after bar line), not at m2.tick; "
        "next-dynamic endpoint must win over bar-line clamp";
    EXPECT_EQ(dim->tick2(), m2tick + Fraction(1, 4))
        << "hairpin must end at the MF dynamic tick (m2 + 1/4)";
    delete score;
}

// Regression: ORNs at tick > durTicks (volta-grouped dynamics) were dropped.
TEST_F(Tst_Hairpins, v0c4_two_dynamics_in_one_measure)
{
    MasterScore* score = readEncoreScore("importer_two_dynamics_in_one_measure.enc");
    ASSERT_NE(score, nullptr)
        << "Failed to load importer_two_dynamics_in_one_measure.enc";

    Measure* first = score->firstMeasure();
    ASSERT_NE(first, nullptr);
    std::vector<DynamicType> dynTypes;
    std::vector<String> textValues;
    for (Segment* s = first->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        for (EngravingItem* ann : s->annotations()) {
            if (!ann) {
                continue;
            }
            if (ann->isDynamic()) {
                dynTypes.push_back(toDynamic(ann)->dynamicType());
            } else if (ann->isStaffText()) {
                textValues.push_back(toStaffText(ann)->plainText());
            }
        }
    }
    ASSERT_EQ(dynTypes.size(), 2u)
        << "both the start-of-measure F and the end-of-measure PP must import";
    EXPECT_EQ(dynTypes[0], DynamicType::F);
    EXPECT_EQ(dynTypes[1], DynamicType::PP);
    ASSERT_EQ(textValues.size(), 2u)
        << "both volta-specific stafftext labels must import";
    EXPECT_EQ(textValues[0], String(u"la 1ª vez"));
    EXPECT_EQ(textValues[1], String(u"la 2ª"));
    delete score;
}

// Regression: TEXT block payloads in legacy Latin-1 files were decoded as UTF-16 LE.
TEST_F(Tst_Hairpins, v0c4_text_block_latin1_decoding)
{
    MasterScore* score = readEncoreScore("text_text_block_latin1_decoding.enc");
    ASSERT_NE(score, nullptr)
        << "Failed to load text_text_block_latin1_decoding.enc";

    StaffText* found = nullptr;
    for (MeasureBase* mb = score->first(); mb && !found; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* ann : s->annotations()) {
                if (ann && ann->isStaffText()) {
                    found = toStaffText(ann);
                    break;
                }
            }
        }
    }
    ASSERT_NE(found, nullptr) << "expected at least one StaffText from a Latin-1 TEXT entry";
    EXPECT_EQ(found->plainText(), String(u"la 1ª vez"))
        << "Latin-1 TEXT payload must decode as readable text, not UTF-16 gibberish";
    delete score;
}

ENC_SANITY_TEST(dynamics_size16,            "ornaments_dynamics.enc")
ENC_SANITY_TEST(dynamics_full,              "ornaments_dynamics_full.enc")
ENC_SANITY_TEST(wedgestart_at_measure_end,  "ornaments_wedgestart_at_measure_end.enc")
ENC_SANITY_TEST(double_barline_multi_staff, "ornaments_double_barline_multi_staff.enc")
