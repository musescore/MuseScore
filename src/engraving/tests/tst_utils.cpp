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

#include "testing/qtestsuite.h"
#include "testbase.h"
#include "libmscore/utils.h"

using namespace Ms;

//---------------------------------------------------------
//   TestNote
//---------------------------------------------------------

class TestUtils : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase();
    void tst_compareVersion();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestUtils::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
///   test_version
//---------------------------------------------------------

void TestUtils::tst_compareVersion()
{
    QVERIFY(compareVersion("0.22", "1.0") == true);
    QVERIFY(compareVersion("1", "2") == true);
    QVERIFY(compareVersion("1.0", "2.0") == true);
    QVERIFY(compareVersion("1.14", "1.16") == true);
    QVERIFY(compareVersion("1.16", "1.14") == false);
    QVERIFY(compareVersion("2.1", "2.0") == false);
    QVERIFY(compareVersion("2.0", "2.1") == true);
    QVERIFY(compareVersion("2.1.1.2", "2.0") == false);
    QVERIFY(compareVersion("2.0", "2.1.1.3") == true);
    QVERIFY(compareVersion("2.1", "2.1.1.3") == true);
    QVERIFY(compareVersion("2.2.0.3", "2.1.1.3") == false);
    QVERIFY(compareVersion("2.1.0.3", "2.1.1.3") == true);
    QVERIFY(compareVersion("2.1.0.0", "2.1.1.3") == true);
    QVERIFY(compareVersion("2.1.1.2", "2.1.1.3") == true);
    QVERIFY(compareVersion("2.1.1.9", "2.1.1.10") == true);
    QVERIFY(compareVersion("2.1.1.9", "2.1.1.100") == true);
    QVERIFY(compareVersion("2.1.1.99", "2.1.1.100") == true);
    QVERIFY(compareVersion("test", "2.1") == true);
    QVERIFY(compareVersion("test1", "test") == false);
}

QTEST_MAIN(TestUtils)

#include "tst_utils.moc"
