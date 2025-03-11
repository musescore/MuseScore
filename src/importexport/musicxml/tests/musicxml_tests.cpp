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
#include "importexport/musicxml/internal/musicxml/import/importmusicxml.h"
#include "importexport/musicxml/internal/musicxml/export/exportmusicxml.h"

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

static const String XML_IO_DATA_DIR("data/");

static const std::string MODULE_NAME("iex_musicxml");

static const std::string PREF_EXPORT_MUSICXML_EXPORTBREAKS("export/musicXml/exportBreaks");
static const std::string PREF_IMPORT_MUSICXML_IMPORTBREAKS("import/musicXml/importBreaks");
static const std::string PREF_IMPORT_MUSICXML_INFERTEXT("import/musicXml/importInferTextType");
static const std::string PREF_EXPORT_MUSICXML_EXPORTLAYOUT("export/musicXml/exportLayout");
static const std::string PREF_EXPORT_MUSICXML_EXPORTINVISIBLE("export/musicXml/exportInvisibleElements");

class MusicXml_Tests : public ::testing::Test
{
public:
    void musicXmlIoTest(const char* file, bool exportLayout = false);
    void musicXmlIoTestRef(const char* file);
    void musicXmlIoTestRefBreaks(const char* file);
    void musicXmlMscxExportTestRef(const char* file, bool exportLayout = false);
    void musicXmlMscxExportTestRefBreaks(const char* file);
    void musicXmlMscxExportTestRefInvisibleElements(const char* file);
    void musicXmlReadTestCompr(const char* file);
    void musicXmlReadWriteTestCompr(const char* file);
    void musicXmlImportTestRef(const char* file);

    void setValue(const std::string& key, const Val& value);

    MasterScore* readScore(const String& fileName, bool isAbsolutePath = false);
    bool saveCompareMusicXmlScore(MasterScore* score, const String& saveName, const String& compareWith);
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

void MusicXml_Tests::setValue(const std::string& key, const Val& value)
{
    settings()->setSharedValue(Settings::Key(MODULE_NAME, key), value);
}

MasterScore* MusicXml_Tests::readScore(const String& fileName, bool isAbsolutePath)
{
    String suffix = io::FileInfo::suffix(fileName);

    auto importXml = [](MasterScore* score, const muse::io::path_t& path) -> engraving::Err {
        return importMusicXml(score, path.toQString(), false);
    };

    auto importMxl = [](MasterScore* score, const muse::io::path_t& path) -> engraving::Err {
        return importCompressedMusicXml(score, path.toQString(), false);
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

bool MusicXml_Tests::saveCompareMusicXmlScore(MasterScore* score, const String& saveName, const String& compareWithLocalPath)
{
    EXPECT_TRUE(saveXml(score, saveName));
    return ScoreComp::compareFiles(ScoreRW::rootPath() + u"/" + compareWithLocalPath, saveName);
}

//---------------------------------------------------------
//   musicXmlIoTest
//   read a MusicXML file, write to a new file and verify both files are identical
//---------------------------------------------------------

void MusicXml_Tests::musicXmlIoTest(const char* file, bool exportLayout)
{
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicXmlExportBreaksType::Manual));
    setValue(PREF_IMPORT_MUSICXML_IMPORTBREAKS, Val(true));
    setValue(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, Val(exportLayout));
    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));
    setValue(PREF_IMPORT_MUSICXML_INFERTEXT, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".xml");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();
    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u".xml"));
    delete score;
}

//---------------------------------------------------------
//   musicXmlIoTestRef
//   read a MusicXML file, write to a new file and verify against reference
//---------------------------------------------------------

void MusicXml_Tests::musicXmlIoTestRef(const char* file)
{
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicXmlExportBreaksType::Manual));
    setValue(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, Val(false));
    setValue(PREF_IMPORT_MUSICXML_IMPORTBREAKS, Val(true));
    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));
    setValue(PREF_IMPORT_MUSICXML_INFERTEXT, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".xml");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();
    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_ref.xml"));
    delete score;
}

//---------------------------------------------------------
//   musicXmlIoTestRefBreaks
//   read a MusicXML file, write to a new file and verify against reference
//   using all possible settings for PREF_EXPORT_MUSICXML_EXPORTBREAKS
//---------------------------------------------------------

void MusicXml_Tests::musicXmlIoTestRefBreaks(const char* file)
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

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicXmlExportBreaksType::No));

    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_no_ref.xml"));

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicXmlExportBreaksType::Manual));

    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_manual_ref.xml"));

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicXmlExportBreaksType::All));

    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_all_ref.xml"));
    delete score;
}

//---------------------------------------------------------
//   musicXmlMscxExportTestRef
//   read a MuseScore mscx file, write to a MusicXML file and verify against reference
//---------------------------------------------------------

void MusicXml_Tests::musicXmlMscxExportTestRef(const char* file, bool exportLayout)
{
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicXmlExportBreaksType::Manual));
    setValue(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, Val(exportLayout));
    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));
    setValue(PREF_IMPORT_MUSICXML_INFERTEXT, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".mscx");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();

    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_ref.xml"));
    delete score;
}

//---------------------------------------------------------
//   musicXmlMscxExportTestRefBreaks
//   read a MuseScore mscx file, write to a MusicXML file and verify against reference
//   using all possible settings for PREF_EXPORT_MUSICXML_EXPORTBREAKS
//---------------------------------------------------------

void MusicXml_Tests::musicXmlMscxExportTestRefBreaks(const char* file)
{
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicXmlExportBreaksType::Manual));
    setValue(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, Val(false));
    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));
    setValue(PREF_IMPORT_MUSICXML_INFERTEXT, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".mscx");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicXmlExportBreaksType::No));

    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_no_ref.xml"));

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicXmlExportBreaksType::Manual));

    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_manual_ref.xml"));

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicXmlExportBreaksType::All));

    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_all_ref.xml"));
    delete score;
}

void MusicXml_Tests::musicXmlMscxExportTestRefInvisibleElements(const char* file)
{
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicXmlExportBreaksType::Manual));
    setValue(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, Val(false));
    setValue(PREF_IMPORT_MUSICXML_INFERTEXT, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".mscx");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();

    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));

    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_invisible_ref.xml"));

    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(false));

    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_noinvisible_ref.xml"));

    delete score;
}

