/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include <algorithm>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "engraving/dom/chord.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/segment.h"
#include "engraving/types/constants.h"
#include "engraving/tests/utils/scorerw.h"

#include "importexport/midi/internal/midirecordingimporter.h"
#include "importexport/midi/internal/midishared/midifile.h"

using namespace muse;
using namespace mu;
using namespace mu::engraving;
using namespace mu::iex::midi;

static const String MIDI_RECORDING_DATA_DIR(u"midirecording_data");

static constexpr int QUARTER = Constants::DIVISION;
static constexpr int MEASURE_4_4 = 4 * QUARTER;
static constexpr int SIXTEENTH = QUARTER / 4;

static MidiRecordingOptions options16th()
{
    MidiRecordingOptions options;
    options.minQuantizationTicks = SIXTEENTH;
    options.searchTuplets = false;
    options.legato = true;
    return options;
}

namespace {
struct ChordRestInfo {
    int relTick = 0;  // relative to the start of the measure
    int ticks = 0;
    std::vector<int> pitches;  // empty for rests
    bool tiedForward = false;
    bool tiedBack = false;
};

std::vector<ChordRestInfo> voiceContent(const Measure* measure, track_idx_t track)
{
    std::vector<ChordRestInfo> result;

    for (const Segment* seg = measure->first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
        const EngravingItem* item = seg->element(track);
        if (!item || !item->isChordRest()) {
            continue;
        }

        const ChordRest* cr = toChordRest(item);
        ChordRestInfo info;
        info.relTick = (cr->tick() - measure->tick()).ticks();
        info.ticks = cr->actualTicks().ticks();

        if (cr->isChord()) {
            for (const Note* note : toChord(cr)->notes()) {
                info.pitches.push_back(note->pitch());
                info.tiedForward |= (note->tieFor() != nullptr);
                info.tiedBack |= (note->tieBack() != nullptr);
            }
        }

        result.push_back(info);
    }

    return result;
}

bool hasNotesInVoices2to4(const Measure* measure, staff_idx_t staffIdx)
{
    for (voice_idx_t voice = 1; voice < VOICES; ++voice) {
        for (const ChordRestInfo& info : voiceContent(measure, staffIdx * VOICES + voice)) {
            if (!info.pitches.empty()) {
                return true;
            }
        }
    }

    return false;
}
}

class MidiRecording_Tests : public ::testing::Test
{
public:
    std::unique_ptr<MasterScore> readTestScore()
    {
        return std::unique_ptr<MasterScore>(ScoreRW::readScore(MIDI_RECORDING_DATA_DIR + u"/piano_4_measures.mscx"));
    }

    // A slightly imprecise performance of: quarter C4, eighth E4, eighth G4, whole C5 (from beat 3, across the bar line)
    RecordedMidiNoteList humanPerformance() const
    {
        return {
            { 60, 90, 8, 470 },
            { 64, 85, 485, 715 },
            { 67, 80, 710, 962 },
            { 72, 95, 950, 2890 },
        };
    }
};

TEST_F(MidiRecording_Tests, sanitizeNotes)
{
    const RecordedMidiNoteList notes = {
        { 60, 90, 500, 900 },     // ok
        { 62, 200, -30, 100 },    // started just before the first beat: moved onto it, velocity clamped
        { 64, 90, 1000, 5000 },   // longer than the available measures: shortened
        { 65, 90, 4000, 4100 },   // after the available measures: dropped
        { 67, 90, 300, 300 },     // zero length: dropped
        { 128, 90, 0, 100 },      // invalid pitch: dropped
    };

    const RecordedMidiNoteList result = MidiRecordingImporter::sanitizeNotes(notes, 4000);

    ASSERT_EQ(result.size(), 3);

    // sorted by start time
    EXPECT_EQ(result[0].pitch, 62);
    EXPECT_EQ(result[0].onTick, 0);
    EXPECT_EQ(result[0].offTick, 100);
    EXPECT_EQ(result[0].velocity, 127);

    EXPECT_EQ(result[1].pitch, 60);

    EXPECT_EQ(result[2].pitch, 64);
    EXPECT_EQ(result[2].offTick, 4000);
}

