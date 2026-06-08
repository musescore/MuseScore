/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <QBuffer>
#include <QString>

#include "global/io/ifilesystem.h"
#include "global/io/buffer.h"
#include "global/io/path.h"

#include "engraving/compat/midi/event.h"
#include "engraving/compat/midi/compatmidirender.h"
#include "engraving/compat/midi/compatmidirenderinternal.h"
#include "engraving/dom/masterscore.h"
#include "engraving/types/types.h"

#include "engraving/tests/utils/scorecomp.h"
#include "engraving/tests/utils/scorerw.h"

#include "importexport/midi/internal/midiexport/exportmidi.h"
#include "importexport/midi/internal/midishared/midifile.h"

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
    auto yamlBuffer = io::Buffer::opened(io::IODevice::WriteOnly);
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
    if (ScoreComp::compareFiles(midiRefPath, String::fromUtf8(midiFileName))) {
        return;
    }

    const std::string yamlFileName = name + ".yaml";
    serializeToYaml(midiFileName, yamlFileName);

    const std::string yamlRefFileName = name + "-ref.yaml";
    serializeToYaml(midiRefPath.toStdString(), yamlRefFileName);

    ScoreComp::compareFiles(String::fromUtf8(yamlRefFileName), String::fromUtf8(yamlFileName));

    FAIL() << "midi files differ";
}

class MidiExportTests : public ::testing::Test
{
protected:
    /**
     * Search \p track for a MIDI NoteOn event at exactly \p tick with the given \p pitch and
     * \p velocity.  Returns a pointer to the matching event, or \c nullptr if none is found.
     * Used by grace-note tests to assert that specific on/off events exist at expected positions.
     */
    static const MidiEvent* findNoteEvent(const MidiTrack& track, int tick, int pitch, int velocity)
    {
        auto [begin, end] = track.events().equal_range(tick);
        for (auto it = begin; it != end; ++it) {
            const MidiEvent& event = it->second;
            if (event.type() == ME_NOTEON && event.pitch() == pitch && event.velo() == velocity) {
                return &event;
            }
        }

        return nullptr;
    }

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

TEST_F(MidiExportTests, midiPortExport) {
    exportAndCompareWithRef("testMidiPort");
}

TEST_F(MidiExportTests, midiArpeggio) {
    exportAndCompareWithRef("testArpeggio");
}

TEST_F(MidiExportTests, midiMutedUnison) {
    exportAndCompareWithRef("testMutedUnison");
}

/// Verifies acciaccatura (grace-before) MIDI timing: grace notes start before the beat,
/// stealing time from the previous note.  Covers tick=0 edge case (no prior note to steal from),
/// single and multiple grace notes, and both 60 bpm and 320 bpm tempos.
TEST_F(MidiExportTests, midiGraceBefore)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(MIDI_EXPORT_DATA_DIR + u"/testGraceBefore.mscx"));
    ASSERT_TRUE(score);
    score->doLayout();
    score->rebuildMidiMapping();

    ExportMidi exporter(score.get());
    QBuffer buffer;
    ASSERT_TRUE(buffer.open(QIODevice::ReadWrite));
    ASSERT_TRUE(exporter.write(&buffer, true, true));

    buffer.seek(0);
    MidiFile midiFile;
    ASSERT_TRUE(midiFile.read(&buffer));
    ASSERT_FALSE(midiFile.tracks().empty());

    const MidiTrack& track = midiFile.tracks().front();

    // Acciaccatura starts BEFORE the beat, stealing time from the previous note.
    // graceTickSum = min(prevChord->ticks()/2, grace->notatedTicks()).
    // At 60 bpm: quarter=480, eighth=240.

    // Measure 1 beat 1 (tick 0): grace eighth (240 ticks), prevChord=null → graceTickSum=240.
    // No previous note to steal from, so grace is clamped to tick 0 with duration preserved.
    EXPECT_NE(findNoteEvent(track, 0, 71, 80), nullptr);    // grace on (clamped)
    EXPECT_NE(findNoteEvent(track, 239, 71, 0), nullptr);   // grace off at 239 (duration preserved)
    EXPECT_NE(findNoteEvent(track, 0, 72, 80), nullptr);    // principal on at beat

    // Measure 1 beat 3 (tick 480): grace eighth, prevChord=eighth(240) → graceTickSum=min(120,240)=120.
    EXPECT_NE(findNoteEvent(track, 360, 71, 80), nullptr);  // grace on 120 ticks before beat
    EXPECT_NE(findNoteEvent(track, 479, 71, 0), nullptr);   // grace off at beat-1 (off=on-1 for len=0)
    EXPECT_NE(findNoteEvent(track, 480, 72, 80), nullptr);  // principal on at beat

    // Measure 1 beat 4 (tick 960): grace eighth, prevChord=quarter(480) → graceTickSum=min(240,240)=240.
    EXPECT_NE(findNoteEvent(track, 720, 71, 80), nullptr);  // grace on 240 ticks before beat
    EXPECT_NE(findNoteEvent(track, 960, 72, 80), nullptr);  // principal on at beat

    // Measure 2 beat 1 (tick 1920): grace eighth, prevChord=half(960) → graceTickSum=min(480,240)=240.
    EXPECT_NE(findNoteEvent(track, 1680, 71, 80), nullptr); // grace on 240 ticks before beat
    EXPECT_NE(findNoteEvent(track, 1920, 72, 80), nullptr); // principal on at beat

    // Measure 3 beat 1 (tick 3840): 3 grace eighths, prevChord=whole(1920) → graceTickSum=min(960,240)=240, offset=80.
    EXPECT_NE(findNoteEvent(track, 3600, 74, 80), nullptr); // grace 1 on at 3840-240
    EXPECT_NE(findNoteEvent(track, 3680, 72, 80), nullptr); // grace 2 on at 3840-160
    EXPECT_NE(findNoteEvent(track, 3760, 71, 80), nullptr); // grace 3 on at 3840-80
    EXPECT_NE(findNoteEvent(track, 3840, 72, 80), nullptr); // principal on at beat

    // Measures 4-6 at 320 bpm. Tick positions identical to 60 bpm (same score ticks, different wall-clock time).

    // Measure 4 beat 1 (tick 5760): grace eighth, prevChord=measure-3 whole(1920) → graceTickSum=240.
    EXPECT_NE(findNoteEvent(track, 5520, 71, 80), nullptr); // grace on 240 ticks before beat
    EXPECT_NE(findNoteEvent(track, 5760, 72, 80), nullptr); // principal on at beat

    // Measure 4 beat 3 (tick 6240): grace eighth, prevChord=eighth(240) → graceTickSum=120.
    EXPECT_NE(findNoteEvent(track, 6120, 71, 80), nullptr); // grace on 120 ticks before beat
    EXPECT_NE(findNoteEvent(track, 6240, 72, 80), nullptr); // principal on at beat

    // Measure 4 beat 4 (tick 6720): grace eighth, prevChord=quarter(480) → graceTickSum=240.
    EXPECT_NE(findNoteEvent(track, 6480, 71, 80), nullptr); // grace on 240 ticks before beat
    EXPECT_NE(findNoteEvent(track, 6720, 72, 80), nullptr); // principal on at beat

    // Measure 5 beat 1 (tick 7680): grace eighth, prevChord=half(960) → graceTickSum=240.
    EXPECT_NE(findNoteEvent(track, 7440, 71, 80), nullptr); // grace on 240 ticks before beat
    EXPECT_NE(findNoteEvent(track, 7680, 72, 80), nullptr); // principal on at beat

    // Measure 6 beat 1 (tick 9600): 3 grace eighths, prevChord=whole(1920) → graceTickSum=240, offset=80.
    EXPECT_NE(findNoteEvent(track, 9360, 74, 80), nullptr); // grace 1 on at 9600-240
    EXPECT_NE(findNoteEvent(track, 9440, 72, 80), nullptr); // grace 2 on at 9600-160
    EXPECT_NE(findNoteEvent(track, 9520, 71, 80), nullptr); // grace 3 on at 9600-80
    EXPECT_NE(findNoteEvent(track, 9600, 72, 80), nullptr); // principal on at beat
}

