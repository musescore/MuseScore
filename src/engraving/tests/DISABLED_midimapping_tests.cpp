/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "testutils.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/staff.h"

static const QString MIDIMAPPING_DATA_DIR("midimapping_data/");

namespace Ms {
extern Score::FileError importMidi(MasterScore*, const QString&);
}
using namespace Ms;

//---------------------------------------------------------
//   TestMidiMapping
//---------------------------------------------------------

class TestMidiMapping : public QObject, public MTest
{
    Q_OBJECT

    void testReadWrite(const char* f1);
    void testReadChangeWrite(const char* f1, const char* ref, int p);                     // delete part p
    void testReadChangeWrite2(const char* f1, const char* ref);                           // delete last measure
    void testReadChangeOrderWrite(const char* f1, const char* ref, int p1, int p2);       // swap parts p1 and p2
    void testReadWriteMusicXML(const char* file, const char* ref);
    void testReadWriteOther(const char* f1, const char* ref);

private slots:
    void initTestCase();
    void midiMapping1() { testReadWrite("test1withDrums.mscx"); }      // No channels => no channels
    void midiMapping2() { testReadWrite("test1withoutDrums.mscx"); }   // No channels => no channels
    void midiMapping3() { testReadWrite("test2.mscx"); }               // Mapping => mapping
    // with Instrument Change elements
    void midiMapping4() { testReadWrite("test3withMapping.mscx"); }    // Mapping => mapping
    void midiMapping5() { testReadWrite("test3withoutMapping.mscx"); }    // No channels => no channels
    // Delete first part
    void midiMapping6() { testReadChangeWrite("test1withDrums", "test6-ref.mscx", 0); }      // no channels => Mapping
    // Delete part #13
    void midiMapping7() { testReadChangeWrite2("test3withMapping", "test7-ref.mscx"); }      // Mapping => no channels
    // Swap two parts
    void midiMapping8() { testReadChangeOrderWrite("test2", "test8-ref.mscx", 0, 1); }            // Mapping => no channels
    void midiMapping9() { testReadChangeOrderWrite("test1withDrums", "test9-ref.mscx", 1, 3); }   // No channels => Mapping

