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

// Multi-measure rest expansion: a single MEAS block with mrestCount>1 expands to that many measures,
// across successor/predecessor rest cases and multi-staff files, plus the createMultiMeasureRests style flag.

#include <gtest/gtest.h>

#include "engraving/dom/barline.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/part.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/volta.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftype.h"

#include "testbase.h"

static const QString ENC_DIR(QString(iex_encore_tests_DATA_ROOT) + "/data/");

using namespace mu::engraving;

class Tst_NotesMrest : public ::testing::Test, public MTest
{
protected:
    void SetUp() override
    {
        setRootDir(ENC_DIR);
    }
};

// A single MEAS block whose lone REST has mrestCount>1 must expand to that many measures,
// keeping system locks aligned. Fixture: 7 blocks with MEAS[3] mrest=3 -> 9 measures.
TEST_F(Tst_NotesMrest, mrest_single_block_expands_and_system_locks_correct)
{
    MasterScore* score = readEncoreScore("importer_mrest_single_block.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_mrest_single_block.enc";
    ASSERT_EQ(score->nmeasures(), 9)
        << "7 MEAS blocks + 2 extra from MEAS[3] mrestCount=3";

    auto measureAt = [](MasterScore* sc, int idx) -> Measure* {
        Measure* m = sc->firstMeasure();
        for (int i = 0; i < idx && m; ++i) {
            m = m->nextMeasure();
        }
        return m;
    };
    auto hasPitchedNotes = [](Measure* m) -> bool {
        if (!m) {
            return false;
        }
        for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            for (size_t v = 0; v < VOICES; ++v) {
                EngravingItem* el = s->element(v);
                if (el && el->isChord()) {
                    return true;
                }
            }
        }
        return false;
    };

    EXPECT_TRUE(hasPitchedNotes(measureAt(score, 0))) << "measure 1 must have notes";
    EXPECT_TRUE(hasPitchedNotes(measureAt(score, 1))) << "measure 2 must have notes";
    EXPECT_TRUE(hasPitchedNotes(measureAt(score, 2))) << "measure 3 must have notes";
    EXPECT_FALSE(hasPitchedNotes(measureAt(score, 3))) << "measure 4 must be empty";
    EXPECT_FALSE(hasPitchedNotes(measureAt(score, 4))) << "measure 5 must be empty";
    EXPECT_FALSE(hasPitchedNotes(measureAt(score, 5))) << "measure 6 must be empty";
    EXPECT_TRUE(hasPitchedNotes(measureAt(score, 6))) << "measure 7 must have notes";

    delete score;
}

// Regression: mrest expansion must not depend on the successor block having notes; a mrest=3
// followed by a plain rest measure must still expand (encMeasDisplayCount used to collapse it).
TEST_F(Tst_NotesMrest, mrest_single_block_expands_when_successor_is_rest)
{
    MasterScore* score = readEncoreScore("importer_mrest_followed_by_rest.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_mrest_followed_by_rest.enc";
    ASSERT_EQ(score->nmeasures(), 8)
        << "5 real MEAS blocks + 2 extra from mrestCount=3 + 1 filler; without fix only 6";

    auto measureAt = [](MasterScore* sc, int idx) -> Measure* {
        Measure* m = sc->firstMeasure();
        for (int i = 0; i < idx && m; ++i) {
            m = m->nextMeasure();
        }
        return m;
    };
    auto hasPitchedNotes = [](Measure* m) -> bool {
        if (!m) {
            return false;
        }
        for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            for (size_t v = 0; v < VOICES; ++v) {
                EngravingItem* el = s->element(v);
                if (el && el->isChord()) {
                    return true;
                }
            }
        }
        return false;
    };

    EXPECT_TRUE(hasPitchedNotes(measureAt(score, 0))) << "measure 1 must have notes";
    EXPECT_TRUE(hasPitchedNotes(measureAt(score, 1))) << "measure 2 must have notes";
    EXPECT_FALSE(hasPitchedNotes(measureAt(score, 2))) << "measure 3 (mrest) must be empty";
    EXPECT_FALSE(hasPitchedNotes(measureAt(score, 3))) << "measure 4 (mrest) must be empty";
    EXPECT_FALSE(hasPitchedNotes(measureAt(score, 4))) << "measure 5 (mrest) must be empty";
    // Measure 6 is the single-rest successor: the case that used to suppress expansion.
    EXPECT_FALSE(hasPitchedNotes(measureAt(score, 5))) << "measure 6 (single rest) must be empty";
    EXPECT_TRUE(hasPitchedNotes(measureAt(score, 6))) << "measure 7 must have notes";
    EXPECT_FALSE(hasPitchedNotes(measureAt(score, 7))) << "measure 8 (filler) must be empty";

    delete score;
}

