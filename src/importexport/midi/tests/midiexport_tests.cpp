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

#include <filesystem>
#include <memory>
#include <string>

#include <gtest/gtest.h>

#include <QString>

#include "global/io/path.h"

#include "engraving/compat/scoreaccess.h"
#include "engraving/compat/midi/event.h"
#include "engraving/compat/midi/compatmidirender.h"
#include "engraving/compat/midi/compatmidirenderinternal.h"
#include "engraving/dom/durationtype.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/mcursor.h"
#include "engraving/types/types.h"

#include "engraving/tests/utils/scorecomp.h"
#include "engraving/tests/utils/scorerw.h"

#include "importexport/midi/internal/midiexport/exportmidi.h"

using namespace muse;
using namespace mu::engraving;
using namespace mu::iex::midi;

// forward declaration of private functions used in tests
namespace mu::iex::midi {
extern engraving::Err importMidi(engraving::MasterScore*, const QString& name);
}

static const String MIDI_EXPORT_DATA_DIR("midiexport_data");

class MidiExportTests : public ::testing::Test
{
protected:
    static void testTimeStretchFermata(MasterScore* score, const String& file, const String& testName)
    {
        const String writeFile = String(u"%1-%2-test-%3.mid").arg(file).arg(testName);
        const String reference(MIDI_EXPORT_DATA_DIR + file + u"-ref.mid");

        testMidiExport(score, writeFile.arg(1), reference);

        // 3rd measure, 3rd beat
        const Fraction frac1 = 2 * Fraction(4, 4) + Fraction(2, 4);
        score->doLayoutRange(frac1, frac1);
        testMidiExport(score, writeFile.arg(2), reference);

        // 7th measure
        const Fraction frac2 = 6 * Fraction(4, 4);
        score->doLayoutRange(frac2, frac2);
        testMidiExport(score, writeFile.arg(3), reference);
    }

    /// see the issue https://musescore.org/node/290997
    static void testTimeStretchFermataTempoEdit(MasterScore* score, const String& file, const String& testName)
    {
        const String writeFile = String(u"%1-%2-test-%3.mid").arg(file).arg(testName);
        const QString reference(MIDI_EXPORT_DATA_DIR + file + u"-%1-ref.mid");

        EngravingItem* tempo = score->firstSegment(SegmentType::ChordRest)->findAnnotation(ElementType::TEMPO_TEXT, 0, 3);
        ASSERT_TRUE(tempo && tempo->isTempoText());

        const int scoreTempo = 200;
        const int defaultTempo = 120;
        const double defaultTempoBps = defaultTempo / 60.0;

        testMidiExport(score, writeFile.arg(u"init"), reference.arg(scoreTempo));

        score->startCmd(TranslatableString());
        tempo->undoChangeProperty(Pid::TEMPO_FOLLOW_TEXT, false, PropertyFlags::UNSTYLED);
        tempo->undoChangeProperty(Pid::TEMPO, defaultTempoBps, PropertyFlags::UNSTYLED);
        score->endCmd();
        testMidiExport(score, writeFile.arg(u"change-tempo"), reference.arg(defaultTempo));

        // undo the last changes
        score->startCmd(TranslatableString());
        score->undoRedo(/* undo */ true, /* EditData */ nullptr);
        score->endCmd();
        testMidiExport(score, writeFile.arg(u"undo-change-tempo"), reference.arg(scoreTempo));

        score->startCmd(TranslatableString());
        score->undoRemoveElement(tempo);
        score->endCmd();
        testMidiExport(score, writeFile.arg(u"remove-tempo"), reference.arg(defaultTempo));

        // undo the last changes
        score->startCmd(TranslatableString());
        score->undoRedo(/* undo */ true, /* EditData */ nullptr);
        score->endCmd();
        testMidiExport(score, writeFile.arg(u"undo-remove-tempo"), reference.arg(scoreTempo));
    }

    static void midiExportTestRef(const String& file)
    {
        MScore::debugMode = true;
        std::unique_ptr<MasterScore> score { ScoreRW::readScore(MIDI_EXPORT_DATA_DIR + u"/" + file + u".mscx") };
        ASSERT_TRUE(score);

        score->doLayout();
        score->rebuildMidiMapping();

        testMidiExport(score.get(), file + u".mid", MIDI_EXPORT_DATA_DIR + u"/" + file + "-ref.mid");
    }

    static void testMidiExport(MasterScore* score, const String& writeFile, const String& refFile)
    {
        ASSERT_PRED2(saveMidi, score, writeFile.toStdString());
        ASSERT_PRED2(ScoreComp::compareFiles, writeFile, ScoreRW::rootPath() + u"/" + refFile);
    }

