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

#define DIR QString("testoves/ove3/")

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

      //void ove3PianoChangeClefTest() { oveReadTest("piano - change clef"); }
      //void ove3BeamCrossBarTest() { oveReadTest("[ove3].bdat - beam cross bar"); }
      //void ove3BeamTest() { oveReadTest("[ove3].bdat - beam"); }
      //void ove3Clef2Test() { oveReadTest("[ove3].bdat - clef 2"); }
      //void ove3ClefTest() { oveReadTest("[ove3].bdat - clef"); }
      //void ove3MeasureRepeatTest() { oveReadTest("[ove3].bdat - decorator - measure repeat"); }
      //void ove3DynamicsTest() { oveReadTest("[ove3].bdat - dynamics"); }
      //void ove3ExpressionTest() { oveReadTest("[ove3].bdat - expression"); }
      //void ove3GlissandoTest() { oveReadTest("[ove3].bdat - glissando"); }
      //void ove3BDatGraphicsTest() { oveReadTest("[ove3].bdat - graphics"); }
      //void ove3GuitarBarreTest() { oveReadTest("[ove3].bdat - guitar - barre"); }
      //void ove3GuitarBendTest() { oveReadTest("[ove3].bdat - guitar - bend"); }
      //void ove3GuitarFrameTest() { oveReadTest("[ove3].bdat - harmony - guitar frame"); }
      void ove3HarmonyTest() { oveReadTest("[ove3].bdat - harmony"); }
      //void ove3HarpPedalTest() { oveReadTest("[ove3].bdat - harp pedal"); }
      //void ove3InvisibleTest() { oveReadTest("[ove3].bdat - invisible object"); }
      //void ove3KeyTransposeInstrumentTest() { oveReadTest("[ove3].bdat - key - transpose instrument"); }
      //void ove3KeyTest() { oveReadTest("[ove3].bdat - key"); }
      //void ove3ParenthesisTest() { oveReadTest("[ove3].bdat - kuohao - brace.bracket.parentheses"); }
      //void ove3LyricsTest() { oveReadTest("[ove3].bdat - lyric"); }
      //void ove3MultimeasureRestTest() { oveReadTest("[ove3].bdat - multi-measure rest"); }
      //void ove3AccidentalTest() { oveReadTest("[ove3].bdat - note - accidental"); }
      //void ove3TrillTest() { oveReadTest("[ove3].bdat - note - articulation - trill"); }
      //void ove3Articulation2Test() { oveReadTest("[ove3].bdat - note - articulation 2"); }
      //void ove3FermataOnRestTest() { oveReadTest("[ove3].bdat - note - articulation on rest"); }
      //void ove3ArticulationTest() { oveReadTest("[ove3].bdat - note - articulation"); }
      //void ove3CelfAndScaleTest() { oveReadTest("[ove3].bdat - note - clef & scale"); }
      //void ove3CrossStaffTest() { oveReadTest("[ove3].bdat - note - cross staff"); }
      //void ove3CueAndGraceTest() { oveReadTest("[ove3].bdat - note - cue & grace"); }
      //void ove3DotTest() { oveReadTest("[ove3].bdat - note - dot"); }
      //void ove3NoteRawTest() { oveReadTest("[ove3].bdat - note - raw"); }
      //void ove3RestTest() { oveReadTest("[ove3].bdat - note - rest"); }
      //void ove3ScaleTest() { oveReadTest("[ove3].bdat - note - scale"); }
      //void ove3NoteTypeTest() { oveReadTest("[ove3].bdat - note - type"); }
      //void ove3NoteVoicesTest() { oveReadTest("[ove3].bdat - note - voices"); }
      //void ove3OctaveTest() { oveReadTest("[ove3].bdat - octave shift"); }
      //void ove3PedalHalfTest() { oveReadTest("[ove3].bdat - pedal - half"); }
      //void ove3PedalTest() { oveReadTest("[ove3].bdat - pedal"); }
      //void ove3SlurTest() { oveReadTest("[ove3].bdat - slur"); }
      //void ove3TextLinesTest() { oveReadTest("[ove3].bdat - text - measure - line break"); }
      //void ove3TextTest() { oveReadTest("[ove3].bdat - text - measure"); }
      //void ove3TieTest() { oveReadTest("[ove3].bdat - tie"); }
      //void ove3TupletRestTest() { oveReadTest("[ove3].bdat - tuplet - rest start"); }
      //void ove3TupletTest() { oveReadTest("[ove3].bdat - tuplet"); }
      //void ove3WedgeTest() { oveReadTest("[ove3].bdat - wedge"); }
      //void ove3EndingTest() { oveReadTest("[ove3].cond - ending"); }
      //void ove3RehearsalTest() { oveReadTest("[ove3].cond - rehearsal"); }
      //void ove3RepeatTest() { oveReadTest("[ove3].cond - repeat"); }
      //void ove3TempoTest() { oveReadTest("[ove3].cond - tempo"); }
      //void ove3GraphicsTest() { oveReadTest("[ove3].graphics"); }
      //void ove3BarlineTest() { oveReadTest("[ove3].meas - barline"); }
      //void ove3PickupTest() { oveReadTest("[ove3].meas - pickup"); }
      //void ove3FourHandsTest() { oveReadTest("[ove3].track - four hands"); }
      //void ove3TrebleBassCopyTest() { oveReadTest("[ove3].track - treble bass - Copy"); }
      //void ove3TrebleBassTest() { oveReadTest("[ove3].track - treble bass"); }
      //void ove3VoicesTest() { oveReadTest("[ove3].track - voices"); }
      void ove3UntitledTest() { oveReadTest("[ove3].Untitled"); }

/* TODO midi tests ?
[ove3].bdat - midi - channel pressure
[ove3].bdat - midi - controller
[ove3].bdat - midi - pitch wheel
[ove3].bdat - midi - program change
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
#include "tst_ove_ove3.moc"
