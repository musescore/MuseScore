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

#define DIR QString("libmscore/rhythmicGrouping/")

using namespace Ms;

//---------------------------------------------------------
//   TestRhythmicGrouping
//---------------------------------------------------------

class TestRhythmicGrouping : public QObject, public MTest
      {
      Q_OBJECT

      void group(const char* p1, const char* p2);

   private slots:
      void initTestCase();
      void group8ths44()            { group("group8ths4-4.mscx",        "group8ths4-4-ref.mscx");      }
      void group8thsSimple()        { group("group8thsSimple.mscx",     "group8thsSimple-ref.mscx");   }
      void group8thsCompound()      { group("group8thsCompound.mscx",   "group8thsCompound-ref.mscx"); }
      void groupVoices()            { group("groupVoices.mscx",         "groupVoices-ref.mscx");       }

      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestRhythmicGrouping::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   group
//---------------------------------------------------------

void TestRhythmicGrouping::group(const char* p1, const char* p2)
      {
      Score* score = readScore(DIR + p1);
      score->doLayout();
      score->cmdSelectAll();
      score->cmdResetNoteAndRestGroupings();
      score->doLayout();
      QVERIFY(saveCompareScore(score, p1, DIR + p2));
      delete score;
      }

QTEST_MAIN(TestRhythmicGrouping)
#include "tst_rhythmicGrouping.moc"