    static bool saveMidi(Score* score, const std::string& name)
    {
        ExportMidi em(score);
        return em.write(QString::fromStdString(name), true, true);
    }

    static std::unique_ptr<MasterScore> importMidi(const std::string& fileName)
    {
        const auto doImportMidi = [](MasterScore* score, const io::path_t& path) -> mu::engraving::Err {
            return ::mu::iex::midi::importMidi(score, path.toQString());
        };

        std::string absPath = std::filesystem::absolute(fileName)
                              .generic_string();

        return std::unique_ptr<MasterScore> { ScoreRW::readScore(String::fromStdString(absPath), true, doImportMidi) };
    }
};

/// write/read midi file with timesig 4/4
//! FIXME: ScoreComp always prepends ScoreRW::rootPath() to compareWithLocalPath
//! FIXME: test<n>a.mscx & test<n>b.mscx are different
TEST_F(MidiExportTests, DISABLED_midi01) {
    MCursor c{ compat::ScoreAccess::createMasterScoreWithBaseStyle(nullptr) };
    c.setTimeSig(Fraction(4, 4));
    c.addPart(u"voice");
    c.move(0, Fraction(0, 1));

    c.addKeySig(Key::G);
    c.addTimeSig(Fraction(4, 4));
    c.addChord(60, TDuration(DurationType::V_QUARTER));
    c.addChord(61, TDuration(DurationType::V_QUARTER));
    c.addChord(62, TDuration(DurationType::V_QUARTER));
    c.addChord(63, TDuration(DurationType::V_QUARTER));
    std::unique_ptr<MasterScore> score { c.score() };
    ASSERT_TRUE(score);

    score->doLayout();
    score->rebuildMidiMapping();
    ASSERT_TRUE(ScoreRW::saveScore(score.get(), u"test1a.mscx"));
    ASSERT_TRUE(saveMidi(score.get(), "test1.mid"));

    std::unique_ptr<MasterScore> score2 = importMidi("test1.mid");
    ASSERT_TRUE(score2);

    score2->doLayout();
    score2->rebuildMidiMapping();

    EXPECT_PRED3(ScoreComp::saveCompareScore, score2.get(), u"test1b.mscx", u"test1a.mscx");
}

/// write/read midi file with timesig 3/4
//! FIXME: ScoreComp always prepends ScoreRW::rootPath() to compareWithLocalPath
//! FIXME: test<n>a.mscx & test<n>b.mscx are different
TEST_F(MidiExportTests, DISABLED_midi02) {
    MCursor c{ compat::ScoreAccess::createMasterScoreWithBaseStyle(nullptr) };
    c.setTimeSig(Fraction(3, 4));
    c.addPart(u"voice");
    c.move(0, Fraction(0, 1));

    c.addKeySig(Key::D);
    c.addTimeSig(Fraction(3, 4));
    c.addChord(60, TDuration(DurationType::V_QUARTER));
    c.addChord(61, TDuration(DurationType::V_QUARTER));
    c.addChord(62, TDuration(DurationType::V_QUARTER));
    std::unique_ptr<MasterScore> score { c.score() };
    ASSERT_TRUE(score);

    score->doLayout();
    score->rebuildMidiMapping();
    ASSERT_TRUE(ScoreRW::saveScore(score.get(), u"test2a.mscx"));
    ASSERT_TRUE(saveMidi(score.get(), "test2.mid"));

    std::unique_ptr<MasterScore> score2 = importMidi("test1.mid");
    ASSERT_TRUE(score2);

    score2->doLayout();
    score2->rebuildMidiMapping();

    EXPECT_PRED3(ScoreComp::saveCompareScore, score2.get(), u"test2b.mscx", u"test2a.mscx");
}