TEST_F(MidiRecording_Tests, makeMidiFileReproducesBarsAndKey)
{
    std::unique_ptr<MasterScore> score = readTestScore();
    ASSERT_TRUE(score);

    const Fraction startTick = score->firstMeasure()->nextMeasure()->tick();
    const MidiFile midiFile = MidiRecordingImporter::makeMidiFile(score.get(), 0, startTick, humanPerformance());

    ASSERT_EQ(midiFile.tracks().size(), 1);
    EXPECT_EQ(midiFile.division(), Constants::DIVISION);

    int timeSigCount = 0;
    int keySigCount = 0;
    int noteCount = 0;

    for (const auto& [tick, event] : midiFile.tracks().front().events()) {
        if (event.type() == ME_META && event.metaType() == META_TIME_SIGNATURE) {
            ++timeSigCount;
            EXPECT_EQ(tick, 0);
            EXPECT_EQ(event.edata()[0], 4); // numerator
            EXPECT_EQ(event.edata()[1], 2); // log2(denominator)
        } else if (event.type() == ME_META && event.metaType() == META_KEY_SIGNATURE) {
            ++keySigCount;
            EXPECT_EQ(tick, 0);
        } else if (event.type() == ME_NOTE) {
            ++noteCount;
        }
    }

    // All the measures are 4/4: a single time signature event is enough
    EXPECT_EQ(timeSigCount, 1);
    EXPECT_EQ(keySigCount, 1);
    EXPECT_EQ(noteCount, 4);
}

TEST_F(MidiRecording_Tests, makeMidiFileRepresentsShortPickupMeasure)
{
    std::unique_ptr<MasterScore> score = readTestScore();
    ASSERT_TRUE(score);

    // A 1/8 pickup under 4/4: 1/8 is not a whole number of quarters
    score->startCmd(TranslatableString::untranslatable("MIDI recording tests"));
    score->firstMeasure()->adjustToLen(Fraction(1, 8));
    score->endCmd();

    ASSERT_EQ(score->firstMeasure()->ticks(), Fraction(1, 8));

    const RecordedMidiNoteList notes = { { 60, 90, 0, QUARTER / 2 }, { 62, 90, QUARTER / 2, QUARTER / 2 + QUARTER } };
    const MidiFile midiFile = MidiRecordingImporter::makeMidiFile(score.get(), 0, score->firstMeasure()->tick(), notes);

    std::vector<std::pair<int, std::pair<int, int> > > timeSigs;
    for (const auto& [tick, event] : midiFile.tracks().front().events()) {
        if (event.type() == ME_META && event.metaType() == META_TIME_SIGNATURE) {
            timeSigs.push_back({ tick, { event.edata()[0], event.edata()[1] } });
        }
    }

    ASSERT_EQ(timeSigs.size(), 2);
    EXPECT_EQ(timeSigs[0].first, 0);
    EXPECT_EQ(timeSigs[0].second, std::make_pair(1, 3)); // 1/8
    EXPECT_EQ(timeSigs[1].first, QUARTER / 2);
    EXPECT_EQ(timeSigs[1].second, std::make_pair(4, 2)); // 4/4
}

