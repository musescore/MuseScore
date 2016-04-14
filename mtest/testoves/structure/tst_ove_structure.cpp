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

#define DIR QString("testoves/structure/")

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

      //void oveBarNumberTest() { oveReadTest("cond - bar number"); }
      //void oveEndingTest() { oveReadTest("cond - ending"); }
      //void oveExpressionsTest() { oveReadTest("cond - expressions"); }
      //void oveRepeatBackwardTest() { oveReadTest("cond - repeat - backward"); }
      //void oveRepeatBackwardForward2Test() { oveReadTest("cond - repeat - backward_forward 2"); }
      //void oveRepeatBackwardForwardTest() { oveReadTest("cond - repeat - backward_forward"); }
      //void oveBingyu2Test() { oveReadTest("cond - repeat - bingyu 2"); }
      //void oveBingyuTest() { oveReadTest("cond - repeat - bingyu"); }
      //void oveChunniTest() { oveReadTest("cond - repeat - chunni"); }
      //void oveRepeatDCCodaTest() { oveReadTest("cond - repeat - dc al coda"); }
      //void oveRepeatDCCodaToCodaTest() { oveReadTest("cond - repeat - dc al coda_to coda"); }
      //void oveRepeatDCFineTest() { oveReadTest("cond - repeat - dc al fine"); }
      //void oveRepeatDSCodaTest() { oveReadTest("cond - repeat - ds al coda_segno"); }
      //void oveRepeatDSCodaToCodaTest() { oveReadTest("cond - repeat - ds al coda_segno_to coda"); }
      //void oveRepepatDSFineTest() { oveReadTest("cond - repeat - ds al fine_segno"); }
      //void oveRepeatNumericEndingTest() { oveReadTest("cond - repeat - numeric ending"); }
      //void oveRepeatTheMomentTest() { oveReadTest("cond - repeat - the moment"); }
      //void oveRepeatToCodaCodaTest() { oveReadTest("cond - repeat - to coda_coda"); }
      //void oveRepeatTest() { oveReadTest("cond - repeat"); }
      //void oveTempoTest() { oveReadTest("cond - tempo"); }
      //void oveTimeCommonTest() { oveReadTest("cond - time - common"); }
      void oveTime24Test() { oveReadTest("cond - time 24"); }
      //void oveTimeTest() { oveReadTest("cond - time"); }
      //void oveLineTest() { oveReadTest("line"); }
      //void oveBarlineLastTest() { oveReadTest("meas - barline - last one"); }
      //void oveBarlineTest() { oveReadTest("meas - barline"); }
      //void ovePickupTest() { oveReadTest("meas - pickup"); }
      //void ovePageTextTest() { oveReadTest("page text"); }
      //void ovePageTest() { oveReadTest("page"); }
      //void oveStaffTest() { oveReadTest("staff"); }
      //void oveFourHandsTest() { oveReadTest("track - four hands"); }
      //void ovePercussionCopyTest() { oveReadTest("track - percussion - Copy"); }
      //void ovePercussionTest() { oveReadTest("track - percussion"); }
      //void ovePianoTest() { oveReadTest("track - piano accompaniment"); }
      //void oveTransposeInstrumentTest() { oveReadTest("track - transpose instrument"); }
      //void oveTrebleBassTest() { oveReadTest("track - treble bass"); }
      //void oveVoicesPatchTest() { oveReadTest("track - voices patch"); }
      //void oveVoicesTest() { oveReadTest("track - voices"); }
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
#include "tst_ove_structure.moc"
