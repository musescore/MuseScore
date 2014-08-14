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
#include <QFile>
#include <QCoreApplication>
#include <QTextStream>
#include "libmscore/mscore.h"
#include "libmscore/score.h"
#include "libmscore/durationtype.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/keysig.h"
#include "mscore/exportmidi.h"
#include <QIODevice>

#include "libmscore/mcursor.h"
#include "mtest/testutils.h"
#define DIR QString("libmscore/midi/")

namespace Ms {
      extern Score::FileError importMidi(Score*, const QString&);
      }

using namespace Ms;

//---------------------------------------------------------
//   TestMidi
//---------------------------------------------------------

class TestMidi : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void midi01();
      void midi02();
      void midi03();
      void events_data();
      void events();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestMidi::initTestCase()
      {
      initMTest();
      }


void TestMidi::events_data()
      {
      QTest::addColumn<QString>("file");
      // Test Eighth Swing
      QTest::newRow("testSwing8thSimple") <<  "testSwing8thSimple";
      QTest::newRow("testSwing8thTies") <<  "testSwing8thTies";
      QTest::newRow("testSwing8thTriplets") <<  "testSwing8thTriplets";
      QTest::newRow("testSwing8thDots") <<  "testSwing8thDots";
      // Test Sixteenth Swing
      QTest::newRow("testSwing16thSimple") <<  "testSwing16thSimple";
      QTest::newRow("testSwing16thTies") <<  "testSwing16thTies";
      QTest::newRow("testSwing16thTriplets") <<  "testSwing16thTriplets";
      QTest::newRow("testSwing16thDots") <<  "testSwing16thDots";
      QTest::newRow("testSwingOdd") <<  "testSwingOdd";
      QTest::newRow("testSwingPickup") <<  "testSwingPickup";
      // Test Text Cominations
      QTest::newRow("testSwingStyleText") <<  "testSwingStyleText";
      QTest::newRow("testSwingTexts") <<  "testSwingTexts";
      }

//---------------------------------------------------------
//   saveMidi
//---------------------------------------------------------

bool saveMidi(Score* score, const QString& name)
      {
      ExportMidi em(score);
      return em.write(name, true);
      }


//---------------------------------------------------------
//   compareElements
//---------------------------------------------------------

bool compareElements(Element* e1, Element* e2)
      {
      if (e1->type() != e2->type())
            return false;
      if (e1->type() == Element::Type::TIMESIG) {
            }
      else if (e1->type() == Element::Type::KEYSIG) {
            KeySig* ks1 = static_cast<KeySig*>(e1);
            KeySig* ks2 = static_cast<KeySig*>(e2);
            if (ks1->key() != ks2->key()) {
                  qDebug("      key signature %d  !=  %d", ks1->key(), ks2->key());
                  return false;
                  }
            }
      else if (e1->type() == Element::Type::CLEF) {
            }
      else if (e1->type() == Element::Type::REST) {
            }
      else if (e1->type() == Element::Type::CHORD) {
            Ms::Chord* c1 = static_cast<Ms::Chord*>(e1);
            Ms::Chord* c2 = static_cast<Ms::Chord*>(e2);
            if (c1->duration() != c2->duration()) {
                  Fraction f1 = c1->duration();
                  Fraction f2 = c2->duration();
                  qDebug("      chord duration %d/%d  !=  %d/%d",
                     f1.numerator(), f1.denominator(),
                     f2.numerator(), f2.denominator()
                     );
                  return false;
                  }
            if (c1->notes().size() != c2->notes().size()) {
                  qDebug("      != note count");
                  return false;
                  }
            int n = c1->notes().size();
            for (int i = 0; i < n; ++i) {
                  Note* n1 = c1->notes()[i];
                  Note* n2 = c2->notes()[i];
                  if (n1->pitch() != n2->pitch()) {
                        qDebug("      != pitch note %d", i);
                        return false;
                        }
                  if (n1->tpc() != n2->tpc()) {
                        qDebug("      note tcp %d != %d", n1->tpc(), n2->tpc());
                        // return false;
                        }
                  }
            }

      return true;
      }

//---------------------------------------------------------
//   compareScores
//---------------------------------------------------------

bool compareScores(Score* score1, Score* score2)
      {
      int staves = score1->nstaves();
      if (score2->nstaves() != staves) {
            printf("   stave count different %d %d\n", staves, score2->nstaves());
            return false;
            }
      Segment* s1 = score1->firstMeasure()->first();
      Segment* s2 = score2->firstMeasure()->first();

      int tracks = staves * VOICES;
      for (;;) {
            for (int track = 0; track < tracks; ++track) {
                  Element* e1 = s1->element(track);
                  Element* e2 = s2->element(track);
                  if ((e1 && !e2) || (e2 && !e1)) {
                        printf("   elements different\n");
                        return false;
                        }
                  if (e1 == 0)
                        continue;
                  if (!compareElements(e1, e2)) {
                        printf("   %s != %s\n", e1->name(), e2->name());
                        return false;
                        }
                  printf("   ok: %s\n", e1->name());
                  }
            s1 = s1->next1();
            s2 = s2->next1();
            if ((s1 && !s2) || (s2 && !s2)) {
                  printf("   segment count different\n");
                  return false;
                  }
            if (s1 == 0)
                  break;
            }
      return true;
      }

//---------------------------------------------------------
///   midi01
///   write/read midi file with timesig 4/4
//---------------------------------------------------------

