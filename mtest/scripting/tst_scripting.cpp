//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>
#include "libmscore/measure.h"
#include "libmscore/musescoreCore.h"
#include "libmscore/score.h"
#include "mscore/preferences.h"
#include "mscore/qmlplugin.h"
#include "mtest/testutils.h"

#define DIR QString("scripting/")

using namespace Ms;

//---------------------------------------------------------
//   TestScripting
//---------------------------------------------------------

class TestScripting : public QObject, public MTest
      {
      Q_OBJECT

      void readTest1(const char*scoreFName, const char*scriptFName, const char* logFName);
      void read1(const char* scoreFName);
      void doTest(Score* score, const char* scriptFName, const char* logFName);

   private slots:
      void initTestCase();
      void test1() { readTest1("s1", "p1", "p1"); }
      void test2();           // cursor seeking methods without any selection
      void test3();           // cursor seeking methods with middle selection on second staff
      void test4();           // cursor seeking methods with selection up to score end
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestScripting::initTestCase()
      {
      initMTest();
      qmlRegisterType<QmlPlugin>  ("MuseScore", 1, 0, "MuseScore");
      }

//---------------------------------------------------------
//   test2
//   test cursor seeking methods (scoreStart(), scoreEnd(), selectionStart() and selectionEnd() )
//    in absence of any selection
//---------------------------------------------------------

void TestScripting::test2()
      {
      // load a score with two staves and three measures
      read1("cursor_seek");
      Score*      score = MuseScoreCore::mscoreCore->currentScore();

      // apply script and test
      doTest(score, "cursor_seek2", "cursor_seek2");
      }

//---------------------------------------------------------
//   test3
//   test cursor seeking methods (scoreStart(), scoreEnd(), selectionStart() and selectionEnd() )
//    with a middle selection on second staff
//---------------------------------------------------------

void TestScripting::test3()
      {
      // load a score with two staves and three measures
      read1("cursor_seek");
      Score*      score = MuseScoreCore::mscoreCore->currentScore();

      // from second rest of first measure
      Measure*    msr   = score->firstMeasure();
      QVERIFY(msr);
      Segment* segFrom  = msr->first(Segment::Type::ChordRest);
      segFrom     = segFrom->next1();
      // to beyond first rest of second measure
      msr = msr->nextMeasure();
      QVERIFY(msr);
      Segment* segTo    = msr->first(Segment::Type::ChordRest);
      segTo       = segTo->next1();
      Selection&  sel   = score->selection();

      sel.setStartSegment(segFrom);
      sel.setEndSegment(segTo);
      sel.setStaffStart(1);               // select staff no. 1 (2nd staff)
      sel.setStaffEnd(2);
      sel.setState(SelState::RANGE);

      // apply script and test
      doTest(score, "cursor_seek3", "cursor_seek3");
      }

//---------------------------------------------------------
//   test4
//   test cursor seeking methods (scoreStart(), scoreEnd(), selectionStart() and selectionEnd() )
//    with a selection on second staff up to score end
//---------------------------------------------------------

void TestScripting::test4()
      {
      // load a score with two staves and three measures
      read1("cursor_seek");
      Score*      score = MuseScoreCore::mscoreCore->currentScore();

      // 'go' to third measure
      Measure*    msr   = score->firstMeasure();
      QVERIFY(msr);
      msr = msr->nextMeasure();
      QVERIFY(msr);
      msr = msr->nextMeasure();
      QVERIFY(msr);

      // select last measure of second staff
      Selection&  sel   = score->selection();
      Segment* segFrom  = msr->first();
      sel.setStartSegment(segFrom);
      sel.setEndSegment(nullptr);         // beyond end of score
      sel.setStaffStart(1);               // select staff no. 1 (2nd staff)
      sel.setStaffEnd(2);
      sel.setState(SelState::RANGE);

      // apply script and test
      doTest(score, "cursor_seek4", "cursor_seek4");
      }

//---------------------------------------------------------
//   read1
//   read a score, make it current and return it, if successful
//---------------------------------------------------------

void TestScripting::read1(const char* scoreFName)
      {
      Score* score = readScore(DIR + scoreFName + ".mscx");
      MuseScoreCore::mscoreCore->setCurrentScore(score);

      QVERIFY(score);
      score->doLayout();
      }

//---------------------------------------------------------
//   doTest
//   apply script to current score and compare script output with reference
//---------------------------------------------------------

void TestScripting::doTest(Score* score, const char* scriptFName, const char* logFName)
      {
      QQmlEngine* engine = Ms::MScore::qml();
      QVERIFY(engine);

      QString scriptPath = root + "/" + DIR + scriptFName + ".qml";

      QFileInfo fi(scriptPath);
      QVERIFY(fi.exists());

      QQmlComponent component(engine);
      component.loadUrl(QUrl::fromLocalFile(scriptPath));
      if (component.isError()) {
            for (QQmlError e : component.errors()) {
                  qDebug("qml error: %s", qPrintable(e.toString()));
                  }
            }

      QObject* obj = component.create();
      QVERIFY(obj);

      QmlPlugin* item = qobject_cast<QmlPlugin*>(obj);
      item->runPlugin();

      QVERIFY(compareFiles(QString(logFName) + ".log", DIR + logFName + ".log.ref"));
      delete score;
      }

//---------------------------------------------------------
//   readTest1
//   read a score, apply script and compare script output with reference
//---------------------------------------------------------

void TestScripting::readTest1(const char* scoreFName, const char* scriptFName, const char* logFName)
      {
      read1(scoreFName);
      Score* score = MuseScoreCore::mscoreCore->currentScore();
      doTest(score, scriptFName, logFName);
      }

QTEST_MAIN(TestScripting)
#include "tst_scripting.moc"
