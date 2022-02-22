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

// start includes required for fixupScore()
#include "libmscore/measure.h"
#include "libmscore/staff.h"
#include "libmscore/keysig.h"
// end includes required for fixupScore()

namespace Ms {
extern bool saveBraille(Score*, const QString&);
}

static const QString BRAILLE_DIR("data/");

using namespace Ms;

//---------------------------------------------------------
//   TestMxmlIO
//---------------------------------------------------------

class TestBrailleIO : public QObject, public MTest
{
    Q_OBJECT

    void brailleMscxExportTestRef(const char* file);

private slots:
    void initTestCase();

    void pitches() { brailleMscxExportTestRef("testPitches"); }
    void octaveMarks() { brailleMscxExportTestRef("testOctaveMarks"); }
    void tempo1() { brailleMscxExportTestRef("testTempo_Example_1.8.1.1_MBC2015"); }
    void tempo2() { brailleMscxExportTestRef("testTempo_Example_1.8.1.2_MBC2015"); }
    void sectionalBarline() { brailleMscxExportTestRef("testBarline_Example_1.10.3.2_MBC2015"); }
    void specialBarline() { brailleMscxExportTestRef("testBarline_Example_1.10.1.1_MBC2015"); }
    void notes() { brailleMscxExportTestRef("testNotes_Example_2.1_MBC2015"); }
    void octavesNoChords() { brailleMscxExportTestRef("testOctavesNoChords_Example_3.2.2.1_MBC2015"); }        // TODO a,b,c section names not exported
    void clefs() { brailleMscxExportTestRef("testClefs_Example_4.2_MBC2015"); }
    void mmrests() { brailleMscxExportTestRef("testMMRests_Example_5.3.1_MBC2015"); }
    void accidentals() { brailleMscxExportTestRef("testAccidentals_Example_6.1_MBC2015"); }
    void quarterAccidentals() { brailleMscxExportTestRef("testQToneAccidentals_Example_6.3_MBC2015"); }
    void keySigs() { brailleMscxExportTestRef("testKeySig_Example_6.5_MBC2015"); }                             // a bit changed. the second key sig does not have naturals
    void timeSignature() { brailleMscxExportTestRef("testTimeSig_Example_7.1_MBC2015"); }
    void chords1() { brailleMscxExportTestRef("testChords_Example_9.1.MBC2015"); }
    void chords2() { brailleMscxExportTestRef("testChords_Example_9.1.1_MBC2015"); }                           // TODO 9.1.1.d octave mark in unison intervals
    void chords3() { brailleMscxExportTestRef("testChords_Example_9.2.1_MBC2015"); }
    void chords4() { brailleMscxExportTestRef("testChords_Example_9.2.2_MBC2015"); }
    void ties1() { brailleMscxExportTestRef("testTie"); }
    void ties2() { brailleMscxExportTestRef("testTie_Example_10.1_MBC2015"); }
    void tiesChords1() { brailleMscxExportTestRef("testTie_Example_10.2.1_MBC2015"); }
    void tiesChords2() { brailleMscxExportTestRef("testTie_Example_10.2.2_MBC2015"); }
    void tiesChords3() { brailleMscxExportTestRef("testTie_Example_10.2.3_MBC2015"); }
    void voices1() { brailleMscxExportTestRef("testVoices_Example_11.1.1.1_MBC2015"); }
    void voices2() { brailleMscxExportTestRef("testVoices_Example_11.1.1.2_MBC2015"); }
    void voices3() { brailleMscxExportTestRef("testVoices_Example_11.1.1.3_MBC2015"); }
    void voices4() { brailleMscxExportTestRef("testVoices_Example_11.1.1.4_MBC2015"); }
    void voices5() { brailleMscxExportTestRef("testVoices_Example_11.1.4.1_MBC2015"); }
    void slursShort() { brailleMscxExportTestRef("testSlur_Example_13.2_MBC2015"); }
    void slursLong() { brailleMscxExportTestRef("testSlur_Example_13.3.b_MBC2015"); }
    // the Braille ref does not use part measure repeats
    void slursWithRest() { brailleMscxExportTestRef("testSlur_Example_13.3.2_MBC2015"); }
    // the Braille ref uses bracket slurs even if layered instead of doubled-slur
    void slursLayered() { brailleMscxExportTestRef("testSlur_Example_13.3.3_MBC2015"); }
    void slursShortCovergence() { brailleMscxExportTestRef("testSlur_Example_13.4.1_MBC2015"); }
    void slursMixConvergence() { brailleMscxExportTestRef("testSlur_Example_13.4.2_b_MBC2015"); }
    void slursMixAndTies() { brailleMscxExportTestRef("testSlur_Example_13.5.1_b_MBC2015"); }
    void tremolo() { brailleMscxExportTestRef("testTremolo_Example_14.2.1_MBC2015"); }
    void tremoloAlt() { brailleMscxExportTestRef("testTremoloAlt_Example_14.3.1_MBC2015"); }
    void fingering1() { brailleMscxExportTestRef("testFingering_Example_15.1.1_MBC2015"); }
    void fingering2() { brailleMscxExportTestRef("testFingering_Example_15.2.1_MBC2015"); }
    // TODO: last measure with doubling grace mark for >4 grace notes
    void graceNotes() { brailleMscxExportTestRef("testGrace_Example_16.2.1_MBC2015"); }
    void graceChords() { brailleMscxExportTestRef("testGrace_Example_16.2.1.1_MBC2015"); }
    void ornaments() { brailleMscxExportTestRef("testOrnaments_Example_16.5_MBC2015"); }
    void glissando() { brailleMscxExportTestRef("testGlisando_Example_16.6.1_MBC2015"); }
    void repeats() { brailleMscxExportTestRef("testRepeats_Example_17.1.1_MBC2015"); }
    void voltas1() { brailleMscxExportTestRef("testVolta_Example_17.1.1.1_MBC2015"); }
    void voltas2() { brailleMscxExportTestRef("testVolta_Example_17.1.1.2_MBC2015"); }
    void voltas3() { brailleMscxExportTestRef("testVolta_Example_17.1.1.3_MBC2015"); }
    void testMarkersJumps() { brailleMscxExportTestRef("testJumps_Example_20.2.1_MBC2015"); }
    void breath() { brailleMscxExportTestRef("testBreaths_Example_22.2.1_MBC2015"); }
    void articulations() { brailleMscxExportTestRef("testArticulations_Example_22.1_MBC2015"); }
    // removed the 4th measure from the example as MuseScore does not have a representations for mordents with accidentals
    void hairpins() { brailleMscxExportTestRef("testHairpins_Example_22.3.3.2_MBC2015"); }
    void sectionBreak() { brailleMscxExportTestRef("testSectionBreak"); }
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestBrailleIO::initTestCase()
{
    initMTest(QString(iex_braille_tests_DATA_ROOT));
}

//---------------------------------------------------------
//   fixupScore -- do required fixups after MusicXML import
//   see mscore/file.cpp MuseScore::readScore(Score* score, QString name)
//---------------------------------------------------------

static void fixupScore(MasterScore* score)
{
    score->connectTies();
    score->masterScore()->rebuildMidiMapping();
    score->setNewlyCreated(false);
    score->setSaved(false);
}

//---------------------------------------------------------
//   mxmlMscxExportTestRef
//   read a MuseScore mscx file, write to a MusicXML file and verify against reference
//---------------------------------------------------------

void TestBrailleIO::brailleMscxExportTestRef(const char* file)
{
    MScore::debugMode = true;
    MasterScore* score = readScore(BRAILLE_DIR + file + ".mscx");
    QVERIFY(score);
    fixupScore(score);
    score->doLayout();
    QVERIFY(saveBraille(score, QString(file) + ".brf"));
    QVERIFY(saveCompareBrailleScore(score, QString(file) + ".brf", BRAILLE_DIR + file + "_ref.brf"));
    delete score;
}

QTEST_MAIN(TestBrailleIO)
#include "tst_braille_io.moc"
