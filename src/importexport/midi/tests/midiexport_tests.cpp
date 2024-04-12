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

#include <QFile>
#include <QCoreApplication>
#include <QTextStream>
#include <QIODevice>

#include "testutils.h"

#include "engraving/dom/mscore.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/durationtype.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/note.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/mcursor.h"

//#include "audio/exports/exportmidi.h"

static const QString MIDI_DATA_DIR("midi_data/");

namespace Ms {
extern Score::FileError importMidi(MasterScore*, const QString&);
}

using namespace Ms;

//---------------------------------------------------------
//   TestMidi
//---------------------------------------------------------

class TestMidi : public QObject, public MTest
{
    Q_OBJECT
    void midiExportTestRef(const QString& file);
    void testMidiExport(MasterScore* score, const QString& writeFile, const QString& refFile);

    void testTimeStretchFermata(MasterScore* score, const QString& file, const QString& testName);
    void testTimeStretchFermataTempoEdit(MasterScore* score, const QString& file, const QString& testName);

private slots:
    void initTestCase();
    void midi01();
    void midi02();
    void midi03();
    void events_data();
    void events();
    void midiBendsExport1() { midiExportTestRef("testBends1"); }
    void midiBendsExport2() { midiExportTestRef("testBends2"); }        // Play property test
    void midiPortExport() { midiExportTestRef("testMidiPort"); }
    void midiArpeggio() { midiExportTestRef("testArpeggio"); }
    void midiMutedUnison() { midiExportTestRef("testMutedUnison"); }
    void midiMeasureRepeats() { midiExportTestRef("testMeasureRepeats"); }
    void midi184376ExportMidiInitialKeySig()
    {
        midiExportTestRef("testInitialKeySigThenRepeatToMeas2");        // tick 0 has Bb keysig.  Meas 2 has no key sig. Meas 2 repeats back to start of Meas 2.  Result should have initial Bb keysig
        midiExportTestRef("testRepeatsWithKeySigs");                    // 5 measures, with a key sig on every measure. Meas 3-4 are repeated.
        midiExportTestRef("testRepeatsWithKeySigsExceptFirstMeas");     // 5 measures, with a key sig on every measure except meas 0.  Meas 3-4 are repeated.
    }

    void midiVolta()
    {
        midiExportTestRef("testVoltaTemp");   // test changing temp in prima and seconda volta
        midiExportTestRef("testVoltaDynamic");   // test changing Dynamic in prima and seconda volta
        midiExportTestRef("testVoltaStaffText");   // test changing StaffText in prima and seconda volta
    }

