/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/

#include <QtTest/QtTest>

#include "mtest/testutils.h"

#include "audio/midi/zerberus/instrument.h"
#include "audio/midi/zerberus/zerberus.h"
#include "audio/midi/zerberus/zone.h"
#include "mscore/preferences.h"

using namespace Ms;

//---------------------------------------------------------
//   TestSfzIncludes
//---------------------------------------------------------

class TestSfzIncludes : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testincludes();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestSfzIncludes::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   testincludes
//---------------------------------------------------------

void TestSfzIncludes::testincludes()
{
    Zerberus* synth = new Zerberus();
    preferences.setPreference(PREF_APP_PATHS_MYSOUNDFONTS, root);
    synth->loadInstrument("includeMain.sfz");
    QCOMPARE(synth->instrument(0)->zones().size(), (size_t)2);
    QCOMPARE(synth->instrument(0)->zones().front()->keyLo, (char)23);
    QCOMPARE(synth->instrument(0)->zones().front()->keyHi, (char)42);
    QCOMPARE(synth->instrument(0)->zones().front()->keyBase, (char)40);
    QCOMPARE(synth->instrument(0)->zones().back()->keyLo, (char)42);
    QCOMPARE(synth->instrument(0)->zones().back()->keyHi, (char)23);
    QCOMPARE(synth->instrument(0)->zones().back()->keyBase, (char)40);
}

QTEST_MAIN(TestSfzIncludes)

#include "tst_sfzincludes.moc"
