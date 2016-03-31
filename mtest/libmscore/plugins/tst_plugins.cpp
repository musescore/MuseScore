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
#include "libmscore/musescoreCore.h"
#include "libmscore/undo.h"
#include "mscore/qmlplugin.h"

#define DIR QString("libmscore/plugins/")

using namespace Ms;

//---------------------------------------------------------
//   TestPlugins
//---------------------------------------------------------

class TestPlugins : public QObject, public MTest
      {
      Q_OBJECT

      QQmlEngine engine;
      QQmlEngine* MsQmlEngine;

      QmlPlugin* loadPlugin(QString path);

   private slots:
      void initTestCase();
      void plugins01();
      void plugins02();
      void testTextStyle();
      };

//---------------------------------------------------------
///   loadPlugin
///   Loads the qml plugin located at path
///   Returns pointer to the plugin or nullptr upon failure
///   Note: ensure to cleanup the returned pointer
//---------------------------------------------------------
QmlPlugin* TestPlugins::loadPlugin(QString path)
      {
      QQmlComponent component(MsQmlEngine);
      component.loadUrl(QUrl::fromLocalFile(path));
      QObject* obj = component.create();
      if (obj == 0) {
            foreach(QQmlError e, component.errors())
                  qDebug("   line %d: %s", e.line(), qPrintable(e.description()));
            return nullptr;
            }

      return qobject_cast<QmlPlugin*>(obj);
      }

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestPlugins::initTestCase()
      {
      initMTest();
      qmlRegisterType<MScore>    ("MuseScore", 1, 0, "MScore");
      qmlRegisterType<QmlPlugin> ("MuseScore", 1, 0, "MuseScore");
      MsQmlEngine = Ms::MScore::qml();
      }

//---------------------------------------------------------
///   plugin01
///   Create a QML item and retrieve its coordinates
//---------------------------------------------------------

void TestPlugins::plugins01()
      {
      QString path = root + "/" + DIR + "plugins01.qml";
      QQmlComponent component(&engine,
         QUrl::fromLocalFile(path));
      QObject* object = component.create();
      //QVERIFY(object != 0);
      if (object == 0) {
            qDebug("creating component <%s> failed", qPrintable(path));
            foreach(QQmlError e, component.errors())
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
      QString path = root + "/" + DIR + "plugins02.qml";
      QQmlComponent component(&engine,
         QUrl::fromLocalFile(path));
      QObject* object = component.create();
      //QVERIFY(object != 0);
      if (object == 0) {
            qDebug("creating component <%s> failed", qPrintable(path));
            foreach(QQmlError e, component.errors())
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

//---------------------------------------------------------
///   testTextStyle
///   Reading and writing of a text style through the plugin framework
//---------------------------------------------------------

void TestPlugins::testTextStyle()
      {
      QmlPlugin* item = loadPlugin(root + "/" + DIR + "testTextStyle.qml");
      QVERIFY(item != nullptr);

      Score* score = readScore(DIR + "testTextStyle.mscx");
      MuseScoreCore::mscoreCore->setCurrentScore(score);
      item->runPlugin();
      QVERIFY(saveCompareScore(item->curScore(), "testTextStyle-test.mscx", DIR + "testTextStyle-ref.mscx"));
      score->undo()->undo();
      QVERIFY(saveCompareScore(item->curScore(), "testTextStyle-test2.mscx", DIR + "testTextStyle.mscx"));

      delete item;
      }

QTEST_MAIN(TestPlugins)
#include "tst_plugins.moc"

