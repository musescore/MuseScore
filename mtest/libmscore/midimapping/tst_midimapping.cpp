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

#define DIR QString("libmscore/midimapping/")

using namespace Ms;

//---------------------------------------------------------
//   TestMidiMapping
//---------------------------------------------------------

class TestMidiMapping : public QObject, public MTest
      {
      Q_OBJECT

      void checkloadsave(const char*);

   private slots:
      void initTestCase();
      void midimapping01() { checkloadsave("01"); } // default midi mapping
      void midimapping02() { checkloadsave("02"); } // different channels and ports
      void midimapping03() { checkloadsave("03"); } // same channels
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestMidiMapping::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   checkloadsave
//   load file, save and compare
//---------------------------------------------------------

void TestMidiMapping::checkloadsave(const char* idx)
      {
      Score* score = readScore(DIR + QString("midimapping%1.mscx").arg(idx));
      score->doLayout();
      score->rebuildMidiMapping();
      QVERIFY(saveCompareScore(score, QString("midimapping%1.mscx").arg(idx),
         DIR + QString("midimapping%1.mscx").arg(idx)));
      delete score;
      }

QTEST_MAIN(TestMidiMapping)
#include "tst_midimapping.moc"

