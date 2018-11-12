//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "mscore/script/script.h"

#define DIR QString("testscript/")

using namespace Ms;

//---------------------------------------------------------
//   TestScripts
//---------------------------------------------------------

class TestScripts : public QObject, public MTest
      {
      Q_OBJECT

      QString scriptsPath;

   private slots:
      void initTestCase();
      void runTestScripts();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestScripts::initTestCase()
      {
      initMTest();
      scriptsPath = root + '/' + DIR + "scripts";
      }

//---------------------------------------------------------
//   runTestScripts
//---------------------------------------------------------

void TestScripts::runTestScripts()
      {
      Q_ASSERT(QDir::setCurrent(scriptsPath));

      QDir cwd = QDir::current();
      QStringList nameFilters({ "*.script" });
      cwd.setNameFilters(nameFilters);
      cwd.setFilter(QDir::Files);
      cwd.setSorting(QDir::Name);
      QStringList scripts = cwd.entryList();

      QStringList args({ "--run-test-script" });
      args << scripts;

      if (!QFileInfo(MSCORE_EXECUTABLE).exists())
            qFatal("Cannot find executable: %s", MSCORE_EXECUTABLE);
      QVERIFY(QProcess::execute(MSCORE_EXECUTABLE, args) == 0);
      }

QTEST_MAIN(TestScripts)
#include "tst_runscripts.moc"