//---------------------------------------------------------
//   musicXmlReadTestCompr
//   read a compressed MusicXML file, write to a new file and verify against reference
//---------------------------------------------------------

void MusicXml_Tests::musicXmlReadTestCompr(const char* file)
{
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicXmlExportBreaksType::Manual));
    setValue(PREF_IMPORT_MUSICXML_IMPORTBREAKS, Val(true));
    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));
    setValue(PREF_IMPORT_MUSICXML_INFERTEXT, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".mxl");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();
    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u"_mxl_read.xml", XML_IO_DATA_DIR + fileName + u".xml"));
    delete score;
}

//---------------------------------------------------------
//   musicXmlReadWriteTestCompr
//   read a MusicXML file, write to a compressed MusicXML file,
//   read the compressed MusicXML file, write to a new file and verify files are identical
//---------------------------------------------------------

void MusicXml_Tests::musicXmlReadWriteTestCompr(const char* file)
{
    // read xml
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicXmlExportBreaksType::Manual));
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
    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u"_mxl_read_write.xml", XML_IO_DATA_DIR + fileName + u".xml"));
    delete score;
}

//---------------------------------------------------------
//   musicXmlImportTestRef
//   read a MusicXML file, write to a new MuseScore mscx file
//   and verify against a MuseScore mscx reference file
//---------------------------------------------------------

void MusicXml_Tests::musicXmlImportTestRef(const char* file)
{
    MScore::debugMode = false;
    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicXmlExportBreaksType::Manual));
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

