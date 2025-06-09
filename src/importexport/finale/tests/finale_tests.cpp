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

#include "engraving/engravingerrors.h"
#include "engraving/dom/masterscore.h"

#include "importexport/finale/internal/importfinale.h"

#include "engraving/tests/utils/scorerw.h"
#include "engraving/tests/utils/scorecomp.h"

#include "io/fileinfo.h"

using namespace mu;
using namespace muse;
using namespace mu::iex::finale;
using namespace mu::engraving;

static const String FINALE_IO_DATA_DIR("data/");

static const std::string MODULE_NAME("iex_finale");

class Finale_Tests : public ::testing::Test
{
public:
    void finaleImportTestRef(const char* file);
    void enigmaXmlImportTestRef(const char* file);

    MasterScore* readScore(const String& fileName, bool isAbsolutePath = false);
};

//---------------------------------------------------------
//   fixupScore -- do required fixups after reading/importing score
//---------------------------------------------------------
static void fixupScore(MasterScore* score) // probably not needed
{
    score->connectTies();
    score->masterScore()->rebuildMidiMapping();
    score->setSaved(false);
}

MasterScore* Finale_Tests::readScore(const String& fileName, bool isAbsolutePath)
{
    String suffix = io::FileInfo::suffix(fileName);

    auto loadMusx = [](MasterScore* score, const muse::io::path_t& path) -> engraving::Err {
        return importMusx(score, path.toQString());
    };

    auto loadEnigmaXml = [](MasterScore* score, const muse::io::path_t& path) -> engraving::Err {
        return importEnigmaXml(score, path.toQString());
    };

    ScoreRW::ImportFunc importFunc = nullptr;
    if (suffix == u"musx") {
        importFunc = loadMusx;
    } else if (suffix == u"enigmaxml") {
        importFunc = loadEnigmaXml;
    }

    MasterScore* score = ScoreRW::readScore(fileName, isAbsolutePath, importFunc);
    return score;
}

//---------------------------------------------------------
//   finaleImportTestRef
//   read a Musx file, write to a new MuseScore mscx file
//   and verify against a MuseScore mscx reference file
//---------------------------------------------------------

void Finale_Tests::finaleImportTestRef(const char* file)
{
    MScore::debugMode = false;

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(FINALE_IO_DATA_DIR + fileName + u".musx");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, fileName + u".mscx", FINALE_IO_DATA_DIR + fileName + u"_ref.mscx"));
    delete score;
}

//---------------------------------------------------------
//   enigmaXmlImportTestRef
//   read an .enigmaxml file, write to a new MuseScore mscx file
//   and verify against a MuseScore mscx reference file
//---------------------------------------------------------

void Finale_Tests::enigmaXmlImportTestRef(const char* file)
{
    MScore::debugMode = false;

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(FINALE_IO_DATA_DIR + fileName + u".enigmaxml");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, fileName + u".mscx", FINALE_IO_DATA_DIR + fileName + u"_ref.mscx"));
    delete score;
}

TEST_F(Finale_Tests, DISABLED_onePartNoMeasures) {
    finaleImportTestRef("onePartNoMeasures");
}

TEST_F(Finale_Tests, grandStaffPartNoMeasures) {
    finaleImportTestRef("grandStaffPartNoMeasures");
}

TEST_F(Finale_Tests, DISABLED_twoPartsNoMeasures) {
    finaleImportTestRef("twoPartsNoMeasures");
}

TEST_F(Finale_Tests, twoPartsNoMeasuresWithBracket) {
    finaleImportTestRef("twoPartsNoMeasuresWithBracket");
}

TEST_F(Finale_Tests, DISABLED_onePartOneMeasure) {
    finaleImportTestRef("onePartOneMeasure");
}

TEST_F(Finale_Tests, onePartOneMeasureWithNotes) {
    finaleImportTestRef("onePartOneMeasureWithNotes");
}

TEST_F(Finale_Tests, oneMeasureCrossStaff) {
    finaleImportTestRef("oneMeasureCrossStaff");
}

TEST_F(Finale_Tests, onePartOneMeasureWithTuplets) {
    finaleImportTestRef("onePartOneMeasureWithTuplets");
}

TEST_F(Finale_Tests, onePartOneMeasureWithNestedTuplets) {
    finaleImportTestRef("onePartOneMeasureWithNestedTuplets");
}

TEST_F(Finale_Tests, oneMeasureWithTies) {
    finaleImportTestRef("oneMeasureWithTies");
}

TEST_F(Finale_Tests, multistaffInst) {
    finaleImportTestRef("multistaffInst");
}

TEST_F(Finale_Tests, v1v2Ties) {
    finaleImportTestRef("v1v2Ties");
}

TEST_F(Finale_Tests, v1v2Ties2) {
    finaleImportTestRef("v1v2Ties2");
}
