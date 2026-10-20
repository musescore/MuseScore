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

#include "global/io/file.h"
#include "global/types/bytearray.h"

#include "engraving/engravingerrors.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"

#include "engraving/tests/utils/scorecomp.h"
#include "engraving/tests/utils/scorerw.h"

#include "importexport/bb/internal/bb.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::iex::bb {
extern engraving::Err importBB(MasterScore*, const QString& name);
}

using namespace mu::iex::bb;

static const String BIAB_DIR("data/");

class Bb_Tests : public ::testing::Test
{
public:
    void biabReadTest(const char* file);

    MasterScore* biabRead(const char* file, engraving::Err& err);
    size_t biabNoteEventCount(const char* file);
};

//---------------------------------------------------------
//   collect
//   every element of the given type in the score
//---------------------------------------------------------

static std::vector<EngravingItem*> collect(Score* score, ElementType type)
{
    std::vector<EngravingItem*> found;
    for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
        for (Segment* s = m->first(); s; s = s->next()) {
            for (EngravingItem* e : s->elist()) {
                if (e && e->type() == type) {
                    found.push_back(e);
                }
            }
            for (EngravingItem* e : s->annotations()) {
                if (e && e->type() == type) {
                    found.push_back(e);
                }
            }
        }
    }
    return found;
}

//---------------------------------------------------------
//   biabReadTest
//   read a Band-in-a-Box file, write to a MuseScore file and verify against reference
//   every "xxx" test requires a *.SGU file and a *.mscx file:
//          xxx.SGU      is the SGU file
//          xxx-ref.mscx is the corresponding (correct) mscore file
//---------------------------------------------------------