TEST_F(MusicXml_Tests, accidentals1) {
    musicXmlIoTest("testAccidentals1");
}
TEST_F(MusicXml_Tests, accidentals2) {
    musicXmlIoTest("testAccidentals2");
}
TEST_F(MusicXml_Tests, accidentals3) {
    musicXmlIoTest("testAccidentals3");
}
TEST_F(MusicXml_Tests, arpCrossVoice) {
    musicXmlImportTestRef("testArpCrossVoice");
}
TEST_F(MusicXml_Tests, arpGliss1) {
    musicXmlIoTest("testArpGliss1");
}
TEST_F(MusicXml_Tests, arpGliss2) {
    musicXmlIoTest("testArpGliss2");
}
TEST_F(MusicXml_Tests, arpGliss3) {
    musicXmlIoTest("testArpGliss3");
}
TEST_F(MusicXml_Tests, arpOnRest) {
    musicXmlImportTestRef("testArpOnRest");
}
TEST_F(MusicXml_Tests, articulationCombination) {
    musicXmlIoTestRef("testArticulationCombination");
}
TEST_F(MusicXml_Tests, backupRoundingError) {
    musicXmlImportTestRef("testBackupRoundingError");
}
TEST_F(MusicXml_Tests, barlineLoc) {
    musicXmlImportTestRef("testBarlineLoc");
}
TEST_F(MusicXml_Tests, barlineSpan) {
    musicXmlIoTest("testBarlineSpan");
}
TEST_F(MusicXml_Tests, barlineFermatas) {
    musicXmlIoTest("testBarlineFermatas");
}
TEST_F(MusicXml_Tests, barStyles) {
    musicXmlIoTest("testBarStyles");
}
TEST_F(MusicXml_Tests, barStyles2) {
    musicXmlIoTest("testBarStyles2");
}
TEST_F(MusicXml_Tests, barStyles3) {
    musicXmlIoTest("testBarStyles3");
}
TEST_F(MusicXml_Tests, barStyles4) {
    musicXmlIoTest("testBarStyles4");
}
TEST_F(MusicXml_Tests, bracketTypes) {
    musicXmlImportTestRef("testBracketTypes");
}
TEST_F(MusicXml_Tests, DISABLED_beamEnd) {
    musicXmlIoTest("testBeamEnd");
}
TEST_F(MusicXml_Tests, beamModes) {
    musicXmlImportTestRef("testBeamModes");
}
TEST_F(MusicXml_Tests, beams1) {
    musicXmlIoTest("testBeams1");
}
TEST_F(MusicXml_Tests, beams2) {
    musicXmlIoTest("testBeams2");
}
TEST_F(MusicXml_Tests, beams3) {
    musicXmlIoTestRef("testBeams3");
}
TEST_F(MusicXml_Tests, beams4) {
    musicXmlIoTest("testBeams4");
}
TEST_F(MusicXml_Tests, breaksImplExpl) {
    musicXmlMscxExportTestRefBreaks("testBreaksImplExpl");
}
TEST_F(MusicXml_Tests, breaksMMRest) {
    musicXmlMscxExportTestRefBreaks("testBreaksMMRest");
}
TEST_F(MusicXml_Tests, DISABLED_breaksManual) { // fail after sync with 3.x
    musicXmlIoTestRefBreaks("testBreaksManual");
}
TEST_F(MusicXml_Tests, DISABLED_breaksPage) { // fail after sync with 3.x
    musicXmlMscxExportTestRefBreaks("testBreaksPage");
}
TEST_F(MusicXml_Tests, breaksSystem) {
    musicXmlMscxExportTestRefBreaks("testBreaksSystem");
}
TEST_F(MusicXml_Tests, breathMarks) {
    musicXmlIoTest("testBreathMarks");
}
TEST_F(MusicXml_Tests, buzzRoll) {
    musicXmlImportTestRef("testBuzzRoll");
}
TEST_F(MusicXml_Tests, buzzRoll2) {
    musicXmlIoTest("testBuzzRoll2");
}
TEST_F(MusicXml_Tests, changeTranspose) {
    musicXmlIoTest("testChangeTranspose");
}
TEST_F(MusicXml_Tests, changeTransposeNoDiatonic) {
    musicXmlIoTestRef("testChangeTranspose-no-diatonic");
}
TEST_F(MusicXml_Tests, chordDiagrams1) {
    musicXmlIoTest("testChordDiagrams1");
}
TEST_F(MusicXml_Tests, chordNoVoice) {
    musicXmlIoTestRef("testChordNoVoice");
}
TEST_F(MusicXml_Tests, chordSymbols) {
    musicXmlMscxExportTestRef("testChordSymbols");
}
TEST_F(MusicXml_Tests, chordSymbols2) {
    musicXmlImportTestRef("testChordSymbols2");
}
TEST_F(MusicXml_Tests, clefs1) {
    musicXmlIoTest("testClefs1");
}
TEST_F(MusicXml_Tests, clefs2) {
    musicXmlIoTest("testClefs2");
}
TEST_F(MusicXml_Tests, codaHBox) {
    musicXmlImportTestRef("testCodaHBox");
}
TEST_F(MusicXml_Tests, colorExport) {
    musicXmlMscxExportTestRef("testColorExport");
}
TEST_F(MusicXml_Tests, colors) {
    musicXmlIoTest("testColors");
}
TEST_F(MusicXml_Tests, completeMeasureRests) {
    musicXmlIoTest("testCompleteMeasureRests");
}
TEST_F(MusicXml_Tests, copyrightScale) {
    musicXmlImportTestRef("testCopyrightScale");
}
TEST_F(MusicXml_Tests, cueGraceNotes1) {
    musicXmlImportTestRef("testCueGraceNotes");
}
TEST_F(MusicXml_Tests, cueGraceNotes2) {
    musicXmlIoTestRef("testCueGraceNotes");
}
TEST_F(MusicXml_Tests, cueNotes) {
    musicXmlIoTest("testCueNotes");
}
TEST_F(MusicXml_Tests, cueNotes2) {
    musicXmlMscxExportTestRef("testCueNotes2");
}
TEST_F(MusicXml_Tests, cueNotes3) {
    musicXmlImportTestRef("testCueNotes3");
}
TEST_F(MusicXml_Tests, dalSegno) {
    musicXmlIoTest("testDalSegno");
}
TEST_F(MusicXml_Tests, dcalCoda) {
    musicXmlIoTest("testDCalCoda");
}
TEST_F(MusicXml_Tests, dcalFine) {
    musicXmlIoTest("testDCalFine");
}
TEST_F(MusicXml_Tests, directions1) {
    musicXmlIoTestRef("testDirections1");
}
TEST_F(MusicXml_Tests, directions2) {
    musicXmlIoTest("testDirections2");
}
TEST_F(MusicXml_Tests, displayStepOctave) {
    musicXmlMscxExportTestRef("testDisplayStepOctave");
}
TEST_F(MusicXml_Tests, divisionsDefinedTooLate1) {
    musicXmlIoTestRef("testDivsDefinedTooLate1");
}
TEST_F(MusicXml_Tests, divisionsDefinedTooLate2) {
    musicXmlIoTestRef("testDivsDefinedTooLate2");
}
TEST_F(MusicXml_Tests, divisionsDuration) {
    musicXmlIoTest("testDivisionsDuration");
}
TEST_F(MusicXml_Tests, doletOttavas) {
    musicXmlImportTestRef("testDoletOttavas");
}
TEST_F(MusicXml_Tests, doubleClefError) {
    musicXmlIoTestRef("testDoubleClefError");
}
TEST_F(MusicXml_Tests, drumset1) {
    musicXmlIoTest("testDrumset1");
}
TEST_F(MusicXml_Tests, drumset2) {
    musicXmlIoTest("testDrumset2");
}
TEST_F(MusicXml_Tests, dsalCoda) {
    musicXmlImportTestRef("testDSalCoda");
}
TEST_F(MusicXml_Tests, dsalCodaMisplaced) {
    musicXmlImportTestRef("testDSalCodaMisplaced");
}
TEST_F(MusicXml_Tests, durationLargeErrorMscx) {
    musicXmlImportTestRef("testDurationLargeError");
}
TEST_F(MusicXml_Tests, duplicateFermataOnGraceNote) {
    musicXmlImportTestRef("testDuplicateFermataOnGraceNote");
}
TEST_F(MusicXml_Tests, duplicateFermataOnGraceNoteAndMainNote) {
    musicXmlImportTestRef("testDuplicateFermataOnGraceNoteAndMainNote");
}
TEST_F(MusicXml_Tests, duplicateInstrChange) {
    musicXmlImportTestRef("testDuplicateInstrChange");
}
TEST_F(MusicXml_Tests, durationLargeErrorXml) {
    musicXmlIoTestRef("testDurationLargeError");
}
TEST_F(MusicXml_Tests, durationRoundingErrorMscx) {
    musicXmlImportTestRef("testDurationRoundingError");
}
TEST_F(MusicXml_Tests, durationRoundingErrorXml) {
    musicXmlIoTestRef("testDurationRoundingError");
}
TEST_F(MusicXml_Tests, dynamics1) {
    musicXmlIoTest("testDynamics1");
}
TEST_F(MusicXml_Tests, dynamics2) {
    musicXmlIoTest("testDynamics2");
}
TEST_F(MusicXml_Tests, dynamics3) {
    musicXmlIoTestRef("testDynamics3");
}
TEST_F(MusicXml_Tests, elision) {
    musicXmlImportTestRef("testElision");
}
TEST_F(MusicXml_Tests, emptyMeasure) {
    musicXmlIoTestRef("testEmptyMeasure");
}
TEST_F(MusicXml_Tests, emptyVoice1) {
    musicXmlIoTestRef("testEmptyVoice1");
}
TEST_F(MusicXml_Tests, excludeInvisibleElements) {
    musicXmlMscxExportTestRefInvisibleElements("testExcludeInvisibleElements");
}
TEST_F(MusicXml_Tests, excessHiddenStaves) {
    musicXmlImportTestRef("testExcessHiddenStaves");
}
TEST_F(MusicXml_Tests, extendedLyrics) {
    musicXmlIoTestRef("testExtendedLyrics");
}
TEST_F(MusicXml_Tests, figuredBass1) {
    musicXmlIoTest("testFiguredBass1");
}
TEST_F(MusicXml_Tests, figuredBass2) {
    musicXmlIoTest("testFiguredBass2");
}
TEST_F(MusicXml_Tests, figuredBass3) {
    musicXmlIoTest("testFiguredBass3");
}
TEST_F(MusicXml_Tests, figuredBassDivisions) {
    musicXmlIoTest("testFiguredBassDivisions");
}
TEST_F(MusicXml_Tests, finaleDynamics) {
    musicXmlImportTestRef("testFinaleDynamics");
}
TEST_F(MusicXml_Tests, finaleInstr) {
    musicXmlImportTestRef("testFinaleInstr");
}
TEST_F(MusicXml_Tests, finaleInstr2) {
    musicXmlImportTestRef("testFinaleInstr2");
}
TEST_F(MusicXml_Tests, finaleSystemObjects) {
    musicXmlImportTestRef("testFinaleSystemObjects");
}
TEST_F(MusicXml_Tests, formattedThings) {
    musicXmlIoTest("testFormattedThings");
}
TEST_F(MusicXml_Tests, fractionMinus) {
    musicXmlIoTestRef("testFractionMinus");
}
TEST_F(MusicXml_Tests, fractionPlus) {
    musicXmlIoTestRef("testFractionPlus");
}
TEST_F(MusicXml_Tests, fractionTicks) {
    musicXmlIoTestRef("testFractionTicks");
}
TEST_F(MusicXml_Tests, glissFall) {
    musicXmlImportTestRef("testGlissFall");
}
TEST_F(MusicXml_Tests, grace1) {
    musicXmlIoTest("testGrace1");
}
TEST_F(MusicXml_Tests, grace2) {
    musicXmlIoTest("testGrace2");
}
TEST_F(MusicXml_Tests, grace3) {
    musicXmlIoTest("testGrace3");
}
TEST_F(MusicXml_Tests, graceAfter1) {
    musicXmlIoTest("testGraceAfter1");
}
TEST_F(MusicXml_Tests, graceAfter2) {
    musicXmlIoTest("testGraceAfter2");
}
TEST_F(MusicXml_Tests, graceAfter3) {
    musicXmlIoTest("testGraceAfter3");
}
TEST_F(MusicXml_Tests, DISABLED_graceAfter4) {
    musicXmlIoTest("testGraceAfter4");
}
TEST_F(MusicXml_Tests, graceFermata) {
    musicXmlIoTest("testGraceFermata");
}
TEST_F(MusicXml_Tests, guitarBends) {
    bool useRead302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;
    musicXmlMscxExportTestRef("testGuitarBends");
    MScore::useRead302InTestMode = useRead302;
}
TEST_F(MusicXml_Tests, harpPedals) {
    musicXmlMscxExportTestRef("testHarpPedals");
}
TEST_F(MusicXml_Tests, hairpinDynamics) {
    musicXmlMscxExportTestRef("testHairpinDynamics");
}
TEST_F(MusicXml_Tests, handbells) {
    musicXmlIoTest("testHandbells");
}
TEST_F(MusicXml_Tests, harmony1) {
    musicXmlIoTest("testHarmony1");
}
TEST_F(MusicXml_Tests, harmony2) {
    musicXmlIoTest("testHarmony2");
}
TEST_F(MusicXml_Tests, harmony3) {
    musicXmlIoTest("testHarmony3");
}
TEST_F(MusicXml_Tests, harmony4) {
    musicXmlIoTest("testHarmony4");
}
TEST_F(MusicXml_Tests, harmony5) {
    musicXmlIoTest("testHarmony5");
}                                                                      // chordnames without chordrest
TEST_F(MusicXml_Tests, harmony6) {
    musicXmlMscxExportTestRef("testHarmony6");
}
TEST_F(MusicXml_Tests, harmony7) {
    musicXmlMscxExportTestRef("testHarmony7");
}
TEST_F(MusicXml_Tests, harmony8) {
    musicXmlIoTest("testHarmony8");
}
TEST_F(MusicXml_Tests, harmony9) {
    musicXmlIoTest("testHarmony9");
}                                                                      // chordnames without chordrest
TEST_F(MusicXml_Tests, harmonMutes) {
    musicXmlIoTest("testHarmonMutes");
}
TEST_F(MusicXml_Tests, hello) {
    musicXmlIoTest("testHello");
}
TEST_F(MusicXml_Tests, helloReadCompr) {
    musicXmlReadTestCompr("testHello");
}
TEST_F(MusicXml_Tests, helloReadWriteCompr) {
    musicXmlReadWriteTestCompr("testHello");
}
TEST_F(MusicXml_Tests, holes) {
    musicXmlIoTest("testHoles");
}
TEST_F(MusicXml_Tests, implicitMeasure1) {
    musicXmlIoTest("testImplicitMeasure1");
}
TEST_F(MusicXml_Tests, importDrums) {
    musicXmlImportTestRef("testImportDrums");
}
TEST_F(MusicXml_Tests, importDrums2) {
    musicXmlImportTestRef("testImportDrums2");
}
TEST_F(MusicXml_Tests, incompleteTuplet) {
    musicXmlIoTestRef("testIncompleteTuplet");
}
TEST_F(MusicXml_Tests, incorrectMidiProgram) {
    musicXmlIoTestRef("testIncorrectMidiProgram");
}

