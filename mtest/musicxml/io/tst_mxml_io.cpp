//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012-2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "libmscore/score.h"
#include "mscore/preferences.h"
// start includes required for fixupScore()
#include "libmscore/measure.h"
#include "libmscore/staff.h"
// end includes required for fixupScore()

namespace Ms {
extern bool saveMxl(Score*, const QString&);
}

#define DIR QString("musicxml/io/")

using namespace Ms;

//---------------------------------------------------------
//   TestMxmlIO
//---------------------------------------------------------

class TestMxmlIO : public QObject, public MTest
      {
      Q_OBJECT

      void mxmlIoTest(const char* file, bool exportLayout = false);
      void mxmlIoTestRef(const char* file);
      void mxmlIoTestRefBreaks(const char* file);
      void mxmlMscxExportTestRef(const char* file, bool exportLayout = false);
      void mxmlMscxExportTestRefBreaks(const char* file);
      void mxmlReadTestCompr(const char* file);
      void mxmlReadWriteTestCompr(const char* file);
      void mxmlImportTestRef(const char* file);


      // The list of MusicXML regression tests
      // Currently failing tests are commented out and annotated with the failure reason
      // To extract the list in a shell script use:
      // cat tst_mxml_io.cpp | grep "{ <test>" | awk -F\" '{print $2}'
      // where <test> is mxmlIoTest or mxmlIoTestRef

private slots:
      void initTestCase();

      void accidentals1() { mxmlIoTest("testAccidentals1"); }
      void accidentals2() { mxmlIoTest("testAccidentals2"); }
      void accidentals3() { mxmlIoTest("testAccidentals3"); }
      void additionalFermatas() { mxmlImportTestRef("testAdditionalFermatas"); }
      void arpGliss1() { mxmlIoTest("testArpGliss1"); }
      void arpGliss2() { mxmlIoTest("testArpGliss2"); }
      void arpGliss3() { mxmlIoTest("testArpGliss3"); }
      void articulationCombination() { mxmlIoTestRef("testArticulationCombination"); }
      void backupRoundingError() { mxmlImportTestRef("testBackupRoundingError"); }
      void barlineFermatas() { mxmlIoTest("testBarlineFermatas"); }
      void barlineLoc() { mxmlImportTestRef("testBarlineLoc"); }
      void barlineSpan() { mxmlIoTest("testBarlineSpan"); }
      void barlinesGrandStaff1() { mxmlImportTestRef("testBarlinesGrandStaff"); }
      void barlinesGrandStaff2() { mxmlIoTest("testBarlinesGrandStaff"); }
      void barStyles() { mxmlIoTest("testBarStyles"); }
      void barStyles2() { mxmlIoTest("testBarStyles2"); }
      void barStyles3() { mxmlIoTest("testBarStyles3"); }
      void barStyles4() { mxmlIoTest("testBarStyles4"); }
      void beamEnd() { mxmlIoTest("testBeamEnd"); }
      void beamModes() { mxmlImportTestRef("testBeamModes"); }
      void beams1() { mxmlIoTest("testBeams1"); }
      void beams2() { mxmlIoTest("testBeams2"); }
      void beams3() { mxmlIoTestRef("testBeams3"); }
      void bracketTypes() { mxmlImportTestRef("testBracketTypes"); }
      void breaksImplExpl() { mxmlMscxExportTestRefBreaks("testBreaksImplExpl"); }
      void breaksMMRest() { mxmlMscxExportTestRefBreaks("testBreaksMMRest"); }
      void breaksManual() { mxmlIoTestRefBreaks("testBreaksManual"); }
      void breaksPage() { mxmlMscxExportTestRefBreaks("testBreaksPage"); }
      void breaksSystem() { mxmlMscxExportTestRefBreaks("testBreaksSystem"); }
      void breathMarks() { mxmlIoTest("testBreathMarks"); }
      void buzzRoll() { mxmlImportTestRef("testBuzzRoll"); }
      void buzzRoll2() { mxmlIoTest("testBuzzRoll2"); }
      void changeTranspose() { mxmlIoTest("testChangeTranspose"); }
      void changeTransposeNoDiatonic() { mxmlIoTestRef("testChangeTranspose-no-diatonic"); }
      void chordDiagrams1() { mxmlIoTest("testChordDiagrams1"); }
      void chordNoVoice() { mxmlIoTestRef("testChordNoVoice"); }
      void chordSymbols() { mxmlMscxExportTestRef("testChordSymbols"); }
      void chordSymbols2() { mxmlImportTestRef("testChordSymbols2"); }
      void clefs1() { mxmlIoTest("testClefs1"); }
      void clefs2() { mxmlIoTest("testClefs2"); }
      void colorExport() { mxmlMscxExportTestRef("testColorExport"); }
      void colors() { mxmlIoTest("testColors"); }
      void completeMeasureRests() { mxmlIoTest("testCompleteMeasureRests"); }
      void copyrightScale() { mxmlImportTestRef("testCopyrightScale"); }
      void connectedArpeggios1() { mxmlImportTestRef("testConnectedArpeggios"); }
      void connectedArpeggios2() { mxmlIoTestRef("testConnectedArpeggios"); }
      void cueGraceNotes1() { mxmlImportTestRef("testCueGraceNotes"); }
      void cueGraceNotes2() { mxmlIoTestRef("testCueGraceNotes"); }
      void cueNotes() { mxmlIoTest("testCueNotes"); }
      void cueNotes2() { mxmlMscxExportTestRef("testCueNotes2"); }
      void cueNotes3() { mxmlImportTestRef("testCueNotes3"); }
      void dalSegno() { mxmlIoTest("testDalSegno"); }
      void dcalCoda() { mxmlIoTest("testDCalCoda"); }
      void dcalFine() { mxmlIoTest("testDCalFine"); }
      void directions1() { mxmlIoTestRef("testDirections1"); }
      void directions2() { mxmlIoTest("testDirections2"); }
      void displayStepOctave() {  mxmlMscxExportTestRef("testDisplayStepOctave"); }
      void divisionsDefinedTooLate1() { mxmlIoTestRef("testDivsDefinedTooLate1"); }
      void divisionsDefinedTooLate2() { mxmlIoTestRef("testDivsDefinedTooLate2"); }
      void divisionsDuration() { mxmlIoTest("testDivisionsDuration"); }
      void doletOttavas() { mxmlImportTestRef("testDoletOttavas"); }
      void doubleClefError() { mxmlIoTestRef("testDoubleClefError"); }
      void drumset1() { mxmlIoTest("testDrumset1"); }
      void drumset2() { mxmlIoTest("testDrumset2"); }
      void dsalCoda() { mxmlImportTestRef("testDSalCoda"); }
      void dsalCodaMisplaced() { mxmlImportTestRef("testDSalCodaMisplaced"); }
      void duplicateInstrChange() { mxmlImportTestRef("testDuplicateInstrChange"); }
      void durationLargeErrorMscx() { mxmlImportTestRef("testDurationLargeError"); }
      void durationLargeErrorXml() { mxmlIoTestRef("testDurationLargeError"); }
      void durationRoundingErrorMscx() { mxmlImportTestRef("testDurationRoundingError"); }
      void durationRoundingErrorXml() { mxmlIoTestRef("testDurationRoundingError"); }
      void dynamics1() { mxmlIoTest("testDynamics1"); }
      void dynamics2() { mxmlIoTest("testDynamics2"); }
      void dynamics3() { mxmlIoTestRef("testDynamics3"); }
      void elision() { mxmlImportTestRef("testElision"); }
      void emptyMeasure() { mxmlIoTestRef("testEmptyMeasure"); }
      void emptyVoice1() { mxmlIoTestRef("testEmptyVoice1"); }
      void excessHiddenStaves() { mxmlImportTestRef("testExcessHiddenStaves"); }
      void excessiveFretDiagrams1() { mxmlImportTestRef("testExcessiveFretDiagrams1"); }
      void excessiveFretDiagrams2() { mxmlImportTestRef("testExcessiveFretDiagrams2"); }
      void extendedLyrics() { mxmlIoTestRef("testExtendedLyrics"); }
      void figuredBass1() { mxmlIoTest("testFiguredBass1"); }
      void figuredBass2() { mxmlIoTest("testFiguredBass2"); }
      void figuredBass3() { mxmlIoTest("testFiguredBass3"); }
      void figuredBassDivisions() { mxmlIoTest("testFiguredBassDivisions"); }
      // void finaleInstr() { mxmlImportTestRef("testFinaleInstr"); } // TODO
      void finaleInstr2() { mxmlImportTestRef("testFinaleInstr2"); }
      // void formattedThings() { mxmlIoTest("testFormattedThings"); } // TODO
      void fractionMinus() { mxmlIoTestRef("testFractionMinus"); }
      void fractionPlus() { mxmlIoTestRef("testFractionPlus"); }
      void fractionTicks() { mxmlIoTestRef("testFractionTicks"); }
      void fretboardDiagrams() { mxmlImportTestRef("testFretboardDiagrams"); }
      void fretDiagramLayoutOrder() { mxmlImportTestRef("testFretDiagramLayoutOrder"); }
      void glissandoLines() { mxmlIoTest("testGlissandoLines"); }
      void grace1() { mxmlIoTest("testGrace1"); }
      void grace2() { mxmlIoTest("testGrace2"); }
      void grace3() { mxmlIoTest("testGrace3"); }
      void hairpinDynamics() { mxmlMscxExportTestRef("testHairpinDynamics"); }
      void handbells() { mxmlIoTest("testHandbells"); }
      void harmony1() { mxmlIoTest("testHarmony1"); }
      void harmony2() { mxmlIoTest("testHarmony2"); }
      void harmony3() { mxmlIoTest("testHarmony3"); }
      void harmony4() { mxmlIoTest("testHarmony4"); }
      void harmony5() { mxmlIoTest("testHarmony5"); } // chordnames without chordrest
      void harmony6() { mxmlMscxExportTestRef("testHarmony6"); }
      void harmony7() { mxmlMscxExportTestRef("testHarmony7"); }
      void harmony8() { mxmlIoTest("testHarmony8"); }
      void harmony9() { mxmlIoTest("testHarmony9"); }
      void harmonMutes() { mxmlIoTest("testHarmonMutes"); }
      void hello() { mxmlIoTest("testHello"); }
      void helloReadCompr() { mxmlReadTestCompr("testHello"); }
      void helloReadWriteCompr() { mxmlReadWriteTestCompr("testHello"); }
      void holes() { mxmlIoTest("testHoles"); }
      void implicitMeasure1() { mxmlIoTest("testImplicitMeasure1"); }
      void incompleteTuplet() { mxmlIoTestRef("testIncompleteTuplet"); }
      void incorrectStaffNumber1() { mxmlIoTestRef("testIncorrectStaffNumber1"); }
      void incorrectStaffNumber2() { mxmlIoTestRef("testIncorrectStaffNumber2"); }
      void inferredCredits1() { mxmlImportTestRef("testInferredCredits1"); }
      void inferredCredits2() { mxmlImportTestRef("testInferredCredits2"); }
      void inferredCrescLines() { mxmlImportTestRef("testInferredCrescLines"); }
      void inferredCrescLines2() { mxmlImportTestRef("testInferredCrescLines2"); }
      void inferredDynamics() { mxmlImportTestRef("testInferredDynamics"); }
      void inferredDynamicsExpressiont() { mxmlImportTestRef("testInferredDynamicsExpression"); }
      void inferreFractions() { mxmlImportTestRef("testInferFraction"); }
      void inferredFingerings() { mxmlImportTestRef("testInferredFingerings"); }
      void inferredRights() { mxmlImportTestRef("testInferredRights"); }
      void inferredTempoText() { mxmlImportTestRef("testInferredTempoText"); }
      void inferredTempoText2() { mxmlImportTestRef("testInferredTempoText2"); }
      void inferredTransposition() { mxmlImportTestRef("testInferredRights"); }
      void instrumentChangeMIDIportExport() { mxmlMscxExportTestRef("testInstrumentChangeMIDIportExport"); }
      void instrumentSound() { mxmlIoTestRef("testInstrumentSound"); }
      void invalidLayout() { mxmlMscxExportTestRef("testInvalidLayout"); }
      void invalidTimesig() { mxmlIoTestRef("testInvalidTimesig"); }
      void invisibleDirection() { mxmlIoTest("testInvisibleDirection"); }
      void invisibleElements() { mxmlIoTest("testInvisibleElements"); }
      void invisibleNote() { mxmlMscxExportTestRef("testInvisibleNote"); }
      void keysig1() { mxmlIoTest("testKeysig1"); }
      void keysig2() { mxmlIoTest("testKeysig2"); }
      void layoutCleanup1() { mxmlImportTestRef("testLayoutCleanup1"); }
      void layoutCleanup2() { mxmlImportTestRef("testLayoutCleanup2"); }
      void layout() { mxmlIoTest("testLayout", true); }
      void lessWhiteSpace() { mxmlIoTestRef("testLessWhiteSpace"); }
      void lines1() { mxmlIoTest("testLines1"); }
      void lines2() { mxmlIoTest("testLines2"); }
      void lines3() { mxmlIoTest("testLines3"); }
      void lines4() { mxmlIoTest("testLines4"); }
      void lineDetails() { mxmlMscxExportTestRef("testLineDetails"); }
      void localTimesig1() { mxmlMscxExportTestRef("testLocalTimesig1"); }
      void localTimesig2() { mxmlMscxExportTestRef("testLocalTimesig2"); }
      void localTimesig3() { mxmlMscxExportTestRef("testLocalTimesig3"); }
      void localTimesig4() { mxmlMscxExportTestRef("testLocalTimesig4"); }
      void localTimesig5() { mxmlMscxExportTestRef("testLocalTimesig5"); }
      void localTimesig6() { mxmlMscxExportTestRef("testLocalTimesig6"); }
      void localTimesig8() { mxmlMscxExportTestRef("testLocalTimesig8"); }
      void localTimesig9() { mxmlMscxExportTestRef("testLocalTimesig9"); }
      void localTimesig10() { mxmlMscxExportTestRef("testLocalTimesig10"); }
      void localTimesig11() { mxmlMscxExportTestRef("testLocalTimesig11"); }
      void localTimesig12() { mxmlMscxExportTestRef("testLocalTimesig12"); }
      //void localTimesig14() { mxmlMscxExportTestRef("testLocalTimesig14"); } TODO, needs #6365 backported to support measure-repeats
      //void localTimesig98() { mxmlMscxExportTestRef("testLocalTimesig98"); }
      //void localTimesig99() { mxmlMscxExportTestRef("testLocalTimesig99"); }
      void lyricBracket() { mxmlImportTestRef("testLyricBracket"); }
      void lyricColor() { mxmlIoTest("testLyricColor"); }
      void lyricExtensions1() { mxmlIoTest("testLyricExtensions"); }
      void lyricExtensions2() { mxmlImportTestRef("testLyricExtensions"); }
      void lyricExtensions3() { mxmlIoTest("testLyricExtension2"); }
      void lyricExtensions4() { mxmlImportTestRef("testLyricExtension2"); }
      void lyricPos() { mxmlImportTestRef("testLyricPos"); }
      void lyrics1() { mxmlIoTestRef("testLyrics1"); }
      void lyricsVoice2a() { mxmlIoTest("testLyricsVoice2a"); }
      void lyricsVoice2b() { mxmlIoTestRef("testLyricsVoice2b"); }
      void maxNumberLevel() { mxmlMscxExportTestRef("testMaxNumberLevel"); }
      void measureLength() { mxmlIoTestRef("testMeasureLength"); }
      void measureNumbers() { mxmlIoTest("testMeasureNumbers"); }
      void measureNumberOffset() { mxmlIoTest("testMeasureNumberOffset"); }
      void measureStyleSlash() { mxmlImportTestRef("testMeasureStyleSlash"); }
      void midiPortExport() { mxmlMscxExportTestRef("testMidiPortExport"); }
      void multiInstrumentPart1() { mxmlIoTest("testMultiInstrumentPart1"); }
      void multiInstrumentPart2() { mxmlIoTest("testMultiInstrumentPart2"); }
      void multiInstrumentPart3() { mxmlMscxExportTestRef("testMultiInstrumentPart3"); }
      void multiMeasureRest1() { mxmlIoTestRef("testMultiMeasureRest1"); }
      void multiMeasureRest2() { mxmlIoTestRef("testMultiMeasureRest2"); }
      void multiMeasureRest3() { mxmlIoTestRef("testMultiMeasureRest3"); }
      void multiMeasureRest4() { mxmlIoTestRef("testMultiMeasureRest4"); }
      void multipleNotations() { mxmlIoTestRef("testMultipleNotations"); }
      void namedNoteheads() { mxmlImportTestRef("testNamedNoteheads"); }
      void negativeOctave() { mxmlMscxExportTestRef("testNegativeOctave"); }
      void negativeOffset() { mxmlImportTestRef("testNegativeOffset"); }
      void nonStandardKeySig1() { mxmlIoTest("testNonStandardKeySig1"); }
      void nonStandardKeySig2() { mxmlIoTest("testNonStandardKeySig2"); }
      void nonStandardKeySig3() { mxmlIoTest("testNonStandardKeySig3"); }
      void nonUniqueThings() { mxmlIoTestRef("testNonUniqueThings"); }
      void noteAttributes1() { mxmlIoTest("testNoteAttributes1"); }
      void noteAttributes2() { mxmlIoTestRef("testNoteAttributes2"); }
      void noteAttributes3() { mxmlIoTest("testNoteAttributes3"); }
      void noteAttributes4() { mxmlImportTestRef("testNoteAttributes2"); }
      void noteColor() { mxmlIoTest("testNoteColor"); }
      void noteheadParentheses() { mxmlIoTest("testNoteheadParentheses"); }
      void noteheads() { mxmlIoTest("testNoteheads"); }
      void noteheads2() { mxmlMscxExportTestRef("testNoteheads2"); }
      void noteheadsFilled() { mxmlIoTest("testNoteheadsFilled"); }
      void noteTuning() { mxmlMscxExportTestRef("testNoteTuning"); }
      void notesRests1() { mxmlIoTest("testNotesRests1"); }
      void notesRests2() { mxmlIoTest("testNotesRests2"); }
      void numberedLyrics() { mxmlIoTestRef("testNumberedLyrics"); }
      void numerals() { mxmlIoTest("testNumerals"); }
      void ornaments() { mxmlIoTest("testOrnaments"); }
      void overlappingSpanners() { mxmlIoTest("testOverlappingSpanners"); }
      void partNames() { mxmlImportTestRef("testPartNames"); }
      void partNames2() { mxmlIoTest("testPartNames2"); }
      void pedalChanges() { mxmlIoTest("testPedalChanges"); }
      void pedalChangesBroken() { mxmlImportTestRef("testPedalChangesBroken"); }
      void pedalStyles() { mxmlIoTest("testPedalStyles"); }
      void placementDefaults() { mxmlImportTestRef("testPlacementDefaults"); }
      void placementOffsetDefaults() { mxmlImportTestRef("testPlacementOffsetDefaults"); }
      void printSpacingNo() { mxmlIoTestRef("testPrintSpacingNo"); }
      void repeatCounts() { mxmlIoTest("testRepeatCounts"); }
      void repeatSingleMeasure() { mxmlIoTest("testRepeatSingleMeasure"); }
      void restNotations() { mxmlIoTestRef("testRestNotations"); }
      void restsNoType() { mxmlIoTestRef("testRestsNoType"); }
      void restsTypeWhole() { mxmlIoTestRef("testRestsTypeWhole"); }
      void secondVoiceMelismata() { mxmlImportTestRef("testSecondVoiceMelismata"); }
      void sibOttavas() { mxmlImportTestRef("testSibOttavas"); }
      void slurs() { mxmlIoTest("testSlurs"); }
      void slurs2() { mxmlIoTest("testSlurs2"); }
      void slurTieDirecton() { mxmlIoTest("testSlurTieDirection"); }
      void slurTieLineStyle() { mxmlIoTest("testSlurTieLineStyle"); }
      void sound1() { mxmlIoTestRef("testSound1"); }
      void sound2() { mxmlIoTestRef("testSound2"); }
      // void specialCharacters() { mxmlIoTest("testSpecialCharacters"); } // TODO
      void staffEmptiness() { mxmlImportTestRef("testStaffEmptiness"); }
      void staffSize() { mxmlIoTest("testStaffSize"); }
      void staffTwoKeySigs() { mxmlIoTest("testStaffTwoKeySigs"); }
      void stringData()      { mxmlIoTest("testStringData"); }
      void stringVoiceName() { mxmlIoTestRef("testStringVoiceName"); }
      void swing() { mxmlIoTest("testSwing"); }
      void systemBrackets1() { mxmlIoTest("testSystemBrackets1"); }
      void systemBrackets2() { mxmlIoTest("testSystemBrackets2"); }
      void systemBrackets3() { mxmlImportTestRef("testSystemBrackets3"); }
      void systemBrackets4() { mxmlIoTest("testSystemBrackets4"); }
      void systemBrackets5() { mxmlIoTest("testSystemBrackets5"); }
      void systemDirection() { mxmlIoTest("testSystemDirection"); }
      void systemDistance() { mxmlMscxExportTestRef("testSystemDistance", true); }
      void systemDividers() { mxmlIoTest("testSystemDividers", true); }
      void tablature1() { mxmlIoTest("testTablature1"); }
      void tablature2() { mxmlIoTest("testTablature2"); }
      void tablature3() { mxmlIoTest("testTablature3"); }
      void tablature4() { mxmlIoTest("testTablature4"); }
      void tablature5() { mxmlIoTestRef("testTablature5"); }
      void tboxAboveBelow1() { mxmlMscxExportTestRef("testTboxAboveBelow1"); }
      void tboxAboveBelow2() { mxmlMscxExportTestRef("testTboxAboveBelow2"); }
      void tboxAboveBelow3() { mxmlMscxExportTestRef("testTboxAboveBelow3"); }
      void tboxMultiPage1() { mxmlMscxExportTestRef("testTboxMultiPage1"); }
      void tboxVbox1() { mxmlMscxExportTestRef("testTboxVbox1"); }
      void tboxWords1() { mxmlMscxExportTestRef("testTboxWords1"); }
      void tempo1() { mxmlIoTest("testTempo1"); }
      void tempo2() { mxmlIoTestRef("testTempo2"); }
      void tempo3() { mxmlIoTestRef("testTempo3"); }
      void tempo4() { mxmlIoTestRef("testTempo4"); }
      void tempo5() { mxmlIoTest("testTempo5"); }
      void tempo6() { mxmlIoTest("testTempo6"); }
      void tempoOverlap() { mxmlIoTestRef("testTempoOverlap"); } // TODO (?): Export of hidden tempo markings is incorrect
      void tempoPrecision() { mxmlMscxExportTestRef("testTempoPrecision"); }
      void tempoTextSpace1() { mxmlImportTestRef("testTempoTextSpace1"); }
      void tempoTextSpace2() { mxmlImportTestRef("testTempoTextSpace2"); }
      void textLines() { mxmlMscxExportTestRef("testTextLines"); }
      void textOrder() { mxmlImportTestRef("testTextOrder"); }
      void textQuirkInference() { mxmlImportTestRef("testTextQuirkInference"); }
      void tieTied() { mxmlIoTestRef("testTieTied"); }
      void importTie1() { mxmlImportTestRef("importTie1"); }
      void importTie2() { mxmlImportTestRef("importTie2"); }
      void importTie3() { mxmlImportTestRef("importTie3"); }
      void importTie4() { mxmlImportTestRef("importTie4"); }
      void importTie5() { mxmlIoTest("importTie5"); }
      void timesig1() { mxmlIoTest("testTimesig1"); }
      void timesig3() { mxmlIoTest("testTimesig3"); }
      void timesig4() { mxmlIoTest("testTimesig4"); }
      void titleSwapMu() {mxmlImportTestRef("testTitleSwapMu"); }
      void titleSwapSib() { mxmlImportTestRef("testTitleSwapSib"); }
      void trackHandling() { mxmlIoTest("testTrackHandling"); }
      void tremolo() { mxmlIoTest("testTremolo"); }
      void trills() { mxmlMscxExportTestRef("testTrills"); }
      void tuplets1() { mxmlIoTestRef("testTuplets1"); }
      void tuplets2() { mxmlIoTestRef("testTuplets2"); }
      void tuplets3() { mxmlIoTestRef("testTuplets3"); }
      void tuplets4() { mxmlIoTest("testTuplets4"); }
      void tuplets5() { mxmlIoTestRef("testTuplets5"); }
      void tuplets6() { mxmlIoTestRef("testTuplets6"); }
      void tuplets7() { mxmlIoTest("testTuplets7"); }
      void tuplets8() { mxmlMscxExportTestRef("testTuplets8"); }
      void tuplets9() { mxmlIoTest("testTuplets9"); }
      void tuplets10() { mxmlIoTest("testTuplets10"); }
      void tupletTie() { mxmlImportTestRef("testTupletTie"); }
      void twoNoteTremoloTuplet() { mxmlIoTest("testTwoNoteTremoloTuplet"); }
      void uninitializedDivisions() { mxmlIoTestRef("testUninitializedDivisions"); }
      void unnecessaryBarlines() { mxmlImportTestRef("testUnnecessaryBarlines"); }
      // void unterminatedTies() { mxmlImportTestRef("testUnterminatedTies"); } // TODO
      void unusualDurations() { mxmlIoTestRef("testUnusualDurations"); }
      void virtualInstruments() { mxmlIoTestRef("testVirtualInstruments"); }
      void voiceMapper1() { mxmlIoTestRef("testVoiceMapper1"); }
      void voiceMapper2() { mxmlIoTestRef("testVoiceMapper2"); }
      void voiceMapper3() { mxmlIoTestRef("testVoiceMapper3"); }
      void voicePiano1() { mxmlIoTest("testVoicePiano1"); }
      void volta1() { mxmlIoTest("testVolta1"); }
      void volta2() { mxmlIoTest("testVolta2"); }
      void voltaHiding1() { mxmlImportTestRef("testVoltaHiding"); }
      void voltaHiding2() { mxmlIoTestRef("testVoltaHiding"); }
      void wedge1() { mxmlIoTest("testWedge1"); }
      void wedge2() { mxmlIoTest("testWedge2"); }
      void wedge3() { mxmlIoTest("testWedge3"); }
      //void wedge4() { mxmlIoTestRef("testWedge4"); }
      // void wedge5() { mxmlIoTestRef("testWedge5"); } // TODO
      void wedgeOffset() { mxmlImportTestRef("testWedgeOffset"); }
      void words1() { mxmlIoTest("testWords1"); }
      // void words2() { mxmlIoTest("testWords2"); } // TODO
      void hiddenStaves()
            {
            MasterScore* score = readScore(DIR + "testHiddenStaves.xml");

            QVERIFY(score->style().value(Sid::hideEmptyStaves).toBool());
            }
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestMxmlIO::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   fixupScore -- do required fixups after MusicXML import
//   see mscore/file.cpp MuseScore::readScore(Score* score, QString name)
//---------------------------------------------------------

static void fixupScore(Score* score)
      {
      score->connectTies();
      score->masterScore()->rebuildMidiMapping();
      score->setCreated(false);
      score->setSaved(false);
      }

//---------------------------------------------------------
//   mxmlIoTest
//   read a MusicXML file, write to a new file and verify both files are identical
//---------------------------------------------------------

void TestMxmlIO::mxmlIoTest(const char* file, bool exportLayout)
      {
      MScore::debugMode = true;
      preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS, MusicxmlExportBreaks::MANUAL);
      preferences.setPreference(PREF_IMPORT_MUSICXML_IMPORTBREAKS, true);
      preferences.setPreference(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, exportLayout);
      preferences.setPreference(PREF_EXPORT_MUSICXML_EXPORTINVISIBLEELEMENTS, true);
      preferences.setPreference(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE, true);
      MasterScore* score = readScore(DIR + file + ".xml");
      QVERIFY(score);
      fixupScore(score);
      score->doLayout();
      QVERIFY(saveMusicXml(score, QString(file) + ".xml"));
      QVERIFY(saveCompareMusicXmlScore(score, QString(file) + ".xml", DIR + file + ".xml"));
      delete score;
      }

