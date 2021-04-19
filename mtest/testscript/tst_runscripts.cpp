/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
    QSKIP("Temporarily disabled tests for the time of refactoring commands in MS4");
    // needed because all.h disables Q_ASSERT ifdef QT_NO_DEBUG
    bool did_cwd = QDir::setCurrent(scriptsPath);
    Q_ASSERT(did_cwd);

    QDir cwd = QDir::current();
    QStringList nameFilters({ "*.script" });
    cwd.setNameFilters(nameFilters);
    cwd.setFilter(QDir::Files);
    cwd.setSorting(QDir::Name);
    QStringList scripts = cwd.entryList();

    QStringList args({ "--run-test-script" });
    args << scripts;

    if (!QFileInfo(MSCORE_EXECUTABLE_PATH).exists()) {
        qFatal("Cannot find executable: %s", MSCORE_EXECUTABLE_PATH);
    }
    QVERIFY(QProcess::execute(MSCORE_EXECUTABLE_PATH, args) == 0);
}

QTEST_MAIN(TestScripts)
#include "tst_runscripts.moc"