/// write/read midi file with key sig
//! FIXME: ScoreComp always prepends ScoreRW::rootPath() to compareWithLocalPath
//! FIXME: test<n>a.mscx & test<n>b.mscx are different
TEST_F(MidiExportTests, DISABLED_midi03) {
    MCursor c{ compat::ScoreAccess::createMasterScoreWithBaseStyle(nullptr) };
    c.setTimeSig(Fraction(4, 4));
    c.addPart(u"voice");
    c.move(0, Fraction(0, 1));

    c.addKeySig(Key::G);
    c.addTimeSig(Fraction(4, 4));
    c.addChord(60, TDuration(DurationType::V_QUARTER));
    c.addChord(61, TDuration(DurationType::V_QUARTER));
    c.addChord(62, TDuration(DurationType::V_QUARTER));
    c.addChord(63, TDuration(DurationType::V_QUARTER));
    std::unique_ptr<MasterScore> score { c.score() };
    ASSERT_TRUE(score);

    score->doLayout();
    score->rebuildMidiMapping();
    ASSERT_TRUE(ScoreRW::saveScore(score.get(), u"test3a.mscx"));
    ASSERT_TRUE(saveMidi(score.get(), "test3.mid"));

    std::unique_ptr<MasterScore> score2 = importMidi("test3.mid");
    ASSERT_TRUE(score2);

    score2->doLayout();
    score2->rebuildMidiMapping();

    EXPECT_PRED3(ScoreComp::saveCompareScore, score2.get(), u"test3b.mscx", u"test3a.mscx");
}

//! FIXME: update ref
TEST_F(MidiExportTests, DISABLED_midiBendsExport1) {
    midiExportTestRef(u"testBends1");
}

//! FIXME: update ref
TEST_F(MidiExportTests, DISABLED_midiBendsExport2) {
    midiExportTestRef(u"testBends2");
}

//! FIXME: update ref
TEST_F(MidiExportTests, DISABLED_midiPortExport) {
    midiExportTestRef(u"testMidiPort");
}

//! FIXME: update ref
TEST_F(MidiExportTests, DISABLED_midiArpeggio) {
    midiExportTestRef(u"testArpeggio");
}

//! FIXME: update ref
TEST_F(MidiExportTests, DISABLED_midiMutedUnison) {
    midiExportTestRef(u"testMutedUnison");
}

//! FIXME: update ref
TEST_F(MidiExportTests, DISABLED_midiMeasureRepeats) {
    midiExportTestRef(u"testMeasureRepeats");
}

//! FIXME: update ref
TEST_F(MidiExportTests, DISABLED_midi184376ExportMidiInitialKeySi) {
    // tick 0 has Bb keysig.  Meas 2 has no key sig. Meas 2 repeats back to start of Meas 2.  Result should have initial Bb keysig
    midiExportTestRef(u"testInitialKeySigThenRepeatToMeas2");
    // 5 measures, with a key sig on every measure. Meas 3-4 are repeated.
    midiExportTestRef(u"testRepeatsWithKeySigs");
    // 5 measures, with a key sig on every measure except meas 0.  Meas 3-4 are repeated.
    midiExportTestRef(u"testRepeatsWithKeySigsExceptFirstMeas");
}

//! FIXME: update ref
TEST_F(MidiExportTests, DISABLED_midiVolta) {
    // test changing temp in prima and seconda volta
    midiExportTestRef(u"testVoltaTemp");
    // test changing Dynamic in prima and seconda volta
    midiExportTestRef(u"testVoltaDynamic");
    // test changing StaffText in prima and seconda volta
    midiExportTestRef(u"testVoltaStaffText");
}

//! FIXME: update ref
TEST_F(MidiExportTests, DISABLED_midiTimeStretchFermata) {
    const String file(u"testTimeStretchFermata");
    const String readFile(MIDI_EXPORT_DATA_DIR + u"/" + file + u".mscx");

    std::unique_ptr<MasterScore> score { ScoreRW::readScore(readFile) };
    ASSERT_TRUE(score);

    testTimeStretchFermata(score.get(), file, u"page");
}

/// Checks continuous view tempo issues like https://musescore.org/node/289922.
//! FIXME: update ref
TEST_F(MidiExportTests, DISABLED_midiTimeStretchFermataContinuousView) {
    const String file(u"testTimeStretchFermata");
    const String readFile(MIDI_EXPORT_DATA_DIR + u"/" + file + u".mscx");

    std::unique_ptr<MasterScore> score { ScoreRW::readScore(readFile) };
    ASSERT_TRUE(score);

    score->setLayoutMode(LayoutMode::LINE);
    score->doLayout();

    testTimeStretchFermata(score.get(), file, u"linear");
}

//! FIXME: update ref
TEST_F(MidiExportTests, DISABLED_midiTimeStretchFermataTempoEdit) {
    const String file(u"testTimeStretchFermataTempoEdit");
    const String readFile(MIDI_EXPORT_DATA_DIR + u"/" + file + u".mscx");

    std::unique_ptr<MasterScore> score { ScoreRW::readScore(readFile) };
    ASSERT_TRUE(score);

    testTimeStretchFermataTempoEdit(score.get(), file, u"page");
}