TEST_F(MusicXml_Tests, incorrectStaffNumber1) {
    musicXmlIoTestRef("testIncorrectStaffNumber1");
}
TEST_F(MusicXml_Tests, incorrectStaffNumber2) {
    musicXmlIoTestRef("testIncorrectStaffNumber2");
}
TEST_F(MusicXml_Tests, DISABLED_EXCEPT_ON_LINUX(inferredCredits1)) {
    musicXmlImportTestRef("testInferredCredits1");
}
TEST_F(MusicXml_Tests, DISABLED_EXCEPT_ON_LINUX(inferredCredits2)) {
    musicXmlImportTestRef("testInferredCredits2");
}
TEST_F(MusicXml_Tests, inferCodaII) {
    musicXmlImportTestRef("testInferCodaII");
}
TEST_F(MusicXml_Tests, inferredDynamicRange) {
    musicXmlImportTestRef("testInferredDynamicRange");
}
TEST_F(MusicXml_Tests, inferSegnoII) {
    musicXmlImportTestRef("testInferSegnoII");
}
TEST_F(MusicXml_Tests, inferFraction) {
    musicXmlImportTestRef("testInferFraction");
}
TEST_F(MusicXml_Tests, inferredFingerings) {
    musicXmlImportTestRef("testInferredFingerings");
}
TEST_F(MusicXml_Tests, inferredCrescLines) {
    musicXmlImportTestRef("testInferredCrescLines");
}
TEST_F(MusicXml_Tests, inferredDynamicsExpression) {
    musicXmlImportTestRef("testInferredDynamicsExpression");
}
TEST_F(MusicXml_Tests, inferredRights) {
    musicXmlImportTestRef("testInferredRights");
}
TEST_F(MusicXml_Tests, DISABLED_inferredTechnique) {
    musicXmlImportTestRef("testInferredTechnique");
}
TEST_F(MusicXml_Tests, inferredTempoText) {
    musicXmlImportTestRef("testInferredTempoText");
}
TEST_F(MusicXml_Tests, inferredTempoText2) {
    musicXmlImportTestRef("testInferredTempoText2");
}
TEST_F(MusicXml_Tests, inferredCrescLines2) {
    musicXmlImportTestRef("testInferredCrescLines2");
}
TEST_F(MusicXml_Tests, instrumentChangeMIDIportExport) {
    musicXmlMscxExportTestRef("testInstrumentChangeMIDIportExport");
}
TEST_F(MusicXml_Tests, instrumentSound) {
    musicXmlIoTestRef("testInstrumentSound");
}
TEST_F(MusicXml_Tests, instrImport) {
    musicXmlImportTestRef("testInstrImport");
}
TEST_F(MusicXml_Tests, invalidLayout) {
    musicXmlMscxExportTestRef("testInvalidLayout");
}
TEST_F(MusicXml_Tests, invalidTimesig) {
    musicXmlIoTestRef("testInvalidTimesig");
}
TEST_F(MusicXml_Tests, invisibleDirection) {
    musicXmlIoTest("testInvisibleDirection");
}
TEST_F(MusicXml_Tests, invisibleElements) {
    musicXmlIoTest("testInvisibleElements");
}
TEST_F(MusicXml_Tests, invisibleNote) {
    musicXmlMscxExportTestRef("testInvisibleNote");
}
TEST_F(MusicXml_Tests, keysig1) {
    musicXmlIoTest("testKeysig1");
}
TEST_F(MusicXml_Tests, keysig2) {
    musicXmlIoTest("testKeysig2");
}
TEST_F(MusicXml_Tests, DISABLED_EXCEPT_ON_LINUX(layout)) {
    musicXmlIoTest("testLayout", true);
}
TEST_F(MusicXml_Tests, lessWhiteSpace) {
    musicXmlIoTestRef("testLessWhiteSpace");
}
TEST_F(MusicXml_Tests, lines1) {
    musicXmlIoTest("testLines1");
}
TEST_F(MusicXml_Tests, lines2) {
    musicXmlIoTest("testLines2");
}
TEST_F(MusicXml_Tests, lines3) {
    musicXmlIoTest("testLines3");
}
TEST_F(MusicXml_Tests, lines4) {
    musicXmlMscxExportTestRef("testLines4");
}
TEST_F(MusicXml_Tests, lineDetails) {
    musicXmlIoTest("testLineDetails");
}
TEST_F(MusicXml_Tests, lyricBracket) {
    musicXmlImportTestRef("testLyricBracket");
}
TEST_F(MusicXml_Tests, lyricColor) {
    musicXmlIoTest("testLyricColor");
}
TEST_F(MusicXml_Tests, lyricPos) {
    musicXmlImportTestRef("testLyricPos");
}
TEST_F(MusicXml_Tests, lyrics1) {
    musicXmlIoTestRef("testLyrics1");
}
TEST_F(MusicXml_Tests, lyricExtension1) {
    musicXmlIoTest("testLyricExtensions");
}
TEST_F(MusicXml_Tests, lyricExtension2) {
    musicXmlImportTestRef("testLyricExtensions");
}
TEST_F(MusicXml_Tests, lyricExtension3) {
    musicXmlIoTest("testLyricExtension2");
}
TEST_F(MusicXml_Tests, lyricExtension4) {
    musicXmlImportTestRef("testLyricExtension2");
}
TEST_F(MusicXml_Tests, lyricsVoice2a) {
    musicXmlIoTest("testLyricsVoice2a");
}
TEST_F(MusicXml_Tests, lyricsVoice2b) {
    musicXmlIoTestRef("testLyricsVoice2b");
}
TEST_F(MusicXml_Tests, maxNumberLevel) {
    musicXmlMscxExportTestRef("testMaxNumberLevel");
}
TEST_F(MusicXml_Tests, measureLength) {
    musicXmlIoTestRef("testMeasureLength");
}
TEST_F(MusicXml_Tests, measureNumbers) {
    musicXmlIoTest("testMeasureNumbers");
}
TEST_F(MusicXml_Tests, measureNumberOffset) {
    musicXmlIoTest("testMeasureNumberOffset");
}
TEST_F(MusicXml_Tests, measureRepeats1) {
    musicXmlIoTestRef("testMeasureRepeats1");
}
TEST_F(MusicXml_Tests, DISABLED_measureRepeats2) {
    musicXmlIoTestRef("testMeasureRepeats2");
}
TEST_F(MusicXml_Tests, measureRepeats3) {
    musicXmlIoTest("testMeasureRepeats3");
}
TEST_F(MusicXml_Tests, measureStyleSlash) {
    musicXmlImportTestRef("testMeasureStyleSlash");
}
TEST_F(MusicXml_Tests, midiPortExport) {
    musicXmlMscxExportTestRef("testMidiPortExport");
}
TEST_F(MusicXml_Tests, ms3KitAndPerc) {
    musicXmlImportTestRef("testMS3KitAndPerc");
}
TEST_F(MusicXml_Tests, multiInstrumentPart1) {
    musicXmlIoTest("testMultiInstrumentPart1");
}
TEST_F(MusicXml_Tests, multiInstrumentPart2) {
    musicXmlIoTest("testMultiInstrumentPart2");
}
TEST_F(MusicXml_Tests, multiInstrumentPart3) {
    musicXmlMscxExportTestRef("testMultiInstrumentPart3");
}
TEST_F(MusicXml_Tests, multiMeasureRest1) {
    musicXmlIoTestRef("testMultiMeasureRest1");
}
TEST_F(MusicXml_Tests, multiMeasureRest2) {
    musicXmlIoTestRef("testMultiMeasureRest2");
}
TEST_F(MusicXml_Tests, multiMeasureRest3) {
    musicXmlIoTestRef("testMultiMeasureRest3");
}
TEST_F(MusicXml_Tests, multiMeasureRest4) {
    musicXmlIoTestRef("testMultiMeasureRest4");
}
TEST_F(MusicXml_Tests, multipleNotations) {
    musicXmlIoTestRef("testMultipleNotations");
}
TEST_F(MusicXml_Tests, namedNoteheads) {
    musicXmlImportTestRef("testNamedNoteheads");
}
TEST_F(MusicXml_Tests, negativeOffset) {
    musicXmlImportTestRef("testNegativeOffset");
}
TEST_F(MusicXml_Tests, negativeOctave) {
    musicXmlMscxExportTestRef("testNegativeOctave");
}
TEST_F(MusicXml_Tests, nonStandardKeySig1) {
    musicXmlIoTest("testNonStandardKeySig1");
}
TEST_F(MusicXml_Tests, nonStandardKeySig2) {
    musicXmlIoTest("testNonStandardKeySig2");
}
TEST_F(MusicXml_Tests, nonStandardKeySig3) {
    musicXmlIoTest("testNonStandardKeySig3");
}
TEST_F(MusicXml_Tests, nonUniqueThings) {
    musicXmlIoTestRef("testNonUniqueThings");
}
TEST_F(MusicXml_Tests, noteAttributes1) {
    musicXmlIoTest("testNoteAttributes1");
}
TEST_F(MusicXml_Tests, noteAttributes2) {
    musicXmlIoTestRef("testNoteAttributes2");
}
TEST_F(MusicXml_Tests, noteAttributes3) {
    musicXmlIoTest("testNoteAttributes3");
}
TEST_F(MusicXml_Tests, DISABLED_noteAttributes4) {
    musicXmlImportTestRef("testNoteAttributes2");
}
TEST_F(MusicXml_Tests, noteColor) {
    musicXmlIoTest("testNoteColor");
}
TEST_F(MusicXml_Tests, noteheadNames) {
    musicXmlIoTest("testNoteheadNames");
}
TEST_F(MusicXml_Tests, noteheadParentheses) {
    musicXmlIoTest("testNoteheadParentheses");
}
TEST_F(MusicXml_Tests, noteheads) {
    musicXmlIoTest("testNoteheads");
}
TEST_F(MusicXml_Tests, noteheads2) {
    musicXmlMscxExportTestRef("testNoteheads2");
}
TEST_F(MusicXml_Tests, noteheadsFilled) {
    musicXmlIoTest("testNoteheadsFilled");
}
TEST_F(MusicXml_Tests, notesRests1) {
    musicXmlIoTest("testNotesRests1");
}
TEST_F(MusicXml_Tests, notesRests2) {
    musicXmlIoTest("testNotesRests2");
}
TEST_F(MusicXml_Tests, numberedLyrics) {
    musicXmlIoTestRef("testNumberedLyrics");
}
TEST_F(MusicXml_Tests, numerals) {
    musicXmlIoTest("testNumerals");
}
TEST_F(MusicXml_Tests, ornaments) {
    musicXmlImportTestRef("testOrnaments");
}
TEST_F(MusicXml_Tests, overlappingSpanners) {
    musicXmlIoTest("testOverlappingSpanners");
}
TEST_F(MusicXml_Tests, partNames) {
    musicXmlImportTestRef("testPartNames");
}
TEST_F(MusicXml_Tests, partNames2) {
    musicXmlIoTest("testPartNames2");
}
TEST_F(MusicXml_Tests, pedalChanges) {
    musicXmlIoTest("testPedalChanges");
}
TEST_F(MusicXml_Tests, pedalChangesBroken) {
    musicXmlImportTestRef("testPedalChangesBroken");
}
TEST_F(MusicXml_Tests, pedalStyles) {
    musicXmlIoTest("testPedalStyles");
}
TEST_F(MusicXml_Tests, placementDefaults) {
    musicXmlImportTestRef("testPlacementDefaults");
}
TEST_F(MusicXml_Tests, playtech) {
    musicXmlIoTest("testPlaytech");
}
TEST_F(MusicXml_Tests, printSpacingNo) {
    musicXmlIoTestRef("testPrintSpacingNo");
}
TEST_F(MusicXml_Tests, placementOffsetDefaults) {
    musicXmlImportTestRef("testPlacementOffsetDefaults");
}
TEST_F(MusicXml_Tests, repeatCounts) {
    musicXmlIoTest("testRepeatCounts");
}
TEST_F(MusicXml_Tests, repeatSingleMeasure) {
    musicXmlIoTest("testRepeatSingleMeasure");
}
TEST_F(MusicXml_Tests, restNotations) {
    musicXmlIoTestRef("testRestNotations");
}
TEST_F(MusicXml_Tests, restsNoType) {
    musicXmlIoTestRef("testRestsNoType");
}
TEST_F(MusicXml_Tests, restsTypeWhole) {
    musicXmlIoTestRef("testRestsTypeWhole");
}
TEST_F(MusicXml_Tests, secondVoiceMelismata) {
    musicXmlImportTestRef("testSecondVoiceMelismata");
}
TEST_F(MusicXml_Tests, sibMetronomeMarks) {
    musicXmlImportTestRef("testSibMetronomeMarks");
}
TEST_F(MusicXml_Tests, sibOttavas) {
    musicXmlImportTestRef("testSibOttavas");
}
TEST_F(MusicXml_Tests, sibRitLine) {
    musicXmlImportTestRef("testSibRitLine");
}
TEST_F(MusicXml_Tests, slurTieDirection) {
    musicXmlIoTest("testSlurTieDirection");
}
TEST_F(MusicXml_Tests, slurTieLineStyle) {
    musicXmlIoTest("testSlurTieLineStyle");
}
TEST_F(MusicXml_Tests, slurs) {
    musicXmlIoTest("testSlurs");
}
TEST_F(MusicXml_Tests, slurs2) {
    musicXmlIoTest("testSlurs2");
}
TEST_F(MusicXml_Tests, sound1) {
    musicXmlIoTestRef("testSound1");
}
TEST_F(MusicXml_Tests, sound2) {
    musicXmlIoTestRef("testSound2");
}
TEST_F(MusicXml_Tests, specialCharacters) {
    musicXmlIoTest("testSpecialCharacters");
}
TEST_F(MusicXml_Tests, staffEmptiness) {
    musicXmlImportTestRef("testStaffEmptiness");
}
TEST_F(MusicXml_Tests, staffSize) {
    musicXmlIoTest("testStaffSize");
}
TEST_F(MusicXml_Tests, staffTwoKeySigs) {
    musicXmlIoTest("testStaffTwoKeySigs");
}
TEST_F(MusicXml_Tests, sticking) {
    musicXmlImportTestRef("testSticking");
}
TEST_F(MusicXml_Tests, stickingLyrics) {
    musicXmlImportTestRef("testStickingLyrics");
}
TEST_F(MusicXml_Tests, stringData) {
    musicXmlIoTest("testStringData");
}
TEST_F(MusicXml_Tests, stringVoiceName) {
    musicXmlIoTestRef("testStringVoiceName");
}
TEST_F(MusicXml_Tests, swing) {
    musicXmlMscxExportTestRef("testSwing");
}
TEST_F(MusicXml_Tests, systemBrackets1) {
    musicXmlIoTest("testSystemBrackets1");
}
TEST_F(MusicXml_Tests, systemBrackets2) {
    musicXmlIoTest("testSystemBrackets2");
}
TEST_F(MusicXml_Tests, systemBrackets3) {
    musicXmlImportTestRef("testSystemBrackets3");
}
TEST_F(MusicXml_Tests, systemBrackets4) {
    musicXmlIoTest("testSystemBrackets1");
}
TEST_F(MusicXml_Tests, systemBrackets5) {
    musicXmlIoTest("testSystemBrackets1");
}
TEST_F(MusicXml_Tests, systemDirection) {
    musicXmlIoTest("testSystemDirection");
}
TEST_F(MusicXml_Tests, DISABLED_EXCEPT_ON_LINUX(systemDistance)) {
    musicXmlMscxExportTestRef("testSystemDistance", true);
}
TEST_F(MusicXml_Tests, DISABLED_EXCEPT_ON_LINUX(systemDividers)) {
    musicXmlIoTest("testSystemDividers", true);
}
TEST_F(MusicXml_Tests, systemObjectStaves) {
    musicXmlImportTestRef("testSystemObjectStaves");
}
TEST_F(MusicXml_Tests, tablature1) {
    musicXmlIoTest("testTablature1");
}
TEST_F(MusicXml_Tests, tablature2) {
    musicXmlIoTest("testTablature2");
}
TEST_F(MusicXml_Tests, tablature3) {
    musicXmlIoTest("testTablature3");
}
TEST_F(MusicXml_Tests, tablature4) {
    musicXmlIoTest("testTablature4");
}
TEST_F(MusicXml_Tests, tablature5) {
    musicXmlIoTestRef("testTablature5");
}
TEST_F(MusicXml_Tests, tboxAboveBelow1) {
    musicXmlMscxExportTestRef("testTboxAboveBelow1");
}
TEST_F(MusicXml_Tests, tboxAboveBelow2) {
    musicXmlMscxExportTestRef("testTboxAboveBelow2");
}
TEST_F(MusicXml_Tests, tboxAboveBelow3) {
    musicXmlMscxExportTestRef("testTboxAboveBelow3");
}
TEST_F(MusicXml_Tests, tboxMultiPage1) {
    musicXmlMscxExportTestRef("testTboxMultiPage1");
}
TEST_F(MusicXml_Tests, tboxVbox1) {
    musicXmlMscxExportTestRef("testTboxVbox1");
}
TEST_F(MusicXml_Tests, tboxWords1) {
    musicXmlMscxExportTestRef("testTboxWords1");
}
TEST_F(MusicXml_Tests, tempo1) {
    musicXmlIoTest("testTempo1");
}
TEST_F(MusicXml_Tests, tempo2) {
    musicXmlIoTestRef("testTempo2");
}
TEST_F(MusicXml_Tests, tempo3) {
    musicXmlIoTestRef("testTempo3");
}
TEST_F(MusicXml_Tests, tempo4) {
    musicXmlIoTestRef("testTempo4");
}
TEST_F(MusicXml_Tests, tempo5) {
    musicXmlIoTest("testTempo5");
}
TEST_F(MusicXml_Tests, tempo6) {
    musicXmlIoTest("testTempo6");
}
TEST_F(MusicXml_Tests, tempoLineFermata) {
    musicXmlImportTestRef("testTempoLineFermata");
}
TEST_F(MusicXml_Tests, tempoOverlap) {
    musicXmlIoTestRef("testTempoOverlap");
}
TEST_F(MusicXml_Tests, tempoPrecision) {
    musicXmlMscxExportTestRef("testTempoPrecision");
}
TEST_F(MusicXml_Tests, tempoTextSpace1) {
    musicXmlImportTestRef("testTempoTextSpace1");
}
TEST_F(MusicXml_Tests, tempoTextSpace2) {
    musicXmlImportTestRef("testTempoTextSpace2");
}
TEST_F(MusicXml_Tests, textLines) {
    musicXmlMscxExportTestRef("testTextLines");
}
TEST_F(MusicXml_Tests, testTextOrder) {
    musicXmlImportTestRef("testTextOrder");
}
TEST_F(MusicXml_Tests, textQuirkInference) {
    musicXmlImportTestRef("testTextQuirkInference");
}
TEST_F(MusicXml_Tests, tieTied) {
    musicXmlIoTestRef("testTieTied");
}
TEST_F(MusicXml_Tests, importTie1) {
    musicXmlImportTestRef("importTie1");
}
TEST_F(MusicXml_Tests, importTie2) {
    // Finale ties to different voices
    musicXmlImportTestRef("importTie2");
}
TEST_F(MusicXml_Tests, importTie3) {
    // Dolet6 ties to different voices & staves
    musicXmlImportTestRef("importTie3");
}
TEST_F(MusicXml_Tests, importTie4) {
    // Dolet8 ties to different voices & staves
    musicXmlImportTestRef("importTie4");
}
TEST_F(MusicXml_Tests, timesig1) {
    musicXmlIoTest("testTimesig1");
}
TEST_F(MusicXml_Tests, timesig3) {
    musicXmlIoTest("testTimesig3");
}
TEST_F(MusicXml_Tests, timesig4) {
    musicXmlIoTest("testTimesig4");
}
TEST_F(MusicXml_Tests, timeTick) {
    musicXmlImportTestRef("testTimeTick");
}
TEST_F(MusicXml_Tests, timeTickExport) {
    bool use302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;
    musicXmlMscxExportTestRef("testTimeTickExport");
    MScore::useRead302InTestMode = use302;
}
TEST_F(MusicXml_Tests, titleSwapMu) {
    musicXmlImportTestRef("testTitleSwapMu");
}
TEST_F(MusicXml_Tests, titleSwapSib) {
    musicXmlImportTestRef("testTitleSwapSib");
}
TEST_F(MusicXml_Tests, trackHandling) {
    musicXmlIoTest("testTrackHandling");
}
TEST_F(MusicXml_Tests, tremolo) {
    musicXmlIoTest("testTremolo");
}
TEST_F(MusicXml_Tests, trills) {
    musicXmlMscxExportTestRef("testTrills");
}
TEST_F(MusicXml_Tests, tuplets1) {
    musicXmlIoTestRef("testTuplets1");
}
TEST_F(MusicXml_Tests, tuplets2) {
    musicXmlIoTestRef("testTuplets2");
}
TEST_F(MusicXml_Tests, tuplets3) {
    musicXmlIoTestRef("testTuplets3");
}
TEST_F(MusicXml_Tests, tuplets4) {
    musicXmlIoTest("testTuplets4");
}
TEST_F(MusicXml_Tests, tuplets5) {
    musicXmlIoTestRef("testTuplets5");
}
TEST_F(MusicXml_Tests, tuplets6) {
    musicXmlIoTestRef("testTuplets6");
}
TEST_F(MusicXml_Tests, tuplets7) {
    musicXmlIoTest("testTuplets7");
}
TEST_F(MusicXml_Tests, tuplets8) {
    musicXmlMscxExportTestRef("testTuplets8");
}
TEST_F(MusicXml_Tests, tuplets9) {
    musicXmlIoTest("testTuplets9");
}
TEST_F(MusicXml_Tests, tupletTie) {
    musicXmlImportTestRef("testTupletTie");
}
TEST_F(MusicXml_Tests, twoNoteTremoloTuplet) {
    musicXmlIoTest("testTwoNoteTremoloTuplet");
}
TEST_F(MusicXml_Tests, uninitializedDivisions) {
    musicXmlIoTestRef("testUninitializedDivisions");
}
TEST_F(MusicXml_Tests, unnecessaryBarlines) {
    musicXmlImportTestRef("testUnnecessaryBarlines");
}
TEST_F(MusicXml_Tests, unusualDurations) {
    musicXmlIoTestRef("testUnusualDurations");
}
TEST_F(MusicXml_Tests, unterminatedTies) {
    musicXmlImportTestRef("testUnterminatedTies");
}
TEST_F(MusicXml_Tests, virtualInstruments) {
    musicXmlIoTestRef("testVirtualInstruments");
}
TEST_F(MusicXml_Tests, voiceMapper1) {
    musicXmlIoTestRef("testVoiceMapper1");
}
TEST_F(MusicXml_Tests, voiceMapper2) {
    musicXmlIoTestRef("testVoiceMapper2");
}
TEST_F(MusicXml_Tests, voiceMapper3) {
    musicXmlIoTestRef("testVoiceMapper3");
}
TEST_F(MusicXml_Tests, voicePiano1) {
    musicXmlIoTest("testVoicePiano1");
}
TEST_F(MusicXml_Tests, volta1) {
    musicXmlIoTest("testVolta1");
}
TEST_F(MusicXml_Tests, volta2) {
    musicXmlIoTest("testVolta2");
}
TEST_F(MusicXml_Tests, voltaHiding1) {
    musicXmlImportTestRef("testVoltaHiding");
}
TEST_F(MusicXml_Tests, voltaHiding2) {
    musicXmlIoTestRef("testVoltaHiding");
}
TEST_F(MusicXml_Tests, wedgeOffset) {
    musicXmlImportTestRef("testWedgeOffset");
}
TEST_F(MusicXml_Tests, wedge1) {
    musicXmlIoTest("testWedge1");
}
TEST_F(MusicXml_Tests, wedge2) {
    musicXmlIoTest("testWedge2");
}
TEST_F(MusicXml_Tests, wedge3) {
    musicXmlIoTest("testWedge3");
}
TEST_F(MusicXml_Tests, wedge4) {
    musicXmlMscxExportTestRef("testWedge4");
}
TEST_F(MusicXml_Tests, wedge5) {
    musicXmlIoTestRef("testWedge5");
}
TEST_F(MusicXml_Tests, words1) {
    musicXmlIoTest("testWords1");
}
TEST_F(MusicXml_Tests, words2) {
    musicXmlIoTest("testWords2");
}
TEST_F(MusicXml_Tests, hiddenStaves)
{
    String fileName = String::fromUtf8("testHiddenStaves.xml");
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName);

    EXPECT_EQ(score->style().value(Sid::hideEmptyStaves).toBool(), true);
}
