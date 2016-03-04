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
#include "libmscore/scale.h"

#define DIR QString("libmscore/scale/")

using namespace Ms;
using namespace std;

//---------------------------------------------------------
//   TestScale
//---------------------------------------------------------

class TestScale : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void testStandardScale();
      void testLoadScale();
   private:
      void testScale(int nbNotes,
                     vector<QString> initialNotes,
                     vector<float> expectedTunings);
      void testConversion(Scale& scale,
                          ScaleParams& from,
                          ScaleParams& to,
                          vector<float> expectedTunings);

      float err = 0.2;
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestScale::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   testStandardScale
//---------------------------------------------------------

void TestScale::testStandardScale()
      {
      testScale(Scale::NB_ALL_NOTES,
                vector<QString>(),
                vector<float>(Scale::NB_ALL_NOTES, 0));
      }

//---------------------------------------------------------
//   testLoadScale
//---------------------------------------------------------

void TestScale::testLoadScale()
      {
      static const vector<QString> originalNoteValues =
            { "495.70313", "2/1", "703.12500", "162.89063",
              "866.01563", "288.28125", "992.57813" };

      static const vector<float> expectedTunings =
            { -40.4, -36.7,  29.6,  33.9,  37.1,  -3.1,   0.0,
              -77.7, -73.4,  33.3,  15.4,  18.5, -40.4, -36.7,
               29.6,  33.9,  37.1,  -3.1,   0.0, -77.7, -73.4,
               33.3,  15.4,  18.5, -40.4, -36.7,  29.6,  33.9,
               37.1,  -3.1,   0.0, -77.7, -73.4,  33.3,  15.4};

      ScaleParams params;

      for (int i = 0; i < TPC_NUM_OF; ++i)
            params.notes[i] = "";
      for (int tpc = TPC_F; tpc < TPC_F_S; ++tpc)
            params.notes[tpc - TPC_MIN] = originalNoteValues[tpc - TPC_F];
      params.nbNotes = 7;

      Scale scale(params);
      scale.computeTunings();
      float* computedTunings = scale.getComputedTunings();
      for (int index = 0; index < TPC_NUM_OF; index++)
            QVERIFY(fabs(computedTunings[index] - expectedTunings[index]) < err);

      vector<QString> initialNoteValues;
      for (int i = 0; i < 14; ++i)
            initialNoteValues.push_back("");
      for (int i = 0; i < 7; ++i)
            initialNoteValues.push_back(originalNoteValues[i]);
      for (int i = 0; i < 14; ++i)
            initialNoteValues.push_back("");
      testScale(Scale::NB_ONLY_NOTES,
                initialNoteValues,
                expectedTunings);
      }

//---------------------------------------------------------
//   testScale
//---------------------------------------------------------