//! FIXME: update ref
TEST_F(MidiExportTests, DISABLED_midiTimeStretchFermataTempoEditContinuousView)
{
    const String file(u"testTimeStretchFermataTempoEdit");
    const String readFile(MIDI_EXPORT_DATA_DIR + u"/" + file + u".mscx");

    std::unique_ptr<MasterScore> score { ScoreRW::readScore(readFile) };
    ASSERT_TRUE(score);

    score->setLayoutMode(LayoutMode::LINE);
    score->doLayout();

    testTimeStretchFermataTempoEdit(score.get(), file, u"linear");
}

//! FIXME: update ref
TEST_F(MidiExportTests, DISABLED_midiSingleNoteDynamics)
{
    const String file("testSingleNoteDynamics");
    String readFile(MIDI_EXPORT_DATA_DIR + u"/" + file + u".mscx");
    String writeFile(file + u"-test.mid");
    String reference(MIDI_EXPORT_DATA_DIR + u"/" + file + u"-ref.mid");

    std::unique_ptr<MasterScore> score { ScoreRW::readScore(readFile) };
    ASSERT_TRUE(score);

    score->doLayout();
    testMidiExport(score.get(), writeFile, reference);
}

// TODO: move to midirenderer_tests.cpp
class MidiExportEventsTests : public MidiExportTests, public ::testing::WithParamInterface<std::string_view>
{
};

//! TODO: investigate failures
TEST_P(MidiExportEventsTests, DISABLED_eventsTest) {
    const String file = String::fromUtf8(GetParam());

    String readFile(MIDI_EXPORT_DATA_DIR + u"/" + file + ".mscx");
    String writeFile(file + "-test.txt");
    String reference(MIDI_EXPORT_DATA_DIR + u"/" + file + "-ref.txt");

    std::unique_ptr<MasterScore> score { ScoreRW::readScore(readFile) };
    ASSERT_TRUE(score);

    EventsHolder events;
    // a temporary, uninitialized synth state so we can render the midi - should fall back correctly
    SynthesizerState ss;
    CompatMidiRendererInternal::Context ctx;
    ctx.sndController = CompatMidiRender::getControllerForSnd(score.get(), ss.ccToUse());
    ctx.eachStringHasChannel = false;
    CompatMidiRender::renderScore(score.get(), events, ctx, true);

    QFile filehandler(writeFile);
    filehandler.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&filehandler);

    for (std::size_t i = 0; i < events.size(); i++) {
        for (auto iter = events[i].begin(); iter != events[i].end(); ++iter) {
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
            out << Qt::endl;
        }
    }
    filehandler.close();

    EXPECT_TRUE(ScoreComp::compareFiles(writeFile, ScoreRW::rootPath() + u"/" + reference));
}

// empty prefix does not work in QtCreator
INSTANTIATE_TEST_SUITE_P(Midi, MidiExportEventsTests,
                         ::testing::Values(
                             "testMetronomeSimple",
                             "testMetronomeCompound",
                             "testMetronomeAnacrusis",
                             "testSwing8thSimple",
                             "testSwing8thTies",
                             "testSwing8thTriplets",
                             "testSwing8thDots",
                             "testSwing16thSimple",
                             "testSwing16thTies",
                             "testSwing16thTriplets",
                             "testSwing16thDots",
                             "testSwingOdd",
                             "testSwingPickup",
                             "testSwingStyleText",
                             //"testSwingTexts", // TODO
                             "testMordents",
                             //"testBaroqueOrnaments", // fail, at least a problem with the first note and stretch
                             "testOrnamentAccidentals",
                             //"testGraceBefore" // TODO
                             "testBeforeAfterGraceTrill",
                             "testBeforeAfterGraceTrillPlay=false",
                             "testKantataBWV140Excerpts",
                             "testTrillTransposingInstrument",
                             "testAndanteExcerpts",
                             "testTrillLines",
                             "testTrillTempos",
                             //"testTrillCrossStaff",
                             "testOrnaments",
                             "testOrnamentsTrillsOttava",
                             "testTieTrill",
                             "testGlissando",
                             "testGlissandoAcrossStaffs",
                             "testGlissando-71826",
                             //"testPedal",
                             "testMultiNoteTremolo",
                             "testMultiNoteTremoloTuplet",
                             "testPauses",
                             "testPausesRepeats",
                             "testPausesTempoTimesigChange",
                             "testGuitarTrem",
                             "testPlayArticulation",
                             "testTremoloDynamics",
                             "testRepeatsDynamics",
                             "testArticulationDynamics"
                             ));