void TestMidi::midi01()
      {
      MCursor c;
      c.setTimeSig(Fraction(4,4));
      c.createScore("test1a");
      c.addPart("voice");
      c.move(0, 0);     // move to track 0 tick 0

      c.addKeySig(Key(1));
      c.addTimeSig(Fraction(4,4));
      c.addChord(60, TDuration(TDuration::DurationType::V_QUARTER));
      c.addChord(61, TDuration(TDuration::DurationType::V_QUARTER));
      c.addChord(62, TDuration(TDuration::DurationType::V_QUARTER));
      c.addChord(63, TDuration(TDuration::DurationType::V_QUARTER));
      Score* score = c.score();

      score->doLayout();
      score->rebuildMidiMapping();
      c.saveScore();
      saveMidi(score, "test1.mid");

      Score* score2 = new Score(mscore->baseStyle());
      score2->setName("test1b");
      QCOMPARE(importMidi(score2, "test1.mid"), Score::FileError::FILE_NO_ERROR);

      score2->doLayout();
      score2->rebuildMidiMapping();
      MCursor c2(score2);
      c2.saveScore();

      QVERIFY(compareScores(score, score2));

      delete score;
      delete score2;
      }

//---------------------------------------------------------
///   midi02
///   write/read midi file with timesig 3/4
//---------------------------------------------------------

void TestMidi::midi02()
      {
      MCursor c;
      c.setTimeSig(Fraction(3,4));
      c.createScore("test2a");
      c.addPart("voice");
      c.move(0, 0);     // move to track 0 tick 0

      c.addKeySig(Key(2));
      c.addTimeSig(Fraction(3,4));
      c.addChord(60, TDuration(TDuration::DurationType::V_QUARTER));
      c.addChord(61, TDuration(TDuration::DurationType::V_QUARTER));
      c.addChord(62, TDuration(TDuration::DurationType::V_QUARTER));
      Score* score = c.score();

      score->doLayout();
      score->rebuildMidiMapping();
      c.saveScore();
      saveMidi(score, "test2.mid");

      Score* score2 = new Score(mscore->baseStyle());
      score2->setName("test2b");

      QCOMPARE(importMidi(score2, "test2.mid"), Score::FileError::FILE_NO_ERROR);

      score2->doLayout();
      score2->rebuildMidiMapping();
      MCursor c2(score2);
      c2.saveScore();

      QVERIFY(compareScores(score, score2));

      delete score;
      delete score2;
      }

//---------------------------------------------------------
///   midi03
///   write/read midi file with key sig
//---------------------------------------------------------

void TestMidi::midi03()
      {
      MCursor c;
      c.setTimeSig(Fraction(4,4));
      c.createScore("test3a");
      c.addPart("voice");
      c.move(0, 0);     // move to track 0 tick 0

      c.addKeySig(Key(1));
      c.addTimeSig(Fraction(4,4));
      c.addChord(60, TDuration(TDuration::DurationType::V_QUARTER));
      c.addChord(61, TDuration(TDuration::DurationType::V_QUARTER));
      c.addChord(62, TDuration(TDuration::DurationType::V_QUARTER));
      c.addChord(63, TDuration(TDuration::DurationType::V_QUARTER));
      Score* score = c.score();

      score->doLayout();
      score->rebuildMidiMapping();
      c.saveScore();
      saveMidi(score, "test3.mid");

      Score* score2 = new Score(mscore->baseStyle());
      score2->setName("test3b");
      QCOMPARE(importMidi(score2, "test3.mid"), Score::FileError::FILE_NO_ERROR);

      score2->doLayout();
      score2->rebuildMidiMapping();
      MCursor c2(score2);
      c2.saveScore();

      QVERIFY(compareScores(score, score2));

      delete score;
      delete score2;
      }

//---------------------------------------------------------
//   events
//---------------------------------------------------------

void TestMidi::events()
      {
      QFETCH(QString, file);

      QString readFile(DIR   + file + ".mscx");
      QString writeFile(file + "-test.txt");
      QString reference(DIR + file + "-ref.txt");

      Score* score = readScore(readFile);
      score->doLayout();
      EventMap events;
      score->renderMidi(&events);
      qDebug() << "Opened score " << readFile;
      QFile filehandler(writeFile);
      filehandler.open(QIODevice::WriteOnly | QIODevice::Text);
      QTextStream out(&filehandler);
      multimap<int, NPlayEvent> ::iterator iter;
      for (auto iter = events.begin(); iter!= events.end(); ++iter){
            out << qSetFieldWidth(5) << "Tick  =  ";
            out << qSetFieldWidth(5) << iter->first;
            out << qSetFieldWidth(5) << "   Type  = ";
            out << qSetFieldWidth(5) << iter->second.type();
            out << qSetFieldWidth(5) << "   Pitch  = ";
            out << qSetFieldWidth(5) << iter->second.dataA();
            out << qSetFieldWidth(5) << "   Velocity  = ";
            out << qSetFieldWidth(5) << iter->second.dataB();
            out << qSetFieldWidth(5) << "   Channel  = ";
            out << qSetFieldWidth(5) << iter->second.channel();
            out << endl;
            }
      filehandler.close();

      QVERIFY(score);
      QVERIFY(compareFiles(writeFile, reference));
     // QVERIFY(saveCompareScore(score, writeFile, reference));
      }

QTEST_MAIN(TestMidi)

#include "tst_midi.moc"

