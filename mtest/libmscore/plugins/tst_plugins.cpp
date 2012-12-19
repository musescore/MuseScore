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
#include "libmscore/mscore.h"
#include "mscore/qmlplugin.h"

#define DIR QString("/libmscore/plugins/")

//---------------------------------------------------------
//   TestPlugins
//---------------------------------------------------------

class TestPlugins : public QObject, public MTest
      {
      Q_OBJECT

      Score* score;
      QDeclarativeEngine engine;

   private slots:
      void initTestCase();
      void test1();
      void test2();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestPlugins::initTestCase()
      {
      initMTest();
      qmlRegisterType<MScore>    ("MuseScore", 1, 0, "MScore");
      qmlRegisterType<QmlPlugin> ("MuseScore", 1, 0, "MuseScore");
      }

//---------------------------------------------------------
//   test1
//---------------------------------------------------------

void TestPlugins::test1()
      {
      QString path = root + DIR + "test1.qml";
      QDeclarativeComponent component(&engine,
         QUrl::fromLocalFile(path));
      QObject* object = component.create();
      QVERIFY(object != 0);
      if (object == 0) {
            qDebug("creating component <%s> failed", qPrintable(path));
            foreach(QDeclarativeError e, component.errors())
                  qDebug("   line %d: %s", e.line(), qPrintable(e.description()));
            }
      else {
            qreal x = object->property("x").toDouble();
            qreal y = object->property("y").toDouble();
            QCOMPARE(x, 50.0);
            QCOMPARE(y, 60.0);
            }
      delete object;
      }

//---------------------------------------------------------
//   test2
//---------------------------------------------------------

void TestPlugins::test2()
      {
      QString path = root + DIR + "test2.qml";
      QDeclarativeComponent component(&engine,
         QUrl::fromLocalFile(path));
      QObject* object = component.create();
      QVERIFY(object != 0);
      if (object == 0) {
            qDebug("creating component <%s> failed", qPrintable(path));
            foreach(QDeclarativeError e, component.errors())
                  qDebug("   line %d: %s", e.line(), qPrintable(e.description()));
            }
      else {
            qreal width = object->property("width").toDouble();
            qreal height = object->property("height").toDouble();
            QCOMPARE(width, 150.0);
            QCOMPARE(height, 75.0);
            }
      delete object;
      }

QTEST_MAIN(TestPlugins)
#include "tst_plugins.moc"

