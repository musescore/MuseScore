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

static const QString COMPAT206_DATA_DIR("compat206_data/");

using namespace Ms;

//---------------------------------------------------------
//   TestCompat206
//---------------------------------------------------------

class TestCompat206 : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase();
    void compat(const QString&);
    void accidentals() { compat("accidentals"); }
    void ambitus() { compat("ambitus"); }
    void articulations() { compat("articulations"); }
    void articulationsDouble() { compat("articulations-double"); }
    void breath() { compat("breath"); }
    void clefs() { compat("clefs"); }
    void drumset() { compat("drumset"); }
    void markers() { compat("markers"); }
    void noteheads() { compat("noteheads"); }
//TODO::ws      void textstyles()       { compat("textstyles");       }
    void tuplets() { compat("tuplets"); }
    void hairpin() { compat("hairpin"); }
    void brlines() { compat("barlines"); }
    void lidEmptyText() { compat("lidemptytext"); }
    void intrumentNameAlign() { compat("intrumentNameAlign"); }
    void fermata() { compat("fermata"); }
    void frame_utf8() { compat("frame_text2"); }
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestCompat206::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   compat
//---------------------------------------------------------

void TestCompat206::compat(const QString& file)
{
    QString readFile(COMPAT206_DATA_DIR + file + ".mscx");
    QString writeFile(file + "-test.mscx");
    QString reference(COMPAT206_DATA_DIR + file + "-ref.mscx");

    MasterScore* score = readScore(readFile);
    QVERIFY(score);
    QVERIFY(saveCompareScore(score, writeFile, reference));
}

QTEST_MAIN(TestCompat206)
#include "tst_compat206.moc"
