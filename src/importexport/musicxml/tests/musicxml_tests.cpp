/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "settings.h"
#include "importexport/musicxml/imusicxmlconfiguration.h"
#include "importexport/musicxml/internal/musicxml/exportxml.h"

#include "engraving/tests/utils/scorerw.h"
#include "engraving/tests/utils/scorecomp.h"

#include "io/fileinfo.h"

//! NOTE Different platforms have different font metrics, which is why some tests fail
#ifdef Q_OS_LINUX
#define DISABLED_EXCEPT_ON_LINUX(testName) testName
#else
#define DISABLED_EXCEPT_ON_LINUX(testName) DISABLED_##testName
#endif

using namespace mu;
using namespace muse;
using namespace mu::iex::musicxml;
using namespace mu::engraving;

namespace mu::engraving {
extern bool saveMxl(Score*, const String&);
extern engraving::Err importMusicXML(MasterScore*, const String&, bool forceMode);
extern engraving::Err importCompressedMusicXML(MasterScore*, const String&, bool forceMode);
}

static const String XML_IO_DATA_DIR("data/");

static const std::string MODULE_NAME("iex_musicxml");

static const std::string PREF_EXPORT_MUSICXML_EXPORTBREAKS("export/musicXML/exportBreaks");
static const std::string PREF_IMPORT_MUSICXML_IMPORTBREAKS("import/musicXML/importBreaks");
static const std::string PREF_IMPORT_MUSICXML_INFERTEXT("import/musicXML/importInferTextType");
static const std::string PREF_EXPORT_MUSICXML_EXPORTLAYOUT("export/musicXML/exportLayout");
static const std::string PREF_EXPORT_MUSICXML_EXPORTINVISIBLE("export/musicXML/exportInvisibleElements");

class MusicXML_Tests : public ::testing::Test
{
public:
    void musicXMLIoTest(const char* file, bool exportLayout = false);
    void musicXMLIoTestRef(const char* file);
    void musicXMLIoTestRefBreaks(const char* file);
    void musicXMLMscxExportTestRef(const char* file, bool exportLayout = false);
    void musicXMLMscxExportTestRefBreaks(const char* file);
    void musicXMLMscxExportTestRefInvisibleElements(const char* file);
    void musicXMLReadTestCompr(const char* file);
    void musicXMLReadWriteTestCompr(const char* file);
    void musicXMLImportTestRef(const char* file);

    void setValue(const std::string& key, const Val& value);

    MasterScore* readScore(const String& fileName, bool isAbsolutePath = false);
    bool saveCompareMusicXMLScore(MasterScore* score, const String& saveName, const String& compareWith);
};

//---------------------------------------------------------
//   fixupScore -- do required fixups after reading/importing score
//---------------------------------------------------------

static void fixupScore(MasterScore* score)
{
    score->connectTies();
    score->masterScore()->rebuildMidiMapping();
    score->setSaved(false);
}

void MusicXML_Tests::setValue(const std::string& key, const Val& value)
{
    settings()->setSharedValue(Settings::Key(MODULE_NAME, key), value);
}

MasterScore* MusicXML_Tests::readScore(const String& fileName, bool isAbsolutePath)
{
    String suffix = io::FileInfo::suffix(fileName);

    auto importXml = [](MasterScore* score, const muse::io::path_t& path) -> engraving::Err {
        return mu::engraving::importMusicXML(score, path.toQString(), false);
    };

    auto importMxl = [](MasterScore* score, const muse::io::path_t& path) -> engraving::Err {
        return mu::engraving::importCompressedMusicXML(score, path.toQString(), false);
    };

    ScoreRW::ImportFunc importFunc = nullptr;
    if (suffix == u"xml" || suffix == u"musicxml") {
        importFunc = importXml;
    } else if (suffix == u"mxl") {
        importFunc = importMxl;
    }

    MasterScore* score = ScoreRW::readScore(fileName, isAbsolutePath, importFunc);
    return score;
}

bool MusicXML_Tests::saveCompareMusicXMLScore(MasterScore* score, const String& saveName, const String& compareWithLocalPath)
{
    EXPECT_TRUE(saveXml(score, saveName));
    return ScoreComp::compareFiles(ScoreRW::rootPath() + u"/" + compareWithLocalPath, saveName);
}

//---------------------------------------------------------
//   musicXMLIoTest
//   read a MusicXML file, write to a new file and verify both files are identical
//---------------------------------------------------------

void MusicXML_Tests::musicXMLIoTest(const char* file, bool exportLayout)
{
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXMLConfiguration::MusicXMLExportBreaksType::Manual));
    setValue(PREF_IMPORT_MUSICXML_IMPORTBREAKS, Val(true));
    setValue(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, Val(exportLayout));
    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));
    setValue(PREF_IMPORT_MUSICXML_INFERTEXT, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".xml");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();
    EXPECT_TRUE(saveCompareMusicXMLScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u".xml"));
    delete score;
}

//---------------------------------------------------------
//   musicXMLIoTestRef
//   read a MusicXML file, write to a new file and verify against reference
//---------------------------------------------------------

void MusicXML_Tests::musicXMLIoTestRef(const char* file)
{
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXMLConfiguration::MusicXMLExportBreaksType::Manual));
    setValue(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, Val(false));
    setValue(PREF_IMPORT_MUSICXML_IMPORTBREAKS, Val(true));
    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));
    setValue(PREF_IMPORT_MUSICXML_INFERTEXT, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".xml");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();
    EXPECT_TRUE(saveCompareMusicXMLScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_ref.xml"));
    delete score;
}

//---------------------------------------------------------
//   musicXMLIoTestRefBreaks
//   read a MusicXML file, write to a new file and verify against reference
//   using all possible settings for PREF_EXPORT_MUSICXML_EXPORTBREAKS
//---------------------------------------------------------

void MusicXML_Tests::musicXMLIoTestRefBreaks(const char* file)
{
    MScore::debugMode = true;

    setValue(PREF_IMPORT_MUSICXML_IMPORTBREAKS, Val(true));
    setValue(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, Val(false));
    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));
    setValue(PREF_IMPORT_MUSICXML_INFERTEXT, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".xml");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXMLConfiguration::MusicXMLExportBreaksType::No));

    EXPECT_TRUE(saveCompareMusicXMLScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_no_ref.xml"));

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXMLConfiguration::MusicXMLExportBreaksType::Manual));

    EXPECT_TRUE(saveCompareMusicXMLScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_manual_ref.xml"));

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXMLConfiguration::MusicXMLExportBreaksType::All));

    EXPECT_TRUE(saveCompareMusicXMLScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_all_ref.xml"));
    delete score;
}