    // MusicXML
    void midiMapping10() { testReadWriteMusicXML("test10", "test10-ref"); }   // No channels => Mapping
    void midiMapping11() { testReadWriteMusicXML("test11", "test11"); }       // Mapping => mapping
    void midiMapping12() { testReadWriteMusicXML("test12", "test12-ref"); }   // Partial channels(>16 drum tracks) => Partial mapping
    void midiMapping13() { testReadWriteMusicXML("test13", "test13-ref"); }   // Partial channels (port and channel are absent) => mapping
    // Guitar pro
    void midiMapping14() { testReadWriteOther("test14.gp3", "test14-ref.mscx"); }   // Mapping => Mapping
    void midiMapping15() { testReadWriteOther("test15.gp4", "test15-ref.mscx"); }   // Mapping => Mapping
    void midiMapping16() { testReadWriteOther("test16.gp5", "test16-ref.mscx"); }   // Mapping => Mapping
    void midiMapping17() { testReadWriteOther("test17.gpx", "test17-ref.mscx"); }   // Mapping => Mapping
    // MIDI
    void midiMapping18() { testReadWriteOther("test18.mid", "test18-ref.mscx"); }   // Mapping => Mapping
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestMidiMapping::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   testReadWrite
//---------------------------------------------------------

void TestMidiMapping::testReadWrite(const char* f1)
{
    MasterScore* score = readScore(MIDIMAPPING_DATA_DIR + f1);
    score->doLayout();
    QVERIFY(score);
    score->rebuildMidiMapping();
    QVERIFY(saveCompareScore(score, f1, MIDIMAPPING_DATA_DIR + QString(f1)));
    delete score;
}

//---------------------------------------------------------
//   testReadChangeWrite
//   read file; delete part p; write and compare
//---------------------------------------------------------

void TestMidiMapping::testReadChangeWrite(const char* f1, const char* ref, int p)
{
    MasterScore* score = readScore(MIDIMAPPING_DATA_DIR + f1 + QString(".mscx"));
    score->doLayout();
    QVERIFY(score);
    score->rebuildMidiMapping();
    score->cmdRemovePart(score->parts()[p]);
    score->rebuildMidiMapping();
    QVERIFY(saveCompareScore(score, f1 + QString("_changed.mscx"), MIDIMAPPING_DATA_DIR + ref));
    delete score;
}

//---------------------------------------------------------
//   testReadChangeWrite2
//   read file; delete last measure; write and compare
//---------------------------------------------------------

void TestMidiMapping::testReadChangeWrite2(const char* f1, const char* ref)
{
    MasterScore* score = readScore(MIDIMAPPING_DATA_DIR + f1 + QString(".mscx"));
    score->doLayout();
    QVERIFY(score);
    score->rebuildMidiMapping();
    MeasureBase* mb = score->measures()->last();
    while (mb && mb->type() != ElementType::MEASURE) {
        mb = mb->prev();
    }
    score->deleteItem(static_cast<Measure*>(mb));
    score->rebuildMidiMapping();
    QVERIFY(saveCompareScore(score, f1 + QString("_changed.mscx"), MIDIMAPPING_DATA_DIR + ref));
    delete score;
}

//---------------------------------------------------------
//   testReadChangeOrderWrite
//   read file; change order of parts p1 and p2; write and compare
//---------------------------------------------------------

void TestMidiMapping::testReadChangeOrderWrite(const char* f1, const char* ref, int p1, int p2)
{
    MasterScore* score = readScore(MIDIMAPPING_DATA_DIR + f1 + QString(".mscx"));
    score->doLayout();
    QVERIFY(score);
    score->rebuildMidiMapping();

    std::vector<int> dl;
    for (Staff* staff : score->staves()) {
        int idx = muse::indexOf(score->staves(), staff);
        if ((idx == p1 || idx == p2)
            && ((idx != 0 && staff->part() == score->staves()[idx - 1]->part())
                || (idx != score->nstaves() && staff->part() == score->staves()[idx + 1]->part()))) {
            qDebug() << "You're probably trying to swap a part with several staves. This can lead to wrong results!";
        }

        if (idx != -1) {
            dl.push_back(idx);
        }
    }
    dl.swap(p1, p2);
    score->sortStaves(dl);
    score->rebuildMidiMapping();
    QVERIFY(saveCompareScore(score, f1 + QString("_changed3.mscx"), MIDIMAPPING_DATA_DIR + ref));
    delete score;
}

//---------------------------------------------------------
//   testReadWriteMusicXML
//   read a MusicXML file, write to a new file and verify both files are identical
//---------------------------------------------------------

void TestMidiMapping::testReadWriteMusicXML(const char* file, const char* ref)
{
    MScore::debugMode = true;
    preferences.setCustomPreference<MusicxmlExportBreaks>(PREF_EXPORT_MUSICXML_EXPORTBREAKS,
                                                          MusicxmlExportBreaks::MANUAL);
    preferences.setPreference(PREF_EXPORT_MUSICXML_EXPORTLAYOUT, false);
    preferences.setPreference(PREF_IMPORT_MUSICXML_IMPORTBREAKS, true);
    MasterScore* score = readScore(MIDIMAPPING_DATA_DIR + file + ".xml");
    QVERIFY(score);
    score->rebuildMidiMapping();
    score->doLayout();
    QVERIFY(saveMusicXml(score, QString(file) + ".xml"));
    QVERIFY(saveCompareMusicXmlScore(score, QString(file) + ".xml", MIDIMAPPING_DATA_DIR + ref + ".xml"));
    delete score;
}

//---------------------------------------------------------
//   testReadWriteOther
//   read file; write to mscx and compare
//---------------------------------------------------------

void TestMidiMapping::testReadWriteOther(const char* f1, const char* ref)
{
    MasterScore* score;
    auto qf = QFileInfo(f1);

    if (qf.suffix() == "mid") {
        score = new MasterScore(mscore->baseStyle());
        QString fullPath = QString(root + "/" + MIDIMAPPING_DATA_DIR + f1);
        QCOMPARE(importMidi(score,  fullPath), Score::FileError::FILE_NO_ERROR);
    } else {
        score = readScore(MIDIMAPPING_DATA_DIR + f1);
    }
    score->doLayout();
    QVERIFY(score);
    score->rebuildMidiMapping();
    QVERIFY(saveCompareScore(score, qf.completeBaseName() + QString(".mscx"), MIDIMAPPING_DATA_DIR + ref));
    delete score;
}

QTEST_MAIN(TestMidiMapping)
#include "tst_midimapping.moc"
