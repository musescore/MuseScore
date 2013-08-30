//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2012 Werner Schweer
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
#include "libmscore/keysig.h"
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

      void mxmlIoTest(const char* file);
      void mxmlIoTestRef(const char* file);
      void mxmlReadTestCompr(const char* file);
      void mxmlReadWriteTestCompr(const char* file);


      // The list of MusicXML regression tests
      // Currently failing tests are commented out and annotated with the failure reason
      // To extract the list in a shell scipt use:
      // cat tst_mxml_io.cpp | grep "{ <test>" | awk -F\" '{print $2}'
      // where <test> is mxmlIoTest or mxmlIoTestRef

private slots:
      void initTestCase();

      void accidentals1() { mxmlIoTest("testAccidentals1"); }
      void accidentals2() { mxmlIoTest("testAccidentals2"); }
      void arpGliss1() { mxmlIoTest("testArpGliss1"); }
      void arpGliss2() { mxmlIoTest("testArpGliss2"); }
      void barStyles() { mxmlIoTest("testBarStyles"); }
      void chordDiagrams1() { mxmlIoTest("testChordDiagrams1"); }
      void chordNoVoice() { mxmlIoTestRef("testChordNoVoice"); }
      void clefs1() { mxmlIoTest("testClefs1"); }
      void completeMeasureRests() { mxmlIoTest("testCompleteMeasureRests"); }
      void dalSegno() { mxmlIoTest("testDalSegno"); }
      void dcalCoda() { mxmlIoTest("testDCalCoda"); }
      void dcalFine() { mxmlIoTest("testDCalFine"); }
//      void directions1() { mxmlIoTest("testDirections1"); }
      void drumset1() { mxmlIoTest("testDrumset1"); }
      void drumset2() { mxmlIoTest("testDrumset2"); }
      void durationRoundingError() { mxmlIoTestRef("testDurationRoundingError"); }
      void dynamics1() { mxmlIoTest("testDynamics1"); }
      void dynamics2() { mxmlIoTest("testDynamics2"); }
      void dynamics3() { mxmlIoTestRef("testDynamics3"); }
      void emptyMeasure() { mxmlIoTestRef("testEmptyMeasure"); }
      void emptyVoice1() { mxmlIoTestRef("testEmptyVoice1"); }
      void figuredBass1() { mxmlIoTest("testFiguredBass1"); }
      void figuredBass2() { mxmlIoTest("testFiguredBass2"); }
      void grace1() { mxmlIoTest("testGrace1"); }
//      void grace2() { mxmlIoTest("testGrace2"); }
      void harmony1() { mxmlIoTest("testHarmony1"); }
      void harmony2() { mxmlIoTest("testHarmony2"); }
//      void harmony3() { mxmlIoTest("testHarmony3"); }
      void harmony4() { mxmlIoTest("testHarmony4"); }
      void harmony5() { mxmlIoTest("testHarmony5"); } // chordnames without chordrest
      void hello() { mxmlIoTest("testHello"); }
      void helloReadCompr() { mxmlReadTestCompr("testHello"); }
      void helloReadWriteCompr() { mxmlReadWriteTestCompr("testHello"); }
      void implicitMeasure1() { mxmlIoTest("testImplicitMeasure1"); }
      void invisibleElements() { mxmlIoTest("testInvisibleElements"); }
      void keysig1() { mxmlIoTest("testKeysig1"); }
//      void lines1() { mxmlIoTestRef("testLines1"); }
//      void lines2() { mxmlIoTestRef("testLines2"); }
      void lyricsVoice2a() { mxmlIoTest("testLyricsVoice2a"); }
      void lyricsVoice2b() { mxmlIoTestRef("testLyricsVoice2b"); }
      void manualBreaks() { mxmlIoTest("testManualBreaks"); }
      void measureLength() { mxmlIoTestRef("testMeasureLength"); }
      void multiMeasureRest1() { mxmlIoTestRef("testMultiMeasureRest1"); }
      void multiMeasureRest2() { mxmlIoTestRef("testMultiMeasureRest2"); }
      void multiMeasureRest3() { mxmlIoTestRef("testMultiMeasureRest3"); }
//      void nonUniqueThings() { mxmlIoTestRef("testNonUniqueThings"); }
//      void noteAttributes1() { mxmlIoTest("testNoteAttributes1"); }
      void noteAttributes2() { mxmlIoTestRef("testNoteAttributes2"); }
      void noteAttributes3() { mxmlIoTest("testNoteAttributes3"); }
      void noteheads() { mxmlIoTest("testNoteheads"); }
      void notesRests1() { mxmlIoTest("testNotesRests1"); }
      void notesRests2() { mxmlIoTest("testNotesRests2"); }
      void numberedLyrics() { mxmlIoTestRef("testNumberedLyrics"); }
      void restsNoType() { mxmlIoTestRef("testRestsNoType"); }
