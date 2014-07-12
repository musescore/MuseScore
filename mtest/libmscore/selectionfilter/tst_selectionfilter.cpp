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
      void testFilterSpanner(int idx, SelectionFilterType filter);
   private slots:
      void initTestCase();
      void filterDynamic()          { testFilter(1,SelectionFilterType::DYNAMIC); }
      void filterArticulation()     { testFilter(2,SelectionFilterType::ARTICULATION); }
      void filterLyrics()           { testFilter(3,SelectionFilterType::LYRICS); }
      void filterFingering()        { testFilter(4,SelectionFilterType::FINGERING); }
      void filterChordSymbol()      { testFilter(5,SelectionFilterType::CHORD_SYMBOL); }
      void filterSlur()             { testFilter(6,SelectionFilterType::SLUR); }
      void filterFiguredBass()      { testFilter(7,SelectionFilterType::FIGURED_BASS); }
      void filterOttava()           { testFilter(8,SelectionFilterType::OTTAVA); }
      void filterPedalLine()        { testFilter(9,SelectionFilterType::PEDAL_LINE); }
      void filterArpeggio()         { testFilter(10,SelectionFilterType::ARPEGGIO); }
      void filterFretDiagram()      { testFilter(11,SelectionFilterType::FRET_DIAGRAM); }
      void filterGlissando()        { testFilter(12,SelectionFilterType::GLISSANDO); }
      void filterBreath()           { testFilter(13,SelectionFilterType::BREATH); }
      void filterOtherText()        { testFilter(14,SelectionFilterType::OTHER_TEXT); }
      void filterOtherLine()        { testFilterSpanner(15,SelectionFilterType::OTHER_LINE); }
      void filterTremolo()          { testFilter(16,SelectionFilterType::TREMOLO); }
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

      QVERIFY(score->selection().canCopy());
      QVERIFY(score->selection().mimeType() == mimeStaffListFormat);

      QVERIFY(saveCompareMimeData(score->selection().mimeData(),QString("selectionfilter%1-base.xml").arg(idx),
         DIR + QString("selectionfilter%1-base-ref.xml").arg(idx)));

      score->selectionFilter().setFiltered(filter,false);

      QVERIFY(score->selection().canCopy());
      QVERIFY(score->selection().mimeType() == mimeStaffListFormat);

      QVERIFY(saveCompareMimeData(score->selection().mimeData(),QString("selectionfilter%1.xml").arg(idx),
         DIR + QString("selectionfilter%1-ref.xml").arg(idx)));
      }

void TestSelectionFilter::testFilterSpanner(int idx, SelectionFilterType filter)
      {
      Score* score = readScore(DIR + QString("selectionfilter%1.mscx").arg(idx));
      Measure* m1 = score->firstMeasure();
      Measure* m2 = score->firstMeasure()->nextMeasure();

      QVERIFY(m1 != 0 && m2 != 0);

      score->select(m1);
      score->select(m2,SelectType::RANGE);

      QVERIFY(score->selection().canCopy());
      QVERIFY(score->selection().mimeType() == mimeStaffListFormat);

      QVERIFY(saveCompareMimeData(score->selection().mimeData(),QString("selectionfilter%1-base.xml").arg(idx),
         DIR + QString("selectionfilter%1-base-ref.xml").arg(idx)));

      score->selectionFilter().setFiltered(filter,false);

      QVERIFY(score->selection().canCopy());
      QVERIFY(score->selection().mimeType() == mimeStaffListFormat);

      QVERIFY(saveCompareMimeData(score->selection().mimeData(),QString("selectionfilter%1.xml").arg(idx),
         DIR + QString("selectionfilter%1-ref.xml").arg(idx)));
      }

QTEST_MAIN(TestSelectionFilter)
#include "tst_selectionfilter.moc"

