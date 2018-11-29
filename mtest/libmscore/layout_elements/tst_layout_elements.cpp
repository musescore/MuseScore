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
#include "mtest/testutils.h"
#include "libmscore/measure.h"
#include "libmscore/page.h"
#include "libmscore/score.h"
#include "libmscore/staff.h"
#include "libmscore/system.h"
#include "libmscore/tuplet.h"

#define DIR QString("libmscore/layout_elements/")

using namespace Ms;

//---------------------------------------------------------
//   TestBechmark
//---------------------------------------------------------

class TestLayoutElements : public QObject, public MTest
      {
      Q_OBJECT

      MasterScore* score;
      void beam(const char* path);
      void tstLayoutAll(QString file);

   private slots:
      void initTestCase();
      void tstLayoutElements()  { tstLayoutAll("layout_elements.mscx"); }
      void tstLayoutTablature() { tstLayoutAll("layout_elements_tab.mscx"); }
      void tstLayoutMoonlight() { tstLayoutAll("moonlight.mscx");       }
      // FIXME goldberg.mscx does not pass the test because of some
      // TimeSig and Clef elements. Need to check it later!
//       void tstLayoutGoldberg()  { tstLayoutAll("goldberg.mscx");        }
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestLayoutElements::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   isLayoutDone
//    For use with Score::scanElements in tstLayoutAll
//    sets data (treated as bool*) to false if element is
//    not laid out, otherwise doesn't change the value of
//    data.
//---------------------------------------------------------

static void isLayoutDone(void* data, Element* e)
      {
      bool* result = static_cast<bool*>(data);
      if (e->isTuplet()) {
            Tuplet* t = toTuplet(e);
            if (!t->hasBracket() || !t->number())
                  // in this case tuplet will not have valid bbox.
                  // TODO: how to check this case?
                  return;
            }
      if (e->isTimeSig()) {
            const Staff* st = e->staff();
            if (!st->staffType(e->tick())->genTimesig()) {
                  // Some staff types require not to have a time
                  // signature displayed. This is a valid exception.
                  return;
                  }
            }
      // If layout of element is done it (usually?) has a valid
      // bounding box (bbox).
      if (e->visible() && !e->bbox().isValid()) {
            (*result) = false;
            // Print some info about the element to make test more useful...
            if (Measure* m = toMeasure(e->findMeasure())) {
                  qDebug("Layout of %s is not done (page %d, measure %d)", e->name(), m->system()->page()->no() + 1, m->no() + 1);
                  }
            else
                  qDebug("Layout of %s is not done", e->name());
            }
      }

//---------------------------------------------------------
//   tstLayoutAll
//    Test that all elements in the score are laid out
//---------------------------------------------------------

void TestLayoutElements::tstLayoutAll(QString file)
      {
      MasterScore* score = readScore(DIR + file);
      // readScore should also do layout of the score

      for (LayoutMode mode : { LayoutMode::PAGE }) {
            score->setLayoutMode(mode);
            bool layoutDone = true;
            for (Score* s : score->scoreList()) {
                  s->scanElements(&layoutDone, isLayoutDone, /* all */ true);
                  QVERIFY(layoutDone);
                  }
            }
      }

QTEST_MAIN(TestLayoutElements)
#include "tst_layout_elements.moc"

