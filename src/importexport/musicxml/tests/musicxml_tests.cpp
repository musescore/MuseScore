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

#include "engraving/engravingerrors.h"
#include "engraving/dom/masterscore.h"

#include "settings.h"
#include "importexport/musicxml/imusicxmlconfiguration.h"
#include "importexport/musicxml/internal/musicxml/exportxml.h"

#include "engraving/tests/utils/scorerw.h"
#include "engraving/tests/utils/scorecomp.h"

#include "io/fileinfo.h"

using namespace mu;
using namespace mu::framework;
using namespace mu::iex::musicxml;
using namespace mu::engraving;

namespace mu::engraving {
extern bool saveMxl(Score*, const QString&);
extern engraving::Err importMusicXml(MasterScore*, const QString&);
extern engraving::Err importCompressedMusicXml(MasterScore*, const QString&);
}

static const String XML_IO_DATA_DIR("data/");

static const std::string MODULE_NAME("iex_musicxml");

static const std::string PREF_EXPORT_MUSICXML_EXPORTBREAKS("export/musicXML/exportBreaks");
static const std::string PREF_IMPORT_MUSICXML_IMPORTBREAKS("import/musicXML/importBreaks");
static const std::string PREF_EXPORT_MUSICXML_EXPORTLAYOUT("export/musicXML/exportLayout");
static const std::string PREF_EXPORT_MUSICXML_EXPORTINVISIBLE("export/musicXML/exportInvisibleElements");

//---------------------------------------------------------
//   TestMxmlIO
//---------------------------------------------------------

class Musicxml_Tests : public ::testing::Test
{
public:
    void mxmlIoTest(const char* file, bool exportLayout = false);
    void mxmlIoTestRef(const char* file);
    void mxmlIoTestRefBreaks(const char* file);
    void mxmlMscxExportTestRef(const char* file, bool exportLayout = false);
    void mxmlMscxExportTestRefBreaks(const char* file);
    void mxmlMscxExportTestRefInvisibleElements(const char* file);
    void mxmlReadTestCompr(const char* file);
    void mxmlReadWriteTestCompr(const char* file);
    void mxmlImportTestRef(const char* file);

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

void Musicxml_Tests::setValue(const std::string& key, const Val& value)
{
    settings()->setSharedValue(Settings::Key(MODULE_NAME, key), value);
}

MasterScore* Musicxml_Tests::readScore(const String& fileName, bool isAbsolutePath)
{
    String suffix = io::FileInfo::suffix(fileName);

    auto importXml = [](MasterScore* score, const io::path_t& path) -> engraving::Err {
        return mu::engraving::importMusicXml(score, path.toQString());
    };

    auto importMxl = [](MasterScore* score, const io::path_t& path) -> engraving::Err {
        return mu::engraving::importCompressedMusicXml(score, path.toQString());
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

bool Musicxml_Tests::saveCompareMusicXmlScore(MasterScore* score, const String& saveName, const String& compareWithLocalPath)
{
    EXPECT_TRUE(saveXml(score, saveName));
    return ScoreComp::compareFiles(saveName,  ScoreRW::rootPath() + u"/" + compareWithLocalPath);
}

//---------------------------------------------------------
//   mxmlIoTest
//   read a MusicXML file, write to a new file and verify both files are identical
//---------------------------------------------------------

void Musicxml_Tests::mxmlIoTest(const char* file, bool exportLayout)
{
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicxmlExportBreaksType::Manual));
    setValue(PREF_IMPORT_MUSICXML_IMPORTBREAKS, Val(true));
    setValue(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, Val(exportLayout));
    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".xml");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();
    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u".xml"));
    delete score;
}

//---------------------------------------------------------
//   mxmlIoTestRef
//   read a MusicXML file, write to a new file and verify against reference
//---------------------------------------------------------

void Musicxml_Tests::mxmlIoTestRef(const char* file)
{
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicxmlExportBreaksType::Manual));
    setValue(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, Val(false));
    setValue(PREF_IMPORT_MUSICXML_IMPORTBREAKS, Val(true));
    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".xml");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();
    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_ref.xml"));
    delete score;
}

//---------------------------------------------------------
//   mxmlIoTestRefBreaks
//   read a MusicXML file, write to a new file and verify against reference
//   using all possible settings for PREF_EXPORT_MUSICXML_EXPORTBREAKS
//---------------------------------------------------------

void Musicxml_Tests::mxmlIoTestRefBreaks(const char* file)
{
    MScore::debugMode = true;

    setValue(PREF_IMPORT_MUSICXML_IMPORTBREAKS, Val(true));
    setValue(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, Val(false));
    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".xml");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicxmlExportBreaksType::No));

    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_no_ref.xml"));

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicxmlExportBreaksType::Manual));

    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_manual_ref.xml"));

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicxmlExportBreaksType::All));

    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_all_ref.xml"));
    delete score;
}

