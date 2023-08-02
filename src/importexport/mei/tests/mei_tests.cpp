/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "io/file.h"

#include "engraving/tests/utils/scorerw.h"
#include "engraving/tests/utils/scorecomp.h"

#include "libmscore/masterscore.h"
#include "libmscore/excerpt.h"

#include "modularity/ioc.h"
#include "importexport/mei/imeiconfiguration.h"
#include "importexport/mei/internal/meireader.h"
#include "importexport/mei/internal/meiwriter.h"

using namespace mu;
using namespace mu::engraving;

static const String MEI_DIR(u"data/");

namespace mu::iex::mei {
class Mei_Tests : public ::testing::Test
{
public:
    void meiReadTest(const char* file,  const char* ext);

    // Change this to true to re-generate reference files
    inline static bool s_generateReferenceFile = false;
};

void Mei_Tests::meiReadTest(const char* file, const char* ext)
{
    String fileName = String::fromUtf8(file);

    auto importFunc = [](MasterScore* score, const io::path_t& path) -> Err {
        MeiReader meiReader;
        return meiReader.import(score, path);
    };

    auto exportFunc = [](Score* score, const io::path_t& path) -> Err {
        MeiWriter meiWriter;
        return meiWriter.writeScore(score, path);
    };

    // Load the .mei file
    MasterScore* score = ScoreRW::readScore(MEI_DIR + fileName + u"." + String::fromUtf8(ext), false, importFunc);
    EXPECT_TRUE(score);

    // Flag to be turned on to generate the test reference .mscx files from the .mei
    if (s_generateReferenceFile) {
        bool res = ScoreRW::saveScore(score, ScoreRW::rootPath() + u"/" + MEI_DIR + fileName + u".mscx");
        EXPECT_TRUE(res);
        return;
    }

    // Compare with the reference MuseScore file
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, fileName + u".mscx", MEI_DIR + fileName + u".mscx"));

    // Save the .mei file for round trip testing
    bool output = ScoreRW::saveScore(score,  fileName + u".test.mei", exportFunc);
    EXPECT_TRUE(output);
    delete score;

    // Compare the mei files
    EXPECT_TRUE(ScoreComp::compareFiles(fileName + u".test.mei", ScoreRW::rootPath() + u"/" + MEI_DIR + fileName + u".mei"));

    /*
    // Reload the generated .mei file
    MasterScore* score2 = ScoreRW::readScore(fileName + u".test.mei", true, importFunc);
    EXPECT_TRUE(score2);

    // Compare with the reference MuseScore file
    EXPECT_TRUE(ScoreComp::saveCompareScore(score2, fileName + u".mscx", MEI_DIR + fileName + u".mscx"));
    delete score2;
    */
}

TEST_F(Mei_Tests, mei_accid_01) {
    meiReadTest("accid-01", "mei");
}

TEST_F(Mei_Tests, mei_accid_02) {
    meiReadTest("accid-02", "mei");
}

TEST_F(Mei_Tests, mei_beam_01) {
    meiReadTest("beam-01", "mei");
}

TEST_F(Mei_Tests, mei_beam_02) {
    meiReadTest("beam-02", "mei");
}

TEST_F(Mei_Tests, mei_beam_03) {
    meiReadTest("beam-03", "mei");
}

TEST_F(Mei_Tests, mei_breaks_01) {
    meiReadTest("breaks-01", "mei");
}

TEST_F(Mei_Tests, mei_breath_01) {
    meiReadTest("breath-01", "mei");
}

TEST_F(Mei_Tests, mei_chord_label_01) {
    meiReadTest("chord-label-01", "mei");
}

TEST_F(Mei_Tests, mei_clef_01) {
    meiReadTest("clef-01", "mei");
}

TEST_F(Mei_Tests, mei_cross_staff_01) {
    meiReadTest("cross-staff-01", "mei");
}

TEST_F(Mei_Tests, mei_dir_01) {
    meiReadTest("dir-01", "mei");
}

TEST_F(Mei_Tests, mei_dynamic_01) {
    meiReadTest("dynamic-01", "mei");
}

TEST_F(Mei_Tests, mei_ending_01) {
    meiReadTest("ending-01", "mei");
}

TEST_F(Mei_Tests, mei_fermata_01) {
    meiReadTest("fermata-01", "mei");
}

TEST_F(Mei_Tests, mei_gracenote_01) {
    meiReadTest("gracenote-01", "mei");
}

TEST_F(Mei_Tests, mei_gracenote_02) {
    meiReadTest("gracenote-02", "mei");
}

TEST_F(Mei_Tests, mei_jump_01) {
    meiReadTest("jump-01", "mei");
}

TEST_F(Mei_Tests, mei_jump_02) {
    meiReadTest("jump-02", "mei");
}

TEST_F(Mei_Tests, mei_key_signature_01) {
    meiReadTest("key-signature-01", "mei");
}

TEST_F(Mei_Tests, mei_measure_01) {
    meiReadTest("measure-01", "mei");
}

TEST_F(Mei_Tests, mei_measure_02) {
    meiReadTest("measure-02", "mei");
}

TEST_F(Mei_Tests, mei_metadata_01) {
    meiReadTest("metadata-01", "mei");
}

TEST_F(Mei_Tests, mei_page_head_01) {
    meiReadTest("page-head-01", "mei");
}

TEST_F(Mei_Tests, mei_page_head_02) {
    meiReadTest("page-head-02", "mei");
}

TEST_F(Mei_Tests, mei_roman_numeral_01) {
    meiReadTest("roman-numeral-01", "mei");
}

TEST_F(Mei_Tests, mei_score_01) {
    meiReadTest("score-01", "mei");
}

TEST_F(Mei_Tests, mei_score_02) {
    meiReadTest("score-02", "mei");
}

TEST_F(Mei_Tests, mei_slur_01) {
    meiReadTest("slur-01", "mei");
}

TEST_F(Mei_Tests, mei_slur_02) {
    meiReadTest("slur-02", "mei");
}

TEST_F(Mei_Tests, mei_stem_01) {
    meiReadTest("stem-01", "mei");
}

TEST_F(Mei_Tests, mei_tempo_01) {
    meiReadTest("tempo-01", "mei");
}

TEST_F(Mei_Tests, mei_tie_01) {
    meiReadTest("tie-01", "mei");
}

TEST_F(Mei_Tests, mei_time_signature_01) {
    meiReadTest("time-signature-01", "mei");
}

TEST_F(Mei_Tests, mei_time_signature_02) {
    meiReadTest("time-signature-02", "mei");
}

TEST_F(Mei_Tests, mei_transpose_01) {
    meiReadTest("transpose-01", "mei");
}

TEST_F(Mei_Tests, mei_tuplet_01) {
    meiReadTest("tuplet-01", "mei");
}

TEST_F(Mei_Tests, mei_tuplet_02) {
    meiReadTest("tuplet-02", "mei");
}

TEST_F(Mei_Tests, mei_tuplet_03) {
    meiReadTest("tuplet-03", "mei");
}
}
