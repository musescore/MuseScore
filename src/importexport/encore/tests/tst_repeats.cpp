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

// Repeats, voltas, jumps and coda/segno markers: bracket coalescing, numbering, play counts and replay skipping.

#include <gtest/gtest.h>

#include "engraving/dom/marker.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/repeatlist.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/volta.h"

#include "testbase.h"

static const QString ENC_DIR(QString(iex_encore_tests_DATA_ROOT) + "/data/");

using namespace mu::engraving;

class Tst_Repeats : public ::testing::Test, public MTest
{
protected:
    void SetUp() override { setRootDir(ENC_DIR); }
};

#define ENC_SANITY_TEST(testName, fileName) \
    TEST_F(Tst_Repeats, testName) { \
        MasterScore* score = readEncoreScore(fileName); \
        ASSERT_NE(score, nullptr) << "Failed to load " << fileName; \
        EXPECT_GT(score->nmeasures(), 0); \
        muse::Ret ret = score->sanityCheck(); \
        EXPECT_TRUE(ret) << "Corrupted: " << ret.text(); \
        delete score; \
    }

// Regression: when a second volta bracket's bitmask contains bits already shown in the
// first bracket (e.g. raw bits {2,4} after {1,2,3}), the second bracket must display
// only the NEW endings ({4}), not the full raw set ("2, 4.").
TEST_F(Tst_Repeats, v0c4_volta_overlapping_bits_filtered)
{
    MasterScore* score = readEncoreScore("importer_volta_overlapping_bits.enc");
    ASSERT_NE(score, nullptr);

    std::vector<Volta*> voltas;
    for (const auto& kv : score->spanner()) {
        Spanner* sp = kv.second;
        if (sp && sp->isVolta()) {
            voltas.push_back(toVolta(sp));
        }
    }
    std::sort(voltas.begin(), voltas.end(),
              [](Volta* a, Volta* b) { return a->tick() < b->tick(); });
    ASSERT_EQ(voltas.size(), 2u);
    EXPECT_EQ(voltas[0]->beginText(), String(u"1, 2, 3."))
        << "First volta (bits 0x07) must show all three endings";
    EXPECT_EQ(voltas[1]->beginText(), String(u"4."))
        << "Second volta (raw bits 0x0A) must show only ending 4 "
        "(ending 2 already covered by the first bracket)";
    EXPECT_EQ(static_cast<int>(voltas[1]->endings().size()), 1)
        << "Endings list must also contain only {4}";
    EXPECT_EQ(voltas[1]->endings()[0], 4);
    delete score;
}

// Regression: importer created one Volta per measure (3 voltas for 1/1/2 bits) and never set begin-text.
// Fix: coalesce equal-bitmask runs into one Volta and set begin-text from the endings list.
TEST_F(Tst_Repeats, v0c4_volta_coalesce_and_numbered_text)
{
    MasterScore* score = readEncoreScore("importer_volta_coalesce_and_text.enc");
    ASSERT_NE(score, nullptr)
        << "Failed to load importer_volta_coalesce_and_text.enc";

    std::vector<Volta*> voltas;
    for (const auto& kv : score->spanner()) {
        Spanner* sp = kv.second;
        if (sp && sp->isVolta()) {
            voltas.push_back(toVolta(sp));
        }
    }
    std::sort(voltas.begin(), voltas.end(),
              [](Volta* a, Volta* b) { return a->tick() < b->tick(); });
    ASSERT_EQ(voltas.size(), 2u)
        << "consecutive measures with the same repeatAlternative bitmask "
        "must collapse into one Volta";
    EXPECT_EQ(voltas[0]->beginText(), String(u"1."))
        << "first ending must render the number '1.'";
    EXPECT_EQ(voltas[1]->beginText(), String(u"2."))
        << "second ending must render the number '2.'";
    EXPECT_GT(voltas[0]->tick2() - voltas[0]->tick(), Fraction(4, 4))
        << "first Volta must span both alt-1 measures, not just one";
    delete score;
}

// Regression: both coda bytes (0x85 CODA1 and 0x89 CODA2) mapped to CODA, losing the TOCODA distinction.
// Fix: 0x85 -> TOCODA, 0x89 -> CODA.
TEST_F(Tst_Repeats, v0c4_to_coda_distinct_from_coda)
{
    MasterScore* score = readEncoreScore("importer_to_coda_vs_coda_marker.enc");
    ASSERT_NE(score, nullptr)
        << "Failed to load importer_to_coda_vs_coda_marker.enc";

    std::vector<MarkerType> orderedTypes;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (EngravingItem* el : toMeasure(mb)->el()) {
            if (el && el->isMarker()) {
                orderedTypes.push_back(toMarker(el)->markerType());
            }
        }
    }
    ASSERT_EQ(orderedTypes.size(), 2u)
        << "expected one marker per of the two coda-bearing measures";
    EXPECT_EQ(orderedTypes[0], MarkerType::TOCODA)
        << "CODA1 (0x85) must import as TOCODA, not CODA";
    EXPECT_EQ(orderedTypes[1], MarkerType::CODA)
        << "CODA2 (0x89) must import as CODA";
    delete score;
}