/// Verifies grace-after (trailtime) MIDI timing: the grace note starts at the midpoint
/// of the principal note (trailtime=500 per-1000).
TEST_F(MidiExportTests, midiGraceAfter)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(MIDI_EXPORT_DATA_DIR + u"/testGraceAfter.mscx"));
    ASSERT_TRUE(score);
    score->doLayout();
    score->rebuildMidiMapping();

    ExportMidi exporter(score.get());
    QBuffer buffer;
    ASSERT_TRUE(buffer.open(QIODevice::ReadWrite));
    ASSERT_TRUE(exporter.write(&buffer, true, true));

    buffer.seek(0);
    MidiFile midiFile;
    ASSERT_TRUE(midiFile.read(&buffer));
    ASSERT_FALSE(midiFile.tracks().empty());

    const MidiTrack& track = midiFile.tracks().front();

    // At 60 bpm, quarter = 480 ticks. Grace-after (trailtime=500) starts at 480*500/1000 = 240.
    EXPECT_NE(findNoteEvent(track, 0, 60, 80), nullptr);    // principal C4 on beat
    EXPECT_NE(findNoteEvent(track, 240, 62, 80), nullptr);  // grace-after D4 at midpoint
}

/// Verifies appoggiatura MIDI timing: the grace note plays on the beat and takes a
/// proportional slice of the principal note's duration — min(500, graceNotated/principal * 1000)
/// per-1000.  Covers both quarter and half principal note durations.
TEST_F(MidiExportTests, midiGraceAppoggiatura)
{
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(MIDI_EXPORT_DATA_DIR + u"/testGraceAppoggiatura.mscx"));
    ASSERT_TRUE(score);
    score->doLayout();
    score->rebuildMidiMapping();

    ExportMidi exporter(score.get());
    QBuffer buffer;
    ASSERT_TRUE(buffer.open(QIODevice::ReadWrite));
    ASSERT_TRUE(exporter.write(&buffer, true, true));

    buffer.seek(0);
    MidiFile midiFile;
    ASSERT_TRUE(midiFile.read(&buffer));
    ASSERT_FALSE(midiFile.tracks().empty());

    const MidiTrack& track = midiFile.tracks().front();

    // At 60 bpm, quarter = 480 ticks. Eighth appoggiatura takes 50% of quarter = 240 ticks.
    EXPECT_NE(findNoteEvent(track, 0, 71, 80), nullptr);    // appoggiatura B4 on beat
    EXPECT_NE(findNoteEvent(track, 240, 72, 80), nullptr);  // principal C5 at +240

    // Eighth appoggiatura before half (960 ticks). Grace notated duration = eighth = 240 ticks,
    // which is 240/960 = 25% of the principal → ontime = min(500, 250) = 250 per-1000
    // → grace duration = 960 * 250/1000 = 240 ticks.
    EXPECT_NE(findNoteEvent(track, 480, 71, 80), nullptr);  // appoggiatura B4 on beat
    EXPECT_NE(findNoteEvent(track, 720, 72, 80), nullptr);  // principal C5 at +240
}