//---------------------------------------------------------
//   mxmlMscxExportTestRef
//   read a MuseScore mscx file, write to a MusicXML file and verify against reference
//---------------------------------------------------------

void Musicxml_Tests::mxmlMscxExportTestRef(const char* file, bool exportLayout)
{
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicxmlExportBreaksType::Manual));
    setValue(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, Val(exportLayout));
    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".mscx");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();

    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_ref.xml"));
    delete score;
}

//---------------------------------------------------------
//   mxmlMscxExportTestRefBreaks
//   read a MuseScore mscx file, write to a MusicXML file and verify against reference
//   using all possible settings for PREF_EXPORT_MUSICXML_EXPORTBREAKS
//---------------------------------------------------------

void Musicxml_Tests::mxmlMscxExportTestRefBreaks(const char* file)
{
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicxmlExportBreaksType::Manual));
    setValue(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, Val(false));
    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".mscx");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicxmlExportBreaksType::No));

    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_no_ref.xml"));

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicxmlExportBreaksType::Manual));

    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_manual_ref.xml"));

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicxmlExportBreaksType::All));

    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u".xml", XML_IO_DATA_DIR + fileName + u"_all_ref.xml"));
    delete score;
}

void Musicxml_Tests::mxmlMscxExportTestRefInvisibleElements(const char* file)
{
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicxmlExportBreaksType::Manual));
    setValue(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, Val(false));

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
//   mxmlReadTestCompr
//   read a compressed MusicXML file, write to a new file and verify against reference
//---------------------------------------------------------

void Musicxml_Tests::mxmlReadTestCompr(const char* file)
{
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicxmlExportBreaksType::Manual));
    setValue(PREF_IMPORT_MUSICXML_IMPORTBREAKS, Val(true));
    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".mxl");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();
    EXPECT_TRUE(saveCompareMusicXmlScore(score, fileName + u"_mxl_read.xml", XML_IO_DATA_DIR + fileName + u".xml"));
    delete score;
}

//---------------------------------------------------------
//   mxmlReadWriteTestCompr
//   read a MusicXML file, write to a compressed MusicXML file,
//   read the compressed MusicXML file, write to a new file and verify files are identical
//---------------------------------------------------------

void Musicxml_Tests::mxmlReadWriteTestCompr(const char* file)
{
    // read xml
    MScore::debugMode = true;

    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicxmlExportBreaksType::Manual));
    setValue(PREF_IMPORT_MUSICXML_IMPORTBREAKS, Val(true));
    setValue(PREF_EXPORT_MUSICXML_EXPORTINVISIBLE, Val(true));

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
//   mxmlImportTestRef
//   read a MusicXML file, write to a new MuseScore mscx file
//   and verify against a MuseScore mscx reference file
//---------------------------------------------------------

void Musicxml_Tests::mxmlImportTestRef(const char* file)
{
    MScore::debugMode = false;
    setValue(PREF_EXPORT_MUSICXML_EXPORTBREAKS, Val(IMusicXmlConfiguration::MusicxmlExportBreaksType::Manual));
    setValue(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, Val(false));

    String fileName = String::fromUtf8(file);
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName + u".xml");
    EXPECT_TRUE(score);
    fixupScore(score);
    score->doLayout();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, fileName + u".mscx", XML_IO_DATA_DIR + fileName + u"_ref.mscx"));
    delete score;
}

