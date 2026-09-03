/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "engraving/dom/stafftext.h"

#include "importexport/tabledit/internal/tableditreader.h"

using namespace mu::engraving;

static const muse::String TABLEDIT_DIR("data/");

class TablEdit_Tests : public ::testing::Test
{
public:
    void tefReadTest(const char* file);
    MasterScore* tefRead(const char* file, Err& err);
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

//---------------------------------------------------------
//   tefRead
//   read a TablEdit file and report the importer's error code
//   (ScoreRW::readScore() returns nullptr if the error code is not NoError,
//   and a laid out score otherwise)
//---------------------------------------------------------

MasterScore* TablEdit_Tests::tefRead(const char* file, Err& err)
{
    auto importFunc = [&err](MasterScore* score, const muse::io::path_t& path) -> Err {
        mu::iex::tabledit::TablEditReader tablEditReader;
        err = tablEditReader.import(score, path);
        return err;
    };

    return ScoreRW::readScore(TABLEDIT_DIR + String::fromUtf8(file) + u".tef", false, importFunc);
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

TEST_F(TablEdit_Tests, tef_effects) {
    tefReadTest("effects");
}

TEST_F(TablEdit_Tests, tef_fingerings_1) {
    tefReadTest("fingerings_1");
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

TEST_F(TablEdit_Tests, tef_multi_track_frets) {
    tefReadTest("multi_track_frets");
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

TEST_F(TablEdit_Tests, tef_reading_list_2) {
    tefReadTest("reading_list_2");
}

TEST_F(TablEdit_Tests, tef_reading_list_3) {
    tefReadTest("reading_list_3");
}

TEST_F(TablEdit_Tests, tef_reading_list_4) {
    tefReadTest("reading_list_4");
}

TEST_F(TablEdit_Tests, tef_reading_list_5) {
    tefReadTest("reading_list_5");
}

TEST_F(TablEdit_Tests, tef_reading_list_6) {
    tefReadTest("reading_list_6");
}

TEST_F(TablEdit_Tests, tef_reading_list_7) {
    tefReadTest("reading_list_7");
}

TEST_F(TablEdit_Tests, tef_reading_list_8) {
    tefReadTest("reading_list_8");
}

TEST_F(TablEdit_Tests, tef_reading_list_9) {
    tefReadTest("reading_list_9");
}

TEST_F(TablEdit_Tests, tef_reading_list_10) {
    tefReadTest("reading_list_10");
}

TEST_F(TablEdit_Tests, tef_reading_list_11) {
    tefReadTest("reading_list_11");
}

TEST_F(TablEdit_Tests, tef_reading_list_12) {
    tefReadTest("reading_list_12");
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

TEST_F(TablEdit_Tests, tef_tie_3) {
    tefReadTest("tie_3");
}

#if 0
// generate reference file and enable when issue #32769 has been solved
TEST_F(TablEdit_Tests, tef_tie_4) {
    tefReadTest("tie_4");
}
#endif

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

//---------------------------------------------------------
//   malformed input fix tests
//
//   The fixtures used below are minimal byte patches of existing valid test
//   files, each one crafted to reach a specific guard in importtef.cpp.
//   The patch is documented in the test comment as
//   "<source file> @<offset>: <old> -> <new>".
//---------------------------------------------------------

struct TefCounts {
    size_t measures { 0 };
    size_t chords { 0 };
    size_t notes { 0 };
    size_t rests { 0 };
    size_t staffTexts { 0 };
};

static TefCounts tefCount(Score* score)
{
    TefCounts counts;
    counts.measures = score->nmeasures();
    for (Segment* segment = score->firstSegment(SegmentType::All); segment; segment = segment->next1()) {
        for (EngravingItem* annotation : segment->annotations()) {
            if (annotation->isStaffText()) {
                ++counts.staffTexts;
            }
        }
        for (track_idx_t track = 0; track < score->ntracks(); ++track) {
            EngravingItem* element = segment->element(track);
            if (!element) {
                continue;
            }
            if (element->isChord()) {
                ++counts.chords;
                counts.notes += toChord(element)->notes().size();
            } else if (element->isRest()) {
                ++counts.rests;
            }
        }
    }
    return counts;
}

static std::vector<std::string> tefStaffTexts(Score* score)
{
    std::vector<std::string> result;
    for (Segment* segment = score->firstSegment(SegmentType::All); segment; segment = segment->next1()) {
        for (EngravingItem* annotation : segment->annotations()) {
            if (annotation->isStaffText()) {
                result.push_back(toStaffText(annotation)->plainText().toStdString());
            }
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

//---------------------------------------------------------
//   tef_invalid_text_marker_index
//   fix 960fbb7ee9 / 592d6d8f2c: out of range text marker index
//
//   fixture: staff_text_1.tef (1 text, 1 text marker with index 0)
//   patch: @0x379 (byte2 of the 0x39 text marker record, holding the low byte
//          of the text index): 0x00 -> 0x05
//   -> createTexts() finds index 5 while tefTexts holds a single entry
//---------------------------------------------------------

TEST_F(TablEdit_Tests, tef_invalid_text_marker_index) {
    Err err { Err::NoError };
    MasterScore* base = tefRead("staff_text_1", err);
    ASSERT_TRUE(base);
    EXPECT_EQ(err, Err::NoError);
    const TefCounts baseCounts { tefCount(base) };
    EXPECT_EQ(baseCounts.staffTexts, 1);
    delete base;

    err = Err::NoError;
    MasterScore* score = tefRead("invalid_text_marker_index", err);
    ASSERT_TRUE(score);
    EXPECT_EQ(err, Err::NoError);
    const TefCounts counts { tefCount(score) };
    // the invalid text marker is dropped, the rest of the score is unaffected
    EXPECT_EQ(counts.staffTexts, 0);
    EXPECT_EQ(counts.measures, baseCounts.measures);
    EXPECT_EQ(counts.chords, baseCounts.chords);
    EXPECT_EQ(counts.notes, baseCounts.notes);
    delete score;
}

//---------------------------------------------------------
//   tef_invalid_measure_denominator
//   fix 8263d9ad41: measure with denominator zero
//
//   fixture: gaps_1.tef (3 measures of 4/4, one note in each measure at
//            positions 48, 64 and 128)
//   patch a: @0x139 (denominator byte of the third measure record): 0x04 -> 0x00
//   patch b: @0x383 (byte1 of the note record at position 128, i.e. the note in
//            the measure removed by patch a): 0x04 -> 0x3a, a marker value that
//            is neither a note/rest (<= 0x33) nor a text marker (0x39), so
//            readTefContents() ignores the record.
//            Patch b is needed because a note outside every measure makes
//            MeasureHandler::measureIndex() return muse::nidx, after which
//            sumPreviousGaps(nidx) walks off the end of its gap vectors and
//            throws std::out_of_range. That defect is unrelated to the fixes
//            under test and would mask the result of this test.
//   -> readTefMeasures() rejects the corrupt measure record, the score is
//      created with the 2 remaining measures
//---------------------------------------------------------

TEST_F(TablEdit_Tests, tef_invalid_measure_denominator) {
    Err err { Err::NoError };
    MasterScore* base = tefRead("gaps_1", err);
    ASSERT_TRUE(base);
    EXPECT_EQ(err, Err::NoError);
    const TefCounts baseCounts { tefCount(base) };
    EXPECT_EQ(baseCounts.measures, 3);
    delete base;

    err = Err::NoError;
    MasterScore* score = tefRead("invalid_measure_denominator", err);
    ASSERT_TRUE(score);
    EXPECT_EQ(err, Err::NoError);
    const TefCounts counts { tefCount(score) };
    // one measure record less, and (because of patch b) one note less
    EXPECT_EQ(counts.measures, 2);
    EXPECT_EQ(counts.chords, baseCounts.chords - 2);   // 2: one per linked staff
    EXPECT_EQ(counts.notes, baseCounts.notes - 2);
    delete score;
}

//---------------------------------------------------------
//   tef_invalid_text_zero_size
//   fix 7594b2a0b9: utf8 text with size zero
//
//   fixture: staff_text_2.tef (4 texts "Part 1", "P1 M2", "Part 2", "P2 M2",
//            4 text markers using index 0 to 3)
//   patch: the texts table @0x11d is rewritten in place, keeping 4 texts but
//          giving text[0] size 0 (so it has neither payload nor terminating
//          NUL); the 7 bytes that become free at the end of the table are
//          zero filled. The table stays inside its own block: every other
//          block is reached by an absolute seek, so nothing else moves.
//   -> readUtf8Text() reads a size of 0 and must not consume the (absent)
//      terminating NUL, otherwise the texts that follow are read one byte off
//---------------------------------------------------------

TEST_F(TablEdit_Tests, tef_invalid_text_zero_size) {
    Err err { Err::NoError };
    MasterScore* base = tefRead("staff_text_2", err);
    ASSERT_TRUE(base);
    EXPECT_EQ(err, Err::NoError);
    const std::vector<std::string> baseTexts { tefStaffTexts(base) };
    const std::vector<std::string> expectedBaseTexts { "P1 M2", "P2 M2", "Part 1", "Part 2" };
    EXPECT_EQ(baseTexts, expectedBaseTexts);
    delete base;

    err = Err::NoError;
    MasterScore* score = tefRead("invalid_text_zero_size", err);
    ASSERT_TRUE(score);
    EXPECT_EQ(err, Err::NoError);
    // text[0] is empty, the three texts after it are still read correctly
    const std::vector<std::string> texts { tefStaffTexts(score) };
    const std::vector<std::string> expectedTexts { "", "P1 M2", "P2 M2", "Part 2" };
    EXPECT_EQ(texts, expectedTexts);
    delete score;
}