TEST_F(MidiRecording_Tests, clefChangesAndLyricsArePreserved)
{
    std::unique_ptr<MasterScore> score = readTestScore();
    ASSERT_TRUE(score);

    Measure* m2 = score->firstMeasure()->nextMeasure();
    Measure* m3 = m2->nextMeasure();

    score->startCmd(TranslatableString::untranslatable("MIDI recording tests"));

    // A lyric on the whole note of measure 2
    ChordRest* m2Chord = m2->first(SegmentType::ChordRest)->cr(0);
    ASSERT_TRUE(m2Chord && m2Chord->isChord());
    Lyrics* lyrics = Factory::createLyrics(m2Chord);
    lyrics->setTrack(0);
    lyrics->setOwnershipParent(m2Chord);
    lyrics->setPlainText(u"la");
    score->undoAddElement(lyrics);

    // A clef change in the middle of measure 3
    Segment* clefSegment = m3->undoGetSegmentR(SegmentType::Clef, Fraction(1, 2));
    Clef* clef = Factory::createClef(clefSegment);
    clef->setTrack(0);
    clef->setOwnershipParent(clefSegment);
    clef->setClefType(ClefType::F);
    score->undoAddElement(clef);

    score->endCmd();

    MidiRecordingImporter importer;

    score->startCmd(TranslatableString::untranslatable("MIDI recording tests"));
    const RetVal<int> ret = importer.writeRecordedNotes(score.get(), 0, m2->tick(), humanPerformance(), options16th());
    score->endCmd();
    ASSERT_TRUE(ret.ret) << ret.ret.toString();

    // The lyric is on the new first chord of measure 2
    const ChordRest* newChord = m2->first(SegmentType::ChordRest)->cr(0);
    ASSERT_TRUE(newChord && newChord->isChord());
    EXPECT_EQ(toChord(newChord)->notes().front()->pitch(), 60);
    ASSERT_EQ(newChord->lyrics().size(), 1);
    EXPECT_EQ(newChord->lyrics().front()->plainText(), u"la");

    // The clef change is still there
    const Segment* newClefSegment = m3->findSegmentR(SegmentType::Clef, Fraction(1, 2));
    ASSERT_TRUE(newClefSegment);
    const EngravingItem* newClef = newClefSegment->element(0);
    ASSERT_TRUE(newClef && newClef->isClef());
    EXPECT_EQ(toClef(newClef)->clefType(), ClefType::F);
}

TEST_F(MidiRecording_Tests, writesQuantizedNotesFromStartMeasure)
{
    std::unique_ptr<MasterScore> score = readTestScore();
    ASSERT_TRUE(score);

    Measure* m1 = score->firstMeasure();
    Measure* m2 = m1->nextMeasure();
    Measure* m3 = m2->nextMeasure();
    Measure* m4 = m3->nextMeasure();
    ASSERT_TRUE(m4);

    MidiRecordingImporter importer;

    score->startCmd(TranslatableString::untranslatable("MIDI recording tests"));
    const RetVal<int> ret = importer.writeRecordedNotes(score.get(), 0, m2->tick(), humanPerformance(), options16th());
    score->endCmd();

    ASSERT_TRUE(ret.ret) << ret.ret.toString();

    // Measure 1: before the recording, unchanged
    {
        const std::vector<ChordRestInfo> content = voiceContent(m1, 0);
        ASSERT_EQ(content.size(), 1);
        EXPECT_EQ(content[0].pitches, std::vector<int>({ 72 }));
        EXPECT_EQ(content[0].ticks, MEASURE_4_4);
    }

    // Measure 2: the recorded notes, quantized; the whole note D5 that was there is replaced
    {
        const std::vector<ChordRestInfo> content = voiceContent(m2, 0);
        ASSERT_EQ(content.size(), 4);

        EXPECT_EQ(content[0].relTick, 0);
        EXPECT_EQ(content[0].ticks, QUARTER);
        EXPECT_EQ(content[0].pitches, std::vector<int>({ 60 }));

        EXPECT_EQ(content[1].relTick, QUARTER);
        EXPECT_EQ(content[1].ticks, QUARTER / 2);
        EXPECT_EQ(content[1].pitches, std::vector<int>({ 64 }));

        EXPECT_EQ(content[2].relTick, QUARTER + QUARTER / 2);
        EXPECT_EQ(content[2].ticks, QUARTER / 2);
        EXPECT_EQ(content[2].pitches, std::vector<int>({ 67 }));

        EXPECT_EQ(content[3].relTick, 2 * QUARTER);
        EXPECT_EQ(content[3].ticks, 2 * QUARTER);
        EXPECT_EQ(content[3].pitches, std::vector<int>({ 72 }));
        EXPECT_TRUE(content[3].tiedForward);

        EXPECT_FALSE(hasNotesInVoices2to4(m2, 0));
    }

    // Measure 3: the end of the tied note, then a rest
    {
        const std::vector<ChordRestInfo> content = voiceContent(m3, 0);
        ASSERT_GE(content.size(), 2);

        EXPECT_EQ(content[0].relTick, 0);
        EXPECT_EQ(content[0].ticks, 2 * QUARTER);
        EXPECT_EQ(content[0].pitches, std::vector<int>({ 72 }));
        EXPECT_TRUE(content[0].tiedBack);

        for (size_t i = 1; i < content.size(); ++i) {
            EXPECT_TRUE(content[i].pitches.empty());
        }
    }

    // Measure 4: after the recording, unchanged
    {
        const std::vector<ChordRestInfo> content = voiceContent(m4, 0);
        ASSERT_EQ(content.size(), 1);
        EXPECT_EQ(content[0].pitches, std::vector<int>({ 76 }));
    }
}