//---------------------------------------------------------
//   musicXMLMscxExportTestRef
//   read a MuseScore mscx file, write to a MusicXML file and verify against reference
//---------------------------------------------------------

void MusicXML_Tests::musicXMLMscxExportTestRef(const char* file, bool exportLayout)
{
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXMLConfiguration::MusicXMLExportBreaksType::Manual));
    setValue(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, Val(exportLayout));
    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));
    setValue(PREF_IMPORT_MUSICXML_INFERTEXT, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".mscx");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();

    EXPECT_TRUE(saveCompareMusicXMLScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_ref.xml"));
    delete score;
}

//---------------------------------------------------------
//   musicXMLMscxExportTestRefBreaks
//   read a MuseScore mscx file, write to a MusicXML file and verify against reference
//   using all possible settings for PREF_EXPORT_MUSICXML_EXPORTBREAKS
//---------------------------------------------------------

void MusicXML_Tests::musicXMLMscxExportTestRefBreaks(const char* file)
{
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXMLConfiguration::MusicXMLExportBreaksType::Manual));
    setValue(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, Val(false));
    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));
    setValue(PREF_IMPORT_MUSICXML_INFERTEXT, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".mscx");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXMLConfiguration::MusicXMLExportBreaksType::No));

    EXPECT_TRUE(saveCompareMusicXMLScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_no_ref.xml"));

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXMLConfiguration::MusicXMLExportBreaksType::Manual));

    EXPECT_TRUE(saveCompareMusicXMLScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_manual_ref.xml"));

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXMLConfiguration::MusicXMLExportBreaksType::All));

    EXPECT_TRUE(saveCompareMusicXMLScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_all_ref.xml"));
    delete score;
}

void MusicXML_Tests::musicXMLMscxExportTestRefInvisibleElements(const char* file)
{
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXMLConfiguration::MusicXMLExportBreaksType::Manual));
    setValue(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, Val(false));
    setValue(PREF_IMPORT_MUSICXML_INFERTEXT, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".mscx");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();

    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));

    EXPECT_TRUE(saveCompareMusicXMLScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_invisible_ref.xml"));

    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(false));

    EXPECT_TRUE(saveCompareMusicXMLScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_noinvisible_ref.xml"));

    delete score;
}

//---------------------------------------------------------
//   musicXMLReadTestCompr
//   read a compressed MusicXML file, write to a new file and verify against reference
//---------------------------------------------------------

void MusicXML_Tests::musicXMLReadTestCompr(const char* file)
{
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXMLConfiguration::MusicXMLExportBreaksType::Manual));
    setValue(PREF_IMPORT_MUSICXML_IMPORTBREAKS, Val(true));
    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));
    setValue(PREF_IMPORT_MUSICXML_INFERTEXT, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".mxl");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();
    EXPECT_TRUE(saveCompareMusicXMLScore(score, fileName + u"_mxl_read.xml", XML_IO_DATA_DIR + fileName + u".xml"));
    delete score;
}

//---------------------------------------------------------
//   musicXMLReadWriteTestCompr
//   read a MusicXML file, write to a compressed MusicXML file,
//   read the compressed MusicXML file, write to a new file and verify files are identical
//---------------------------------------------------------

void MusicXML_Tests::musicXMLReadWriteTestCompr(const char* file)
{
    // read xml
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXMLConfiguration::MusicXMLExportBreaksType::Manual));
    setValue(PREF_IMPORT_MUSICXML_IMPORTBREAKS, Val(true));
    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));
    setValue(PREF_IMPORT_MUSICXML_INFERTEXT, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".xml");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();
    // write mxl
    EXPECT_TRUE(saveMxl(score, fileName + u"_mxl_read_write.mxl"));
    delete score;
    // read mxl
    score = readScore(fileName + u"_mxl_read_write.mxl", true);
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();
    // write and verify
    EXPECT_TRUE(saveCompareMusicXMLScore(score, fileName + u"_mxl_read_write.xml", XML_IO_DATA_DIR + fileName + u".xml"));
    delete score;
}

//---------------------------------------------------------
//   musicXMLImportTestRef
//   read a MusicXML file, write to a new MuseScore mscx file
//   and verify against a MuseScore mscx reference file
//---------------------------------------------------------

void MusicXML_Tests::musicXMLImportTestRef(const char* file)
{
    MScore::debugMode = false;
    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXMLConfiguration::MusicXMLExportBreaksType::Manual));
    setValue(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, Val(false));
    setValue(PREF_IMPORT_MUSICXML_INFERTEXT, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".xml");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, fileName + u".mscx", XML_IO_DATA_DIR + fileName + u"_ref.mscx"));
    delete score;
}