    void midiTimeStretchFermata();
    void midiTimeStretchFermataContinuousView();
    void midiTimeStretchFermataTempoEdit();
    void midiTimeStretchFermataTempoEditContinuousView();
    void midiSingleNoteDynamics();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestMidi::initTestCase()
{
    initMTest();
}

void TestMidi::events_data()
{
    QTest::addColumn<QString>("file");
    // Test Metronome
    QTest::newRow("testMetronomeSimple") << "testMetronomeSimple";
    QTest::newRow("testMetronomeCompound") << "testMetronomeCompound";
    QTest::newRow("testMetronomeAnacrusis") << "testMetronomeAnacrusis";
    // Test Eighth Swing
    QTest::newRow("testSwing8thSimple") << "testSwing8thSimple";
    QTest::newRow("testSwing8thTies") << "testSwing8thTies";
    QTest::newRow("testSwing8thTriplets") << "testSwing8thTriplets";
    QTest::newRow("testSwing8thDots") << "testSwing8thDots";
    // Test Sixteenth Swing
    QTest::newRow("testSwing16thSimple") << "testSwing16thSimple";
    QTest::newRow("testSwing16thTies") << "testSwing16thTies";
    QTest::newRow("testSwing16thTriplets") << "testSwing16thTriplets";
    QTest::newRow("testSwing16thDots") << "testSwing16thDots";
    QTest::newRow("testSwingOdd") << "testSwingOdd";
    QTest::newRow("testSwingPickup") << "testSwingPickup";
    // Test Text Combinations
    QTest::newRow("testSwingStyleText") << "testSwingStyleText";
//TODO::ws      QTest::newRow("testSwingTexts") <<  "testSwingTexts";
    // ornaments
    QTest::newRow("testMordents") << "testMordents";
    //QTest::newRow("testBaroqueOrnaments") << "testBaroqueOrnaments"; // fail, at least a problem with the first note and stretch
    QTest::newRow("testOrnamentAccidentals") << "testOrnamentAccidentals";
//TODO      QTest::newRow("testGraceBefore") <<  "testGraceBefore";
    QTest::newRow("testBeforeAfterGraceTrill") << "testBeforeAfterGraceTrill";
    QTest::newRow("testBeforeAfterGraceTrillPlay=false") << "testBeforeAfterGraceTrillPlay=false";
    QTest::newRow("testKantataBWV140Excerpts") << "testKantataBWV140Excerpts";
    QTest::newRow("testTrillTransposingInstrument") << "testTrillTransposingInstrument";
    QTest::newRow("testAndanteExcerpts") << "testAndanteExcerpts";
    QTest::newRow("testTrillLines") << "testTrillLines";
    QTest::newRow("testTrillTempos") << "testTrillTempos";
//      QTest::newRow("testTrillCrossStaff") << "testTrillCrossStaff";
    QTest::newRow("testOrnaments") << "testOrnaments";
    QTest::newRow("testOrnamentsTrillsOttava") << "testOrnamentsTrillsOttava";
    QTest::newRow("testTieTrill") << "testTieTrill";
    // glissando
    QTest::newRow("testGlissando") << "testGlissando";
    QTest::newRow("testGlissandoAcrossStaffs") << "testGlissandoAcrossStaffs";
    QTest::newRow("testGlissando-71826") << "testGlissando-71826";
    // pedal
//      QTest::newRow("testPedal") <<  "testPedal";
    // multi note tremolo
    QTest::newRow("testMultiNoteTremolo") << "testMultiNoteTremolo";
    QTest::newRow("testMultiNoteTremoloTuplet") << "testMultiNoteTremoloTuplet";
    // Test Pauses
    QTest::newRow("testPauses") << "testPauses";
    QTest::newRow("testPausesRepeats") << "testPausesRepeats";
    QTest::newRow("testPausesTempoTimesigChange") << "testPausesTempoTimesigChange";
    QTest::newRow("testGuitarTrem") << "testGuitarTrem";
    QTest::newRow("testPlayArticulation") << "testPlayArticulation";
    QTest::newRow("testTremoloDynamics") << "testTremoloDynamics";
    QTest::newRow("testRepeatsDynamics") << "testRepeatsDynamics";
    QTest::newRow("testArticulationDynamics") << "testArticulationDynamics";
}

//---------------------------------------------------------
//   saveMidi
//---------------------------------------------------------

bool saveMidi(Score* score, const QString& name)
{
    return false;
    //! NOTE Not ported from 3.x
//    ExportMidi em(score);
//    return em.write(name, true, true);
}

//---------------------------------------------------------
//   compareElements
//---------------------------------------------------------

bool compareElements(EngravingItem* e1, EngravingItem* e2)
{
    if (e1->type() != e2->type()) {
        return false;
    }
    if (e1->type() == ElementType::TIMESIG) {
    } else if (e1->type() == ElementType::KEYSIG) {
        KeySig* ks1 = static_cast<KeySig*>(e1);
        KeySig* ks2 = static_cast<KeySig*>(e2);
        if (ks1->key() != ks2->key()) {
            qDebug("      key signature %d  !=  %d", int(ks1->key()), int(ks2->key()));
            return false;
        }
    } else if (e1->type() == ElementType::CLEF) {
    } else if (e1->type() == ElementType::REST) {
    } else if (e1->type() == ElementType::CHORD) {
        Ms::Chord* c1 = static_cast<Ms::Chord*>(e1);
        Ms::Chord* c2 = static_cast<Ms::Chord*>(e2);
        if (c1->ticks() != c2->ticks()) {
            Fraction f1 = c1->ticks();
            Fraction f2 = c2->ticks();
            qDebug("      chord duration %d/%d  !=  %d/%d",
                   f1.numerator(), f1.denominator(),
                   f2.numerator(), f2.denominator()
                   );
            return false;
        }
        if (c1->notes().size() != c2->notes().size()) {
            qDebug("      != note count");
            return false;
        }
        int n = c1->notes().size();
        for (int i = 0; i < n; ++i) {
            Note* n1 = c1->notes()[i];
            Note* n2 = c2->notes()[i];
            if (n1->pitch() != n2->pitch()) {
                qDebug("      != pitch note %d", i);
                return false;
            }
            if (n1->tpc() != n2->tpc()) {
                qDebug("      note tcp %d != %d", n1->tpc(), n2->tpc());
                // return false;
            }
        }
    }

    return true;
}

//---------------------------------------------------------
//   compareScores
//---------------------------------------------------------

bool compareScores(Score* score1, Score* score2)
{
    int staves = score1->nstaves();
    if (score2->nstaves() != staves) {
        printf("   stave count different %d %d\n", staves, score2->nstaves());
        return false;
    }
    Segment* s1 = score1->firstMeasure()->first();
    Segment* s2 = score2->firstMeasure()->first();

    int tracks = staves * VOICES;
    for (;;) {
        for (int track = 0; track < tracks; ++track) {
            EngravingItem* e1 = s1->element(track);
            EngravingItem* e2 = s2->element(track);
            if ((e1 && !e2) || (e2 && !e1)) {
                printf("   elements different\n");
                return false;
            }
            if (e1 == 0) {
                continue;
            }
            if (!compareElements(e1, e2)) {
                printf("   %s != %s\n", e1->name(), e2->name());
                return false;
            }
            printf("   ok: %s\n", e1->name());
        }
        s1 = s1->next1();
        s2 = s2->next1();
        if ((s1 && !s2) || (s2 && !s1)) {
            printf("   segment count different\n");
            return false;
        }
        if (s1 == 0) {
            break;
        }
    }
    return true;
}

//---------------------------------------------------------
///   midi01
///   write/read midi file with timesig 4/4
//---------------------------------------------------------

void TestMidi::midi01()
{
    MCursor c;
    c.setTimeSig(Fraction(4, 4));
    c.createScore("test1a");
    c.addPart("voice");
    c.move(0, Fraction(0, 1));       // move to track 0 tick 0

    c.addKeySig(Key(1));
    c.addTimeSig(Fraction(4, 4));
    c.addChord(60, TDuration(TDuration::DurationType::V_QUARTER));
    c.addChord(61, TDuration(TDuration::DurationType::V_QUARTER));
    c.addChord(62, TDuration(TDuration::DurationType::V_QUARTER));
    c.addChord(63, TDuration(TDuration::DurationType::V_QUARTER));
    MasterScore* score = c.score();

    score->doLayout();
    score->rebuildMidiMapping();
    c.saveScore();
    saveMidi(score, "test1.mid");

    MasterScore* score2 = new MasterScore(mscore->baseStyle());
    score2->setName("test1b");
    QCOMPARE(importMidi(score2, "test1.mid"), Score::FileError::FILE_NO_ERROR);

    score2->doLayout();
    score2->rebuildMidiMapping();
    MCursor c2(score2);
    c2.saveScore();

    QVERIFY(compareScores(score, score2));

    delete score;
    delete score2;
}

//---------------------------------------------------------
///   midi02
///   write/read midi file with timesig 3/4
//---------------------------------------------------------

void TestMidi::midi02()
{
    MCursor c;
    c.setTimeSig(Fraction(3, 4));
    c.createScore("test2a");
    c.addPart("voice");
    c.move(0, Fraction(0, 1));       // move to track 0 tick 0

    c.addKeySig(Key(2));
    c.addTimeSig(Fraction(3, 4));
    c.addChord(60, TDuration(TDuration::DurationType::V_QUARTER));
    c.addChord(61, TDuration(TDuration::DurationType::V_QUARTER));
    c.addChord(62, TDuration(TDuration::DurationType::V_QUARTER));
    MasterScore* score = c.score();

    score->doLayout();
    score->rebuildMidiMapping();
    c.saveScore();
    saveMidi(score, "test2.mid");

    MasterScore* score2 = new MasterScore(mscore->baseStyle());
    score2->setName("test2b");

    QCOMPARE(importMidi(score2, "test2.mid"), Score::FileError::FILE_NO_ERROR);

    score2->doLayout();
    score2->rebuildMidiMapping();
    MCursor c2(score2);
    c2.saveScore();

    QVERIFY(compareScores(score, score2));

    delete score;
    delete score2;
}

//---------------------------------------------------------
///   midi03
///   write/read midi file with key sig
//---------------------------------------------------------

void TestMidi::midi03()
{
    MCursor c;
    c.setTimeSig(Fraction(4, 4));
    c.createScore("test3a");
    c.addPart("voice");
    c.move(0, Fraction(0, 1));       // move to track 0 tick 0

    c.addKeySig(Key(1));
    c.addTimeSig(Fraction(4, 4));
    c.addChord(60, TDuration(TDuration::DurationType::V_QUARTER));
    c.addChord(61, TDuration(TDuration::DurationType::V_QUARTER));
    c.addChord(62, TDuration(TDuration::DurationType::V_QUARTER));
    c.addChord(63, TDuration(TDuration::DurationType::V_QUARTER));
    MasterScore* score = c.score();

    score->doLayout();
    score->rebuildMidiMapping();
    c.saveScore();
    saveMidi(score, "test3.mid");

    MasterScore* score2 = new MasterScore(mscore->baseStyle());
    score2->setName("test3b");
    QCOMPARE(importMidi(score2, "test3.mid"), Score::FileError::FILE_NO_ERROR);

    score2->doLayout();
    score2->rebuildMidiMapping();
    MCursor c2(score2);
    c2.saveScore();

    QVERIFY(compareScores(score, score2));

    delete score;
    delete score2;
}

//---------------------------------------------------------
//   testTimeStretchFermata
//---------------------------------------------------------

void TestMidi::testTimeStretchFermata(MasterScore* score, const QString& file, const QString& testName)
{
    const QString writeFile = QString("%1-%2-test-%3.mid").arg(file).arg(testName);
    const QString reference(MIDI_DATA_DIR + file + "-ref.mid");

    testMidiExport(score, writeFile.arg(1), reference);

    const Fraction frac1 = 2 * Fraction(4, 4) + Fraction(2, 4);   // 3rd measure, 3rd beat
    score->doLayoutRange(frac1, frac1);
    testMidiExport(score, writeFile.arg(2), reference);

    const Fraction frac2 = 6 * Fraction(4, 4);   // 7th measure
    score->doLayoutRange(frac2, frac2);
    testMidiExport(score, writeFile.arg(3), reference);
}

//---------------------------------------------------------
//   midiTimeStretchFermata
//---------------------------------------------------------

void TestMidi::midiTimeStretchFermata()
{
    const QString file("testTimeStretchFermata");
    const QString readFile(MIDI_DATA_DIR + file + ".mscx");

    MasterScore* score = readScore(readFile);

    testTimeStretchFermata(score, file, "page");

    delete score;
}

//---------------------------------------------------------
//   midiTimeStretchFermataContinuousView
///   Checks continuous view tempo issues like #289922.
//---------------------------------------------------------

void TestMidi::midiTimeStretchFermataContinuousView()
{
    const QString file("testTimeStretchFermata");
    const QString readFile(MIDI_DATA_DIR + file + ".mscx");

    MasterScore* score = readScore(readFile);
    score->setLayoutMode(LayoutMode::LINE);
    score->doLayout();

    testTimeStretchFermata(score, file, "linear");

    delete score;
}

//---------------------------------------------------------
//   testTimeStretchFermataTempoEdit
///   see the issue #290997
//---------------------------------------------------------

void TestMidi::testTimeStretchFermataTempoEdit(MasterScore* score, const QString& file, const QString& testName)
{
    const QString writeFile = QString("%1-%2-test-%3.mid").arg(file).arg(testName);
    const QString reference(MIDI_DATA_DIR + file + "-%1-ref.mid");

    EngravingItem* tempo = score->firstSegment(SegmentType::ChordRest)->findAnnotation(ElementType::TEMPO_TEXT, -1, 3);
    assert(tempo && tempo->isTempoText());

    const int scoreTempo = 200;
    const int defaultTempo = 120;
    const double defaultTempoBps = defaultTempo / 60.0;

    testMidiExport(score, writeFile.arg("init"), reference.arg(scoreTempo));

    score->startCmd();
    tempo->undoChangeProperty(Pid::TEMPO_FOLLOW_TEXT, false, PropertyFlags::UNSTYLED);
    tempo->undoChangeProperty(Pid::TEMPO, defaultTempoBps, PropertyFlags::UNSTYLED);
    score->endCmd();
    testMidiExport(score, writeFile.arg("change-tempo"), reference.arg(defaultTempo));

    // undo the last changes
    score->startCmd();
    score->undoRedo(/* undo */ true, /* EditData */ nullptr);
    score->endCmd();
    testMidiExport(score, writeFile.arg("undo-change-tempo"), reference.arg(scoreTempo));

    score->startCmd();
    score->undoRemoveElement(tempo);
    score->endCmd();
    testMidiExport(score, writeFile.arg("remove-tempo"), reference.arg(defaultTempo));

    // undo the last changes
    score->startCmd();
    score->undoRedo(/* undo */ true, /* EditData */ nullptr);
    score->endCmd();
    testMidiExport(score, writeFile.arg("undo-remove-tempo"), reference.arg(scoreTempo));
}

//---------------------------------------------------------
//   midiTimeStretchFermataTempoEdit
//---------------------------------------------------------

void TestMidi::midiTimeStretchFermataTempoEdit()
{
    const QString file("testTimeStretchFermataTempoEdit");
    const QString readFile(MIDI_DATA_DIR + file + ".mscx");

    MasterScore* score = readScore(readFile);

    testTimeStretchFermataTempoEdit(score, file, "page");

    delete score;
}

//---------------------------------------------------------
//   midiTimeStretchFermataTempoEditContinuousView
//---------------------------------------------------------

void TestMidi::midiTimeStretchFermataTempoEditContinuousView()
{
    const QString file("testTimeStretchFermataTempoEdit");
    const QString readFile(MIDI_DATA_DIR + file + ".mscx");

    MasterScore* score = readScore(readFile);
    score->setLayoutMode(LayoutMode::LINE);
    score->doLayout();

    testTimeStretchFermataTempoEdit(score, file, "linear");

    delete score;
}

//---------------------------------------------------------
//   midiSingleNoteDynamics
//---------------------------------------------------------

void TestMidi::midiSingleNoteDynamics()
{
    const QString file("testSingleNoteDynamics");
    QString readFile(MIDI_DATA_DIR + file + ".mscx");
    QString writeFile(file + "-test.mid");
    QString reference(MIDI_DATA_DIR + file + "-ref.mid");

    MasterScore* score = readScore(readFile);
    score->doLayout();
    testMidiExport(score, writeFile, reference);

    delete score;
}

//---------------------------------------------------------
//   events
//---------------------------------------------------------

void TestMidi::events()
{
    QFETCH(QString, file);

    QString readFile(MIDI_DATA_DIR + file + ".mscx");
    QString writeFile(file + "-test.txt");
    QString reference(MIDI_DATA_DIR + file + "-ref.txt");

    MasterScore* score = readScore(readFile);
    EventMap events;
    // a temporary, uninitialized synth state so we can render the midi - should fall back correctly
    SynthesizerState ss;
    MidiRenderer::Context ctx;
    ctx.eachStringHasChannel = false;
    ctx.synthState = ss;
    score->renderMidi(&events, ctx, true);
    qDebug() << "Opened score " << readFile;
    QFile filehandler(writeFile);
    filehandler.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&filehandler);