//      void slurs() { mxmlIoTest("testSlurs"); }
      void staffTwoKeySigs() { mxmlIoTest("testStaffTwoKeySigs"); }
      void systemBrackets1() { mxmlIoTest("testSystemBrackets1"); }
      void systemBrackets2() { mxmlIoTest("testSystemBrackets2"); }
      void tablature1() { mxmlIoTest("testTablature1"); }
      void tablature2() { mxmlIoTest("testTablature2"); }
      void tempo1() { mxmlIoTest("testTempo1"); }
      void timesig1() { mxmlIoTest("testTimesig1"); }
      void timesig3() { mxmlIoTest("testTimesig3"); }
      void tremolo() { mxmlIoTest("testTremolo"); }
      void tuplets1() { mxmlIoTestRef("testTuplets1"); }
      void tuplets2() { mxmlIoTestRef("testTuplets2"); }
      void tuplets3() { mxmlIoTestRef("testTuplets3"); }
      void tuplets4() { mxmlIoTest("testTuplets4"); }
      void unusualDurations() { mxmlIoTestRef("testUnusualDurations"); }
      void voiceMapper1() { mxmlIoTestRef("testVoiceMapper1"); }
      void voiceMapper2() { mxmlIoTestRef("testVoiceMapper2"); }
      void voiceMapper3() { mxmlIoTestRef("testVoiceMapper3"); }
      void voicePiano1() { mxmlIoTest("testVoicePiano1"); }
      void volta1() { mxmlIoTest("testVolta1"); }
//      void wedge1() { mxmlIoTest("testWedge1"); }
//      void wedge2() { mxmlIoTestRef("testWedge2"); }
      void words1() { mxmlIoTest("testWords1"); }
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
//   TODO: remove duplication of code
//---------------------------------------------------------

static void fixupScore(Score* score)
      {
//      score->syntiState().append(SyntiParameter("soundfont", MScore::soundFont));
      score->connectTies();
      score->rebuildMidiMapping();
      score->setCreated(false);
      score->setSaved(false);

      int staffIdx = 0;
      foreach(Staff* st, score->staves()) {
            if (st->updateKeymap())
                  st->keymap()->clear();
            int track = staffIdx * VOICES;
            KeySig* key1 = 0;
            for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
                  for (Segment* s = m->first(); s; s = s->next()) {
                        if (!s->element(track))
                              continue;
                        Element* e = s->element(track);
                        if (e->generated())
                              continue;
                        //if ((s->subtype() == SegClef) && st->updateClefList()) {
                        //      Clef* clef = static_cast<Clef*>(e);
                        //      st->setClef(s->tick(), clef->clefTypeList());
                        //      }
                        if ((s->segmentType() == Segment::SegKeySig) && st->updateKeymap()) {
                              KeySig* ks = static_cast<KeySig*>(e);
                              int naturals = key1 ? key1->keySigEvent().accidentalType() : 0;
                              ks->setOldSig(naturals);
                              st->setKey(s->tick(), ks->keySigEvent());
                              key1 = ks;
                              }
                        }
                  if (m->sectionBreak())
                        key1 = 0;
                  }
            st->setUpdateKeymap(false);
            ++staffIdx;
            }

      score->updateNotes();
      }

//---------------------------------------------------------
//   mxmlIoTest
//   read a MusicXML file, write to a new file and verify both files are identical
//---------------------------------------------------------

void TestMxmlIO::mxmlIoTest(const char* file)
      {
      MScore::debugMode = true;
      preferences.musicxmlExportBreaks = MANUAL_BREAKS;
      preferences.musicxmlImportBreaks = true;
      Score* score = readScore(DIR + file + ".xml");
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
      preferences.musicxmlExportBreaks = MANUAL_BREAKS;
      preferences.musicxmlImportBreaks = true;
      Score* score = readScore(DIR + file + ".xml");
      QVERIFY(score);
      fixupScore(score);
      score->doLayout();
      QVERIFY(saveMusicXml(score, QString(file) + ".xml"));
      QVERIFY(saveCompareMusicXmlScore(score, QString(file) + ".xml", DIR + file + "_ref.xml"));
      delete score;
      }

//---------------------------------------------------------
//   mxmlReadTestCompr
//   read a compressed MusicXML file, write to a new file and verify against reference
//---------------------------------------------------------

void TestMxmlIO::mxmlReadTestCompr(const char* file)
      {
      MScore::debugMode = true;
      preferences.musicxmlExportBreaks = MANUAL_BREAKS;
      preferences.musicxmlImportBreaks = true;
      Score* score = readScore(DIR + file + ".mxl");
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
      preferences.musicxmlExportBreaks = MANUAL_BREAKS;
      preferences.musicxmlImportBreaks = true;
      Score* score = readScore(DIR + file + ".xml");
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

QTEST_MAIN(TestMxmlIO)
#include "tst_mxml_io.moc"
