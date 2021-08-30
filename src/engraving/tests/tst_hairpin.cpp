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
#include "libmscore/masterscore.h"
#include "libmscore/hairpin.h"

using namespace Ms;

//---------------------------------------------------------
//   TestHairpin
//---------------------------------------------------------

class TestHairpin : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase();
    void hairpin();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestHairpin::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   hairpin
//---------------------------------------------------------

void TestHairpin::hairpin()
{
    Hairpin* hp = new Hairpin(score->dummy()->segment());

    // subtype
    hp->setHairpinType(HairpinType::DECRESC_HAIRPIN);
    Hairpin* hp2 = static_cast<Hairpin*>(writeReadElement(hp));
    QCOMPARE(hp2->hairpinType(), HairpinType::DECRESC_HAIRPIN);
    delete hp2;

    hp->setHairpinType(HairpinType::CRESC_HAIRPIN);
    hp2 = static_cast<Hairpin*>(writeReadElement(hp));
    QCOMPARE(hp2->hairpinType(), HairpinType::CRESC_HAIRPIN);
    delete hp2;
}

QTEST_MAIN(TestHairpin)

#include "tst_hairpin.moc"
