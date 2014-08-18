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

#define DIR QString("ove/")

using namespace Ms;

//---------------------------------------------------------
//   TestOve
//---------------------------------------------------------

class TestOve : public QObject, public MTest
      {
      Q_OBJECT

      void checkLoadSave(const char*);

   private slots:
      void initTestCase();
      void channels01() { checkLoadSave("01"); } // default midi mapping
      void channels02() { checkLoadSave("02"); } // different channels
      void channels03() { checkLoadSave("03"); } // same channels
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestOve::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   checkLoadSave
//   load file, save and compare
//---------------------------------------------------------

void TestOve::checkLoadSave(const char* idx)
      {
      Score* score = readScore(DIR + QString("ove%1.ove").arg(idx));
      QVERIFY(score);
      score->doLayout();
      score->rebuildMidiMapping();
      QVERIFY(saveCompareScore(score, QString("ove%1.mscx").arg(idx),
         DIR + QString("ove%1-ref.mscx").arg(idx)));
      delete score;
      }

QTEST_MAIN(TestOve)
#include "tst_ove.moc"