void TestScale::testScale(int nbNotes,
                          vector<QString> initialNotes,
                          vector<float> expectedTunings)
{
      Scale scale;
      float* computedTunings = scale.getComputedTunings();

      for (int index = 0; index < TPC_NUM_OF; ++index)
            QVERIFY(fabs(computedTunings[index]) < err);


      QString* originalNotes = scale.getOriginalNotes();
      ScaleParams from, to;
      if (!initialNotes.empty()) {
            for (int i = 0; i < TPC_NUM_OF; ++i)
                  originalNotes[i] = initialNotes[i];
            }

      // ABSOLUTE_CENTS -> DELTA_CENTS note pitch
      from.nbNotes = to.nbNotes = nbNotes;
      from.storingMode = Scale::ABSOLUTE_CENTS;
      from.storeFifths = false;
      to.storingMode = Scale::DELTA_CENTS;
      to.storeFifths = false;
      to.reference = Scale::A_REFERENCE;
      testConversion(scale, from, to, expectedTunings);

      // DELTA_CENTS -> ABSOLUTE_CENTS note pitch
      from.storingMode = Scale::DELTA_CENTS;
      to.storingMode = Scale::ABSOLUTE_CENTS;
      testConversion(scale, from, to, expectedTunings);

      // A_REFERENCE -> C_REFERENCE ABSOLUTE_CENTS note pitch
      from.storingMode = Scale::ABSOLUTE_CENTS;
      to.reference = Scale::C_REFERENCE;
      testConversion(scale, from, to, expectedTunings);

      // C_REFERENCE -> A_REFERENCE ABSOLUTE_CENTS note pitch
      to.reference = Scale::A_REFERENCE;
      testConversion(scale, from, to, expectedTunings);

      // ABSOLUTE_CENTS -> ABSOLUTE_CENTS note pitch -> fifths
      from.storingMode = Scale::ABSOLUTE_CENTS;
      to.storingMode = Scale::ABSOLUTE_CENTS;
      to.storeFifths = true;
      testConversion(scale, from, to, expectedTunings);

      // ABSOLUTE_CENTS -> DELTA_CENTS fifths
      from.storingMode = Scale::ABSOLUTE_CENTS;
      from.storeFifths = true;
      to.storingMode = Scale::DELTA_CENTS;
      testConversion(scale, from, to, expectedTunings);

      // DELTA_CENTS -> ABSOLUTE_FREQUENCY fifths
      from.storingMode = Scale::DELTA_CENTS;
      to.storingMode = Scale::ABSOLUTE_FREQUENCY;
      testConversion(scale, from, to, expectedTunings);

      // ABSOLUTE_FREQUENCY -> ABSOLUTE_CENTS fifths
      from.storingMode = Scale::ABSOLUTE_FREQUENCY;
      to.storingMode = Scale::ABSOLUTE_CENTS;
      testConversion(scale, from, to, expectedTunings);

      // ABSOLUTE_CENTS -> ABSOLUTE_CENTS fifths -> note pitch
      from.storingMode = Scale::ABSOLUTE_CENTS;
      to.storingMode = Scale::ABSOLUTE_CENTS;
      to.storeFifths = false;
      testConversion(scale, from, to, expectedTunings);

      // ABSOLUTE_CENTS -> ABSOLUTE_FREQUENCY note pitch
      from.storingMode = Scale::ABSOLUTE_CENTS;
      from.storeFifths = false;
      to.storingMode = Scale::ABSOLUTE_FREQUENCY;
      testConversion(scale, from, to, expectedTunings);

      // ABSOLUTE_FREQUENCY -> ABSOLUTE_CENTS note pitch
      from.storingMode = Scale::ABSOLUTE_FREQUENCY;
      to.storingMode = Scale::ABSOLUTE_CENTS;
      testConversion(scale, from, to, expectedTunings);
}

//---------------------------------------------------------
//   testConversion
//---------------------------------------------------------

void TestScale::testConversion(Scale& scale,
                               ScaleParams& from,
                               ScaleParams& to,
                               vector<float> expectedTunings)
      {
      QString* originalNotes = scale.getOriginalNotes();
      for (int i = 0; i < TPC_NUM_OF; ++i) {
            from.notes[i] = originalNotes[i];
            to.notes[i] = "";
            }

      Scale::recomputeNotes(from, to);
      scale = Scale(to);
      scale.computeTunings(to.storingMode, to.storeFifths);
      float* computedTunings = scale.getComputedTunings();

      for (int index = 0; index < TPC_NUM_OF; ++index) {
            if (fabs(computedTunings[index] - expectedTunings[index]) >= err)
                  qDebug() << "failed on index " << index << " tune " << computedTunings[index] << " expected " << expectedTunings[index]
                        << " abs value " << fabs(computedTunings[index] - expectedTunings[index]);
            QVERIFY(fabs(computedTunings[index] - expectedTunings[index]) < err);
            }
      }

QTEST_MAIN(TestScale)
#include "tst_scale.moc"
