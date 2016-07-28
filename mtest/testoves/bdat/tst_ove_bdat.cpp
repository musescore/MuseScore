//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2015 Werner Schweer and others
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

#define DIR QString("testoves/bdat/")

using namespace Ms;

//---------------------------------------------------------
//   TestOveIO
//---------------------------------------------------------

class TestOveIO : public QObject, public MTest
      {
      Q_OBJECT

      void oveReadTest(const char* file);

private slots:
      void initTestCase();

      // The list of Ove regression tests
      // Currently failing tests are commented out and annotated with the failure reason

      void oveBeamMultipleVoicesTest() { oveReadTest("beam - multiple voices"); }
      void oveBeam2Test()              { oveReadTest("beam 2"); }
      void oveBeamWinstonTest()        { oveReadTest("beam-George Winston - Joy"); }
      //void oveBeamOverBarlineTest()    { oveReadTest("beam-over-barline"); }
      void oveBeamTest()               { oveReadTest("beam"); }
      void oveClef2Test()              { oveReadTest("clef-2"); }
      //void oveClefTypes()              { oveReadTest("clef-types"); }
      //void oveClefTest()               { oveReadTest("clef"); }
      //void oveArticulationTest()       { oveReadTest("decorator-articulation"); }
      //void oveMeasureRepeatTest()      { oveReadTest("decorator-measure-repeat"); }
      //void oveDynamicsTest()           { oveReadTest("dynamics"); }
      //void oveTempoChangeTest()        { oveReadTest("expression-tempo-change"); }
      //void oveExpressionTest()         { oveReadTest("expression"); }
      //void oveGlissandoSimpleTest()    { oveReadTest("glissando-simple"); }
      //void oveGlissandoTest()          { oveReadTest("glissando"); }
      //void oveGraphicsTest()           { oveReadTest("graphics"); }
      //void oveGuitarBarreTest()        { oveReadTest("guitar-barre"); }
      //void oveGuitarBendTest()         { oveReadTest("guitar-bend"); }
      void oveHarmony2Test()           { oveReadTest("harmony 2"); }
      //void oveGuitarFretTest()         { oveReadTest("harmony-guitar-frame"); }
      //void oveHarmonyTypeTest()        { oveReadTest("harmony-type"); }
      //void oveHarmonyTest()            { oveReadTest("harmony"); }
      //void oveHarpPedalTest()          { oveReadTest("harp-pedal"); }
      //void oveInvisibleTest()          { oveReadTest("invisible-object"); }
      //void oveKeyTransposeTest()       { oveReadTest("key-transpose-instrument"); }
      void oveKeyTest()                { oveReadTest("key"); }
      //void oveParenthesisTest()        { oveReadTest("kuohao-brace-bracket-parentheses"); }
      //void oveLyricsVerseTest()        { oveReadTest("lyric-verse"); }
      void oveLyricsTest()             { oveReadTest("lyric"); }
      //void oveMultimeasureRestTest()   { oveReadTest("multi-measure-rest"); }
      void oveAccidentalsTest()        { oveReadTest("note-accidental"); }
      //void oveArticulations2Test()     { oveReadTest("note-articulation-2"); }
      void oveArpeggioTest()           { oveReadTest("note-articulation-arpeggio"); }
      //void ovePedalTest()              { oveReadTest("note-articulation-pedal"); }
      void oveTremoloTest()            { oveReadTest("note-articulation-tremolo"); }
      //void oveTrillLineTest()          { oveReadTest("note-articulation-trill-section"); }
      void oveTrillTest()              { oveReadTest("note-articulation-trill"); }
      void oveFermataRestTest()        { oveReadTest("note-articulation-with-rest"); }
      //void oveArticulationsTest()      { oveReadTest("note-articulation"); }
      void oveClefChangeTest()         { oveReadTest("note-clef"); }
      void oveCrossStaff2Test()        { oveReadTest("note-cross-staff 2"); }
      //void oveCrossStaff3Test()        { oveReadTest("note-cross-staff 3"); }
      void oveCrossStaffTest()         { oveReadTest("note-cross-staff"); }
      void oveCueTest()                { oveReadTest("note-cue"); }
      void oveNoteDotTest()            { oveReadTest("note-dot"); }
      //void oveGraceBeamTest()          { oveReadTest("note-grace-with-beam"); }
      //void oveGraceTest()              { oveReadTest("note-grace"); }
      //void oveNoteHeadTest()           { oveReadTest("note-head"); }
      //void oveNoteRawTest()            { oveReadTest("note-raw"); }
      //void oveRestsTest()              { oveReadTest("note-rest"); }
      void oveScaleTest()              { oveReadTest("note-scale-c"); }
      //void oveScaleKeysTest()          { oveReadTest("note-scale-key"); }
      //void oveDurationTest()           { oveReadTest("note-type"); }
      void oveVoices5Test()            { oveReadTest("note-voices 5"); }
      //void oveVoices2Test()            { oveReadTest("note-voices-2"); }
      void oveVoices3Test()            { oveReadTest("note-voices-3"); }
      void oveVoices4Test()            { oveReadTest("note-voices-4"); }
      void oveVoicesTest()             { oveReadTest("note-voices"); }
      void oveNoteTest()               { oveReadTest("note"); }
      //void oveOctaveTest()             { oveReadTest("octave-shift"); }
      //void ovePedal2Test()             { oveReadTest("pedal-half-pedal"); }
      //void ovePedal3Test()             { oveReadTest("pedal"); }
      void oveSlur2Test()              { oveReadTest("slur 2"); }
      void oveSlurTest()               { oveReadTest("slur"); }
      //void oveTextLinesTest()          { oveReadTest("text-measure-text-lines"); }
      //void oveTextTest()               { oveReadTest("text-measure-text"); }
      //void oveRehearsalLinesTest()     { oveReadTest("text-rehearsal-lines"); }
      //void oveRehearsalTest()          { oveReadTest("text-rehearsal"); }
      //void oveSystemText2Test()        { oveReadTest("text-system-text-lines"); }
      //void oveSystemTextTest()         { oveReadTest("text-system-text"); }
      //void oveTie2Test()               { oveReadTest("tie-2"); }
      void oveTieTest()                { oveReadTest("tie"); }
      void oveTuplet2Test()            { oveReadTest("tuplet 2"); }
      void oveTuplet3Test()            { oveReadTest("tuplet-beam"); }
      //void oveTuplet4Test()            { oveReadTest("tuplet-in-second-staff"); }
      void oveTuplet5Test()            { oveReadTest("tuplet-rest-start 2"); }
      //void oveTuplet6Test()            { oveReadTest("tuplet-rest-start 3"); }
      void oveTuplet7Test()            { oveReadTest("tuplet-rest-start"); }
      //void oveTupletTest()             { oveReadTest("tuplet"); }
      //void oveWedge2Test()             { oveReadTest("wedge-2"); }
      //void oveWedgeTest()              { oveReadTest("wedge"); }

/* TODO midi tests ?
midi-channel-pressure.ove
midi-controller-11-expression.ove
midi-controller-pedal.ove
midi-pitch-wheel.ove
midi-program-change.ove
midi-tempo-change-sharp.ove
midi-tempo-change.ove
*/
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestOveIO::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   oveReadTest
//   read an Ove file, write to a MuseScore file and verify against reference
//---------------------------------------------------------

void TestOveIO::oveReadTest(const char* file)
      {
      preferences.importCharsetOve = "GBK";
      MasterScore* score = readScore(DIR + file + ".ove");
      QVERIFY(score);
      score->doLayout();
      score->connectTies();
      score->setLayoutAll();
      score->update();
      QVERIFY(saveCompareScore(score, QString("%1.ove.mscx").arg(file),
                               DIR + QString("%1.ove-ref.mscx").arg(file)));
      delete score;
      }

QTEST_MAIN(TestOveIO)
#include "tst_ove_bdat.moc"
