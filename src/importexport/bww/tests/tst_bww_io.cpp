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

// Note: this file is currently unfinished. It does not yet work in the mtest framework,
// but serves as a data file for the iotest in this directory.

#include "testing/qtestsuite.h"

#include "testbase.h"

#include "engraving/dom/masterscore.h"

// start includes required for fixupScore()
#include "engraving/dom/measure.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/keysig.h"
// end includes required for fixupScore()

namespace Ms {
extern bool saveMxl(Score*, const QString&);
}

static const QString BWW_DIR("data/");

using namespace Ms;

//---------------------------------------------------------
//   TestBwwIO
//---------------------------------------------------------

class TestBwwIO : public QObject, public MTest
{
    Q_OBJECT

    void bwwIoTest(const char* file);
    void bwwIoTestRef(const char* file);

    // The list of BWW regression tests
    // Currently failing tests are commented out and annotated with the failure reason
    // To extract the list in a shell script use:
    // cat tst_bww_io.cpp | grep "{ <test>" | awk -F\" '{print $2}'
    // where <test> is bwwIoTest or bwwIoTestRef

private slots:
    void initTestCase();

    void beams() { bwwIoTest("testBeams"); }
    void doublings() { bwwIoTest("testDoublings"); }
    void doublingsShort() { bwwIoTest("testDoublingsShort"); }
    void duration() { bwwIoTest("testDuration"); }
    void graces() { bwwIoTest("testGraces"); }
    void hello() { bwwIoTest("testHello"); }
    void midMeasureRepeat() { bwwIoTest("testMidMeasureRepeat"); }
    void noTimeSig1() { bwwIoTest("testNoTimeSig1"); }
    void noTimeSig2() { bwwIoTest("testNoTimeSig2"); }
    void notes() { bwwIoTest("testNotes"); }
    void repeats() { bwwIoTest("testRepeats"); }
    void tempo120() { bwwIoTest("testTempo120"); }
    void tempo60() { bwwIoTest("testTempo60"); }
    void tieTriplets() { bwwIoTest("testTieTriplet"); }
    void triplets() { bwwIoTest("testTriplets"); }
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestBwwIO::initTestCase()
{
    setRootDir(QString(iex_bww_tests_DATA_ROOT));
}

//---------------------------------------------------------
//   fixupScore -- do required fixups after MusicXML import
//   see mscore/file.cpp MuseScore::readScore(Score* score, QString name)
//   TODO: remove duplication of code
//---------------------------------------------------------

static void fixupScore(Score* score)
{
//      score->syntiState().append(SyntiParameter("soundfont", MScore::soundFont));
    score->connectTies();
    score->rebuildMidiMapping();
    score->setCreated(false);
    score->setSaved(false);

#if 0
    int staffIdx = 0;
    foreach (Staff* st, score->staves()) {
        if (st->updateKeymap()) {
            st->clearKeys();
        }
        int track = staffIdx * VOICES;
        KeySig* key1 = 0;
        for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
            for (Segment* s = m->first(); s; s = s->next()) {
                if (!s->element(track)) {
                    continue;
                }
                EngravingItem* e = s->element(track);
                if (e->generated()) {
                    continue;
                }
                //if ((s->subtype() == SegClef) && st->updateClefList()) {
                //      Clef* clef = static_cast<Clef*>(e);
                //      st->setClef(s->tick(), clef->clefTypeList());
                //      }
                if ((s->segmentType() == Segment::Type::KeySig) && st->updateKeymap()) {
                    KeySig* ks = static_cast<KeySig*>(e);
                    int naturals = key1 ? key1->keySigEvent().accidentalType() : 0;
                    ks->setOldSig(naturals);
                    st->setKey(s->tick(), ks->key());
                    key1 = ks;
                }
            }
            if (m->sectionBreak()) {
                key1 = 0;
            }
        }
        st->setUpdateKeymap(false);
        ++staffIdx;
    }
#endif

    score->updateNotes();
}

//---------------------------------------------------------
//   IoTest
//   read a BWW file, write to a new file and verify against MusicXML reference
//---------------------------------------------------------

void TestBwwIO::IoTest(const char* file)
{
    MScore::debugMode = true;
//    preferences.musicxmlExportBreaks = MusicxmlExportBreaks::MANUAL;
//    preferences.musicxmlImportBreaks = true;
    Score* score = readScore(BWW_DIR + file + ".xml");
    QVERIFY(score);
    fixupScore(score);
    score->doLayout();
    QVERIFY(saveMusicXml(score, QString(file) + ".xml"));
    QVERIFY(saveCompareMusicXmlScore(score, QString(file) + ".xml", BWW_DIR + file + ".xml"));
    delete score;
}

//---------------------------------------------------------
//   IoTestRef
//   read a MusicXML file, write to a new file and verify against reference
//---------------------------------------------------------

void TestBwwIO::IoTestRef(const char* file)
{
    MScore::debugMode = true;
//    preferences.musicxmlExportBreaks = MusicxmlExportBreaks::MANUAL;
//    preferences.musicxmlImportBreaks = true;
    Score* score = readScore(BWW_DIR + file + ".xml");
    QVERIFY(score);
    fixupScore(score);
    score->doLayout();
    QVERIFY(saveMusicXml(score, QString(file) + ".xml"));
    QVERIFY(saveCompareMusicXmlScore(score, QString(file) + ".xml", BWW_DIR + file + "_ref.xml"));
    delete score;
}

QTEST_MAIN(TestBwwIO)
#include "tst_bww_io.moc"