void Bb_Tests::biabReadTest(const char* file)
{
    auto importFunc = [](MasterScore* score, const muse::io::path_t& path) -> engraving::Err {
        return mu::iex::bb::importBB(score, path.toQString());
    };

    String fileName = String::fromUtf8(file);
    MasterScore* score = ScoreRW::readScore(BIAB_DIR + fileName + u".SGU", false, importFunc);
    EXPECT_TRUE(score);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, fileName + u"-test.mscx", BIAB_DIR + fileName + u"-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//   biabRead
//   read a Band-in-a-Box file and report the error importBB() rejects it with;
//   returns nullptr when it is rejected
//---------------------------------------------------------

MasterScore* Bb_Tests::biabRead(const char* file, engraving::Err& err)
{
    auto importFunc = [&err](MasterScore* score, const muse::io::path_t& path) -> engraving::Err {
        err = mu::iex::bb::importBB(score, path.toQString());
        return err;
    };

    return ScoreRW::readScore(BIAB_DIR + String::fromUtf8(file) + u".SGU", false, importFunc);
}

//---------------------------------------------------------
//   biabNoteEventCount
//   parse a Band-in-a-Box file and count the note events it kept, across all tracks
//---------------------------------------------------------

size_t Bb_Tests::biabNoteEventCount(const char* file)
{
    String path = ScoreRW::rootPath() + u"/" + BIAB_DIR + String::fromUtf8(file) + u".SGU";

    BBFile bb;
    if (!bb.read(path.toQString())) {
        ADD_FAILURE() << "BBFile::read rejected " << file;
        return 0;
    }

    size_t count = 0;
    for (const BBTrack* track : *bb.tracks()) {
        count += track->events().size();
    }
    return count;
}

TEST_F(Bb_Tests, biabChords) {
    biabReadTest("chords");
}

TEST_F(Bb_Tests, biabChordCount) {
    engraving::Err err = engraving::Err::UnknownError;
    MasterScore* score = biabRead("chords", err);
    ASSERT_TRUE(score);
    EXPECT_EQ(err, engraving::Err::NoError);
    EXPECT_EQ(collect(score, ElementType::HARMONY).size(), 48u);
    EXPECT_EQ(collect(score, ElementType::CHORD).size(), 0u);
    delete score;
}

//---------------------------------------------------------
//   malformed files
//
//   Each .SGU below is a documented byte patch of chords.SGU that drives the importer
//   into a bounds check. SGU is a flat byte array indexed directly by BBFile::read(),
//   so an unguarded field length walks straight off the end of the buffer.
//---------------------------------------------------------

// The MIDI event loop's range guard: eventStart is moved to 1969, six bytes short of
// what a 12 byte event record needs, so the record runs past the end of the file.
TEST_F(Bb_Tests, biabEventOutOfRange) {
    engraving::Err err = engraving::Err::UnknownError;
    EXPECT_FALSE(biabRead("chords-event-out-of-range", err));
    EXPECT_EQ(err, engraving::Err::FileOpenError);
}

// The chord root/extension count guard: the file lists 47 chord extensions but 48 chord
// roots, so the root loop indexes one past the end of the extension list.
TEST_F(Bb_Tests, biabRootExtensionMismatch) {
    engraving::Err err = engraving::Err::UnknownError;
    EXPECT_FALSE(biabRead("chords-root-extension-mismatch", err));
    EXPECT_EQ(err, engraving::Err::FileOpenError);
}

// table[c.root - 1] with root 0, which used to read table[-1]. The chord is dropped, so
// the score keeps the other 47 chord symbols.
TEST_F(Bb_Tests, biabChordRootZero) {
    engraving::Err err = engraving::Err::UnknownError;
    MasterScore* score = biabRead("chords-root-zero", err);
    ASSERT_TRUE(score);
    EXPECT_EQ(err, engraving::Err::NoError);
    EXPECT_EQ(collect(score, ElementType::HARMONY).size(), 47u);
    delete score;
}

// table[c.bass - 1] with bass 18, one past the end of the 17 entry table. The chord is
// still imported -- only its bass note is dropped -- so all 48 survive with no bass.
TEST_F(Bb_Tests, biabChordBassOutOfRange) {
    engraving::Err err = engraving::Err::UnknownError;
    MasterScore* score = biabRead("chords-bass-out-of-range", err);
    ASSERT_TRUE(score);
    EXPECT_EQ(err, engraving::Err::NoError);

    std::vector<EngravingItem*> harmonies = collect(score, ElementType::HARMONY);
    EXPECT_EQ(harmonies.size(), 48u);
    for (EngravingItem* e : harmonies) {
        for (const HarmonyInfo* info : toHarmony(e)->chords()) {
            EXPECT_EQ(info->bassTpc(), int(Tpc::TPC_INVALID));
        }
    }
    delete score;
}

// A note event whose 32 bit tick field is 0xFFFFFFFF. Summing the four bytes into an int
// overflowed and yielded -1, which was accepted as a tick; the event is now skipped. Both
// fixtures below hold that bad event followed by a valid one, so the surviving count
// separates "skipped" from "kept with a garbage tick".
//
// Asserted against BBFile::read() rather than importBB(), which cannot import a file with
// a melody at all: once a track exists but does not cover every measure, the trailing fill
// loop in importBB() passes the null segment findSegment() just failed to return straight
// to Factory::createRest().
TEST_F(Bb_Tests, biabEventTickOverflow) {
    EXPECT_EQ(biabNoteEventCount("chords-event-tick-overflow"), 1u);
}

// The same, for a note event's 32 bit length field. A negative length also overflowed the
// `len1 * Constants::DIVISION` product used to compute the duration.
TEST_F(Bb_Tests, biabEventLengthOverflow) {
    EXPECT_EQ(biabNoteEventCount("chords-event-length-overflow"), 1u);
}

// Truncation sweep over every prefix of chords.SGU. BBFile::read() checkRange()s each
// field before indexing it, so no prefix may read past the end of the buffer -- which is
// what this asserts, with the sanitizer as the oracle. A prefix that still parses is
// legitimate rather than a failure: the trailing four bytes are the event header, so a
// short file can happen to declare zero events and read as a melody-less score.
//
// This drives BBFile::read() directly. The guards it covers all live there, and importBB()
// could not be swept the same way: a prefix that happens to declare a melody would hit the
// createRest() crash described above.
TEST_F(Bb_Tests, biabTruncated) {
    muse::ByteArray full;
    ASSERT_TRUE(muse::io::File::readFile(ScoreRW::rootPath() + u"/" + BIAB_DIR + u"chords.SGU", full));
    ASSERT_EQ(full.size(), 1975u);

    const muse::io::path_t truncated = "chords-truncated.SGU";
    size_t rejected = 0;

    for (size_t len = 0; len < full.size(); ++len) {
        ASSERT_TRUE(muse::io::File::writeFile(truncated, full.left(len)));

        BBFile bb;
        if (!bb.read(truncated.toQString())) {
            ++rejected;
        }
    }

    EXPECT_GT(rejected, 0u);
    muse::io::File::remove(truncated);
}
