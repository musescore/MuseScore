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
#include "libmscore/excerpt.h"
#include "libmscore/score.h"
#include "libmscore/spanner.h"

#define DIR QString("libmscore/remove/")

using namespace Ms;

//---------------------------------------------------------
//   TestRemove
//---------------------------------------------------------

class TestRemove : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void removeStaff();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestRemove::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   StaffCheckData
//    For passing to the defined below inStaff() function.
//---------------------------------------------------------

struct StaffCheckData {
      int staffIdx;
      bool staffHasElements;
      };

//---------------------------------------------------------
//   inStaff
//    for usage with Score::scanElements to check whether
//    the element belongs to a staff with a certain number.
//---------------------------------------------------------

static void inStaff(void* staffCheckData, Element* e)
      {
      StaffCheckData* checkData = static_cast<StaffCheckData*>(staffCheckData);
      if (e->staffIdx() == checkData->staffIdx) {
            qDebug() << e->name() << "is in staff" << checkData->staffIdx;
            checkData->staffHasElements = true;
            }
      }

//---------------------------------------------------------
//   staffHasElements
//---------------------------------------------------------

static bool staffHasElements(Score* score, int staffIdx)
      {
      for (auto i = score->spannerMap().cbegin(); i != score->spannerMap().cend(); ++i) {
            Spanner* s = i->second;
            if (s->staffIdx() == staffIdx) {
                  qDebug() << s->name() << "is in staff" << staffIdx;
                  return true;
                  }
            }
      for (Spanner* s : score->unmanagedSpanners()) {
            if (s->staffIdx() == staffIdx) {
                  qDebug() << s->name() << "is in staff" << staffIdx;
                  return true;
                  }
            }
      StaffCheckData checkData { staffIdx, false };
      score->scanElements(&checkData, inStaff, true);
      return checkData.staffHasElements;
      }

//---------------------------------------------------------
//   removeStaff
//    Checks that after a staff removal all elements
//    belonging to it are removed in all parts.
//---------------------------------------------------------

void TestRemove::removeStaff()
      {
      MasterScore* score = readScore(DIR + "remove_staff.mscx");

      // Remove the second staff and see what happens
      score->startCmd();
      score->cmdRemoveStaff(1);
      score->endCmd();

      QVERIFY(!staffHasElements(score, 1));
      for (Excerpt* ex : score->excerpts()) {
            Score* s = ex->partScore();
            QVERIFY(!staffHasElements(s, 1));
            }

      delete score;
      }

QTEST_MAIN(TestRemove);
#include "tst_remove.moc"

