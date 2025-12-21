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

#include <memory>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include <QString>

#include "global/io/ifilesystem.h"
#include "global/io/buffer.h"
#include "global/io/path.h"

#include "engraving/compat/midi/event.h"
#include "engraving/compat/midi/compatmidirender.h"
#include "engraving/compat/midi/compatmidirenderinternal.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/mcursor.h"
#include "engraving/types/types.h"

#include "engraving/tests/utils/scorecomp.h"
#include "engraving/tests/utils/scorerw.h"

#include "importexport/midi/internal/midiexport/exportmidi.h"

#include "utils/smfyamlserializer.h"

using namespace muse;
using namespace mu::engraving;
using namespace mu::iex::midi;

static const String MIDI_EXPORT_DATA_DIR("midiexport_data");

static const GlobalInject<io::IFileSystem> fileSystem;

static bool saveMidi(Score* score, const std::string_view name)
{
    ExportMidi em(score);
    return em.write(QString::fromUtf8(name), true, true);
}

static void serializeToYaml(const std::string_view midiPath, const std::string& yamlPath)
{
    io::Buffer yamlBuffer;
    ASSERT_TRUE(yamlBuffer.open(io::Buffer::WriteOnly));
    ASSERT_TRUE(SmfYamlSerializer::serialize(midiPath, &yamlBuffer));
    yamlBuffer.close();
    ASSERT_TRUE(fileSystem()->writeFile(yamlPath, yamlBuffer.data()));
}

static void exportAndCompareWithRef(const std::string& name)
{
    const std::string midiFileName = name + ".mid";

    {
        const std::string scoreFileName = name + ".mscx";
        std::unique_ptr<MasterScore> score(ScoreRW::readScore(MIDI_EXPORT_DATA_DIR + u'/' + String::fromUtf8(scoreFileName)));
        ASSERT_TRUE(score);
        score->doLayout();
        score->rebuildMidiMapping();

        ASSERT_PRED2(saveMidi, score.get(), midiFileName);
    }

    const std::string midiRefFileName = name + "-ref.mid";
    const String midiRefPath = ScoreRW::rootPath() + u'/' + MIDI_EXPORT_DATA_DIR + u'/' + String::fromUtf8(midiRefFileName);
    if (ScoreComp::compareFiles(String::fromUtf8(midiFileName),
                                midiRefPath)) {
        return;
    }

    const std::string yamlFileName = name + ".yaml";
    serializeToYaml(midiFileName, yamlFileName);

    const std::string yamlRefFileName = name + "-ref.yaml";
    serializeToYaml(midiRefPath.toStdString(), yamlRefFileName);

    ScoreComp::compareFiles(String::fromUtf8(yamlFileName), String::fromUtf8(yamlRefFileName));

    FAIL() << "midi files differ";
}

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
};

/// write/read midi file with timesig 4/4
TEST_F(MidiExportTests, midi01) {
    exportAndCompareWithRef("midi01");
}

/// write/read midi file with timesig 3/4
TEST_F(MidiExportTests, midi02) {
    exportAndCompareWithRef("midi02");
}

/// write/read midi file with key sig
TEST_F(MidiExportTests, midi03) {
    exportAndCompareWithRef("midi03");
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
    bool open = filehandler.open(QIODevice::WriteOnly | QIODevice::Text);
    ASSERT_TRUE(open);

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
