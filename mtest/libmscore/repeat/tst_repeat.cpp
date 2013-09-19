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
#include "libmscore/repeatlist.h"

#define DIR QString("libmscore/repeat/")

using namespace Ms;

//---------------------------------------------------------
//   TestRepeat
//---------------------------------------------------------

class TestRepeat : public QObject, public MTest
      {
      Q_OBJECT
      void repeat(const char* f1, const QString & ref);

   private slots:
      void initTestCase();
      void repeat01() { repeat("repeat01.mscx", "1;2;3;2;3;4;5;6"); }
      void repeat02() { repeat("repeat02.mscx", "1;2;2;3;4;5;6"); }
      void repeat03() { repeat("repeat03.mscx", "1;2;1;2;3;4;5;6"); }
      void repeat04() { repeat("repeat04.mscx", "1;2;3;2;3;4;2;3;4;5;6"); }
      void repeat05() { repeat("repeat05.mscx", "1;2;3;2;3;2;3;4;2;3;4;5;6"); }
      void repeat06() { repeat("repeat06.mscx", "1;2;3;2;4;5;6"); }
      void repeat07() { repeat("repeat07.mscx", "1;2;3;4;5;6;1;2;3"); }
      void repeat08() { repeat("repeat08.mscx", "1;2;3;4;5;6;2;3;4;7;8;9;10;11"); }

      void repeat09() { repeat("repeat09.mscx", "1;2;3;2;4;2;5;6"); }
      void repeat10() { repeat("repeat10.mscx", "1;2;3;4;1;2;5;6;7;8;1;2;9;10;1;2;11;12"); }
      void repeat11() { repeat("repeat11.mscx", "1;2;3;4;2;3;5;6;7;8;2;9;10"); }
      void repeat12() { repeat("repeat12.mscx", "1;2;3;4;3;5;6;2;3;5;6;7"); }
      void repeat13() { repeat("repeat13.mscx", "1;2;3;4;5"); }
      void repeat14() { repeat("repeat14.mscx", "1;2;3;4;5;6;7;8;9;10; 2;3;4;5;6;7;8;11;12; 2;3;4;5;6;7;8;13;14;15; 16;17;18; 16;17;18; 19;20;21;22;23; 5;6;7; 24;25;26"); }
      };


//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestRepeat::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestRepeat::repeat(const char* f1, const QString & ref)
      {
      Score* score = readScore(DIR + f1);
      score->doLayout();
      QVERIFY(score);
      score->updateRepeatList(true);
      QStringList sl;
      foreach (const RepeatSegment* rs, *score->repeatList()) {
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len;
            for (Measure* m = score->tick2measure(startTick); m; m = m->nextMeasure()) {
                  sl.append(QString::number(m->no()+1));
                  if (m->tick() + m->ticks() >= endTick)
                        break;
                  }
            }
      QString s = sl.join(";");
      QString ref1 = ref;
      ref1.replace(" ","");
      qDebug("sequence %s", qPrintable(s));
      QCOMPARE(s, ref1);
      delete score;
      }


QTEST_MAIN(TestRepeat)
#include "tst_repeat.moc"