//---------------------------------------------------------
//   mxmlIoTestRef
//   read a MusicXML file, write to a new file and verify against reference
//---------------------------------------------------------

void TestMxmlIO::mxmlIoTestRef(const char* file)
      {
      MScore::debugMode = true;
      preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS, MusicxmlExportBreaks::MANUAL);
      preferences.setPreference(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, false);
      preferences.setPreference(PREF_IMPORT_MUSICXML_IMPORTBREAKS, true);
      preferences.setPreference(PREF_EXPORT_MUSICXML_EXPORTINVISIBLEELEMENTS, true);
      preferences.setPreference(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE, true);
      MasterScore* score = readScore(DIR + file + ".xml");
      QVERIFY(score);
      fixupScore(score);
      score->doLayout();
      QVERIFY(saveMusicXml(score, QString(file) + ".xml"));
      QVERIFY(saveCompareMusicXmlScore(score, QString(file) + ".xml", DIR + file + "_ref.xml"));
      delete score;
      }

//---------------------------------------------------------
//   mxmlIoTestRefBreaks
//   read a MusicXML file, write to a new file and verify against reference
//   using all possible settings for PREF_EXPORT_MUSICXML_EXPORTBREAKS
//---------------------------------------------------------

void TestMxmlIO::mxmlIoTestRefBreaks(const char* file)
      {
      QSKIP("Tests show different results every time");
      MScore::debugMode = true;
      preferences.setPreference(PREF_IMPORT_MUSICXML_IMPORTBREAKS, true);
      preferences.setPreference(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, false);
      preferences.setPreference(PREF_EXPORT_MUSICXML_EXPORTINVISIBLEELEMENTS, true);
      preferences.setPreference(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE, true);
      MasterScore* score = readScore(DIR + file + ".xml");
      QVERIFY(score);
      fixupScore(score);
      score->doLayout();
      preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS, MusicxmlExportBreaks::NO);
      QVERIFY(saveMusicXml(score, QString(file) + ".xml"));
      QVERIFY(saveCompareMusicXmlScore(score, QString(file) + ".xml", DIR + file + "_no_ref.xml"));
      preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS, MusicxmlExportBreaks::MANUAL);
      QVERIFY(saveMusicXml(score, QString(file) + ".xml"));
      QVERIFY(saveCompareMusicXmlScore(score, QString(file) + ".xml", DIR + file + "_manual_ref.xml"));
      preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS, MusicxmlExportBreaks::ALL);
      QVERIFY(saveMusicXml(score, QString(file) + ".xml"));
      QVERIFY(saveCompareMusicXmlScore(score, QString(file) + ".xml", DIR + file + "_all_ref.xml"));
      delete score;
      }

