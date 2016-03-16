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
#include "mtest/testutils.h"
#include "libmscore/score.h"
#include "libmscore/musescoreCore.h"
#include "mscore/preferences.h"
#include "mscore/qmlplugin.h"

#define DIR QString("scripting/")

using namespace Ms;

//---------------------------------------------------------
//   TestScripting
//---------------------------------------------------------

class TestScripting : public QObject, public MTest
      {
      Q_OBJECT

      void read1(const QString&, const QString&);

   private slots:
      void initTestCase();
      void test1() { read1("s1", "p1"); }       // scan note rest
      void test2() { read1("s2", "p2"); }       // scan segment attributes
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
//   read1
//   read a score, apply script and compare script output with
//    reference
//---------------------------------------------------------

void TestScripting::read1(const QString& file, const QString& script)
      {
      Score* score = readScore(DIR + file + ".mscx");
      MuseScoreCore::mscoreCore->setCurrentScore(score);

      QVERIFY(score);
      score->doLayout();

      QQmlEngine* engine = Ms::MScore::qml();
      QVERIFY(engine);

      QString scriptPath = root + "/" + DIR + script + ".qml";

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

      QVERIFY(compareFiles(script + ".log", DIR + script + ".log.ref"));
      delete score;
      }

QTEST_MAIN(TestScripting)
#include "tst_scripting.moc"