TEST_F(MidiRecording_Tests, recordingIsASingleUndoStep)
{
    std::unique_ptr<MasterScore> score = readTestScore();
    ASSERT_TRUE(score);

    Measure* m2 = score->firstMeasure()->nextMeasure();
    MidiRecordingImporter importer;

    score->startCmd(TranslatableString::untranslatable("MIDI recording tests"));
    const RetVal<int> ret = importer.writeRecordedNotes(score.get(), 0, m2->tick(), humanPerformance(), options16th());
    score->endCmd();
    ASSERT_TRUE(ret.ret) << ret.ret.toString();

    score->undoRedo(true, nullptr);

    m2 = score->firstMeasure()->nextMeasure();
    Measure* m3 = m2->nextMeasure();

    const std::vector<ChordRestInfo> m2Content = voiceContent(m2, 0);
    ASSERT_EQ(m2Content.size(), 1);
    EXPECT_EQ(m2Content[0].pitches, std::vector<int>({ 74 }));
    EXPECT_EQ(m2Content[0].ticks, MEASURE_4_4);

    const std::vector<ChordRestInfo> m3Content = voiceContent(m3, 0);
    ASSERT_EQ(m3Content.size(), 1);
    EXPECT_TRUE(m3Content[0].pitches.empty());
}

TEST_F(MidiRecording_Tests, noNotesIsCancel)
{
    std::unique_ptr<MasterScore> score = readTestScore();
    ASSERT_TRUE(score);

    MidiRecordingImporter importer;
    const RetVal<int> ret = importer.writeRecordedNotes(score.get(), 0, score->firstMeasure()->tick(), {}, options16th());

    EXPECT_EQ(ret.ret.code(), static_cast<int>(Ret::Code::Cancel));
}

TEST_F(MidiRecording_Tests, applyLegatoFillsShortGaps)
{
    const RecordedMidiNoteList notes = MidiRecordingImporter::sanitizeNotes({
        { 60, 90, 0, 400 },     // chord C-E, played slightly detached
        { 64, 90, 10, 405 },
        { 67, 90, 480, 900 },   // gap of 60 ticks (< 16th) from the next note: lengthened
        { 69, 90, 960, 1200 },  // gap of 240 ticks (> 16th) from the next note: kept
        { 71, 90, 1440, 1700 }, // the last note: kept
    }, 4 * MEASURE_4_4);

    const RecordedMidiNoteList result = MidiRecordingImporter::applyLegato(notes, SIXTEENTH);
    ASSERT_EQ(result.size(), 5);

    // The notes of the chord are lengthened up to the next note, not up to each other
    EXPECT_EQ(result[0].offTick, 480);
    EXPECT_EQ(result[1].offTick, 480);

    EXPECT_EQ(result[2].offTick, 960);
    EXPECT_EQ(result[3].offTick, 1200);
    EXPECT_EQ(result[4].offTick, 1700);

    // The start times never change
    for (size_t i = 0; i < notes.size(); ++i) {
        EXPECT_EQ(result[i].onTick, notes[i].onTick);
    }
}

