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

#include <QFile>

#include "engraving/engravingerrors.h"
#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/masterscore.h"

#include "engraving/tests/utils/scorecomp.h"
#include "engraving/tests/utils/scorerw.h"

#include "importexport/capella/internal/capella.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::iex::capella {
extern engraving::Err importCapella(MasterScore*, const QString& name);
extern engraving::Err importCapXml(MasterScore*, const QString& name);
extern void convertCapella(Score* score, Capella* cap, bool capxMode);
}

using namespace mu::iex::capella;

static const String CAPELLA_DIR("data/");

class Capella_Tests : public ::testing::Test
{
public:
    void capReadTest(const char* file);
    void capxReadTest(const char* file);

    Capella::Error capReadError(const char* file);
    engraving::Err capImport(const char* file);
};

//---------------------------------------------------------
//   capReadTest
//   read a Capella file, write to a MuseScore file and verify against reference
//---------------------------------------------------------

void Capella_Tests::capReadTest(const char* file)
{
    auto importFunc = [](MasterScore* score, const muse::io::path_t& path) -> engraving::Err {
        return mu::iex::capella::importCapella(score, path.toQString());
    };

    String fileName = String::fromUtf8(file);
    MasterScore* score = ScoreRW::readScore(CAPELLA_DIR + fileName + ".cap", false, importFunc);
    EXPECT_TRUE(score);
    score->setMetaTag(u"originalFormat", u"cap");

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, fileName + u".mscx", CAPELLA_DIR + fileName + u".cap-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//   capxReadTest
//   read a CapellaXML file, write to a MuseScore file and verify against reference
//---------------------------------------------------------

void Capella_Tests::capxReadTest(const char* file)
{
    auto importFunc = [](MasterScore* score, const muse::io::path_t& path) -> engraving::Err {
        return mu::iex::capella::importCapXml(score, path.toQString());
    };

    String fileName = String::fromUtf8(file);
    MasterScore* score = ScoreRW::readScore(CAPELLA_DIR + fileName + ".capx", false, importFunc);
    EXPECT_TRUE(score);
    score->setMetaTag(u"originalFormat", u"capx");

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, fileName + u".mscx", CAPELLA_DIR + fileName + u".capx-ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//   capReadError
//   read a Capella file the way importCapella() does and report the error it is
//   rejected with, or Error::CAP_NO_ERROR if the importer accepts it
//---------------------------------------------------------

Capella::Error Capella_Tests::capReadError(const char* file)
{
    String path = ScoreRW::rootPath() + u"/" + CAPELLA_DIR + String::fromUtf8(file) + u".cap";
    QFile fp(path.toQString());
    if (!fp.open(QIODevice::ReadOnly)) {
        ADD_FAILURE() << "cannot open " << file;
        return Capella::Error::CAP_NO_ERROR;
    }

    MasterScore* score = compat::ScoreAccess::createMasterScoreWithBaseStyle(nullptr);
    Capella::Error error = Capella::Error::CAP_NO_ERROR;

    Capella cf;
    try {
        cf.read(&fp);
        convertCapella(score, &cf, false);
    } catch (Capella::Error errNo) {
        error = errNo;
    }

    delete score;
    return error;
}

//---------------------------------------------------------
//   capImport
//   run the public entry point over a Capella file
//---------------------------------------------------------

engraving::Err Capella_Tests::capImport(const char* file)
{
    String path = ScoreRW::rootPath() + u"/" + CAPELLA_DIR + String::fromUtf8(file) + u".cap";
    MasterScore* score = compat::ScoreAccess::createMasterScoreWithBaseStyle(nullptr);
    engraving::Err err = mu::iex::capella::importCapella(score, path.toQString());
    delete score;
    return err;
}

TEST_F(Capella_Tests, capTest1) {
    capReadTest("test1");
}
TEST_F(Capella_Tests, capTest2) {
    capReadTest("test2");
}
TEST_F(Capella_Tests, capTest3) {
    capReadTest("test3");
}
TEST_F(Capella_Tests, capTest4) {
    // wrong enharmonic spelling
    capReadTest("test4");
}
TEST_F(Capella_Tests, capTest5) {
    capReadTest("test5");
}
TEST_F(Capella_Tests, capTest6) {
    capReadTest("test6");
}
TEST_F(Capella_Tests, capTest7) {
    // double bar missing (auto-generated by Capella programs)
    capReadTest("test7");
}
TEST_F(Capella_Tests, capTest8) {
    capReadTest("test8");
}
TEST_F(Capella_Tests, capTestTuplet2) {
    // generates different beaming with respect to the original
    capReadTest("testTuplet2");
}
TEST_F(Capella_Tests, capxTest1) {
    capxReadTest("test1");
}
TEST_F(Capella_Tests, capxTest2) {
    capxReadTest("test2");
}
TEST_F(Capella_Tests, capxTest3) {
    capxReadTest("test3");
}
TEST_F(Capella_Tests, capxTest4) {
    // wrong enharmonic spelling
    capxReadTest("test4");
}
TEST_F(Capella_Tests, capxTest5) {
    capxReadTest("test5");
}
TEST_F(Capella_Tests, capxTest6) {
    capxReadTest("test6");
}
TEST_F(Capella_Tests, capxTest7) {
    // double bar missing (auto-generated by Capella programs)
    capxReadTest("test7");
}
TEST_F(Capella_Tests, capxTestEmptyStaff1) {
    capxReadTest("testEmptyStaff1");
}
TEST_F(Capella_Tests, capxTestEmptyStaff2) {
    capxReadTest("testEmptyStaff2");
}
TEST_F(Capella_Tests, capxTestPianoG4G5) {
    capxReadTest("testPianoG4G5");
}
TEST_F(Capella_Tests, capxTestScaleC4C5) {
    capxReadTest("testScaleC4C5");
}
TEST_F(Capella_Tests, capxTestSlurTie) {
    capxReadTest("testSlurTie");
}
TEST_F(Capella_Tests, capxTestText1) {
    capxReadTest("testText1");
}
TEST_F(Capella_Tests, capxTestTuplet1) {
    // generates different (incorrect ?) l1 and l2 values in beams
    capxReadTest("testTuplet1");
}
TEST_F(Capella_Tests, capxTestTuplet2) {
    // generates different beaming with respect to the original
    capxReadTest("testTuplet2");
}
TEST_F(Capella_Tests, capxTestTuplet3) {
    // generates different beaming with respect to the original
    capxReadTest("testTuplet3");
}
TEST_F(Capella_Tests, capxTestVolta1) {
    capxReadTest("testVolta1");
}
TEST_F(Capella_Tests, capxTestBarline) {
    capxReadTest("testBarline");
}

//---------------------------------------------------------
//   malformed files
//
//   Every file below is a documented byte patch of one of the valid files above,
//   crafted to drive the importer into a bounds check. The importer must reject the
//   file rather than read out of bounds, and importCapella() must swallow the error
//   (it reports it to the user itself) instead of letting it escape.
//---------------------------------------------------------

// Capella::readString(): a string length of 0xFFFFFFFF used to overflow `len + 1` on
// 32 bit and ask for a 4GB buffer everywhere else. Patches the author string length.
TEST_F(Capella_Tests, capCorruptStringLength) {
    EXPECT_EQ(capReadError("test1-corrupt-string-length"), Capella::Error::BAD_FORMAT);
    EXPECT_EQ(capImport("test1-corrupt-string-length"), engraving::Err::NoError);
}

// TextObj::read(): same, for the text length of a TEXT draw object added to the gallery.
TEST_F(Capella_Tests, capCorruptTextSize) {
    EXPECT_EQ(capReadError("test1-corrupt-text-size"), Capella::Error::BAD_FORMAT);
    EXPECT_EQ(capImport("test1-corrupt-text-size"), engraving::Err::NoError);
}

// MetafileObj::read(): same, for the bitmap length of a METAFILE draw object.
TEST_F(Capella_Tests, capCorruptMetafileSize) {
    EXPECT_EQ(capReadError("test1-corrupt-metafile-size"), Capella::Error::BAD_FORMAT);
    EXPECT_EQ(capImport("test1-corrupt-metafile-size"), engraving::Err::NoError);
}

// readCapVoice(): a key signature outside -7..+7, which keyOffsets[] has no entry for.
// Patches the first CapKey of test7 from signature 3 to 93.
//
// This does not reach the keyOffsets[] bounds check: readCapVoice() indexes the table
// with the key it reads back from the staff, and KeySigEvent::setKey() clamps that to
// -7..+7 on the way in. The check is unreachable defensive code as long as it does, so
// all this file can assert is that the importer accepts it and clamps rather than
// mangling the score.
TEST_F(Capella_Tests, capCorruptKeySignature) {
    EXPECT_EQ(capReadError("test7-corrupt-key"), Capella::Error::CAP_NO_ERROR);
    EXPECT_EQ(capImport("test7-corrupt-key"), engraving::Err::NoError);
}

// Capella::staffLayout(): a CapStaff::iLayout of 5 in a file with one stave layout,
// caught by the validation loop at the top of convertCapella().
TEST_F(Capella_Tests, capCorruptStaveIndex) {
    EXPECT_EQ(capReadError("test1-corrupt-stave-index"), Capella::Error::BAD_FORMAT);
    EXPECT_EQ(capImport("test1-corrupt-stave-index"), engraving::Err::NoError);
}

// Capella::staffLayout() again, reached from the part creation loop: the first system
// has two staves but the file describes only one stave layout. Both staves have a
// valid iLayout, so the validation loop lets this through.
TEST_F(Capella_Tests, capExtraStaff) {
    EXPECT_EQ(capReadError("test1-extra-staff"), Capella::Error::BAD_FORMAT);
    EXPECT_EQ(capImport("test1-extra-staff"), engraving::Err::NoError);
}
