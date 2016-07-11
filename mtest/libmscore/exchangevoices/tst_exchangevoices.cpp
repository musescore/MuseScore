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

#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"

#define DIR QString("libmscore/exchangevoices/")

using namespace Ms;

//---------------------------------------------------------
//   TestExchangevoices
//---------------------------------------------------------

class TestExchangevoices : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();

      void slurs();
      void glissandi();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestExchangevoices::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   slurs
//---------------------------------------------------------

void TestExchangevoices::slurs()
      {
      QString p1 = DIR + "exchangevoices-slurs.mscx";
      QVERIFY(score);
      Score* score = readScore(p1);
      score->doLayout();

      // select all
      score->startCmd();
      score->cmdSelectAll();
      score->endCmd();

      // do
      score->startCmd();
      score->cmdExchangeVoice(0,1);
      score->endCmd();

      // compare
      QVERIFY(saveCompareScore(score, "exchangevoices-slurs.mscx", DIR + "exchangevoices-slurs-ref.mscx"));
      }

//---------------------------------------------------------
//   glissandi
//---------------------------------------------------------

void TestExchangevoices::glissandi()
      {
      QString p1 = DIR + "exchangevoices-gliss.mscx";
      QVERIFY(score);
      Score* score = readScore(p1);
      score->doLayout();

      // select all
      score->startCmd();
      score->cmdSelectAll();
      score->endCmd();

      // do
      score->startCmd();
      score->cmdExchangeVoice(0,1);
      score->endCmd();

      // compare
      QVERIFY(saveCompareScore(score, "exchangevoices-gliss.mscx", DIR + "exchangevoices-gliss-ref.mscx"));
      }

QTEST_MAIN(TestExchangevoices)
#include "tst_exchangevoices.moc"
