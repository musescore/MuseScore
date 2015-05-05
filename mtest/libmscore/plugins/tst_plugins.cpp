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

using namespace Ms;

//---------------------------------------------------------
//   TestPlugins
//---------------------------------------------------------

class TestPlugins : public QObject, public MTest
      {
      Q_OBJECT

      Score* score;
      QQmlEngine engine;

   private slots:
      void initTestCase();
      void plugins01();
      void plugins02();
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
///   plugin01
///   Create a QML item and retrieve its coordinates
//---------------------------------------------------------

void TestPlugins::plugins01()
      {
      QString path = root + DIR + "plugins01.qml";
      QQmlComponent component(&engine,
         QUrl::fromLocalFile(path));
      QObject* object = component.create();
      //QVERIFY(object != 0);
      if (object == 0) {
            qDebug("creating component <%s> failed", qPrintable(path));
            for (QQmlError e : component.errors())
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
///   plugin02
///   Create a MuseScore plugin and get width and height of the dialog
//---------------------------------------------------------

void TestPlugins::plugins02()
      {
      QString path = root + DIR + "plugins02.qml";
      QQmlComponent component(&engine,
         QUrl::fromLocalFile(path));
      QObject* object = component.create();
      //QVERIFY(object != 0);
      if (object == 0) {
            qDebug("creating component <%s> failed", qPrintable(path));
            for (QQmlError e : component.errors())
                  qDebug("   line %d: %s", e.line(), qPrintable(e.description()));
            }
      else {
            qreal width  = object->property("width").toDouble();
            qreal height = object->property("height").toDouble();
            QCOMPARE(width, 150.0);
            QCOMPARE(height, 75.0);
            }
      delete object;
      }

QTEST_MAIN(TestPlugins)
#include "tst_plugins.moc"

