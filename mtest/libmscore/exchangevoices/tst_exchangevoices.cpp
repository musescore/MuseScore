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

#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"
#include "libmscore/chord.h"
#include "libmscore/segment.h"

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
      void undoChangeVoice();
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

//---------------------------------------------------------
//   undoChangeVoice
//---------------------------------------------------------

void TestExchangevoices::undoChangeVoice()
      {
      QString readFile(DIR + "undoChangeVoice.mscx");
      QString writeFile1("undoChangeVoice01-test.mscx");
      QString reference1(DIR  + "undoChangeVoice01-ref.mscx");
      QString writeFile2("undoChangeVoice02-test.mscx");
      QString reference2(DIR  + "undoChangeVoice02-ref.mscx");

      MasterScore* score = readScore(readFile);
      score->doLayout();

      // do
      score->deselectAll();
      // select bottom note of all voice 1 chords
      for (Segment* s = score->firstSegment(SegmentType::ChordRest); s; s = s->next1()) {
            ChordRest* cr = static_cast<ChordRest*>(s->element(0));
            if (cr && cr->type() == ElementType::CHORD) {
                  Ms::Chord* c = static_cast<Ms::Chord*>(cr);
                  score->select(c->downNote(), SelectType::ADD);
                  }
            }
      // change voice
      score->changeVoice(1);
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // undo
      score->undoStack()->undo(&ed);
      QVERIFY(saveCompareScore(score, writeFile2, reference2));

      delete score;
      }

QTEST_MAIN(TestExchangevoices)
#include "tst_exchangevoices.moc"