/// Regression test for the stale-events bug (issue #22669): grace notes whose NoteEvent
/// was previously saved to the .mscx file with len=0 must still export with correct MIDI
/// timing.  Loading such a file used to mark the chord PlayEventType::User, causing
/// createGraceNotesPlayEvents() to skip recomputation and leave the silent len=0 in place.
TEST_F(MidiExportTests, midiGraceStaleEvents)
{
    // Regression test: grace notes whose NoteEvent was saved to the score file with len=0
    // (by an older MuseScore version) must still be exported with correct timing.
    // When a note has stored <Events>, loading marks the chord PlayEventType::User, which
    // previously caused createGraceNotesPlayEvents() to skip recomputing the grace events,
    // leaving the stale len=0 in place and making the grace note silent in MIDI export.
    std::unique_ptr<MasterScore> score(ScoreRW::readScore(MIDI_EXPORT_DATA_DIR + u"/testGraceStaleEvents.mscx"));
    ASSERT_TRUE(score);
    score->doLayout();
    score->rebuildMidiMapping();

    ExportMidi exporter(score.get());
    QBuffer buffer;
    ASSERT_TRUE(buffer.open(QIODevice::ReadWrite));
    ASSERT_TRUE(exporter.write(&buffer, true, true));

    buffer.seek(0);
    MidiFile midiFile;
    ASSERT_TRUE(midiFile.read(&buffer));
    ASSERT_FALSE(midiFile.tracks().empty());

    const MidiTrack& track = midiFile.tracks().front();

    // At 60 bpm: quarter=480, half=960 ticks.
    // Appoggiatura (eighth) before half at beat 1 (tick 0):
    //   ontime = min(500, 500ms/2000ms*1000) = 250 per-1000 → grace ends at tick 239, principal at tick 240.
    EXPECT_NE(findNoteEvent(track, 0, 71, 80), nullptr);    // appoggiatura on at beat (stale len=0 overridden)
    EXPECT_NE(findNoteEvent(track, 240, 72, 80), nullptr);  // principal on after grace

    // Acciaccatura (eighth) before quarter at beat 3 (tick 960):
    //   prevChord=half(960), graceTickSum=min(480,240)=240 → grace starts 240 ticks before beat.
    EXPECT_NE(findNoteEvent(track, 720, 71, 80), nullptr);  // acciaccatura on before beat (stale len=0 overridden)
    EXPECT_NE(findNoteEvent(track, 960, 72, 80), nullptr);  // principal on at beat
}

TEST_F(MidiExportTests, midiMeasureRepeats) {
    exportAndCompareWithRef("testMeasureRepeats");
}

TEST_F(MidiExportTests, midi184376ExportMidiInitialKeySi) {
    // tick 0 has Bb keysig.  Meas 2 has no key sig. Meas 2 repeats back to start of Meas 2.  Result should have initial Bb keysig
    exportAndCompareWithRef("testInitialKeySigThenRepeatToMeas2");
    // 5 measures, with a key sig on every measure. Meas 3-4 are repeated.
    exportAndCompareWithRef("testRepeatsWithKeySigs");
    // 5 measures, with a key sig on every measure except meas 0.  Meas 3-4 are repeated.
    exportAndCompareWithRef("testRepeatsWithKeySigsExceptFirstMeas");
}

// test changing temp in prima and seconda volta
TEST_F(MidiExportTests, testVoltaTemp) {
    exportAndCompareWithRef("testVoltaTemp");
}

// test changing Dynamic in prima and seconda volta
TEST_F(MidiExportTests, testVoltaDynamic) {
    exportAndCompareWithRef("testVoltaDynamic");
}

// test changing StaffText in prima and seconda volta
//! FIXME: playing technique annotations not honored
TEST_F(MidiExportTests, DISABLED_testVoltaStaffText) {
    exportAndCompareWithRef("testVoltaStaffText");
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
