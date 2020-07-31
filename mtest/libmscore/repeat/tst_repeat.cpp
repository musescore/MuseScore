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
      void repeat01() { repeat("repeat01.mscx", "1;2;3; 2;3;4;5;6"); }        // repeat barline 2 measures ||: | :||
      void repeat02() { repeat("repeat02.mscx", "1;2; 2;3;4;5;6"); }          // repeat barline 1 measure ||: :||
      void repeat03() { repeat("repeat03.mscx", "1;2; 1;2;3;4;5;6"); }        // repeat barline end to start :||
      void repeat04() { repeat("repeat04.mscx", "1;2;3; 2;3;4; 2;3;4;5;6"); }  // repeat barline ||: | :|| :||
      void repeat05() { repeat("repeat05.mscx", "1;2;3; 2;3; 2;3;4; 2;3;4;5;6"); } // repeat barline ||: | x2 :|| :||
      void repeat06() { repeat("repeat06.mscx", "1;2;3; 2; 4;5;6"); }             // simple volta
      void repeat07() { repeat("repeat07.mscx", "1;2;3;4;5;6; 1;2;3"); }         // DC al fine
      void repeat08() { repeat("repeat08.mscx", "1;2;3;4;5;6; 2;3;4; 7;8;9;10;11"); } // DS al coda

      void repeat09() { repeat("repeat09.mscx", "1;2;3; 2; 5;6"); }                        // 3 voltas, but twice a volta1
      void repeat10() { repeat("repeat10.mscx", "1;2;3;4; 1;2; 5;6;7;8; 1;2; 9;10; 1;2; 11;12"); } // 3 voltas
      void repeat11() { repeat("repeat11.mscx", "1;2;3;4; 2;3; 5;6;7;8; 2; 9;10"); }  // volta after to coda
      void repeat12() { repeat("repeat12.mscx", "1;2;3;4; 3; 5;6; 2;3; 5;6;7"); }     // volta between segno & DS
      void repeat13() { repeat("repeat13.mscx", "1;2;3;4;5; 1;2;3;4;5"); }                   // incomplete jump -> carry on as good as you can
      void repeat14() { repeat("repeat14.mscx", "1;2;3;4;5;6;7;8;9;10; 2;3;4;5;6;7;8; 11;12; 2;3;4;5;6;7;8; 13;14;15;16;17;18; 16;17;18; 19;20;21;22;23; 5;6;7; 24;25;26"); } // complex roadmap DS al coda, volta, repeat
      void repeat15() { repeat("repeat15.mscx", "1;2; 2; 2; 2; 2; 2; 2; 2;3"); } // repeat barline ||: x8 :||

      void repeat16() { repeat("repeat16.mscx", "1;2;3;4; 4; 1;2"); } // jump in simple repeat
      void repeat17() { repeat("repeat17.mscx", "1;2; 1; 3;4;5; 4; 6;7;8; 7; 9"); } // volta after coda

      void repeat18() { repeat("repeat18.mscx", "1;2; 1; 3;4;5;6; 5; 7;8"); } // twice volta
      void repeat19() { repeat("repeat19.mscx", "1;2;3;4; 1;2; 4"); } // DS al coda after the coda

      void repeat20() { repeat("repeat20.mscx", "1;2;3; 1; 4;5;6;7;8; 5;6"); } // 1) DS al Coda, 2) DS1 al Fine
      void repeat21() { repeat("repeat21.mscx", "1;2;3; 1;2;3;4;5;6;7; 5; 8"); } // 1) DS, 2) DS1 al Coda

      void repeat22() { repeat("repeat22.mscx", "1;2;3; 2;3;4;5; 5; 5;6"); } // DS and ||: 3x :||
      // complex roadmap
      void repeat23() { repeat("repeat23.mscx", "1;2; 1;2;3; 2;3;4;5;6;7; 6;7;8;9;10;11; 9;10; 12; 12;13;14; 13;14;15;16; 13;14"); }

      void repeat24() { repeat("repeat24.mscx", "1;2;3;4;5; 3;4; 2;3;4;5;6"); } // S ||: DS :||
      void repeat25() { repeat("repeat25.mscx", "1;2; 1;2;3;4;5; 4; 2;3;4;5"); } // ||: S :||  ||: DS :||

      void repeat26() { repeat("repeat26.mscx", "1; 1;2; 2;3"); } // empty and garbage jump

      void repeat27() { repeat("repeat27.mscx", "1;2; 2; 1"); }        // #73486 single-measure repeat at end of section
      void repeat28() { repeat("repeat28.mscx", "1;2; 2; 1;2; 1"); }   // #73486 single-measure repeat at end of section w/DC
      void repeat29() { repeat("repeat29.mscx", "1;2;3; 3; 2;3; 1"); } // #73486 single-measure repeat at end of section w/DS

      void repeat30() { repeat("repeat30.mscx", "1; 1;2; 1;2"); }     // #73496 single measure section at beginning of score followed by a section with end repeat (without beginning repeat)

      void repeat31() { repeat("repeat31.mscx", "1;2; 2; 1;2"); }               // #73531 ending measure has jump and repeat m1 |: m2 DC :|
      void repeat32() { repeat("repeat32.mscx", "1;2;3; 3; 2;3"); }             // #73531 ending measure has jump and repeat m1 |S m2 |: m3 DS :|
      void repeat33() { repeat("repeat33.mscx", "1;2;3; 2;3; 1;2;3"); }         // #73531 ending measure has jump and repeat m1 |: m2 | m3 DC :|
      void repeat34() { repeat("repeat34.mscx", "1;2;3; 2; 4;5; 5; 1;2; 4;5"); }// #73531 ending measure has jump and repeat m1 |: m2 |1e m3 :| 2e m4 |: m5 | DC :|

      void repeat35() { repeat("repeat35.mscx", "1;2; 1;2; 1;2;3;4; 2;3; 1;2;3; 1;2; 1;2;3;4; 2;3; 5"); } // #65161 multiple sections, each with possible DC, DS, al Fine
      void repeat36() { repeat("repeat36.mscx", "1;2;3;4;5; 1;2;3"); } // incomplete jump, missing Coda from D.S. al Coda

      void repeat37() { repeat("repeat37.mscx", "1;2;1; 1;2;1; 1;2;1; 1;2;1; 1;2;1"); } // #65161, with section breaks occurring on non-measure frames
      void repeat38() { repeat("repeat38.mscx", "1;2; 2;3;4; 2; 2;3; 5"); } // D.S. with repeats at start repeat

      void repeat39() { repeat("repeat39.mscx", "1;2;3; 2; 4;5; 2;3; 2; 4;5; 2; 6"); } // volta 1.3.,2.4.

      void repeat40() { repeat("repeat40.mscx", "1;2; 1; 3;4; 1; 5"); } // #148276 Open volta lines must extend over entire ending: next endRepeat
      void repeat41() { repeat("repeat41.mscx", "1;2; 1; 3; 1; 4;5"); } // #148276 Open volta lines must extend over entire ending: next endRepeat at volta (SLine) end
      void repeat42() { repeat("repeat42.mscx", "1;2; 1; 3; 5; 1; 4;5"); } // #148276 Open volta lines must extend over entire ending: next volta
      void repeat43() { repeat("repeat43.mscx", "1;2; 1;2; 1; 3; 1;2; 1; 4; 1; 5"); } // #8604 Complex alternate endings 1.2.4. / 3./ 5.
      void repeat44() { repeat("repeat44.mscx", "1;2;3;4;5; 3; 6; 3;4; 1; 7"); } // Jump from within a volta

      void repeat45() { repeat("repeat45.mscx", "1;2;3;4; 3; 5;6; 2;3;4; 3; 5;6;7"); } // repeat12 but with 'play repeats' enabled
      void repeat46() { repeat("repeat46.mscx", "1;2;3;4;5; 3;4; 2;3;4;5; 3;4;5;6"); } // repeat24 but with 'play repeats' enabled

      void repeat47() { repeat("repeat47.mscx", "1;2;3; 2; 4;5;6;7; 6; 8;9;10;11; 2; 4; 13;14;15; 9;10; 16;17;18"); } // #269378 Double Coda messed up repeat rewind logic

      void repeat48() { repeat("repeat48.mscx", "1;2;3;4; 1; 5;6;7; 3;4; 1; 5;8"); } // jump into first volta, without playRepeats
      void repeat49() { repeat("repeat49.mscx", "1;2;3; 1;2;3;4;5;6; 3; 1;2;3;4; 7"); } // D.S. with playRepeats
      void repeat50() { repeat("repeat50.mscx", "1;2;3;4; 1;2;3;4;5;6; 1;2;3;4; 1;2;3; 7"); } // D.S. with playRepeats with ToCoda inside the repeat
      void repeat51() { repeat("repeat51.mscx", "1;2;3;4;5;6; 3;4; 7;8;9; 3;4; 10;11"); } // #270332 twice D.S. with playRepeats to same target with different Coda

      void repeat52() { repeat("repeat52.mscx", "1;2;3; 1; 4;5; 1;2;3; 1; 4;5; 1; 6; 3; 1; 4;5; 1; 6"); } // Jump into volta "final" playthrough
      void repeat53() { repeat("repeat53.mscx", "1;2;3; 1; 4;5; 1;2;3; 1; 4;5; 1; 6; 5; 1;2;3; 1; 4;5; 1; 6"); } // Jump into volta with repeats

      void repeat54() { repeat("repeat54.mscx", "1;2;3; 1; 4;5;6;7;8; 5;6"); } // repeat20 without label changes, pick correct segno
      void repeat55() { repeat("repeat55.mscx", "1;2;3; 1;2;3;4;5;6;7; 5; 8"); } // repeat21 without label changes, pick correct segno

      void repeat56() { repeat("repeat56.mscx", "1;2;3;4; 2; 5;6;7; 5;6;7;8"); } // start of volta and start repeat on same measure
      void repeat57() { repeat("repeat57.mscx", "1;2;3"); } // no repeat, skip volta until section end, relates to #274690
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestRepeat::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   repeat
//---------------------------------------------------------

void TestRepeat::repeat(const char* f1, const QString & ref)
      {
      MasterScore* score = readScore(DIR + f1);
      QVERIFY(score);
      score->setExpandRepeats(true);
      QStringList sl;
      for (const RepeatSegment* rs : score->repeatList()) {
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len();
            for (Measure* m = score->tick2measure(Fraction::fromTicks(startTick)); m; m = m->nextMeasure()) {
                  sl.append(QString::number(m->no()+1));
                  if (m->endTick().ticks() >= endTick)
                        break;
                  }
            }
      QString s = sl.join(";");
      QString ref1 = ref;
      ref1.replace(" ","");
      qDebug("File <%s> sequence %s", f1, qPrintable(s));
      QCOMPARE(s, ref1);
      delete score;
      }


QTEST_MAIN(TestRepeat)
#include "tst_repeat.moc"