//---------------------------------------------------------
//   mxmlMscxExportTestRef
//   read a MuseScore mscx file, write to a MusicXML file and verify against reference
//---------------------------------------------------------

void TestMxmlIO::mxmlMscxExportTestRef(const char* file, bool exportLayout)
      {
      MScore::debugMode = true;
      preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS, MusicxmlExportBreaks::MANUAL);
      preferences.setPreference(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, exportLayout);
      preferences.setPreference(PREF_EXPORT_MUSICXML_EXPORTINVISIBLEELEMENTS, true);
      preferences.setPreference(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE, true);
      MasterScore* score = readScore(DIR + file + ".mscx");
      QVERIFY(score);
      fixupScore(score);
      score->doLayout();
      QVERIFY(saveMusicXml(score, QString(file) + ".xml"));
      QVERIFY(saveCompareMusicXmlScore(score, QString(file) + ".xml", DIR + file + "_ref.xml"));
      delete score;
      }

//---------------------------------------------------------
//   mxmlMscxExportTestRefBreaks
//   read a MuseScore mscx file, write to a MusicXML file and verify against reference
//   using all possible settings for PREF_EXPORT_MUSICXML_EXPORTBREAKS
//---------------------------------------------------------

void TestMxmlIO::mxmlMscxExportTestRefBreaks(const char* file)
      {
      QSKIP("Tests show different results every time");
      MScore::debugMode = true;
      preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS, MusicxmlExportBreaks::MANUAL);
      preferences.setPreference(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, false);
      preferences.setPreference(PREF_EXPORT_MUSICXML_EXPORTINVISIBLEELEMENTS, true);
      preferences.setPreference(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE, true);
      MasterScore* score = readScore(DIR + file + ".mscx");
      QVERIFY(score);
      fixupScore(score);
      score->doLayout();
      preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS, MusicxmlExportBreaks::NO);
      QVERIFY(saveMusicXml(score, QString(file) + ".xml"));
      QVERIFY(saveCompareMusicXmlScore(score, QString(file) + ".xml", DIR + file + "_no_ref.xml"));
      preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS, MusicxmlExportBreaks::MANUAL);
      QVERIFY(saveMusicXml(score, QString(file) + ".xml"));
      QVERIFY(saveCompareMusicXmlScore(score, QString(file) + ".xml", DIR + file + "_manual_ref.xml"));
      preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS, MusicxmlExportBreaks::ALL);
      QVERIFY(saveMusicXml(score, QString(file) + ".xml"));
      QVERIFY(saveCompareMusicXmlScore(score, QString(file) + ".xml", DIR + file + "_all_ref.xml"));
      delete score;
      }