TEST_F(MusicXML_Tests, accidentals1) {
    musicXMLIoTest("testAccidentals1");
}
TEST_F(MusicXML_Tests, accidentals2) {
    musicXMLIoTest("testAccidentals2");
}
TEST_F(MusicXML_Tests, accidentals3) {
    musicXMLIoTest("testAccidentals3");
}
TEST_F(MusicXML_Tests, arpCrossVoice) {
    musicXMLImportTestRef("testArpCrossVoice");
}
TEST_F(MusicXML_Tests, arpGliss1) {
    musicXMLIoTest("testArpGliss1");
}
TEST_F(MusicXML_Tests, arpGliss2) {
    musicXMLIoTest("testArpGliss2");
}
TEST_F(MusicXML_Tests, arpGliss3) {
    musicXMLIoTest("testArpGliss3");
}
TEST_F(MusicXML_Tests, arpOnRest) {
    musicXMLImportTestRef("testArpOnRest");
}
TEST_F(MusicXML_Tests, articulationCombination) {
    musicXMLIoTestRef("testArticulationCombination");
}
TEST_F(MusicXML_Tests, backupRoundingError) {
    musicXMLImportTestRef("testBackupRoundingError");
}
TEST_F(MusicXML_Tests, barlineLoc) {
    musicXMLImportTestRef("testBarlineLoc");
}
TEST_F(MusicXML_Tests, barlineSpan) {
    musicXMLIoTest("testBarlineSpan");
}
TEST_F(MusicXML_Tests, barlineFermatas) {
    musicXMLIoTest("testBarlineFermatas");
}
TEST_F(MusicXML_Tests, barStyles) {
    musicXMLIoTest("testBarStyles");
}
TEST_F(MusicXML_Tests, barStyles2) {
    musicXMLIoTest("testBarStyles2");
}
TEST_F(MusicXML_Tests, barStyles3) {
    musicXMLIoTest("testBarStyles3");
}
TEST_F(MusicXML_Tests, barStyles4) {
    musicXMLIoTest("testBarStyles4");
}
TEST_F(MusicXML_Tests, bracketTypes) {
    musicXMLImportTestRef("testBracketTypes");
}
TEST_F(MusicXML_Tests, DISABLED_beamEnd) {
    musicXMLIoTest("testBeamEnd");
}
TEST_F(MusicXML_Tests, beamModes) {
    musicXMLImportTestRef("testBeamModes");
}
TEST_F(MusicXML_Tests, beams1) {
    musicXMLIoTest("testBeams1");
}
TEST_F(MusicXML_Tests, beams2) {
    musicXMLIoTest("testBeams2");
}
TEST_F(MusicXML_Tests, beams3) {
    musicXMLIoTestRef("testBeams3");
}
TEST_F(MusicXML_Tests, breaksImplExpl) {
    musicXMLMscxExportTestRefBreaks("testBreaksImplExpl");
}
TEST_F(MusicXML_Tests, breaksMMRest) {
    musicXMLMscxExportTestRefBreaks("testBreaksMMRest");
}
TEST_F(MusicXML_Tests, DISABLED_breaksManual) { // fail after sync with 3.x
    musicXMLIoTestRefBreaks("testBreaksManual");
}
TEST_F(MusicXML_Tests, DISABLED_breaksPage) { // fail after sync with 3.x
    musicXMLMscxExportTestRefBreaks("testBreaksPage");
}
TEST_F(MusicXML_Tests, breaksSystem) {
    musicXMLMscxExportTestRefBreaks("testBreaksSystem");
}
TEST_F(MusicXML_Tests, breathMarks) {
    musicXMLIoTest("testBreathMarks");
}
TEST_F(MusicXML_Tests, buzzRoll) {
    musicXMLImportTestRef("testBuzzRoll");
}
TEST_F(MusicXML_Tests, buzzRoll2) {
    musicXMLIoTest("testBuzzRoll2");
}
TEST_F(MusicXML_Tests, changeTranspose) {
    musicXMLIoTest("testChangeTranspose");
}
TEST_F(MusicXML_Tests, changeTransposeNoDiatonic) {
    musicXMLIoTestRef("testChangeTranspose-no-diatonic");
}
TEST_F(MusicXML_Tests, chordDiagrams1) {
    musicXMLIoTest("testChordDiagrams1");
}
TEST_F(MusicXML_Tests, chordNoVoice) {
    musicXMLIoTestRef("testChordNoVoice");
}
TEST_F(MusicXML_Tests, chordSymbols) {
    musicXMLMscxExportTestRef("testChordSymbols");
}
TEST_F(MusicXML_Tests, chordSymbols2) {
    musicXMLImportTestRef("testChordSymbols2");
}
TEST_F(MusicXML_Tests, clefs1) {
    musicXMLIoTest("testClefs1");
}
TEST_F(MusicXML_Tests, clefs2) {
    musicXMLIoTest("testClefs2");
}
TEST_F(MusicXML_Tests, codaHBox) {
    musicXMLImportTestRef("testCodaHBox");
}
TEST_F(MusicXML_Tests, colorExport) {
    musicXMLMscxExportTestRef("testColorExport");
}
TEST_F(MusicXML_Tests, colors) {
    musicXMLIoTest("testColors");
}
TEST_F(MusicXML_Tests, completeMeasureRests) {
    musicXMLIoTest("testCompleteMeasureRests");
}
TEST_F(MusicXML_Tests, copyrightScale) {
    musicXMLImportTestRef("testCopyrightScale");
}
TEST_F(MusicXML_Tests, cueGraceNotes1) {
    musicXMLImportTestRef("testCueGraceNotes");
}
TEST_F(MusicXML_Tests, cueGraceNotes2) {
    musicXMLIoTestRef("testCueGraceNotes");
}
TEST_F(MusicXML_Tests, cueNotes) {
    musicXMLIoTest("testCueNotes");
}
TEST_F(MusicXML_Tests, cueNotes2) {
    musicXMLMscxExportTestRef("testCueNotes2");
}
TEST_F(MusicXML_Tests, cueNotes3) {
    musicXMLImportTestRef("testCueNotes3");
}
TEST_F(MusicXML_Tests, dalSegno) {
    musicXMLIoTest("testDalSegno");
}
TEST_F(MusicXML_Tests, dcalCoda) {
    musicXMLIoTest("testDCalCoda");
}
TEST_F(MusicXML_Tests, dcalFine) {
    musicXMLIoTest("testDCalFine");
}
TEST_F(MusicXML_Tests, directions1) {
    musicXMLIoTestRef("testDirections1");
}
TEST_F(MusicXML_Tests, directions2) {
    musicXMLIoTest("testDirections2");
}
TEST_F(MusicXML_Tests, displayStepOctave) {
    musicXMLMscxExportTestRef("testDisplayStepOctave");
}
TEST_F(MusicXML_Tests, divisionsDefinedTooLate1) {
    musicXMLIoTestRef("testDivsDefinedTooLate1");
}
TEST_F(MusicXML_Tests, divisionsDefinedTooLate2) {
    musicXMLIoTestRef("testDivsDefinedTooLate2");
}
TEST_F(MusicXML_Tests, divisionsDuration) {
    musicXMLIoTest("testDivisionsDuration");
}
TEST_F(MusicXML_Tests, doletOttavas) {
    musicXMLImportTestRef("testDoletOttavas");
}
TEST_F(MusicXML_Tests, doubleClefError) {
    musicXMLIoTestRef("testDoubleClefError");
}
TEST_F(MusicXML_Tests, drumset1) {
    musicXMLIoTest("testDrumset1");
}
TEST_F(MusicXML_Tests, drumset2) {
    musicXMLIoTest("testDrumset2");
}
TEST_F(MusicXML_Tests, dsalCoda) {
    musicXMLImportTestRef("testDSalCoda");
}
TEST_F(MusicXML_Tests, dsalCodaMisplaced) {
    musicXMLImportTestRef("testDSalCodaMisplaced");
}
TEST_F(MusicXML_Tests, durationLargeErrorMscx) {
    musicXMLImportTestRef("testDurationLargeError");
}
TEST_F(MusicXML_Tests, duplicateInstrChange) {
    musicXMLImportTestRef("testDuplicateInstrChange");
}
TEST_F(MusicXML_Tests, durationLargeErrorXml) {
    musicXMLIoTestRef("testDurationLargeError");
}
TEST_F(MusicXML_Tests, durationRoundingErrorMscx) {
    musicXMLImportTestRef("testDurationRoundingError");
}
TEST_F(MusicXML_Tests, durationRoundingErrorXml) {
    musicXMLIoTestRef("testDurationRoundingError");
}
TEST_F(MusicXML_Tests, dynamics1) {
    musicXMLIoTest("testDynamics1");
}
TEST_F(MusicXML_Tests, dynamics2) {
    musicXMLIoTest("testDynamics2");
}
TEST_F(MusicXML_Tests, dynamics3) {
    musicXMLIoTestRef("testDynamics3");
}
TEST_F(MusicXML_Tests, elision) {
    musicXMLImportTestRef("testElision");
}
TEST_F(MusicXML_Tests, emptyMeasure) {
    musicXMLIoTestRef("testEmptyMeasure");
}
TEST_F(MusicXML_Tests, emptyVoice1) {
    musicXMLIoTestRef("testEmptyVoice1");
}
TEST_F(MusicXML_Tests, excludeInvisibleElements) {
    musicXMLMscxExportTestRefInvisibleElements("testExcludeInvisibleElements");
}
TEST_F(MusicXML_Tests, excessHiddenStaves) {
    musicXMLImportTestRef("testExcessHiddenStaves");
}
TEST_F(MusicXML_Tests, extendedLyrics) {
    musicXMLIoTestRef("testExtendedLyrics");
}
TEST_F(MusicXML_Tests, figuredBass1) {
    musicXMLIoTest("testFiguredBass1");
}
TEST_F(MusicXML_Tests, figuredBass2) {
    musicXMLIoTest("testFiguredBass2");
}
TEST_F(MusicXML_Tests, figuredBass3) {
    musicXMLIoTest("testFiguredBass3");
}
TEST_F(MusicXML_Tests, figuredBassDivisions) {
    musicXMLIoTest("testFiguredBassDivisions");
}
TEST_F(MusicXML_Tests, finaleDynamics) {
    musicXMLImportTestRef("testFinaleDynamics");
}
TEST_F(MusicXML_Tests, finaleInstr) {
    musicXMLImportTestRef("testFinaleInstr");
}
TEST_F(MusicXML_Tests, finaleInstr2) {
    musicXMLImportTestRef("testFinaleInstr2");
}
TEST_F(MusicXML_Tests, formattedThings) {
    musicXMLIoTest("testFormattedThings");
}
TEST_F(MusicXML_Tests, fractionMinus) {
    musicXMLIoTestRef("testFractionMinus");
}
TEST_F(MusicXML_Tests, fractionPlus) {
    musicXMLIoTestRef("testFractionPlus");
}
TEST_F(MusicXML_Tests, fractionTicks) {
    musicXMLIoTestRef("testFractionTicks");
}
TEST_F(MusicXML_Tests, glissFall) {
    musicXMLImportTestRef("testGlissFall");
}
TEST_F(MusicXML_Tests, grace1) {
    musicXMLIoTest("testGrace1");
}
TEST_F(MusicXML_Tests, grace2) {
    musicXMLIoTest("testGrace2");
}
TEST_F(MusicXML_Tests, grace3) {
    musicXMLIoTest("testGrace3");
}
TEST_F(MusicXML_Tests, graceAfter1) {
    musicXMLIoTest("testGraceAfter1");
}
TEST_F(MusicXML_Tests, graceAfter2) {
    musicXMLIoTest("testGraceAfter2");
}
TEST_F(MusicXML_Tests, graceAfter3) {
    musicXMLIoTest("testGraceAfter3");
}
TEST_F(MusicXML_Tests, DISABLED_graceAfter4) {
    musicXMLIoTest("testGraceAfter4");
}
TEST_F(MusicXML_Tests, graceFermata) {
    musicXMLIoTest("testGraceFermata");
}
TEST_F(MusicXML_Tests, harpPedals) {
    musicXMLMscxExportTestRef("testHarpPedals");
}
TEST_F(MusicXML_Tests, hairpinDynamics) {
    musicXMLMscxExportTestRef("testHairpinDynamics");
}
TEST_F(MusicXML_Tests, harmony1) {
    musicXMLIoTest("testHarmony1");
}
TEST_F(MusicXML_Tests, harmony2) {
    musicXMLIoTest("testHarmony2");
}
TEST_F(MusicXML_Tests, harmony3) {
    musicXMLIoTest("testHarmony3");
}
TEST_F(MusicXML_Tests, harmony4) {
    musicXMLIoTest("testHarmony4");
}
TEST_F(MusicXML_Tests, harmony5) {
    musicXMLIoTest("testHarmony5");
}                                                                      // chordnames without chordrest
TEST_F(MusicXML_Tests, harmony6) {
    musicXMLMscxExportTestRef("testHarmony6");
}
TEST_F(MusicXML_Tests, harmony7) {
    musicXMLMscxExportTestRef("testHarmony7");
}
TEST_F(MusicXML_Tests, harmony8) {
    musicXMLIoTest("testHarmony8");
}
TEST_F(MusicXML_Tests, hello) {
    musicXMLIoTest("testHello");
}
TEST_F(MusicXML_Tests, helloReadCompr) {
    musicXMLReadTestCompr("testHello");
}
TEST_F(MusicXML_Tests, helloReadWriteCompr) {
    musicXMLReadWriteTestCompr("testHello");
}
TEST_F(MusicXML_Tests, implicitMeasure1) {
    musicXMLIoTest("testImplicitMeasure1");
}
TEST_F(MusicXML_Tests, importDrums) {
    musicXMLImportTestRef("testImportDrums");
}
TEST_F(MusicXML_Tests, importDrums2) {
    musicXMLImportTestRef("testImportDrums2");
}
TEST_F(MusicXML_Tests, incompleteTuplet) {
    musicXMLIoTestRef("testIncompleteTuplet");
}
TEST_F(MusicXML_Tests, incorrectMidiProgram) {
    musicXMLIoTestRef("testIncorrectMidiProgram");
}