    for (auto iter = events.begin(); iter != events.end(); ++iter) {
        if (iter->second.discard()) {
            continue;
        }
        out << qSetFieldWidth(5) << "Tick  =  ";
        out << qSetFieldWidth(5) << iter->first;
        out << qSetFieldWidth(5) << "   Type  = ";
        out << qSetFieldWidth(5) << iter->second.type();
        out << qSetFieldWidth(5) << "   Pitch  = ";
        out << qSetFieldWidth(5) << iter->second.dataA();
        out << qSetFieldWidth(5) << "   Velocity  = ";
        out << qSetFieldWidth(5) << iter->second.dataB();
        out << qSetFieldWidth(5) << "   Channel  = ";
        out << qSetFieldWidth(5) << iter->second.channel();
        out << endl;
    }
    filehandler.close();

    QVERIFY(score);
    QVERIFY(compareFiles(writeFile, reference));
    // QVERIFY(saveCompareScore(score, writeFile, reference));

    delete score;
}

//---------------------------------------------------------
//   testMidiExport
//---------------------------------------------------------

void TestMidi::testMidiExport(MasterScore* score, const QString& writeFile, const QString& refFile)
{
    assert(writeFile.endsWith(".mid") && refFile.endsWith(".mid"));
    QVERIFY(saveMidi(score, writeFile));
    QVERIFY(compareFiles(writeFile, refFile));
}

//---------------------------------------------------------
//   midiExportTest
//   read a MuseScore mscx file, write to a MIDI file and verify against reference
//---------------------------------------------------------

void TestMidi::midiExportTestRef(const QString& file)
{
    MScore::debugMode = true;
    MasterScore* score = readScore(MIDI_DATA_DIR + file + ".mscx");
    QVERIFY(score);
    score->doLayout();
    score->rebuildMidiMapping();
    testMidiExport(score, QString(file) + ".mid", MIDI_DATA_DIR + QString(file) + "-ref.mid");
    delete score;
}

QTEST_MAIN(TestMidi)

#include "tst_midi.moc"