TEST_F(MidiRecording_Tests, detachedNotesAreWrittenLegato)
{
    std::unique_ptr<MasterScore> score = readTestScore();
    ASSERT_TRUE(score);

    Measure* m2 = score->firstMeasure()->nextMeasure();
    Measure* m3 = m2->nextMeasure();

    // Four quarter notes played detached (released 80-100 ticks before the next one),
    // then a real rest before the last note
    const RecordedMidiNoteList notes = {
        { 60, 90, 0, 380 },
        { 62, 90, 480, 880 },
        { 64, 90, 960, 1370 },
        { 65, 90, 1440, 1840 },
        { 67, 90, 1920, 2280 },
        { 69, 90, 2880, 3300 },
    };

    MidiRecordingImporter importer;

    score->startCmd(TranslatableString::untranslatable("MIDI recording tests"));
    const RetVal<int> ret = importer.writeRecordedNotes(score.get(), 0, m2->tick(), notes, options16th());
    score->endCmd();
    ASSERT_TRUE(ret.ret) << ret.ret.toString();

    // Measure 2: four quarter notes, without rests in between
    const std::vector<ChordRestInfo> m2Content = voiceContent(m2, 0);
    ASSERT_EQ(m2Content.size(), 4);

    const std::vector<int> expectedPitches = { 60, 62, 64, 65 };
    for (size_t i = 0; i < m2Content.size(); ++i) {
        EXPECT_EQ(m2Content[i].relTick, static_cast<int>(i) * QUARTER);
        EXPECT_EQ(m2Content[i].ticks, QUARTER);
        EXPECT_EQ(m2Content[i].pitches, std::vector<int>({ expectedPitches[i] }));
    }

    // Measure 3: the gap before the last note (longer than a 16th) is kept as a rest
    const std::vector<ChordRestInfo> m3Content = voiceContent(m3, 0);
    ASSERT_FALSE(m3Content.empty());
    EXPECT_EQ(m3Content.front().pitches, std::vector<int>({ 67 }));

    bool restBeforeLastNote = false;
    for (const ChordRestInfo& info : m3Content) {
        if (info.pitches == std::vector<int>({ 69 })) {
            EXPECT_EQ(info.relTick, 2 * QUARTER);
            break;
        }
        if (info.pitches.empty()) {
            restBeforeLastNote = true;
        }
    }
    EXPECT_TRUE(restBeforeLastNote);
}

//! "Re-quantize last recording": the same performance written again with other settings,
//! replacing also what the previous take wrote
TEST_F(MidiRecording_Tests, requantizeReplacesThePreviousTake)
{
    std::unique_ptr<MasterScore> score = readTestScore();
    ASSERT_TRUE(score);

    Measure* m2 = score->firstMeasure()->nextMeasure();
    Measure* m3 = m2->nextMeasure();
    Measure* m4 = m3->nextMeasure();

    // Eighth notes, with the last note lasting until measure 4
    const RecordedMidiNoteList notes = {
        { 60, 90, 0, 230 },
        { 62, 90, 240, 470 },
        { 64, 90, 480, 950 },
        { 65, 90, 2 * MEASURE_4_4 + 10, 2 * MEASURE_4_4 + 470 },
    };

    MidiRecordingImporter importer;

    score->startCmd(TranslatableString::untranslatable("MIDI recording tests"));
    const RetVal<int> firstTake = importer.writeRecordedNotes(score.get(), 0, m2->tick(), notes, options16th());
    score->endCmd();
    ASSERT_TRUE(firstTake.ret) << firstTake.ret.toString();

    // Measures 2, 3 and 4 were replaced
    EXPECT_EQ(firstTake.val, 3 * MEASURE_4_4);
    EXPECT_EQ(voiceContent(m2, 0).front().ticks, QUARTER / 2);
    EXPECT_EQ(voiceContent(m4, 0).front().pitches, std::vector<int>({ 65 }));

    // Second take: only the first two notes (e.g. the user changed the quantization to quarter notes)
    MidiRecordingOptions quarterOptions = options16th();
    quarterOptions.minQuantizationTicks = QUARTER;
    quarterOptions.minReplacedTicks = firstTake.val;

    const RecordedMidiNoteList shorterNotes(notes.begin(), notes.begin() + 3);

    score->startCmd(TranslatableString::untranslatable("MIDI recording tests"));
    const RetVal<int> secondTake = importer.writeRecordedNotes(score.get(), 0, m2->tick(), shorterNotes, quarterOptions);
    score->endCmd();
    ASSERT_TRUE(secondTake.ret) << secondTake.ret.toString();

    // The previous take is replaced entirely, even if the new one is shorter
    EXPECT_EQ(secondTake.val, 3 * MEASURE_4_4);

    // Quarter note quantization: no eighth notes any more
    for (const ChordRestInfo& info : voiceContent(m2, 0)) {
        EXPECT_GE(info.ticks, QUARTER);
    }

    // Measure 4 does not contain the note of the first take any more
    for (const ChordRestInfo& info : voiceContent(m4, 0)) {
        EXPECT_TRUE(info.pitches.empty());
    }

    // Each take is a separate undo step: undo goes back to the first take
    score->undoRedo(true, nullptr);

    m2 = score->firstMeasure()->nextMeasure();
    m4 = m2->nextMeasure()->nextMeasure();
    EXPECT_EQ(voiceContent(m2, 0).front().ticks, QUARTER / 2);
    EXPECT_EQ(voiceContent(m4, 0).front().pitches, std::vector<int>({ 65 }));
}