TEST_F(Musicxml_Tests, accidentals1) {
    mxmlIoTest("testAccidentals1");
}
TEST_F(Musicxml_Tests, accidentals2) {
    mxmlIoTest("testAccidentals2");
}
TEST_F(Musicxml_Tests, accidentals3) {
    mxmlIoTest("testAccidentals3");
}
TEST_F(Musicxml_Tests, arpGliss1) {
    mxmlIoTest("testArpGliss1");
}
TEST_F(Musicxml_Tests, arpGliss2) {
    mxmlIoTest("testArpGliss2");
}
TEST_F(Musicxml_Tests, arpGliss3) {
    mxmlIoTest("testArpGliss3");
}
TEST_F(Musicxml_Tests, articulationCombination) {
    mxmlIoTestRef("testArticulationCombination");
}
TEST_F(Musicxml_Tests, backupRoundingError) {
    mxmlImportTestRef("testBackupRoundingError");
}
TEST_F(Musicxml_Tests, barlineFermatas) {
    mxmlIoTest("testBarlineFermatas");
}
TEST_F(Musicxml_Tests, barStyles) {
    mxmlIoTest("testBarStyles");
}
TEST_F(Musicxml_Tests, barStyles2) {
    mxmlIoTest("testBarStyles2");
}
TEST_F(Musicxml_Tests, barStyles3) {
    mxmlIoTest("testBarStyles3");
}
TEST_F(Musicxml_Tests, barStyles4) {
    mxmlIoTest("testBarStyles4");
}
TEST_F(Musicxml_Tests, bracketTypes) {
    mxmlImportTestRef("testBracketTypes");
}
TEST_F(Musicxml_Tests, DISABLED_beamEnd) {
    mxmlIoTest("testBeamEnd");
}
TEST_F(Musicxml_Tests, beamModes) {
    mxmlImportTestRef("testBeamModes");
}
TEST_F(Musicxml_Tests, beams1) {
    mxmlIoTest("testBeams1");
}
TEST_F(Musicxml_Tests, beams2) {
    mxmlIoTest("testBeams2");
}
TEST_F(Musicxml_Tests, beams3) {
    mxmlIoTestRef("testBeams3");
}
TEST_F(Musicxml_Tests, breaksImplExpl) {
    mxmlMscxExportTestRefBreaks("testBreaksImplExpl");
}
TEST_F(Musicxml_Tests, breaksMMRest) {
    mxmlMscxExportTestRefBreaks("testBreaksMMRest");
}
TEST_F(Musicxml_Tests, DISABLED_breaksManual) { // fail after sync with 3.x
    mxmlIoTestRefBreaks("testBreaksManual");
}
TEST_F(Musicxml_Tests, DISABLED_breaksPage) { // fail after sync with 3.x
    mxmlMscxExportTestRefBreaks("testBreaksPage");
}
TEST_F(Musicxml_Tests, breaksSystem) {
    mxmlMscxExportTestRefBreaks("testBreaksSystem");
}
TEST_F(Musicxml_Tests, breathMarks) {
    mxmlIoTest("testBreathMarks");
}
TEST_F(Musicxml_Tests, changeTranspose) {
    mxmlIoTest("testChangeTranspose");
}
TEST_F(Musicxml_Tests, changeTransposeNoDiatonic) {
    mxmlIoTestRef("testChangeTranspose-no-diatonic");
}
TEST_F(Musicxml_Tests, chordDiagrams1) {
    mxmlIoTest("testChordDiagrams1");
}
TEST_F(Musicxml_Tests, chordNoVoice) {
    mxmlIoTestRef("testChordNoVoice");
}
TEST_F(Musicxml_Tests, chordSymbols) {
    mxmlMscxExportTestRef("testChordSymbols");
}
TEST_F(Musicxml_Tests, clefs1) {
    mxmlIoTest("testClefs1");
}
TEST_F(Musicxml_Tests, clefs2) {
    mxmlIoTest("testClefs2");
}
TEST_F(Musicxml_Tests, colorExport) {
    mxmlMscxExportTestRef("testColorExport");
}
TEST_F(Musicxml_Tests, colors) {
    mxmlIoTest("testColors");
}
TEST_F(Musicxml_Tests, completeMeasureRests) {
    mxmlIoTest("testCompleteMeasureRests");
}
TEST_F(Musicxml_Tests, cueGraceNotes1) {
    mxmlImportTestRef("testCueGraceNotes");
}
TEST_F(Musicxml_Tests, cueGraceNotes2) {
    mxmlIoTestRef("testCueGraceNotes");
}
TEST_F(Musicxml_Tests, cueNotes) {
    mxmlIoTest("testCueNotes");
}
TEST_F(Musicxml_Tests, cueNotes2) {
    mxmlMscxExportTestRef("testCueNotes2");
}
TEST_F(Musicxml_Tests, cueNotes3) {
    mxmlImportTestRef("testCueNotes3");
}
TEST_F(Musicxml_Tests, dalSegno) {
    mxmlIoTest("testDalSegno");
}
TEST_F(Musicxml_Tests, dcalCoda) {
    mxmlIoTest("testDCalCoda");
}
TEST_F(Musicxml_Tests, dcalFine) {
    mxmlIoTest("testDCalFine");
}
TEST_F(Musicxml_Tests, directions1) {
    mxmlIoTestRef("testDirections1");
}
TEST_F(Musicxml_Tests, directions2) {
    mxmlIoTest("testDirections2");
}
TEST_F(Musicxml_Tests, divisionsDefinedTooLate1) {
    mxmlIoTestRef("testDivsDefinedTooLate1");
}
TEST_F(Musicxml_Tests, divisionsDefinedTooLate2) {
    mxmlIoTestRef("testDivsDefinedTooLate2");
}
TEST_F(Musicxml_Tests, doubleClefError) {
    mxmlIoTestRef("testDoubleClefError");
}
TEST_F(Musicxml_Tests, drumset1) {
    mxmlIoTest("testDrumset1");
}
TEST_F(Musicxml_Tests, drumset2) {
    mxmlIoTest("testDrumset2");
}
TEST_F(Musicxml_Tests, dsalCoda) {
    mxmlImportTestRef("testDSalCoda");
}
TEST_F(Musicxml_Tests, dsalCodaMisplaced) {
    mxmlImportTestRef("testDSalCodaMisplaced");
}
TEST_F(Musicxml_Tests, durationLargeErrorMscx) {
    mxmlImportTestRef("testDurationLargeError");
}
TEST_F(Musicxml_Tests, durationLargeErrorXml) {
    mxmlIoTestRef("testDurationLargeError");
}
TEST_F(Musicxml_Tests, durationRoundingErrorMscx) {
    mxmlImportTestRef("testDurationRoundingError");
}
TEST_F(Musicxml_Tests, durationRoundingErrorXml) {
    mxmlIoTestRef("testDurationRoundingError");
}
TEST_F(Musicxml_Tests, dynamics1) {
    mxmlIoTest("testDynamics1");
}
TEST_F(Musicxml_Tests, dynamics2) {
    mxmlIoTest("testDynamics2");
}
TEST_F(Musicxml_Tests, dynamics3) {
    mxmlIoTestRef("testDynamics3");
}
TEST_F(Musicxml_Tests, emptyMeasure) {
    mxmlIoTestRef("testEmptyMeasure");
}
TEST_F(Musicxml_Tests, emptyVoice1) {
    mxmlIoTestRef("testEmptyVoice1");
}
TEST_F(Musicxml_Tests, excludeInvisibleElements) {
    mxmlMscxExportTestRefInvisibleElements("testExcludeInvisibleElements");
}
TEST_F(Musicxml_Tests, excessHiddenStaves) {
    mxmlImportTestRef("testExcessHiddenStaves");
}
TEST_F(Musicxml_Tests, extendedLyrics) {
    mxmlIoTestRef("testExtendedLyrics");
}
TEST_F(Musicxml_Tests, figuredBass1) {
    mxmlIoTest("testFiguredBass1");
}
TEST_F(Musicxml_Tests, figuredBass2) {
    mxmlIoTest("testFiguredBass2");
}
TEST_F(Musicxml_Tests, figuredBass3) {
    mxmlIoTest("testFiguredBass3");
}
TEST_F(Musicxml_Tests, figuredBassDivisions) {
    mxmlIoTest("testFiguredBassDivisions");
}
TEST_F(Musicxml_Tests, formattedThings) {
    mxmlIoTest("testFormattedThings");
}
TEST_F(Musicxml_Tests, fractionMinus) {
    mxmlIoTestRef("testFractionMinus");
}
TEST_F(Musicxml_Tests, fractionPlus) {
    mxmlIoTestRef("testFractionPlus");
}
TEST_F(Musicxml_Tests, fractionTicks) {
    mxmlIoTestRef("testFractionTicks");
}
TEST_F(Musicxml_Tests, grace1) {
    mxmlIoTest("testGrace1");
}
TEST_F(Musicxml_Tests, grace2) {
    mxmlIoTest("testGrace2");
}
TEST_F(Musicxml_Tests, grace3) {
    mxmlIoTest("testGrace3");
}
TEST_F(Musicxml_Tests, graceAfter1) {
    mxmlIoTest("testGraceAfter1");
}
TEST_F(Musicxml_Tests, graceAfter2) {
    mxmlIoTest("testGraceAfter2");
}
TEST_F(Musicxml_Tests, graceAfter3) {
    mxmlIoTest("testGraceAfter3");
}
TEST_F(Musicxml_Tests, DISABLED_graceAfter4) {
    mxmlIoTest("testGraceAfter4");
}
TEST_F(Musicxml_Tests, graceFermata) {
    mxmlIoTest("testGraceFermata");
}
TEST_F(Musicxml_Tests, hairpinDynamics) {
    mxmlMscxExportTestRef("testHairpinDynamics");
}
TEST_F(Musicxml_Tests, harmony1) {
    mxmlIoTest("testHarmony1");
}
TEST_F(Musicxml_Tests, harmony2) {
    mxmlIoTest("testHarmony2");
}
TEST_F(Musicxml_Tests, harmony3) {
    mxmlIoTest("testHarmony3");
}
TEST_F(Musicxml_Tests, harmony4) {
    mxmlIoTest("testHarmony4");
}
TEST_F(Musicxml_Tests, harmony5) {
    mxmlIoTest("testHarmony5");
}                                                                      // chordnames without chordrest
TEST_F(Musicxml_Tests, harmony6) {
    mxmlMscxExportTestRef("testHarmony6");
}
TEST_F(Musicxml_Tests, hello) {
    mxmlIoTest("testHello");
}
TEST_F(Musicxml_Tests, helloReadCompr) {
    mxmlReadTestCompr("testHello");
}
TEST_F(Musicxml_Tests, helloReadWriteCompr) {
    mxmlReadWriteTestCompr("testHello");
}
TEST_F(Musicxml_Tests, implicitMeasure1) {
    mxmlIoTest("testImplicitMeasure1");
}
TEST_F(Musicxml_Tests, incompleteTuplet) {
    mxmlIoTestRef("testIncompleteTuplet");
}
TEST_F(Musicxml_Tests, incorrectMidiProgram) {
    mxmlIoTestRef("testIncorrectMidiProgram");
}
TEST_F(Musicxml_Tests, incorrectStaffNumber1) {
    mxmlIoTestRef("testIncorrectStaffNumber1");
}
TEST_F(Musicxml_Tests, incorrectStaffNumber2) {
    mxmlIoTestRef("testIncorrectStaffNumber2");
}
TEST_F(Musicxml_Tests, inferredSubtitle) {
    mxmlImportTestRef("testInferredSubtitle");
}
TEST_F(Musicxml_Tests, instrumentChangeMIDIportExport) {
    mxmlMscxExportTestRef("testInstrumentChangeMIDIportExport");
}
TEST_F(Musicxml_Tests, instrumentSound) {
    mxmlIoTestRef("testInstrumentSound");
}
TEST_F(Musicxml_Tests, invalidLayout) {
    mxmlMscxExportTestRef("testInvalidLayout");
}
TEST_F(Musicxml_Tests, invalidTimesig) {
    mxmlIoTestRef("testInvalidTimesig");
}
TEST_F(Musicxml_Tests, invisibleElements) {
    mxmlIoTest("testInvisibleElements");
}
TEST_F(Musicxml_Tests, keysig1) {
    mxmlIoTest("testKeysig1");
}
TEST_F(Musicxml_Tests, keysig2) {
    mxmlIoTest("testKeysig2");
}
TEST_F(Musicxml_Tests, layout) {
    mxmlIoTest("testLayout", true);
}
TEST_F(Musicxml_Tests, lessWhiteSpace) {
    mxmlIoTestRef("testLessWhiteSpace");
}
TEST_F(Musicxml_Tests, lines1) {
    mxmlIoTest("testLines1");
}
TEST_F(Musicxml_Tests, lines2) {
    mxmlIoTest("testLines2");
}
TEST_F(Musicxml_Tests, lines3) {
    mxmlIoTest("testLines3");
}
TEST_F(Musicxml_Tests, lines4) {
    mxmlMscxExportTestRef("testLines4");
}
TEST_F(Musicxml_Tests, lyricBracket) {
    mxmlImportTestRef("testLyricBracket");
}
TEST_F(Musicxml_Tests, lyricColor) {
    mxmlIoTest("testLyricColor");
}
TEST_F(Musicxml_Tests, lyrics1) {
    mxmlIoTestRef("testLyrics1");
}
TEST_F(Musicxml_Tests, lyricExtension1) {
    mxmlIoTest("testLyricExtensions");
}
TEST_F(Musicxml_Tests, lyricExtension2) {
    mxmlImportTestRef("testLyricExtensions");
}
TEST_F(Musicxml_Tests, lyricsVoice2a) {
    mxmlIoTest("testLyricsVoice2a");
}
TEST_F(Musicxml_Tests, lyricsVoice2b) {
    mxmlIoTestRef("testLyricsVoice2b");
}
TEST_F(Musicxml_Tests, maxNumberLevel) {
    mxmlMscxExportTestRef("testMaxNumberLevel");
}
TEST_F(Musicxml_Tests, measureLength) {
    mxmlIoTestRef("testMeasureLength");
}
TEST_F(Musicxml_Tests, measureNumbers) {
    mxmlIoTest("testMeasureNumbers");
}
TEST_F(Musicxml_Tests, measureRepeats1) {
    mxmlIoTestRef("testMeasureRepeats1");
}
TEST_F(Musicxml_Tests, DISABLED_measureRepeats2) {
    mxmlIoTestRef("testMeasureRepeats2");
}
TEST_F(Musicxml_Tests, measureRepeats3) {
    mxmlIoTest("testMeasureRepeats3");
}
TEST_F(Musicxml_Tests, measureStyleSlash) {
    mxmlImportTestRef("testMeasureStyleSlash");
}
TEST_F(Musicxml_Tests, midiPortExport) {
    mxmlMscxExportTestRef("testMidiPortExport");
}
TEST_F(Musicxml_Tests, multiInstrumentPart1) {
    mxmlIoTest("testMultiInstrumentPart1");
}
TEST_F(Musicxml_Tests, multiInstrumentPart2) {
    mxmlIoTest("testMultiInstrumentPart2");
}
TEST_F(Musicxml_Tests, multiInstrumentPart3) {
    mxmlMscxExportTestRef("testMultiInstrumentPart3");
}
TEST_F(Musicxml_Tests, multiMeasureRest1) {
    mxmlIoTestRef("testMultiMeasureRest1");
}
TEST_F(Musicxml_Tests, multiMeasureRest2) {
    mxmlIoTestRef("testMultiMeasureRest2");
}
TEST_F(Musicxml_Tests, multiMeasureRest3) {
    mxmlIoTestRef("testMultiMeasureRest3");
}
TEST_F(Musicxml_Tests, multiMeasureRest4) {
    mxmlIoTestRef("testMultiMeasureRest4");
}
TEST_F(Musicxml_Tests, multipleNotations) {
    mxmlIoTestRef("testMultipleNotations");
}
TEST_F(Musicxml_Tests, negativeOffset) {
    mxmlImportTestRef("testNegativeOffset");
}
TEST_F(Musicxml_Tests, negativeOctave) {
    mxmlMscxExportTestRef("testNegativeOctave");
}
TEST_F(Musicxml_Tests, nonStandardKeySig1) {
    mxmlIoTest("testNonStandardKeySig1");
}
TEST_F(Musicxml_Tests, nonStandardKeySig2) {
    mxmlIoTest("testNonStandardKeySig2");
}
TEST_F(Musicxml_Tests, nonStandardKeySig3) {
    mxmlIoTest("testNonStandardKeySig3");
}
TEST_F(Musicxml_Tests, nonUniqueThings) {
    mxmlIoTestRef("testNonUniqueThings");
}
TEST_F(Musicxml_Tests, noteAttributes1) {
    mxmlIoTest("testNoteAttributes1");
}
TEST_F(Musicxml_Tests, noteAttributes2) {
    mxmlIoTestRef("testNoteAttributes2");
}
TEST_F(Musicxml_Tests, noteAttributes3) {
    mxmlIoTest("testNoteAttributes3");
}
TEST_F(Musicxml_Tests, noteColor) {
    mxmlIoTest("testNoteColor");
}
TEST_F(Musicxml_Tests, noteheadParentheses) {
    mxmlIoTest("testNoteheadParentheses");
}
TEST_F(Musicxml_Tests, noteheads) {
    mxmlIoTest("testNoteheads");
}
TEST_F(Musicxml_Tests, noteheads2) {
    mxmlMscxExportTestRef("testNoteheads2");
}
TEST_F(Musicxml_Tests, noteheadsFilled) {
    mxmlIoTest("testNoteheadsFilled");
}
TEST_F(Musicxml_Tests, notesRests1) {
    mxmlIoTest("testNotesRests1");
}
TEST_F(Musicxml_Tests, notesRests2) {
    mxmlIoTest("testNotesRests2");
}
TEST_F(Musicxml_Tests, numberedLyrics) {
    mxmlIoTestRef("testNumberedLyrics");
}
TEST_F(Musicxml_Tests, pedalChanges) {
    mxmlIoTest("testPedalChanges");
}
TEST_F(Musicxml_Tests, pedalChangesBroken) {
    mxmlImportTestRef("testPedalChangesBroken");
}
TEST_F(Musicxml_Tests, pedalStyles) {
    mxmlIoTest("testPedalStyles");
}
TEST_F(Musicxml_Tests, placementDefaults) {
    mxmlImportTestRef("testPlacementDefaults");
}
TEST_F(Musicxml_Tests, overlappingSpanners) {
    mxmlIoTest("testOverlappingSpanners");
}
TEST_F(Musicxml_Tests, partNames) {
    mxmlImportTestRef("testPartNames");
}
TEST_F(Musicxml_Tests, printSpacingNo) {
    mxmlIoTestRef("testPrintSpacingNo");
}
TEST_F(Musicxml_Tests, repeatCounts) {
    mxmlIoTest("testRepeatCounts");
}
TEST_F(Musicxml_Tests, repeatSingleMeasure) {
    mxmlIoTest("testRepeatSingleMeasure");
}
TEST_F(Musicxml_Tests, restNotations) {
    mxmlIoTestRef("testRestNotations");
}
TEST_F(Musicxml_Tests, restsNoType) {
    mxmlIoTestRef("testRestsNoType");
}
TEST_F(Musicxml_Tests, restsTypeWhole) {
    mxmlIoTestRef("testRestsTypeWhole");
}
TEST_F(Musicxml_Tests, secondVoiceMelismata) {
    mxmlImportTestRef("testSecondVoiceMelismata");
}
TEST_F(Musicxml_Tests, slurTieDirection) {
    mxmlIoTest("testSlurTieDirection");
}
TEST_F(Musicxml_Tests, slurTieLineStyle) {
    mxmlIoTest("testSlurTieLineStyle");
}
TEST_F(Musicxml_Tests, slurs) {
    mxmlIoTest("testSlurs");
}
TEST_F(Musicxml_Tests, slurs2) {
    mxmlIoTest("testSlurs2");
}
TEST_F(Musicxml_Tests, sound1) {
    mxmlIoTestRef("testSound1");
}
TEST_F(Musicxml_Tests, sound2) {
    mxmlIoTestRef("testSound2");
}
TEST_F(Musicxml_Tests, specialCharacters) {
    mxmlIoTest("testSpecialCharacters");
}
TEST_F(Musicxml_Tests, testStaffEmptiness) {
    mxmlImportTestRef("testStaffEmptiness");
}
TEST_F(Musicxml_Tests, staffTwoKeySigs) {
    mxmlIoTest("testStaffTwoKeySigs");
}
TEST_F(Musicxml_Tests, stringData) {
    mxmlIoTest("testStringData");
}
TEST_F(Musicxml_Tests, stringVoiceName) {
    mxmlIoTestRef("testStringVoiceName");
}
TEST_F(Musicxml_Tests, systemBrackets1) {
    mxmlIoTest("testSystemBrackets1");
}
TEST_F(Musicxml_Tests, systemBrackets2) {
    mxmlIoTest("testSystemBrackets2");
}
TEST_F(Musicxml_Tests, systemBrackets3) {
    mxmlImportTestRef("testSystemBrackets3");
}
TEST_F(Musicxml_Tests, systemBrackets4) {
    mxmlIoTest("testSystemBrackets1");
}
TEST_F(Musicxml_Tests, systemBrackets5) {
    mxmlIoTest("testSystemBrackets1");
}
TEST_F(Musicxml_Tests, systemDistance) {
    mxmlMscxExportTestRef("testSystemDistance", true);
}
TEST_F(Musicxml_Tests, systemDividers) {
    mxmlIoTest("testSystemDividers", true);
}
TEST_F(Musicxml_Tests, tablature1) {
    mxmlIoTest("testTablature1");
}
TEST_F(Musicxml_Tests, tablature2) {
    mxmlIoTest("testTablature2");
}
TEST_F(Musicxml_Tests, tablature3) {
    mxmlIoTest("testTablature3");
}
TEST_F(Musicxml_Tests, tablature4) {
    mxmlIoTest("testTablature4");
}
TEST_F(Musicxml_Tests, tablature5) {
    mxmlIoTestRef("testTablature5");
}
TEST_F(Musicxml_Tests, tboxAboveBelow1) {
    mxmlMscxExportTestRef("testTboxAboveBelow1");
}
TEST_F(Musicxml_Tests, tboxAboveBelow2) {
    mxmlMscxExportTestRef("testTboxAboveBelow2");
}
TEST_F(Musicxml_Tests, tboxAboveBelow3) {
    mxmlMscxExportTestRef("testTboxAboveBelow3");
}
TEST_F(Musicxml_Tests, tboxMultiPage1) {
    mxmlMscxExportTestRef("testTboxMultiPage1");
}
TEST_F(Musicxml_Tests, tboxVbox1) {
    mxmlMscxExportTestRef("testTboxVbox1");
}
TEST_F(Musicxml_Tests, tboxWords1) {
    mxmlMscxExportTestRef("testTboxWords1");
}
TEST_F(Musicxml_Tests, tempo1) {
    mxmlIoTest("testTempo1");
}
TEST_F(Musicxml_Tests, tempo2) {
    mxmlIoTestRef("testTempo2");
}
TEST_F(Musicxml_Tests, tempo3) {
    mxmlIoTestRef("testTempo3");
}
TEST_F(Musicxml_Tests, tempo4) {
    mxmlIoTestRef("testTempo4");
}
TEST_F(Musicxml_Tests, tempo5) {
    mxmlIoTest("testTempo5");
}
TEST_F(Musicxml_Tests, tempo6) {
    mxmlIoTest("testTempo6");
}
TEST_F(Musicxml_Tests, tempoOverlap) {
    mxmlIoTestRef("testTempoOverlap");
}
TEST_F(Musicxml_Tests, tempoPrecision) {
    mxmlMscxExportTestRef("testTempoPrecision");
}
TEST_F(Musicxml_Tests, tempoTextSpace1) {
    mxmlImportTestRef("testTempoTextSpace1");
}
TEST_F(Musicxml_Tests, tempoTextSpace2) {
    mxmlImportTestRef("testTempoTextSpace2");
}
TEST_F(Musicxml_Tests, textLines) {
    mxmlMscxExportTestRef("testTextLines");
}
TEST_F(Musicxml_Tests, testTextOrder) {
    mxmlImportTestRef("testTextOrder");
}
TEST_F(Musicxml_Tests, textQuirkInference) {
    mxmlImportTestRef("testTextQuirkInference");
}
TEST_F(Musicxml_Tests, tieTied) {
    mxmlIoTestRef("testTieTied");
}
TEST_F(Musicxml_Tests, timesig1) {
    mxmlIoTest("testTimesig1");
}
TEST_F(Musicxml_Tests, timesig3) {
    mxmlIoTest("testTimesig3");
}
TEST_F(Musicxml_Tests, trackHandling) {
    mxmlIoTest("testTrackHandling");
}
TEST_F(Musicxml_Tests, tremolo) {
    mxmlIoTest("testTremolo");
}
TEST_F(Musicxml_Tests, tuplets1) {
    mxmlIoTestRef("testTuplets1");
}
TEST_F(Musicxml_Tests, tuplets2) {
    mxmlIoTestRef("testTuplets2");
}
TEST_F(Musicxml_Tests, tuplets3) {
    mxmlIoTestRef("testTuplets3");
}
TEST_F(Musicxml_Tests, tuplets4) {
    mxmlIoTest("testTuplets4");
}
TEST_F(Musicxml_Tests, tuplets5) {
    mxmlIoTestRef("testTuplets5");
}
TEST_F(Musicxml_Tests, tuplets6) {
    mxmlIoTestRef("testTuplets6");
}
TEST_F(Musicxml_Tests, tuplets7) {
    mxmlIoTest("testTuplets7");
}
TEST_F(Musicxml_Tests, tuplets8) {
    mxmlMscxExportTestRef("testTuplets8");
}
TEST_F(Musicxml_Tests, tuplets9) {
    mxmlIoTest("testTuplets9");
}
TEST_F(Musicxml_Tests, twoNoteTremoloTuplet) {
    mxmlIoTest("testTwoNoteTremoloTuplet");
}
TEST_F(Musicxml_Tests, uninitializedDivisions) {
    mxmlIoTestRef("testUninitializedDivisions");
}
TEST_F(Musicxml_Tests, unnecessaryBarlines) {
    mxmlImportTestRef("testUnnecessaryBarlines");
}
TEST_F(Musicxml_Tests, unusualDurations) {
    mxmlIoTestRef("testUnusualDurations");
}
TEST_F(Musicxml_Tests, virtualInstruments) {
    mxmlIoTestRef("testVirtualInstruments");
}
TEST_F(Musicxml_Tests, voiceMapper1) {
    mxmlIoTestRef("testVoiceMapper1");
}
TEST_F(Musicxml_Tests, voiceMapper2) {
    mxmlIoTestRef("testVoiceMapper2");
}
TEST_F(Musicxml_Tests, voiceMapper3) {
    mxmlIoTestRef("testVoiceMapper3");
}
TEST_F(Musicxml_Tests, voicePiano1) {
    mxmlIoTest("testVoicePiano1");
}
TEST_F(Musicxml_Tests, volta1) {
    mxmlIoTest("testVolta1");
}
TEST_F(Musicxml_Tests, volta2) {
    mxmlIoTest("testVolta2");
}
TEST_F(Musicxml_Tests, voltaHiding1) {
    mxmlImportTestRef("testVoltaHiding");
}
TEST_F(Musicxml_Tests, voltaHiding2) {
    mxmlIoTestRef("testVoltaHiding");
}
TEST_F(Musicxml_Tests, wedgeOffset) {
    mxmlImportTestRef("testWedgeOffset");
}
TEST_F(Musicxml_Tests, wedge1) {
    mxmlIoTest("testWedge1");
}
TEST_F(Musicxml_Tests, wedge2) {
    mxmlIoTest("testWedge2");
}
TEST_F(Musicxml_Tests, wedge3) {
    mxmlIoTest("testWedge3");
}
TEST_F(Musicxml_Tests, wedge4) {
    mxmlMscxExportTestRef("testWedge4");
}
TEST_F(Musicxml_Tests, wedge5) {
    mxmlIoTestRef("testWedge5");
}
TEST_F(Musicxml_Tests, words1) {
    mxmlIoTest("testWords1");
}
TEST_F(Musicxml_Tests, words2) {
    mxmlIoTest("testWords2");
}
TEST_F(Musicxml_Tests, hiddenStaves)
{
    String fileName = String::fromUtf8("testHiddenStaves.xml");
    MasterScore* score = readScore(XML_IO_DATA_DIR + fileName);

    EXPECT_EQ(score->style().value(Sid::hideEmptyStaves).toBool(), true);
}
