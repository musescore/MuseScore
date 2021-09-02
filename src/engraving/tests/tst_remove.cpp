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
#include "libmscore/excerpt.h"
#include "libmscore/masterscore.h"
#include "libmscore/spanner.h"

static const QString REMOVE_DATA_DIR("remove_data/");

using namespace Ms;

//---------------------------------------------------------
//   TestRemove
//---------------------------------------------------------

class TestRemove : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase();
    void removeStaff();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestRemove::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   StaffCheckData
//    For passing to the defined below inStaff() function.
//---------------------------------------------------------

struct StaffCheckData {
    int staffIdx;
    bool staffHasElements;
};

//---------------------------------------------------------
//   inStaff
//    for usage with Score::scanElements to check whether
//    the element belongs to a staff with a certain number.
//---------------------------------------------------------

static void inStaff(void* staffCheckData, EngravingItem* e)
{
    StaffCheckData* checkData = static_cast<StaffCheckData*>(staffCheckData);
    if (e->staffIdx() == checkData->staffIdx) {
        qDebug() << e->name() << "is in staff" << checkData->staffIdx;
        checkData->staffHasElements = true;
    }
}

//---------------------------------------------------------
//   staffHasElements
//---------------------------------------------------------

static bool staffHasElements(Score* score, int staffIdx)
{
    for (auto i = score->spannerMap().cbegin(); i != score->spannerMap().cend(); ++i) {
        Spanner* s = i->second;
        if (s->staffIdx() == staffIdx) {
            qDebug() << s->name() << "is in staff" << staffIdx;
            return true;
        }
    }
    for (Spanner* s : score->unmanagedSpanners()) {
        if (s->staffIdx() == staffIdx) {
            qDebug() << s->name() << "is in staff" << staffIdx;
            return true;
        }
    }
    StaffCheckData checkData { staffIdx, false };
    score->scanElements(&checkData, inStaff, true);
    return checkData.staffHasElements;
}

//---------------------------------------------------------
//   removeStaff
//    Checks that after a staff removal all elements
//    belonging to it are not removed in excerpts.
//---------------------------------------------------------

void TestRemove::removeStaff()
{
    MasterScore* score = readScore(REMOVE_DATA_DIR + "remove_staff.mscx");

    // Remove the second staff and see what happens
    score->startCmd();
    score->cmdRemoveStaff(1);
    score->endCmd();

    QVERIFY(!staffHasElements(score, 1));
    for (Excerpt* ex : score->excerpts()) {
        Score* s = ex->partScore();
        QVERIFY(staffHasElements(s, 1));
    }

    delete score;
}

QTEST_MAIN(TestRemove);
#include "tst_remove.moc"