TEST_F(MusicXML_Tests, incorrectStaffNumber1) {
    musicXMLIoTestRef("testIncorrectStaffNumber1");
}
TEST_F(MusicXML_Tests, incorrectStaffNumber2) {
    musicXMLIoTestRef("testIncorrectStaffNumber2");
}
TEST_F(MusicXML_Tests, DISABLED_EXCEPT_ON_LINUX(inferredCredits1)) {
    musicXMLImportTestRef("testInferredCredits1");
}
TEST_F(MusicXML_Tests, DISABLED_EXCEPT_ON_LINUX(inferredCredits2)) {
    musicXMLImportTestRef("testInferredCredits2");
}
TEST_F(MusicXML_Tests, inferCodaII) {
    musicXMLImportTestRef("testInferCodaII");
}
TEST_F(MusicXML_Tests, inferredDynamicRange) {
    musicXMLImportTestRef("testInferredDynamicRange");
}
TEST_F(MusicXML_Tests, inferSegnoII) {
    musicXMLImportTestRef("testInferSegnoII");
}
TEST_F(MusicXML_Tests, inferFraction) {
    musicXMLImportTestRef("testInferFraction");
}
TEST_F(MusicXML_Tests, inferredFingerings) {
    musicXMLImportTestRef("testInferredFingerings");
}
TEST_F(MusicXML_Tests, inferredCrescLines) {
    musicXMLImportTestRef("testInferredCrescLines");
}
TEST_F(MusicXML_Tests, inferredDynamicsExpression) {
    musicXMLImportTestRef("testInferredDynamicsExpression");
}
TEST_F(MusicXML_Tests, inferredRights) {
    musicXMLImportTestRef("testInferredRights");
}
TEST_F(MusicXML_Tests, DISABLED_inferredTechnique) {
    musicXMLImportTestRef("testInferredTechnique");
}
TEST_F(MusicXML_Tests, inferredTempoText) {
    musicXMLImportTestRef("testInferredTempoText");
}
TEST_F(MusicXML_Tests, inferredTempoText2) {
    musicXMLImportTestRef("testInferredTempoText2");
}
TEST_F(MusicXML_Tests, inferredCrescLines2) {
    musicXMLImportTestRef("testInferredCrescLines2");
}
TEST_F(MusicXML_Tests, instrumentChangeMIDIportExport) {
    musicXMLMscxExportTestRef("testInstrumentChangeMIDIportExport");
}
TEST_F(MusicXML_Tests, instrumentSound) {
    musicXMLIoTestRef("testInstrumentSound");
}
TEST_F(MusicXML_Tests, instrImport) {
    musicXMLImportTestRef("testInstrImport");
}
TEST_F(MusicXML_Tests, invalidLayout) {
    musicXMLMscxExportTestRef("testInvalidLayout");
}
TEST_F(MusicXML_Tests, invalidTimesig) {
    musicXMLIoTestRef("testInvalidTimesig");
}
TEST_F(MusicXML_Tests, invisibleDirection) {
    musicXMLIoTest("testInvisibleDirection");
}
TEST_F(MusicXML_Tests, invisibleElements) {
    musicXMLIoTest("testInvisibleElements");
}
TEST_F(MusicXML_Tests, invisibleNote) {
    musicXMLMscxExportTestRef("testInvisibleNote");
}
TEST_F(MusicXML_Tests, keysig1) {
    musicXMLIoTest("testKeysig1");
}
TEST_F(MusicXML_Tests, keysig2) {
    musicXMLIoTest("testKeysig2");
}
TEST_F(MusicXML_Tests, DISABLED_EXCEPT_ON_LINUX(layout)) {
    musicXMLIoTest("testLayout", true);
}
TEST_F(MusicXML_Tests, lessWhiteSpace) {
    musicXMLIoTestRef("testLessWhiteSpace");
}
TEST_F(MusicXML_Tests, lines1) {
    musicXMLIoTest("testLines1");
}
TEST_F(MusicXML_Tests, lines2) {
    musicXMLIoTest("testLines2");
}
TEST_F(MusicXML_Tests, lines3) {
    musicXMLIoTest("testLines3");
}
TEST_F(MusicXML_Tests, lines4) {
    musicXMLMscxExportTestRef("testLines4");
}
TEST_F(MusicXML_Tests, lyricBracket) {
    musicXMLImportTestRef("testLyricBracket");
}
TEST_F(MusicXML_Tests, lyricColor) {
    musicXMLIoTest("testLyricColor");
}
TEST_F(MusicXML_Tests, lyricPos) {
    musicXMLImportTestRef("testLyricPos");
}
TEST_F(MusicXML_Tests, lyrics1) {
    musicXMLIoTestRef("testLyrics1");
}
TEST_F(MusicXML_Tests, lyricExtension1) {
    musicXMLIoTest("testLyricExtensions");
}
TEST_F(MusicXML_Tests, lyricExtension2) {
    musicXMLImportTestRef("testLyricExtensions");
}
TEST_F(MusicXML_Tests, lyricExtension3) {
    musicXMLIoTest("testLyricExtension2");
}
TEST_F(MusicXML_Tests, lyricExtension4) {
    musicXMLImportTestRef("testLyricExtension2");
}
TEST_F(MusicXML_Tests, lyricsVoice2a) {
    musicXMLIoTest("testLyricsVoice2a");
}
TEST_F(MusicXML_Tests, lyricsVoice2b) {
    musicXMLIoTestRef("testLyricsVoice2b");
}
TEST_F(MusicXML_Tests, maxNumberLevel) {
    musicXMLMscxExportTestRef("testMaxNumberLevel");
}
TEST_F(MusicXML_Tests, measureLength) {
    musicXMLIoTestRef("testMeasureLength");
}
TEST_F(MusicXML_Tests, measureNumbers) {
    musicXMLIoTest("testMeasureNumbers");
}
TEST_F(MusicXML_Tests, measureNumberOffset) {
    musicXMLIoTest("testMeasureNumberOffset");
}
TEST_F(MusicXML_Tests, measureRepeats1) {
    musicXMLIoTestRef("testMeasureRepeats1");
}
TEST_F(MusicXML_Tests, DISABLED_measureRepeats2) {
    musicXMLIoTestRef("testMeasureRepeats2");
}
TEST_F(MusicXML_Tests, measureRepeats3) {
    musicXMLIoTest("testMeasureRepeats3");
}
TEST_F(MusicXML_Tests, measureStyleSlash) {
    musicXMLImportTestRef("testMeasureStyleSlash");
}
TEST_F(MusicXML_Tests, midiPortExport) {
    musicXMLMscxExportTestRef("testMidiPortExport");
}
TEST_F(MusicXML_Tests, multiInstrumentPart1) {
    musicXMLIoTest("testMultiInstrumentPart1");
}
TEST_F(MusicXML_Tests, multiInstrumentPart2) {
    musicXMLIoTest("testMultiInstrumentPart2");
}
TEST_F(MusicXML_Tests, multiInstrumentPart3) {
    musicXMLMscxExportTestRef("testMultiInstrumentPart3");
}
TEST_F(MusicXML_Tests, multiMeasureRest1) {
    musicXMLIoTestRef("testMultiMeasureRest1");
}
TEST_F(MusicXML_Tests, multiMeasureRest2) {
    musicXMLIoTestRef("testMultiMeasureRest2");
}
TEST_F(MusicXML_Tests, multiMeasureRest3) {
    musicXMLIoTestRef("testMultiMeasureRest3");
}
TEST_F(MusicXML_Tests, multiMeasureRest4) {
    musicXMLIoTestRef("testMultiMeasureRest4");
}
TEST_F(MusicXML_Tests, multipleNotations) {
    musicXMLIoTestRef("testMultipleNotations");
}
TEST_F(MusicXML_Tests, namedNoteheads) {
    musicXMLImportTestRef("testNamedNoteheads");
}
TEST_F(MusicXML_Tests, negativeOffset) {
    musicXMLImportTestRef("testNegativeOffset");
}
TEST_F(MusicXML_Tests, negativeOctave) {
    musicXMLMscxExportTestRef("testNegativeOctave");
}
TEST_F(MusicXML_Tests, nonStandardKeySig1) {
    musicXMLIoTest("testNonStandardKeySig1");
}
TEST_F(MusicXML_Tests, nonStandardKeySig2) {
    musicXMLIoTest("testNonStandardKeySig2");
}
TEST_F(MusicXML_Tests, nonStandardKeySig3) {
    musicXMLIoTest("testNonStandardKeySig3");
}
TEST_F(MusicXML_Tests, nonUniqueThings) {
    musicXMLIoTestRef("testNonUniqueThings");
}
TEST_F(MusicXML_Tests, noteAttributes1) {
    musicXMLIoTest("testNoteAttributes1");
}
TEST_F(MusicXML_Tests, noteAttributes2) {
    musicXMLIoTestRef("testNoteAttributes2");
}
TEST_F(MusicXML_Tests, noteAttributes3) {
    musicXMLIoTest("testNoteAttributes3");
}
TEST_F(MusicXML_Tests, DISABLED_noteAttributes4) {
    musicXMLImportTestRef("testNoteAttributes2");
}
TEST_F(MusicXML_Tests, noteColor) {
    musicXMLIoTest("testNoteColor");
}
TEST_F(MusicXML_Tests, noteheadParentheses) {
    musicXMLIoTest("testNoteheadParentheses");
}
TEST_F(MusicXML_Tests, noteheads) {
    musicXMLIoTest("testNoteheads");
}
TEST_F(MusicXML_Tests, noteheads2) {
    musicXMLMscxExportTestRef("testNoteheads2");
}
TEST_F(MusicXML_Tests, noteheadsFilled) {
    musicXMLIoTest("testNoteheadsFilled");
}
TEST_F(MusicXML_Tests, notesRests1) {
    musicXMLIoTest("testNotesRests1");
}
TEST_F(MusicXML_Tests, notesRests2) {
    musicXMLIoTest("testNotesRests2");
}
TEST_F(MusicXML_Tests, numberedLyrics) {
    musicXMLIoTestRef("testNumberedLyrics");
}
TEST_F(MusicXML_Tests, overlappingSpanners) {
    musicXMLIoTest("testOverlappingSpanners");
}
TEST_F(MusicXML_Tests, partNames) {
    musicXMLImportTestRef("testPartNames");
}
TEST_F(MusicXML_Tests, partNames2) {
    musicXMLIoTest("testPartNames2");
}
TEST_F(MusicXML_Tests, pedalChanges) {
    musicXMLIoTest("testPedalChanges");
}
TEST_F(MusicXML_Tests, pedalChangesBroken) {
    musicXMLImportTestRef("testPedalChangesBroken");
}
TEST_F(MusicXML_Tests, pedalStyles) {
    musicXMLIoTest("testPedalStyles");
}
TEST_F(MusicXML_Tests, placementDefaults) {
    musicXMLImportTestRef("testPlacementDefaults");
}
TEST_F(MusicXML_Tests, printSpacingNo) {
    musicXMLIoTestRef("testPrintSpacingNo");
}
TEST_F(MusicXML_Tests, repeatCounts) {
    musicXMLIoTest("testRepeatCounts");
}
TEST_F(MusicXML_Tests, repeatSingleMeasure) {
    musicXMLIoTest("testRepeatSingleMeasure");
}
TEST_F(MusicXML_Tests, restNotations) {
    musicXMLIoTestRef("testRestNotations");
}
TEST_F(MusicXML_Tests, restsNoType) {
    musicXMLIoTestRef("testRestsNoType");
}
TEST_F(MusicXML_Tests, restsTypeWhole) {
    musicXMLIoTestRef("testRestsTypeWhole");
}
TEST_F(MusicXML_Tests, secondVoiceMelismata) {
    musicXMLImportTestRef("testSecondVoiceMelismata");
}
TEST_F(MusicXML_Tests, sibMetronomeMarks) {
    musicXMLImportTestRef("testSibMetronomeMarks");
}
TEST_F(MusicXML_Tests, sibOttavas) {
    musicXMLImportTestRef("testSibOttavas");
}
TEST_F(MusicXML_Tests, sibRitLine) {
    musicXMLImportTestRef("testSibRitLine");
}
TEST_F(MusicXML_Tests, slurTieDirection) {
    musicXMLIoTest("testSlurTieDirection");
}
TEST_F(MusicXML_Tests, slurTieLineStyle) {
    musicXMLIoTest("testSlurTieLineStyle");
}
TEST_F(MusicXML_Tests, slurs) {
    musicXMLIoTest("testSlurs");
}
TEST_F(MusicXML_Tests, slurs2) {
    musicXMLIoTest("testSlurs2");
}
TEST_F(MusicXML_Tests, sound1) {
    musicXMLIoTestRef("testSound1");
}
TEST_F(MusicXML_Tests, sound2) {
    musicXMLIoTestRef("testSound2");
}
TEST_F(MusicXML_Tests, specialCharacters) {
    musicXMLIoTest("testSpecialCharacters");
}
TEST_F(MusicXML_Tests, staffEmptiness) {
    musicXMLImportTestRef("testStaffEmptiness");
}
TEST_F(MusicXML_Tests, staffSize) {
    musicXMLIoTest("testStaffSize");
}
TEST_F(MusicXML_Tests, staffTwoKeySigs) {
    musicXMLIoTest("testStaffTwoKeySigs");
}
TEST_F(MusicXML_Tests, sticking) {
    musicXMLImportTestRef("testSticking");
}
TEST_F(MusicXML_Tests, stickingLyrics) {
    musicXMLImportTestRef("testStickingLyrics");
}
TEST_F(MusicXML_Tests, stringData) {
    musicXMLIoTest("testStringData");
}
TEST_F(MusicXML_Tests, stringVoiceName) {
    musicXMLIoTestRef("testStringVoiceName");
}
TEST_F(MusicXML_Tests, swing) {
    musicXMLMscxExportTestRef("testSwing");
}
TEST_F(MusicXML_Tests, systemBrackets1) {
    musicXMLIoTest("testSystemBrackets1");
}
TEST_F(MusicXML_Tests, systemBrackets2) {
    musicXMLIoTest("testSystemBrackets2");
}
TEST_F(MusicXML_Tests, systemBrackets3) {
    musicXMLImportTestRef("testSystemBrackets3");
}
TEST_F(MusicXML_Tests, systemBrackets4) {
    musicXMLIoTest("testSystemBrackets1");
}
TEST_F(MusicXML_Tests, systemBrackets5) {
    musicXMLIoTest("testSystemBrackets1");
}
TEST_F(MusicXML_Tests, systemDirection) {
    musicXMLIoTest("testSystemDirection");
}
TEST_F(MusicXML_Tests, DISABLED_EXCEPT_ON_LINUX(systemDistance)) {
    musicXMLMscxExportTestRef("testSystemDistance", true);
}
TEST_F(MusicXML_Tests, DISABLED_EXCEPT_ON_LINUX(systemDividers)) {
    musicXMLIoTest("testSystemDividers", true);
}
TEST_F(MusicXML_Tests, systemObjectStaves) {
    musicXMLImportTestRef("testSystemObjectStaves");
}
TEST_F(MusicXML_Tests, tablature1) {
    musicXMLIoTest("testTablature1");
}
TEST_F(MusicXML_Tests, tablature2) {
    musicXMLIoTest("testTablature2");
}
TEST_F(MusicXML_Tests, tablature3) {
    musicXMLIoTest("testTablature3");
}
TEST_F(MusicXML_Tests, tablature4) {
    musicXMLIoTest("testTablature4");
}
TEST_F(MusicXML_Tests, tablature5) {
    musicXMLIoTestRef("testTablature5");
}
TEST_F(MusicXML_Tests, tboxAboveBelow1) {
    musicXMLMscxExportTestRef("testTboxAboveBelow1");
}
TEST_F(MusicXML_Tests, tboxAboveBelow2) {
    musicXMLMscxExportTestRef("testTboxAboveBelow2");
}
TEST_F(MusicXML_Tests, tboxAboveBelow3) {
    musicXMLMscxExportTestRef("testTboxAboveBelow3");
}
TEST_F(MusicXML_Tests, tboxMultiPage1) {
    musicXMLMscxExportTestRef("testTboxMultiPage1");
}
TEST_F(MusicXML_Tests, tboxVbox1) {
    musicXMLMscxExportTestRef("testTboxVbox1");
}
TEST_F(MusicXML_Tests, tboxWords1) {
    musicXMLMscxExportTestRef("testTboxWords1");
}
TEST_F(MusicXML_Tests, tempo1) {
    musicXMLIoTest("testTempo1");
}
TEST_F(MusicXML_Tests, tempo2) {
    musicXMLIoTestRef("testTempo2");
}
TEST_F(MusicXML_Tests, tempo3) {
    musicXMLIoTestRef("testTempo3");
}
TEST_F(MusicXML_Tests, tempo4) {
    musicXMLIoTestRef("testTempo4");
}
TEST_F(MusicXML_Tests, tempo5) {
    musicXMLIoTest("testTempo5");
}
TEST_F(MusicXML_Tests, tempo6) {
    musicXMLIoTest("testTempo6");
}
TEST_F(MusicXML_Tests, tempoLineFermata) {
    musicXMLImportTestRef("testTempoLineFermata");
}
TEST_F(MusicXML_Tests, tempoOverlap) {
    musicXMLIoTestRef("testTempoOverlap");
}
TEST_F(MusicXML_Tests, tempoPrecision) {
    musicXMLMscxExportTestRef("testTempoPrecision");
}
TEST_F(MusicXML_Tests, tempoTextSpace1) {
    musicXMLImportTestRef("testTempoTextSpace1");
}
TEST_F(MusicXML_Tests, tempoTextSpace2) {
    musicXMLImportTestRef("testTempoTextSpace2");
}
TEST_F(MusicXML_Tests, textLines) {
    musicXMLMscxExportTestRef("testTextLines");
}
TEST_F(MusicXML_Tests, testTextOrder) {
    musicXMLImportTestRef("testTextOrder");
}
TEST_F(MusicXML_Tests, textQuirkInference) {
    musicXMLImportTestRef("testTextQuirkInference");
}
TEST_F(MusicXML_Tests, tieTied) {
    musicXMLIoTestRef("testTieTied");
}
TEST_F(MusicXML_Tests, importTie1) {
    musicXMLImportTestRef("importTie1");
}
TEST_F(MusicXML_Tests, importTie2) {
    // Finale ties to different voices
    musicXMLImportTestRef("importTie2");
}
TEST_F(MusicXML_Tests, importTie3) {
    // Dolet6 ties to different voices & staves
    musicXMLImportTestRef("importTie3");
}
TEST_F(MusicXML_Tests, importTie4) {
    // Dolet8 ties to different voices & staves
    musicXMLImportTestRef("importTie4");
}
TEST_F(MusicXML_Tests, timesig1) {
    musicXMLIoTest("testTimesig1");
}
TEST_F(MusicXML_Tests, timesig3) {
    musicXMLIoTest("testTimesig3");
}
TEST_F(MusicXML_Tests, timeTick) {
    musicXMLImportTestRef("testTimeTick");
}
TEST_F(MusicXML_Tests, timeTickExport) {
    bool use302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;
    musicXMLMscxExportTestRef("testTimeTickExport");
    MScore::useRead302InTestMode = use302;
}
TEST_F(MusicXML_Tests, titleSwapMu) {
    musicXMLImportTestRef("testTitleSwapMu");
}
TEST_F(MusicXML_Tests, titleSwapSib) {
    musicXMLImportTestRef("testTitleSwapSib");
}
TEST_F(MusicXML_Tests, trackHandling) {
    musicXMLIoTest("testTrackHandling");
}
TEST_F(MusicXML_Tests, tremolo) {
    musicXMLIoTest("testTremolo");
}
TEST_F(MusicXML_Tests, trills) {
    musicXMLMscxExportTestRef("testTrills");
}
TEST_F(MusicXML_Tests, tuplets1) {
    musicXMLIoTestRef("testTuplets1");
}
TEST_F(MusicXML_Tests, tuplets2) {
    musicXMLIoTestRef("testTuplets2");
}
TEST_F(MusicXML_Tests, tuplets3) {
    musicXMLIoTestRef("testTuplets3");
}
TEST_F(MusicXML_Tests, tuplets4) {
    musicXMLIoTest("testTuplets4");
}
TEST_F(MusicXML_Tests, tuplets5) {
    musicXMLIoTestRef("testTuplets5");
}
TEST_F(MusicXML_Tests, tuplets6) {
    musicXMLIoTestRef("testTuplets6");
}
TEST_F(MusicXML_Tests, tuplets7) {
    musicXMLIoTest("testTuplets7");
}
TEST_F(MusicXML_Tests, tuplets8) {
    musicXMLMscxExportTestRef("testTuplets8");
}
TEST_F(MusicXML_Tests, tuplets9) {
    musicXMLIoTest("testTuplets9");
}
TEST_F(MusicXML_Tests, tupletTie) {
    musicXMLImportTestRef("testTupletTie");
}
TEST_F(MusicXML_Tests, twoNoteTremoloTuplet) {
    musicXMLIoTest("testTwoNoteTremoloTuplet");
}
TEST_F(MusicXML_Tests, uninitializedDivisions) {
    musicXMLIoTestRef("testUninitializedDivisions");
}
TEST_F(MusicXML_Tests, unnecessaryBarlines) {
    musicXMLImportTestRef("testUnnecessaryBarlines");
}
TEST_F(MusicXML_Tests, unusualDurations) {
    musicXMLIoTestRef("testUnusualDurations");
}
TEST_F(MusicXML_Tests, unterminatedTies) {
    musicXMLImportTestRef("testUnterminatedTies");
}
TEST_F(MusicXML_Tests, virtualInstruments) {
    musicXMLIoTestRef("testVirtualInstruments");
}
TEST_F(MusicXML_Tests, voiceMapper1) {
    musicXMLIoTestRef("testVoiceMapper1");
}
TEST_F(MusicXML_Tests, voiceMapper2) {
    musicXMLIoTestRef("testVoiceMapper2");
}
TEST_F(MusicXML_Tests, voiceMapper3) {
    musicXMLIoTestRef("testVoiceMapper3");
}
TEST_F(MusicXML_Tests, voicePiano1) {
    musicXMLIoTest("testVoicePiano1");
}
TEST_F(MusicXML_Tests, volta1) {
    musicXMLIoTest("testVolta1");
}
TEST_F(MusicXML_Tests, volta2) {
    musicXMLIoTest("testVolta2");
}
TEST_F(MusicXML_Tests, voltaHiding1) {
    musicXMLImportTestRef("testVoltaHiding");
}
TEST_F(MusicXML_Tests, voltaHiding2) {
    musicXMLIoTestRef("testVoltaHiding");
}
TEST_F(MusicXML_Tests, wedgeOffset) {
    musicXMLImportTestRef("testWedgeOffset");
}
TEST_F(MusicXML_Tests, wedge1) {
    musicXMLIoTest("testWedge1");
}
TEST_F(MusicXML_Tests, wedge2) {
    musicXMLIoTest("testWedge2");
}
TEST_F(MusicXML_Tests, wedge3) {
    musicXMLIoTest("testWedge3");
}
TEST_F(MusicXML_Tests, wedge4) {
    musicXMLMscxExportTestRef("testWedge4");
}
TEST_F(MusicXML_Tests, wedge5) {
    musicXMLIoTestRef("testWedge5");
}
TEST_F(MusicXML_Tests, words1) {
    musicXMLIoTest("testWords1");
}
TEST_F(MusicXML_Tests, words2) {
    musicXMLIoTest("testWords2");
}
TEST_F(MusicXML_Tests, hiddenStaves)
{
    String fileName = String::fromUtf8("testHiddenStaves.xml");
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName);

    EXPECT_EQ(score->style().value(Sid::hideEmptyStaves).toBool(), true);
}