TEST_F(MidiRecording_Tests, legatoCanBeDisabled)
{
    std::unique_ptr<MasterScore> score = readTestScore();
    ASSERT_TRUE(score);

    Measure* m2 = score->firstMeasure()->nextMeasure();

    // A quarter note released 100 ticks before the next one (a gap shorter than a 16th)
    const RecordedMidiNoteList notes = {
        { 60, 90, 0, 380 },
        { 62, 90, 480, 960 },
    };

    MidiRecordingOptions options = options16th();
    options.legato = false;

    MidiRecordingImporter importer;

    score->startCmd(TranslatableString::untranslatable("MIDI recording tests"));
    const RetVal<int> ret = importer.writeRecordedNotes(score.get(), 0, m2->tick(), notes, options);
    score->endCmd();
    ASSERT_TRUE(ret.ret) << ret.ret.toString();

    // Without legato the gap of 100 ticks is quantized to a 16th rest
    const std::vector<ChordRestInfo> content = voiceContent(m2, 0);
    ASSERT_GE(content.size(), 3);
    EXPECT_EQ(content[0].pitches, std::vector<int>({ 60 }));
    EXPECT_LT(content[0].ticks, QUARTER);
    EXPECT_TRUE(content[1].pitches.empty());
}

TEST_F(MidiRecording_Tests, minimumQuantizationIsAFloor)
{
    // A very short note (staccatissimo) and a note slightly off the grid
    const RecordedMidiNoteList notes = {
        { 60, 90, 5, 40 },
        { 62, 90, 250, 700 },
    };

    // Without tuplets: onto the 8th note grid, at least an 8th long
    const RecordedMidiNoteList grid = MidiRecordingImporter::applyMinimumQuantization(notes, QUARTER / 2, false);
    ASSERT_EQ(grid.size(), 2);
    EXPECT_EQ(grid[0].onTick, 0);
    EXPECT_EQ(grid[0].offTick, QUARTER / 2);
    EXPECT_EQ(grid[1].onTick, QUARTER / 2);
    EXPECT_EQ(grid[1].offTick, 720);
}

TEST_F(MidiRecording_Tests, minimumQuantizationKeepsTupletStartTimes)
{
    // Triplet 8th notes, played short
    const RecordedMidiNoteList triplets = {
        { 60, 90, 0, 60 },
        { 62, 90, 160, 220 },
        { 64, 90, 320, 380 },
    };

    // With tuplets and a 16th note floor: the start times are kept (the triplet is still detectable),
    // the notes are lengthened to a 16th, as long as it does not exceed the next note
    const RecordedMidiNoteList result = MidiRecordingImporter::applyMinimumQuantization(triplets, SIXTEENTH, true);
    ASSERT_EQ(result.size(), 3);

    EXPECT_EQ(result[0].onTick, 0);
    EXPECT_EQ(result[0].offTick, SIXTEENTH);
    EXPECT_EQ(result[1].onTick, 160);
    EXPECT_EQ(result[1].offTick, 160 + SIXTEENTH);
    EXPECT_EQ(result[2].onTick, 320);
    EXPECT_EQ(result[2].offTick, 320 + SIXTEENTH);

    // With an 8th note floor, a note is never lengthened over the next one
    const RecordedMidiNoteList eighthFloor = MidiRecordingImporter::applyMinimumQuantization(triplets, QUARTER / 2, true);
    EXPECT_EQ(eighthFloor[0].offTick, 160);
    EXPECT_EQ(eighthFloor[1].offTick, 320);
    EXPECT_EQ(eighthFloor[2].offTick, 320 + QUARTER / 2);
}

