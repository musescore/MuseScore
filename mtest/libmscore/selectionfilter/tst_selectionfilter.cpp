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
#include "libmscore/measure.h"

#define DIR QString("libmscore/selectionfilter/")

using namespace Ms;

//---------------------------------------------------------
//   TestSelectionFilter
//---------------------------------------------------------

class TestSelectionFilter : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void filterDynamic();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestSelectionFilter::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   filterDynamic
//---------------------------------------------------------

void TestSelectionFilter::filterDynamic()
      {
      Score* score = readScore(DIR + QString("selectionfilter1.mscx"));
      Measure* m1 = score->firstMeasure();

      QVERIFY(m1 != 0);

      score->select(m1);
      score->selectionFilter().setFiltered(SelectionFilterType::DYNAMIC,false);

      QVERIFY(score->selection().canCopy());
      QString mimeType = score->selection().mimeType();
      QVERIFY(mimeType == mimeStaffListFormat);

      //qDebug("%s",score->selection().mimeData().data());

      QVERIFY(saveCompareMimeData(score->selection().mimeData(),"selectionfilter1.xml",
         DIR + "selectionfilter1-ref.xml"));
      }

QTEST_MAIN(TestSelectionFilter)
#include "tst_selectionfilter.moc"

