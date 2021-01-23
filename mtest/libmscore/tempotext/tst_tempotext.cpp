//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>

#include "libmscore/score.h"
#include "libmscore/tempotext.h"
#include "mtest/testutils.h"

using namespace Ms;

//---------------------------------------------------------
//   TestTempoText
//---------------------------------------------------------

class TestTempoText : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void tempotext();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestTempoText::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   tempotext
//---------------------------------------------------------

void TestTempoText::tempotext()
      {
      TempoText tempo(score);

      TempoText* tempo2 = static_cast<TempoText*>(writeReadElement(&tempo));
      // Default tempo: 120 BPM
      QCOMPARE(tempo2->tempo(), 2.0);
      delete tempo2;

      // Test roundtrip: writing tempo, reading it back, and comparing the value.
      double testBpms[] = {80, 92, 66, 100.32};
      for (double bpm : testBpms) {
            double beatsPerSecond = bpm / 60.0;
            tempo.setTempo(beatsPerSecond);
            tempo2 = static_cast<TempoText*>(writeReadElement(&tempo));
            QCOMPARE(tempo2->tempo(), beatsPerSecond);
            delete tempo2;
            }
      }

QTEST_MAIN(TestTempoText)

#include "tst_tempotext.moc"