// Regression: Encore has no explicit repeat play count; it is implied by the highest volta ending.
// A repeat with endings "1.-3." then "4." must play four times or the "4." ending is never reached
// (the default count of 2 stopped it early).
TEST_F(Tst_Repeats, v0c4_volta_repeat_playcount_from_endings)
{
    MasterScore* score = readEncoreScore("structure_volta_repeat_playcount.enc");
    ASSERT_NE(score, nullptr);
    score->setExpandRepeats(true);

    const Measure* endRepeat = nullptr;
    const Volta* fourthEnding = nullptr;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (mb->isMeasure() && toMeasure(mb)->repeatEnd()) {
            endRepeat = toMeasure(mb);
            break;
        }
    }
    for (const auto& p : score->spanner()) {
        const Spanner* sp = p.second;
        if (sp && sp->isVolta()) {
            const Volta* v = toVolta(sp);
            if (v->endings().size() == 1 && v->endings()[0] == 4) {
                fourthEnding = v;
            }
        }
    }
    ASSERT_NE(endRepeat, nullptr) << "expected a measure with a repeat-end barline";
    ASSERT_NE(fourthEnding, nullptr) << "expected a 4th-ending volta (endings == {4})";

    EXPECT_EQ(endRepeat->repeatCount(), 4)
        << "end-repeat barline must play 4 times (highest ending is '4.'); the default "
        "of 2 stops before the 4th ending (got " << endRepeat->repeatCount() << ")";

    const int repeatStartTick = endRepeat->tick().ticks() - endRepeat->ticks().ticks();
    const int fourthEndingTick = fourthEnding->tick().ticks();
    int bodyPlays = 0;
    int fourthEndingPlays = 0;
    for (const RepeatSegment* rs : score->repeatList()) {
        if (rs->tick <= repeatStartTick && repeatStartTick < rs->endTick()) {
            ++bodyPlays;
        }
        if (rs->tick <= fourthEndingTick && fourthEndingTick < rs->endTick()) {
            ++fourthEndingPlays;
        }
    }
    EXPECT_EQ(bodyPlays, 4)
        << "repeated body must play 4 times (got " << bodyPlays << ")";
    EXPECT_EQ(fourthEndingPlays, 1)
        << "4th ending must play exactly once, on the final pass (got "
        << fourthEndingPlays << ")";
    delete score;
}

ENC_SANITY_TEST(section_markers,  "structure_section_markers.enc")
ENC_SANITY_TEST(jump_marks,       "structure_jump_marks.enc")
ENC_SANITY_TEST(jump_marks_all,   "structure_jump_marks_all.enc")

// Regression: a repeat list cached during layout (before voltas were anchored) replayed the 1st
// ending on the repeat instead of skipping it; the importer must invalidate it after load.
TEST_F(Tst_Repeats, v0c4_volta_repeat_skips_first_ending_on_replay)
{
    MasterScore* score = readEncoreScore("structure_volta_repeat_playback.enc");
    ASSERT_NE(score, nullptr);
    score->setExpandRepeats(true);

    const Volta* firstEnding = nullptr;
    for (const auto& p : score->spanner()) {
        const Spanner* sp = p.second;
        if (sp && sp->isVolta()) {
            const Volta* v = toVolta(sp);
            if (v->endings().size() == 1 && v->endings()[0] == 1) {
                firstEnding = v;
                break;
            }
        }
    }
    ASSERT_NE(firstEnding, nullptr) << "expected a 1st-ending volta (endings == {1})";
    const int firstEndingTick = firstEnding->tick().ticks();

    int firstEndingPlays = 0;
    for (const RepeatSegment* rs : score->repeatList()) {
        if (rs->tick <= firstEndingTick && firstEndingTick < rs->endTick()) {
            ++firstEndingPlays;
        }
    }
    EXPECT_EQ(firstEndingPlays, 1)
        << "1st ending must play once and be skipped on the repeat; a stale cached "
        "repeat list replays it on every pass (got " << firstEndingPlays << " plays)";
    delete score;
}

// Regression: a volta's end hook must reflect its measure's barline (closed over a repeat-end,
// open otherwise); the importer used to hard-code every volta closed.
TEST_F(Tst_Repeats, v0c4_final_ending_volta_is_open)
{
    MasterScore* score = readEncoreScore("structure_volta_repeat_playback.enc");
    ASSERT_NE(score, nullptr);

    const Volta* firstEnding = nullptr;
    const Volta* secondEnding = nullptr;
    for (const auto& p : score->spanner()) {
        const Spanner* sp = p.second;
        if (!sp || !sp->isVolta()) {
            continue;
        }
        const Volta* v = toVolta(sp);
        if (v->endings().size() == 1 && v->endings()[0] == 1) {
            firstEnding = v;
        } else if (v->endings().size() == 1 && v->endings()[0] == 2) {
            secondEnding = v;
        }
    }
    ASSERT_NE(firstEnding, nullptr) << "expected a 1st-ending volta";
    ASSERT_NE(secondEnding, nullptr) << "expected a 2nd-ending volta";
    EXPECT_EQ(firstEnding->voltaType(), Volta::Type::CLOSED)
        << "the 1st ending ends on a repeat barline and must be closed";
    EXPECT_EQ(secondEnding->voltaType(), Volta::Type::OPEN)
        << "the final ending has no repeat barline and must be open, not a closed box";
    delete score;
}
