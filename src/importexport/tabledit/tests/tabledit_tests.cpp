/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "engraving/tests/utils/scorecomp.h"
#include "engraving/tests/utils/scorerw.h"

#include "importexport/tabledit/internal/tableditreader.h"

using namespace mu::engraving;

static const muse::String TABLEDIT_DIR("data/");

class TablEdit_Tests : public ::testing::Test
{
public:
    void tefReadTest(const char* file);
};

//---------------------------------------------------------
//   tefReadTest
//   read a TablEdit file, write to a MuseScore file and verify against reference
//---------------------------------------------------------

void TablEdit_Tests::tefReadTest(const char* file)
{
    auto importFunc = [](MasterScore* score, const muse::io::path_t& path) -> Err {
        mu::iex::tabledit::TablEditReader tablEditReader;
        return tablEditReader.import(score, path);
    };

    String fileName = String::fromUtf8(file);
    MasterScore* score = ScoreRW::readScore(TABLEDIT_DIR + fileName + ".tef", false, importFunc);
    EXPECT_TRUE(score);
    score->setMetaTag(u"originalFormat", u"tef");

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, fileName + u".mscx", TABLEDIT_DIR + fileName + u".mscx"));
    delete score;
}

TEST_F(TablEdit_Tests, tef_bass) {
    tefReadTest("bass");
}

TEST_F(TablEdit_Tests, tef_chord_C_D) {
    tefReadTest("chord_C_D");
}

TEST_F(TablEdit_Tests, tef_dynamic) {
    tefReadTest("dynamic");
}

TEST_F(TablEdit_Tests, tef_gaps_1) {
    tefReadTest("gaps_1");
}

TEST_F(TablEdit_Tests, tef_gaps_2) {
    tefReadTest("gaps_2");
}

TEST_F(TablEdit_Tests, tef_grace_1) {
    tefReadTest("grace_1");
}

TEST_F(TablEdit_Tests, tef_guitar) {
    tefReadTest("guitar");
}

TEST_F(TablEdit_Tests, tef_guitar_bass) {
    tefReadTest("guitar_bass");
}

TEST_F(TablEdit_Tests, tef_guitar_drop_D) {
    tefReadTest("guitar_drop_D");
}

TEST_F(TablEdit_Tests, tef_guitar_new_standard_tuning) {
    tefReadTest("guitar_new_standard_tuning");
}

TEST_F(TablEdit_Tests, tef_key_signatures) {
    tefReadTest("key_signatures");
}

TEST_F(TablEdit_Tests, tef_key_signatures_2) {
    tefReadTest("key_signatures_2");
}

TEST_F(TablEdit_Tests, tef_metadata) {
    tefReadTest("metadata");
}

TEST_F(TablEdit_Tests, tef_multi_track_rests) {
    tefReadTest("multi_track_rests");
}

TEST_F(TablEdit_Tests, tef_notes_dotted) {
    tefReadTest("notes_dotted");
}

TEST_F(TablEdit_Tests, tef_notes_normal) {
    tefReadTest("notes_normal");
}

TEST_F(TablEdit_Tests, tef_pickup_measure) {
    tefReadTest("pickup_measure");
}

TEST_F(TablEdit_Tests, tef_positions) {
    tefReadTest("positions");
}

TEST_F(TablEdit_Tests, tef_reading_list_1) {
    tefReadTest("reading_list_1");
}

TEST_F(TablEdit_Tests, tef_rests_dotted) {
    tefReadTest("rests_dotted");
}

TEST_F(TablEdit_Tests, tef_rests_normal) {
    tefReadTest("rests_normal");
}

TEST_F(TablEdit_Tests, tef_staff_text_1) {
    tefReadTest("staff_text_1");
}

TEST_F(TablEdit_Tests, tef_staff_text_2) {
    tefReadTest("staff_text_2");
}

TEST_F(TablEdit_Tests, tef_tie_1) {
    tefReadTest("tie_1");
}

TEST_F(TablEdit_Tests, tef_tie_2) {
    tefReadTest("tie_2");
}

TEST_F(TablEdit_Tests, tef_time_signatures) {
    tefReadTest("time_signatures");
}

TEST_F(TablEdit_Tests, tef_time_signatures_2) {
    tefReadTest("time_signatures_2");
}

TEST_F(TablEdit_Tests, tef_triplet_eighths) {
    tefReadTest("triplet_eighths");
}

TEST_F(TablEdit_Tests, tef_triplet_quarters) {
    tefReadTest("triplet_quarters");
}

TEST_F(TablEdit_Tests, tef_triplets_mixed) {
    tefReadTest("triplets_mixed");
}

TEST_F(TablEdit_Tests, tef_voices) {
    tefReadTest("voices");
}

TEST_F(TablEdit_Tests, tef_voices_multi_part) {
    tefReadTest("voices_multi_part");
}