// Regression: a mrest>1 block must expand even when the preceding block is a plain single-measure
// rest; encMeasHasSingleRest used to ignore mrestCount and suppress expansion after any rest.
TEST_F(Tst_NotesMrest, mrest_single_block_expands_when_preceded_by_rest)
{
    MasterScore* score = readEncoreScore("importer_mrest_preceded_by_rest.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_mrest_preceded_by_rest.enc";
    ASSERT_EQ(score->nmeasures(), 12)
        << "4 MEAS blocks + 2 pad + 6 extra from mrestCount=7; without fix only 6 measures";

    auto measureAt = [](MasterScore* sc, int idx) -> Measure* {
        Measure* m = sc->firstMeasure();
        for (int i = 0; i < idx && m; ++i) {
            m = m->nextMeasure();
        }
        return m;
    };
    auto hasPitchedNotes = [](Measure* m) -> bool {
        if (!m) {
            return false;
        }
        for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            for (size_t v = 0; v < VOICES; ++v) {
                EngravingItem* el = s->element(v);
                if (el && el->isChord()) {
                    return true;
                }
            }
        }
        return false;
    };

    // Measures 1-2 are the plain single-measure rests preceding the mrest (the trigger case).
    EXPECT_FALSE(hasPitchedNotes(measureAt(score, 0))) << "measure 1 must be empty";
    EXPECT_FALSE(hasPitchedNotes(measureAt(score, 1))) << "measure 2 must be empty";
    for (int i = 2; i < 9; ++i) {
        EXPECT_FALSE(hasPitchedNotes(measureAt(score, i)))
            << "measure " << (i + 1) << " must be empty (mrest expansion)";
    }
    EXPECT_TRUE(hasPitchedNotes(measureAt(score, 9))) << "measure 10 must have notes";
    EXPECT_FALSE(hasPitchedNotes(measureAt(score, 10))) << "measure 11 (pad) must be empty";
    EXPECT_FALSE(hasPitchedNotes(measureAt(score, 11))) << "measure 12 (pad) must be empty";

    delete score;
}

// Regression: a multi-staff mrest block holds one REST per staff, so the old size!=1 guard
// collapsed the block to a single measure for any multi-staff file. Fixture: 2 staves, mrest=7.
TEST_F(Tst_NotesMrest, mrest_single_block_expands_for_multi_staff_file)
{
    MasterScore* score = readEncoreScore("importer_mrest_multistaff.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_mrest_multistaff.enc";
    ASSERT_EQ(score->nmeasures(), 8)
        << "mrest=7 across 2 staves must expand to 7 measures + 1 note; without fix only 2";

    auto measureAt = [](MasterScore* sc, int idx) -> Measure* {
        Measure* m = sc->firstMeasure();
        for (int i = 0; i < idx && m; ++i) {
            m = m->nextMeasure();
        }
        return m;
    };
    auto hasPitchedNotes = [](Measure* m) -> bool {
        if (!m) {
            return false;
        }
        for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            for (size_t v = 0; v < VOICES; ++v) {
                EngravingItem* el = s->element(v);
                if (el && el->isChord()) {
                    return true;
                }
            }
        }
        return false;
    };

    for (int i = 0; i < 7; ++i) {
        EXPECT_FALSE(hasPitchedNotes(measureAt(score, i)))
            << "measure " << (i + 1) << " must be empty (mrest expansion)";
    }
    EXPECT_TRUE(hasPitchedNotes(measureAt(score, 7))) << "measure 8 must have notes";

    delete score;
}

// createMultiMeasureRests must be set only when the file has a mrestCount>1 block; it used to be
// always true, wrongly collapsing separate rest measures in files with only individual rests.
TEST_F(Tst_NotesMrest, mmrest_flag_off_when_file_has_no_mrest_blocks)
{
    MasterScore* score = readEncoreScore("bazo.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_FALSE(score->style().styleB(Sid::createMultiMeasureRests))
        << "createMultiMeasureRests must be FALSE when the file has no mrestCount>1 blocks";
    delete score;
}

TEST_F(Tst_NotesMrest, mmrest_flag_on_when_file_has_mrest_block)
{
    MasterScore* score = readEncoreScore("importer_mrest_single_block.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->style().styleB(Sid::createMultiMeasureRests))
        << "createMultiMeasureRests must be TRUE when the file contains mrestCount>1 blocks";
    delete score;
}