TEST_F(MidiRecording_Tests, repeatedNoteMovedOntoTheSameGridPositionIsMerged)
{
    const RecordedMidiNoteList notes = {
        { 60, 70, 0, 100 },
        { 60, 90, 130, 200 }, // the same key again, very close: on a quarter grid, both start at 0
    };

    const RecordedMidiNoteList result = MidiRecordingImporter::applyMinimumQuantization(notes, QUARTER, false);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].onTick, 0);
    EXPECT_EQ(result[0].offTick, QUARTER);
    EXPECT_EQ(result[0].velocity, 90);
}

// ---------------------------------------------------------------------------------------------
//  Recording on two staves, with a split point
// ---------------------------------------------------------------------------------------------

static constexpr int MIDDLE_C = 60;

static std::unique_ptr<MasterScore> readTwoStavesScore()
{
    // Staff 0: piano right hand, staff 1: piano left hand (bass clef), staff 2: flute
    return std::unique_ptr<MasterScore>(ScoreRW::readScore(MIDI_RECORDING_DATA_DIR + u"/piano_two_staves_and_flute.mscx"));
}

static std::vector<int> pitchesAt(const std::vector<ChordRestInfo>& content, int relTick)
{
    for (const ChordRestInfo& info : content) {
        if (info.relTick == relTick) {
            std::vector<int> pitches = info.pitches;
            std::sort(pitches.begin(), pitches.end());
            return pitches;
        }
    }

    return { -1 }; // no chord/rest starts there
}

TEST_F(MidiRecording_Tests, splitNotesAboveBelowAndEqual)
{
    const RecordedMidiNoteList notes = {
        { 59, 90, 0, 400 },   // B3: below the split point
        { 60, 90, 480, 880 }, // C4: equal to the split point -> upper staff
        { 61, 90, 960, 1300 } // C#4: above
    };

    const auto [upper, lower] = MidiRecordingImporter::splitNotes(notes, MIDDLE_C);

    ASSERT_EQ(upper.size(), 2);
    EXPECT_EQ(upper[0].pitch, 60);
    EXPECT_EQ(upper[1].pitch, 61);

    ASSERT_EQ(lower.size(), 1);
    EXPECT_EQ(lower[0].pitch, 59);

    // Another split point: everything in the lower staff
    const auto [upperHigh, lowerHigh] = MidiRecordingImporter::splitNotes(notes, 72);
    EXPECT_TRUE(upperHigh.empty());
    EXPECT_EQ(lowerHigh.size(), 3);
}

TEST_F(MidiRecording_Tests, splitChordIsDividedBetweenTheStaves)
{
    // One chord played with both hands: C3 + G3 (left hand), E4 + C5 (right hand)
    const RecordedMidiNoteList chord = {
        { 48, 90, 0, 470 },
        { 55, 90, 5, 470 },
        { 64, 90, 3, 475 },
        { 72, 90, 8, 470 },
    };

    const auto [upper, lower] = MidiRecordingImporter::splitNotes(chord, MIDDLE_C);

    ASSERT_EQ(upper.size(), 2);
    EXPECT_EQ(upper[0].pitch, 64);
    EXPECT_EQ(upper[1].pitch, 72);

    ASSERT_EQ(lower.size(), 2);
    EXPECT_EQ(lower[0].pitch, 48);
    EXPECT_EQ(lower[1].pitch, 55);
}

