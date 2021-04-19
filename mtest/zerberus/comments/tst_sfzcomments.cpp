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

#include "audio/midi/zerberus/instrument.h"
#include "audio/midi/zerberus/zerberus.h"
#include "audio/midi/zerberus/zone.h"
#include "mscore/preferences.h"

using namespace Ms;

//---------------------------------------------------------
//   TestSfzComments
//---------------------------------------------------------

class TestSfzComments : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testcomments();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestSfzComments::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   testcomments
//---------------------------------------------------------

void TestSfzComments::testcomments()
{
    Zerberus* synth = new Zerberus();
    preferences.setPreference(PREF_APP_PATHS_MYSOUNDFONTS, root);
    synth->loadInstrument("commentTest.sfz");

    QCOMPARE(synth->instrument(0)->zones().size(), (size_t)3);

    std::list<Zone*>::iterator curZone = synth->instrument(0)->zones().begin();
    QCOMPARE((*curZone)->keyLo, (char)60);
    QCOMPARE((*curZone)->keyHi, (char)70);
    QCOMPARE((*curZone)->keyBase, (char)40);
    curZone++;
    QCOMPARE((*curZone)->keyLo, (char)23);
    QCOMPARE((*curZone)->keyHi, (char)42);
    QCOMPARE((*curZone)->keyBase, (char)40);
    curZone++;
    QCOMPARE((*curZone)->keyLo, (char)42);
    QCOMPARE((*curZone)->keyHi, (char)23);
    QCOMPARE((*curZone)->keyBase, (char)40);
}

QTEST_MAIN(TestSfzComments)

#include "tst_sfzcomments.moc"
