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

      void testFilter(int idx, SelectionFilterType filter);
   private slots:
      void initTestCase();
      void filterDynamic()          { testFilter(1,SelectionFilterType::DYNAMIC); }
      void filterArticulation()     { testFilter(2,SelectionFilterType::ARTICULATION); }
      void filterLyrics()           { testFilter(3,SelectionFilterType::LYRICS); }
      void filterFingering()        { testFilter(4,SelectionFilterType::FINGERING); }
      void filterChordSymbol()      { testFilter(5,SelectionFilterType::CHORD_SYMBOL); }
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestSelectionFilter::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   testFilter
//---------------------------------------------------------

void TestSelectionFilter::testFilter(int idx, SelectionFilterType filter)
      {
      Score* score = readScore(DIR + QString("selectionfilter%1.mscx").arg(idx));
      Measure* m1 = score->firstMeasure();

      QVERIFY(m1 != 0);

      score->select(m1);
      score->selectionFilter().setFiltered(filter,false);

      QVERIFY(score->selection().canCopy());
      QString mimeType = score->selection().mimeType();
      QVERIFY(mimeType == mimeStaffListFormat);

      //qDebug("%s",score->selection().mimeData().data());

      QVERIFY(saveCompareMimeData(score->selection().mimeData(),QString("selectionfilter%1.xml").arg(idx),
         DIR + QString("selectionfilter%1-ref.xml").arg(idx)));
      }

QTEST_MAIN(TestSelectionFilter)
#include "tst_selectionfilter.moc"