TEST_F(MidiRecording_Tests, writesOnTwoStavesWithSplitPoint)
{
    std::unique_ptr<MasterScore> score = readTwoStavesScore();
    ASSERT_TRUE(score);
    ASSERT_EQ(score->nstaves(), 3);

    Measure* m2 = score->firstMeasure()->nextMeasure();

    const RecordedMidiNoteList notes = {
        // beat 1: a chord with both hands
        { 48, 90, 0, 470 },   // C3 -> left hand
        { 64, 90, 5, 470 },   // E4 -> right hand
        { 72, 90, 8, 470 },   // C5 -> right hand
        // beat 2: only the right hand, exactly on the split point
        { 60, 90, 480, 950 }, // C4 -> right hand (equal to the split point)
        // beat 3: only the left hand, just below the split point
        { 59, 90, 960, 1430 }, // B3 -> left hand
    };

    MidiRecordingImporter importer;

    score->startCmd(TranslatableString::untranslatable("MIDI recording tests"));
    const RetVal<int> ret = importer.writeRecordedNotesOnTwoStaves(score.get(), 0, 1, m2->tick(), notes, MIDDLE_C, options16th());
    score->endCmd();
    ASSERT_TRUE(ret.ret) << ret.ret.toString();

    // The same measures are replaced in both staves
    EXPECT_EQ(ret.val, MEASURE_4_4);

    const std::vector<ChordRestInfo> rightHand = voiceContent(m2, 0 * VOICES);
    const std::vector<ChordRestInfo> leftHand = voiceContent(m2, 1 * VOICES);

    // Right hand: E4+C5, C4, then rests
    EXPECT_EQ(pitchesAt(rightHand, 0), std::vector<int>({ 64, 72 }));
    EXPECT_EQ(pitchesAt(rightHand, QUARTER), std::vector<int>({ 60 }));
    EXPECT_EQ(pitchesAt(rightHand, 2 * QUARTER), std::vector<int>());

    // Left hand: C3, rest, B3; the whole note G2 that was there is replaced
    EXPECT_EQ(pitchesAt(leftHand, 0), std::vector<int>({ 48 }));
    EXPECT_EQ(pitchesAt(leftHand, QUARTER), std::vector<int>());
    EXPECT_EQ(pitchesAt(leftHand, 2 * QUARTER), std::vector<int>({ 59 }));

    for (const ChordRestInfo& info : leftHand) {
        for (int pitch : info.pitches) {
            EXPECT_NE(pitch, 43);
        }
    }

    // The flute is not touched
    const std::vector<ChordRestInfo> flute = voiceContent(m2, 2 * VOICES);
    ASSERT_EQ(flute.size(), 1);
    EXPECT_TRUE(flute[0].pitches.empty());

    // A single undo step for both staves
    score->undoRedo(true, nullptr);
    m2 = score->firstMeasure()->nextMeasure();
    EXPECT_EQ(voiceContent(m2, 0)[0].pitches, std::vector<int>({ 74 }));
    EXPECT_EQ(voiceContent(m2, VOICES)[0].pitches, std::vector<int>({ 43 }));
}

TEST_F(MidiRecording_Tests, splitWithOnlyUpperNotesClearsTheLowerStaff)
{
    std::unique_ptr<MasterScore> score = readTwoStavesScore();
    ASSERT_TRUE(score);

    Measure* m2 = score->firstMeasure()->nextMeasure();

    // Only the right hand played: the left hand of the recorded measure becomes a rest
    const RecordedMidiNoteList notes = {
        { 67, 90, 0, 1900 },
    };

    MidiRecordingImporter importer;

    score->startCmd(TranslatableString::untranslatable("MIDI recording tests"));
    const RetVal<int> ret = importer.writeRecordedNotesOnTwoStaves(score.get(), 0, 1, m2->tick(), notes, MIDDLE_C, options16th());
    score->endCmd();
    ASSERT_TRUE(ret.ret) << ret.ret.toString();

    EXPECT_EQ(voiceContent(m2, 0)[0].pitches, std::vector<int>({ 67 }));

    for (const ChordRestInfo& info : voiceContent(m2, VOICES)) {
        EXPECT_TRUE(info.pitches.empty());
    }
}

TEST_F(MidiRecording_Tests, splitNeedsTwoStavesOfTheSameInstrument)
{
    std::unique_ptr<MasterScore> score = readTwoStavesScore();
    ASSERT_TRUE(score);

    Measure* m2 = score->firstMeasure()->nextMeasure();
    const RecordedMidiNoteList notes = { { 48, 90, 0, 470 }, { 72, 90, 0, 470 } };

    MidiRecordingImporter importer;

    // Piano left hand + flute: refused, nothing is written
    score->startCmd(TranslatableString::untranslatable("MIDI recording tests"));
    const RetVal<int> ret = importer.writeRecordedNotesOnTwoStaves(score.get(), 1, 2, m2->tick(), notes, MIDDLE_C, options16th());
    score->endCmd(/*rollback*/ true);

    EXPECT_FALSE(ret.ret);
    EXPECT_EQ(ret.ret.code(), static_cast<int>(Ret::Code::NotSupported));
    EXPECT_EQ(voiceContent(m2, VOICES)[0].pitches, std::vector<int>({ 43 }));
}