//---------------------------------------------------------
//   mxmlReadTestCompr
//   read a compressed MusicXML file, write to a new file and verify against reference
//---------------------------------------------------------

void TestMxmlIO::mxmlReadTestCompr(const char* file)
      {
      MScore::debugMode = true;
      preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS, MusicxmlExportBreaks::MANUAL);
      preferences.setPreference(PREF_IMPORT_MUSICXML_IMPORTBREAKS, true);
      preferences.setPreference(PREF_EXPORT_MUSICXML_EXPORTINVISIBLEELEMENTS, true);
      preferences.setPreference(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE, true);
      MasterScore* score = readScore(DIR + file + ".mxl");
      QVERIFY(score);
      fixupScore(score);
      score->doLayout();
      QVERIFY(saveMusicXml(score, QString(file) + "_mxl_read.xml"));
      QVERIFY(saveCompareMusicXmlScore(score, QString(file) + "_mxl_read.xml", DIR + file + ".xml"));
      delete score;
      }

//---------------------------------------------------------
//   mxmlReadWriteTestCompr
//   read a MusicXML file, write to a compressed MusicXML file,
//   read the compressed MusicXML file, write to a new file and verify files are identical
//---------------------------------------------------------

void TestMxmlIO::mxmlReadWriteTestCompr(const char* file)
      {
      // read xml
      MScore::debugMode = true;
      preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS, MusicxmlExportBreaks::MANUAL);
      preferences.setPreference(PREF_IMPORT_MUSICXML_IMPORTBREAKS, true);
      preferences.setPreference(PREF_EXPORT_MUSICXML_EXPORTINVISIBLEELEMENTS, true);
      preferences.setPreference(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE, true);
      MasterScore* score = readScore(DIR + file + ".xml");
      QVERIFY(score);
      fixupScore(score);
      score->doLayout();
      // write mxl
      QVERIFY(saveMxl(score, QString(file) + "_mxl_read_write.mxl"));
      delete score;
      // read mxl
      score = readCreatedScore(QString(file) + "_mxl_read_write.mxl");
      QVERIFY(score);
      fixupScore(score);
      score->doLayout();
      // write and verify
      QVERIFY(saveCompareMusicXmlScore(score, QString(file) + "_mxl_read_write.xml", DIR + file + ".xml"));
      delete score;
      }

//---------------------------------------------------------
//   mxmlImportTestRef
//   read a MusicXML file, write to a new MuseScore mscx file
//   and verify against a MuseScore mscx reference file
//---------------------------------------------------------

void TestMxmlIO::mxmlImportTestRef(const char* file)
      {
      MScore::debugMode = false;
      preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS, MusicxmlExportBreaks::MANUAL);
      preferences.setPreference(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, false);
      preferences.setPreference(PREF_IMPORT_MUSICXML_IMPORTINFERTEXTTYPE, true);
      MasterScore* score = readScore(DIR + file + ".xml");
      QVERIFY(score);
      fixupScore(score);
      score->doLayout();
      QVERIFY(saveCompareScore(score, QString(file) + ".mscx", DIR + file + "_ref.mscx"));
      delete score;
      }

QTEST_MAIN(TestMxmlIO)
#if __has_include("tst_mxml_io.moc")
#include "tst_mxml_io.moc"
#endif
