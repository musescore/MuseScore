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

#include <QString>

#include <gtest/gtest.h>

#include "global/io/path.h"
#include "global/types/string.h"

#include "engraving/types/constants.h"
#include "engraving/tests/utils/scorerw.h"
#include "engraving/tests/utils/scorecomp.h"

#include "importexport/midi/internal/midiimport/importmidi_lyrics.h"
#include "importexport/midi/internal/midiimport/importmidi_meter.h"
#include "importexport/midi/internal/midiimport/importmidi_model.h"
#include "importexport/midi/internal/midiimport/importmidi_operations.h"
#include "importexport/midi/internal/midiimport/importmidi_quant.h"
#include "importexport/midi/internal/midiimport/importmidi_tuplet.h"

using namespace muse;
using namespace mu;
using namespace mu::iex::midi;

// forward declaration of private functions used in tests
namespace mu::iex::midi {
extern engraving::Err importMidi(engraving::MasterScore*, const QString& name);

namespace MidiTuplet {
bool isTupletAllowed(const TupletInfo& tupletInfo);
std::vector<int> findTupletNumbers(const ReducedFraction& divLen, const ReducedFraction& barFraction);
TupletInfo findTupletApproximation(const ReducedFraction& tupletLen, int tupletNumber, const ReducedFraction& quantValue,
                                   const ReducedFraction& startTupletTime, const std::multimap<ReducedFraction,
                                                                                               MidiChord>::iterator& startChordIt,
                                   const std::multimap<ReducedFraction,
                                                       MidiChord>::iterator& endChordIt);
void splitFirstTupletChords(std::vector<TupletInfo>& tuplets, std::multimap<ReducedFraction, MidiChord>& chords);
std::set<int> findLongestUncommonGroup(const std::vector<TupletInfo>& tuplets, const ReducedFraction& basicQuant);
}

namespace Meter {
MaxLevel maxLevelBetween(const ReducedFraction& startTickInBar, const ReducedFraction& endTickInBar, const DivisionInfo& divInfo);

MaxLevel findMaxLevelBetween(const ReducedFraction& startTickInBar, const ReducedFraction& endTickInBar,
                             const std::vector<DivisionInfo>& divsInfo);
} // namespace Meter
}

static const String MIDI_IMPORT_DATA_DIR("midiimport_data");

class MidiImportTests : public ::testing::Test
{
protected:
    void dontSimplify(const char* file)
    {
        auto& opers = midiImportOperations;
        opers.addNewMidiFile(midiFilePath(file));
        MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(file));
        auto& data = *opers.data();

        data.trackOpers.simplifyDurations.setDefaultValue(false, false);
        data.trackOpers.maxVoiceCount.setDefaultValue(MidiOperations::VoiceCount::V_1, false);
        data.trackOpers.doStaffSplit.setDefaultValue(false, false);
        data.trackOpers.showTempoText.setDefaultValue(false);

        importThenCompareWithRef(file);
    }

    void noTempoText(const char* file)
    {
        auto& opers = midiImportOperations;
        opers.addNewMidiFile(midiFilePath(file));
        MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(file));
        auto& data = *opers.data();

        data.trackOpers.showTempoText.setDefaultValue(false);

        importThenCompareWithRef(file);
    }

    void staffSplit(const char* file)
    {
        auto& opers = midiImportOperations;
        opers.addNewMidiFile(midiFilePath(file));
        MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(file));
        auto& data = *opers.data();

        data.trackOpers.doStaffSplit.setDefaultValue(true, false);
        data.trackOpers.simplifyDurations.setDefaultValue(false, false);
        data.trackOpers.maxVoiceCount.setDefaultValue(MidiOperations::VoiceCount::V_1, false);
        data.trackOpers.showTempoText.setDefaultValue(false);

        importThenCompareWithRef(file);
    }

    void simplification(const char* file)
    {
        auto& opers = midiImportOperations;
        opers.addNewMidiFile(midiFilePath(file));
        MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(file));
        auto& data = *opers.data();

        data.trackOpers.doStaffSplit.setDefaultValue(false, false);
        data.trackOpers.simplifyDurations.setDefaultValue(true, false);
        data.trackOpers.maxVoiceCount.setDefaultValue(MidiOperations::VoiceCount::V_1, false);
        data.trackOpers.showTempoText.setDefaultValue(false);

        importThenCompareWithRef(file);
    }

    void voiceSeparation(const char* file, bool simplify = false)
    {
        auto& opers = midiImportOperations;
        opers.addNewMidiFile(midiFilePath(file));
        MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(file));
        auto& data = *opers.data();

        data.trackOpers.doStaffSplit.setDefaultValue(false, false);
        data.trackOpers.simplifyDurations.setDefaultValue(simplify, false);
        data.trackOpers.maxVoiceCount.setDefaultValue(MidiOperations::VoiceCount::V_4, false);
        data.trackOpers.showTempoText.setDefaultValue(false);

        importThenCompareWithRef(file);
    }

    void importThenCompareWithRef(const char* file);
    std::unique_ptr<engraving::MasterScore> importMidi(const String& fileName);

    String midiFilePath(const char* file)
    {
        return midiFilePath(String::fromUtf8(file));
    }

    String midiFilePath(const String fileName)
    {
        return engraving::ScoreRW::rootPath() + u"/" + MIDI_IMPORT_DATA_DIR + u"/" + fileName + u".mid";
    }
};

void MidiImportTests::importThenCompareWithRef(const char* file)
{
    const String fileName = String::fromUtf8(file);
    std::unique_ptr<engraving::MasterScore> score = importMidi(midiFilePath(fileName));
    ASSERT_TRUE(score);

    const String outPath = fileName + u".mscx";
    const String refPath = MIDI_IMPORT_DATA_DIR + u"/" + fileName + u"-ref.mscx";
    EXPECT_TRUE(engraving::ScoreComp::saveCompareScore(score.get(), outPath, refPath));
}

std::unique_ptr<engraving::MasterScore> MidiImportTests::importMidi(const String& fileName)
{
    const auto doImportMidi = [](engraving::MasterScore* score, const io::path_t& path) -> mu::engraving::Err {
        return mu::iex::midi::importMidi(score, path.toQString());
    };

    return std::unique_ptr<engraving::MasterScore> { engraving::ScoreRW::readScore(fileName, true, doImportMidi) };
}

TEST_F(MidiImportTests, m1) {
    dontSimplify("m1");
}

// tie across bar line
TEST_F(MidiImportTests, m2) {
    dontSimplify("m2");
}

// voices, typeA, resolve with tie
TEST_F(MidiImportTests, m3) {
    dontSimplify("m3");
}

// voices, typeB, resolve with tie
TEST_F(MidiImportTests, m4) {
    dontSimplify("m4");
}

// same as m1 with division 240
TEST_F(MidiImportTests, m5) {
    dontSimplify("m5");
}

// quantization
TEST_F(MidiImportTests, quantDotted4th) {
    String midiFile(u"quant_dotted_4th");
    auto& opers = midiImportOperations;
    opers.addNewMidiFile(midiFilePath(midiFile));
    MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(midiFile));
    auto& data = *opers.data();

    // 1/4 quantization should preserve 4th dotted note
    data.trackOpers.quantValue.setDefaultValue(MidiOperations::QuantValue::Q_4, false);

    dontSimplify(midiFile.toStdString().c_str());
}

// human-performed (unaligned) files

TEST_F(MidiImportTests, human4_4) {
    dontSimplify("human_4-4");
}

TEST_F(MidiImportTests, humanTempo) {
    importThenCompareWithRef("human_tempo");
}

// chord detection

TEST_F(MidiImportTests, chordSmallError) {
    noTempoText("chord_small_error");
}

TEST_F(MidiImportTests, chordBigError) {
    noTempoText("chord_big_error");
}

TEST_F(MidiImportTests, chordLegato) {
    noTempoText("chord_legato");
}

TEST_F(MidiImportTests, chordCollect) {
    noTempoText("chord_collect");
}

// very short note - don't remove note but show it with min allowed duration (1/128)
TEST_F(MidiImportTests, chordVeryShort) {
    dontSimplify("chord_1_tick_long");
}

// test scores for meter (duration subdivision)

TEST_F(MidiImportTests, meterTimeSig4_4) {
    dontSimplify("meter_4-4");
}

TEST_F(MidiImportTests, metertimeSig9_8) {
    dontSimplify("meter_9-8");
}

TEST_F(MidiImportTests, metertimeSig12_8) {
    dontSimplify("meter_12-8");
}

// TODO: update ref
TEST_F(MidiImportTests, DISABLED_metertimeSig15_8) {
    dontSimplify("meter_15-8");
}

TEST_F(MidiImportTests, meterCentralLongNote) {
    dontSimplify("meter_central_long_note");
}

TEST_F(MidiImportTests, meterCentralLongRest) {
    dontSimplify("meter_central_long_rest");
}

TEST_F(MidiImportTests, meterChordExample) {
    dontSimplify("meter_chord_example");
}

TEST_F(MidiImportTests, meterDotsExample1) {
    dontSimplify("meter_dots_example1");
}

TEST_F(MidiImportTests, meterDotsExample2) {
    dontSimplify("meter_dots_example2");
}

TEST_F(MidiImportTests, meterDotsExample3) {
    dontSimplify("meter_dots_example3");
}

TEST_F(MidiImportTests, meterHalfRest3_4) {
    dontSimplify("meter_half_rest_3-4");
}

TEST_F(MidiImportTests, meterFirst2_8thRestsCompound) {
    dontSimplify("meter_first_2_8th_rests_compound");
}

TEST_F(MidiImportTests, meterLastQuarterRestCompound) {
    dontSimplify("meter_last_quarter_rest_compound");
}

TEST_F(MidiImportTests, meterRests) {
    dontSimplify("meter_rests");
}

TEST_F(MidiImportTests, meterTwoBeatsOver) {
    dontSimplify("meter_two_beats_over");
}

TEST_F(MidiImportTests, meterDotTie) {
    dontSimplify("meter_dot_tie");
}

// time sig

TEST_F(MidiImportTests, timesigChanges) {
    dontSimplify("timesig_changes");
}

// test scores for tuplets

TEST_F(MidiImportTests, tuplet2Voices3_5Tuplets) {
    // requires 1/32 quantization
    QString midiFile("tuplet_2_voices_3_5_tuplets");
    auto& opers = midiImportOperations;
    opers.addNewMidiFile(midiFilePath(midiFile));
    MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(midiFile));
    auto& data = *opers.data();

    data.trackOpers.quantValue.setDefaultValue(MidiOperations::QuantValue::Q_32, false);
    data.trackOpers.showTempoText.setDefaultValue(false);

    importThenCompareWithRef(midiFile.toStdString().c_str());
}

TEST_F(MidiImportTests, tuplet2VoicesTupletNon) {
    noTempoText("tuplet_2_voices_tuplet_non");
}

TEST_F(MidiImportTests, tuplet3_5_7tuplets) {
    QString midiFile("tuplet_3_5_7_tuplets");
    auto& opers = midiImportOperations;
    opers.addNewMidiFile(midiFilePath(midiFile));
    MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(midiFile));
    auto& data = *opers.data();

    data.trackOpers.changeClef.setDefaultValue(false, false);
    data.trackOpers.doStaffSplit.setDefaultValue(false, false);
    data.trackOpers.simplifyDurations.setDefaultValue(false, false);
    data.trackOpers.showTempoText.setDefaultValue(false);

    importThenCompareWithRef(midiFile.toStdString().c_str());
}

TEST_F(MidiImportTests, tuplet5_5TupletsRests) {
    dontSimplify("tuplet_5_5_tuplets_rests");
}

TEST_F(MidiImportTests, tuplet3_4) {
    dontSimplify("tuplet_3-4");
}

TEST_F(MidiImportTests, tupletDuplet) {
    dontSimplify("tuplet_duplet");
}

TEST_F(MidiImportTests, tupletMars) {
    dontSimplify("tuplet_mars");
}

TEST_F(MidiImportTests, tupletNonuplet3_4) {
    // requires 1/64 quantization
    QString midiFile("tuplet_nonuplet_3-4");
    auto& opers = midiImportOperations;
    opers.addNewMidiFile(midiFilePath(midiFile));
    MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(midiFile));
    auto& data = *opers.data();

    data.trackOpers.quantValue.setDefaultValue(MidiOperations::QuantValue::Q_64, false);

    dontSimplify(midiFile.toStdString().c_str());
}

TEST_F(MidiImportTests, tupletNonuplet4_4) {
    // requires 1/64 quantization
    QString midiFile("tuplet_nonuplet_4-4");
    auto& opers = midiImportOperations;
    opers.addNewMidiFile(midiFilePath(midiFile));
    MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(midiFile));
    auto& data = *opers.data();

    data.trackOpers.quantValue.setDefaultValue(MidiOperations::QuantValue::Q_64, false);

    dontSimplify(midiFile.toStdString().c_str());
}

TEST_F(MidiImportTests, tupletQuadruplet) {
    dontSimplify("tuplet_quadruplet");
}

TEST_F(MidiImportTests, tupletSeptuplet) {
    dontSimplify("tuplet_septuplet");
}

TEST_F(MidiImportTests, tupletTripletsMixed) {
    dontSimplify("tuplet_triplets_mixed");
}

TEST_F(MidiImportTests, tupletTriplet) {
    dontSimplify("tuplet_triplet");
}

TEST_F(MidiImportTests, tupletTripletFirstTied) {
    dontSimplify("tuplet_triplet_first_tied");
}

TEST_F(MidiImportTests, tupletTripletFirstTied2) {
    dontSimplify("tuplet_triplet_first_tied2");
}

TEST_F(MidiImportTests, tupletTripletLastTied) {
    dontSimplify("tuplet_triplet_last_tied");
}

TEST_F(MidiImportTests, tupletTied3_5) {
    // requires 1/32 quantization
    QString midiFile("tuplet_tied_3_5_tuplets");
    auto& opers = midiImportOperations;
    opers.addNewMidiFile(midiFilePath(midiFile));
    MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(midiFile));
    auto& data = *opers.data();

    data.trackOpers.quantValue.setDefaultValue(MidiOperations::QuantValue::Q_32, false);
    dontSimplify(midiFile.toStdString().c_str());
}

TEST_F(MidiImportTests, tupletTied3_5_2) {
    // requires 1/32 quantization
    QString midiFile("tuplet_tied_3_5_tuplets2");
    auto& opers = midiImportOperations;
    opers.addNewMidiFile(midiFilePath(midiFile));
    MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(midiFile));
    auto& data = *opers.data();

    data.trackOpers.quantValue.setDefaultValue(MidiOperations::QuantValue::Q_32, false);
    dontSimplify(midiFile.toStdString().c_str());
}

TEST_F(MidiImportTests, tupletOffTimeOtherBar) {
    dontSimplify("tuplet_off_time_other_bar");
}

TEST_F(MidiImportTests, tupletOffTimeOtherBar2) {
    dontSimplify("tuplet_off_time_other_bar2");
}

TEST_F(MidiImportTests, tuplet16th8th) {
    dontSimplify("tuplet_16th_8th");
}

TEST_F(MidiImportTests, tuplet7Staccato) {
    noTempoText("tuplet_7_staccato");
}

TEST_F(MidiImportTests, minDuration) {
    dontSimplify("min_duration");
}

TEST_F(MidiImportTests, pickupMeasure) {
    dontSimplify("pickup");
}

TEST_F(MidiImportTests, pickupMeasureLong) {
    noTempoText("pickup_long");
}

TEST_F(MidiImportTests, pickupMeasureTurnOff) {
    noTempoText("pickup_turn_off");
}

// LH/RH separation

TEST_F(MidiImportTests, LHRH_Nontuplet) {
    staffSplit("split_nontuplet");
}

TEST_F(MidiImportTests, LHRH_Acid) {
    staffSplit("split_acid");
}

TEST_F(MidiImportTests, LHRH_Tuplet) {
    staffSplit("split_tuplet");
}

TEST_F(MidiImportTests, LHRH_2melodies) {
    staffSplit("split_2_melodies");
}

TEST_F(MidiImportTests, LHRH_octave) {
    staffSplit("split_octave");
}

// swing

TEST_F(MidiImportTests, swingTriplets) {
    QString midiFile("swing_triplets");
    auto& opers = midiImportOperations;
    opers.addNewMidiFile(midiFilePath(midiFile));
    MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(midiFile));
    auto& data = *opers.data();

    data.trackOpers.swing.setDefaultValue(MidiOperations::Swing::SWING, false);
    data.trackOpers.doStaffSplit.setDefaultValue(false, false);
    data.trackOpers.simplifyDurations.setDefaultValue(false, false);
    data.trackOpers.maxVoiceCount.setDefaultValue(MidiOperations::VoiceCount::V_1, false);
    data.trackOpers.showTempoText.setDefaultValue(false);

    importThenCompareWithRef(midiFile.toStdString().c_str());
}

TEST_F(MidiImportTests, swingShuffle) {
    QString midiFile("swing_shuffle");
    auto& opers = midiImportOperations;
    opers.addNewMidiFile(midiFilePath(midiFile));
    MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(midiFile));
    auto& data = *opers.data();

    data.trackOpers.swing.setDefaultValue(MidiOperations::Swing::SHUFFLE, false);
    data.trackOpers.doStaffSplit.setDefaultValue(false, false);
    data.trackOpers.simplifyDurations.setDefaultValue(false, false);
    data.trackOpers.maxVoiceCount.setDefaultValue(MidiOperations::VoiceCount::V_1, false);
    data.trackOpers.showTempoText.setDefaultValue(false);

    importThenCompareWithRef(midiFile.toStdString().c_str());
}

TEST_F(MidiImportTests, swingClef) {
    QString midiFile("swing_clef");
    auto& opers = midiImportOperations;
    opers.addNewMidiFile(midiFilePath(midiFile));
    MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(midiFile));
    auto& data = *opers.data();

    data.trackOpers.swing.setDefaultValue(MidiOperations::Swing::SWING, false);
    data.trackOpers.changeClef.setDefaultValue(true, false);
    data.trackOpers.doStaffSplit.setDefaultValue(false, false);
    data.trackOpers.simplifyDurations.setDefaultValue(false, false);
    data.trackOpers.maxVoiceCount.setDefaultValue(MidiOperations::VoiceCount::V_1, false);
    data.trackOpers.showTempoText.setDefaultValue(false);

    importThenCompareWithRef(midiFile.toStdString().c_str());
}

// percussion

// TODO: update ref
TEST_F(MidiImportTests, DISABLED_percDrums) {
    noTempoText("perc_drums");
}

// TODO: update ref
TEST_F(MidiImportTests, DISABLED_percRemoveTies) {
    noTempoText("perc_remove_ties");
}

// TODO: update ref
TEST_F(MidiImportTests, DISABLED_percNoGrandStaff) {
    noTempoText("perc_no_grand_staff");
}

// TODO: update ref
TEST_F(MidiImportTests, DISABLED_percTriplet) {
    noTempoText("perc_triplet");
}

// TODO: update ref
TEST_F(MidiImportTests, DISABLED_percRespectBeat) {
    noTempoText("perc_respect_beat");
}

// TODO: update ref
TEST_F(MidiImportTests, DISABLED_percTupletVoice) {
    noTempoText("perc_tuplet_voice");
}

// TODO: update ref
TEST_F(MidiImportTests, DISABLED_percTupletSimplify) {
    noTempoText("perc_tuplet_simplify");
}

// TODO: update ref
TEST_F(MidiImportTests, DISABLED_percTupletSimplify2) {
    noTempoText("perc_tuplet_simplify2");
}

// TODO: update ref
TEST_F(MidiImportTests, DISABLED_percShortNotes) {
    noTempoText("perc_short_notes");
}

// clef changes along the score

TEST_F(MidiImportTests, clefTied) {
    dontSimplify("clef_tied");
}

TEST_F(MidiImportTests, clefMelody) {
    dontSimplify("clef_melody");
}

TEST_F(MidiImportTests, clefPrev) {
    dontSimplify("clef_prev");
}

// duration simplification

TEST_F(MidiImportTests, simplify16thStaccato) {
    simplification("simplify_16th_staccato");
}

TEST_F(MidiImportTests, simplify8thDont) {
    simplification("simplify_8th_dont");
}

TEST_F(MidiImportTests, simplify32ndStaccato) {
    simplification("simplify_32nd_staccato");
}

TEST_F(MidiImportTests, simplify8thDottedNoStaccato) {
    simplification("simplify_8th_dotted_no_staccato");
}

TEST_F(MidiImportTests, simplify4thDottedTied) {
    simplification("simplify_4th_dotted_tied");
}

TEST_F(MidiImportTests, simplifyTripletStaccato) {
    simplification("simplify_triplet_staccato");
}

TEST_F(MidiImportTests, simplifyDotted3_4) {
    simplification("simplify_dotted_3-4");
}

TEST_F(MidiImportTests, simplifyStaccato9_8) {
    simplification("simplify_staccato_9-8");
}

// voice separation

TEST_F(MidiImportTests, voiceSeparationAcid) {
    voiceSeparation("voice_acid");
}

TEST_F(MidiImportTests, voiceSeparationIntersect) {
    voiceSeparation("voice_intersect");
}

TEST_F(MidiImportTests, voiceSeparationTuplet) {
    voiceSeparation("voice_tuplet", true);
}

TEST_F(MidiImportTests, voiceSeparationCentral) {
    voiceSeparation("voice_central");
}

// division (fps and ticks per frame case)

TEST_F(MidiImportTests, division) {
    importThenCompareWithRef("division");
}

// MIDI instruments and Grand Staff

TEST_F(MidiImportTests, instrumentGrand) {
    importThenCompareWithRef("instrument_grand");
}

TEST_F(MidiImportTests, instrumentGrand2) {
    importThenCompareWithRef("instrument_grand2");
}

// TODO: update ref
TEST_F(MidiImportTests, DISABLED_instrumentChannels) {
    importThenCompareWithRef("instrument_channels");
}

TEST_F(MidiImportTests, instrument3StaffOrgan) {
    importThenCompareWithRef("instrument_3staff_organ");
}

// TODO: update ref
TEST_F(MidiImportTests, DISABLED_instrumentClef) {
    noTempoText("instrument_clef");
}

// lyrics

TEST_F(MidiImportTests, lyricsTime0) {
    noTempoText("lyrics_time_0");
}

TEST_F(MidiImportTests, lyricsVoice1) {
    noTempoText("lyrics_voice_1");
}

//---------------------------------------------------------
//  tuplet recognition functions
//---------------------------------------------------------

TEST_F(MidiImportTests, findChordInBar) {
    std::multimap<ReducedFraction, MidiChord> chords;
    chords.insert({ ReducedFraction::fromTicks(10), MidiChord() });
    chords.insert({ ReducedFraction::fromTicks(360), MidiChord() });
    chords.insert({ ReducedFraction::fromTicks(480), MidiChord() });
    chords.insert({ ReducedFraction::fromTicks(1480), MidiChord() });
    chords.insert({ ReducedFraction::fromTicks(2000), MidiChord() });
    chords.insert({ ReducedFraction::fromTicks(3201), MidiChord() });

    ReducedFraction startBarTick;
    ReducedFraction endBarTick = ReducedFraction::fromTicks(4 * engraving::Constants::DIVISION);   // 4/4

    auto firstChordIt = MChord::findFirstChordInRange(startBarTick, endBarTick,
                                                      chords.begin(), chords.end());
    EXPECT_EQ(firstChordIt, chords.begin());
    firstChordIt = MChord::findFirstChordInRange(chords, startBarTick, endBarTick);
    EXPECT_EQ(firstChordIt, chords.begin());

    auto endChordIt = MChord::findEndChordInRange(endBarTick, firstChordIt, chords.end());
    EXPECT_EQ(endChordIt, chords.find(ReducedFraction::fromTicks(2000)));
    endChordIt = chords.lower_bound(endBarTick);
    EXPECT_EQ(endChordIt, chords.find(ReducedFraction::fromTicks(2000)));

    endBarTick = ReducedFraction(0, 1);

    firstChordIt = MChord::findFirstChordInRange(startBarTick, endBarTick,
                                                 chords.begin(), chords.end());
    EXPECT_EQ(firstChordIt, chords.end());
    firstChordIt = MChord::findFirstChordInRange(chords, startBarTick, endBarTick);
    EXPECT_EQ(firstChordIt, chords.end());

    endChordIt = MChord::findEndChordInRange(endBarTick, firstChordIt, chords.end());
    EXPECT_EQ(endChordIt, chords.end());

    startBarTick = ReducedFraction::fromTicks(10);
    endBarTick = ReducedFraction::fromTicks(-100);

    firstChordIt = MChord::findFirstChordInRange(startBarTick, endBarTick,
                                                 chords.begin(), chords.end());
    EXPECT_EQ(firstChordIt, chords.end());
    firstChordIt = MChord::findFirstChordInRange(chords, startBarTick, endBarTick);
    EXPECT_EQ(firstChordIt, chords.end());

    endChordIt = MChord::findEndChordInRange(endBarTick, firstChordIt, chords.end());
    EXPECT_EQ(endChordIt, chords.end());
}

// tupletNoteNumber - number of note in tuplet (like index):
// i.e. for triplet notes can have numbers 1, 2, 3

static void isSingleNoteInTupletAllowed(int tupletNumber,
                                        int tupletNoteNumber,
                                        double noteLenInTupletLen,
                                        bool expectedResult)
{
    MidiTuplet::TupletInfo tupletInfo;
    tupletInfo.len = ReducedFraction::fromTicks(engraving::Constants::DIVISION);

    std::multimap<ReducedFraction, MidiChord> chords;
    MidiChord chord;
    MidiNote note;
    const ReducedFraction onTime = ReducedFraction::fromTicks(10);
    note.offTime = onTime + ReducedFraction::fromTicks(
        qRound(tupletInfo.len.ticks() * noteLenInTupletLen));
    chord.notes.push_back(note);
    chords.insert({ onTime, chord });

    tupletInfo.chords.insert({ onTime, chords.begin() });
    // tuplet error is less than regular error => allowed
    tupletInfo.tupletSumError = { 0, 1 };
    tupletInfo.regularSumError = ReducedFraction::fromTicks(1);
    tupletInfo.tupletNumber = tupletNumber;
    tupletInfo.firstChordIndex = tupletNoteNumber - 1;
    EXPECT_EQ(MidiTuplet::isTupletAllowed(tupletInfo), expectedResult);
}

static void isChordCountInTupletAllowed(int tupletNumber,
                                        int chordCount,
                                        bool expectedResult)
{
    MidiTuplet::TupletInfo tupletInfo;
    tupletInfo.len = ReducedFraction::fromTicks(engraving::Constants::DIVISION);

    std::multimap<ReducedFraction, MidiChord> chords;
    tupletInfo.firstChordIndex = 0;
    for (int i = 0; i != chordCount; ++i) {
        MidiChord chord;
        mu::iex::midi::MidiNote note;
        const ReducedFraction onTime = tupletInfo.len / tupletNumber * i;
        note.offTime = onTime + tupletInfo.len / tupletNumber;     // allowed
        chord.notes.push_back(note);
        const auto lastChordIt = chords.insert({ onTime, chord });
        tupletInfo.chords.insert({ onTime, lastChordIt });
    }
    // tuplet error is less than regular error => allowed
    tupletInfo.tupletSumError = { 0, 1 };
    tupletInfo.regularSumError = ReducedFraction::fromTicks(1);
    tupletInfo.tupletNumber = tupletNumber;
    EXPECT_EQ(MidiTuplet::isTupletAllowed(tupletInfo), expectedResult);
}

static void isTupletErrorAllowed(int tupletSumError,
                                 int regularSumError,
                                 bool expectedResult)
{
    MidiTuplet::TupletInfo tupletInfo;
    tupletInfo.tupletNumber = 3;
    tupletInfo.len = ReducedFraction::fromTicks(engraving::Constants::DIVISION);

    std::multimap<ReducedFraction, MidiChord> chords;
    MidiChord chord;
    MidiNote note;
    const ReducedFraction onTime = ReducedFraction::fromTicks(10);
    note.offTime = onTime + tupletInfo.len / tupletInfo.tupletNumber;    // allowed
    chord.notes.push_back(note);
    chords.insert({ onTime, chord });

    tupletInfo.chords.insert({ onTime, chords.begin() });
    tupletInfo.tupletSumError = { tupletSumError, 1 };
    tupletInfo.regularSumError = { regularSumError, 1 };
    EXPECT_EQ(MidiTuplet::isTupletAllowed(tupletInfo), expectedResult);
}

TEST_F(MidiImportTests, isTupletAllowed) {
    auto& opers = midiImportOperations;
    const QString fileName = "dummy";
    opers.addNewMidiFile(fileName);
    MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, fileName);

    // special cases
    //

    // duplets
    isSingleNoteInTupletAllowed(2, 1, 1.0 / 2, true);
    isSingleNoteInTupletAllowed(2, 1, 2.0 / 2, false);
    isSingleNoteInTupletAllowed(2, 1, 1.0, false);

    isSingleNoteInTupletAllowed(2, 2, 1.0 / 2, true);
    isSingleNoteInTupletAllowed(2, 2, 2.0 / 2, true);
    isSingleNoteInTupletAllowed(2, 2, 1.0, true);

    // triplets
    isSingleNoteInTupletAllowed(3, 1, 1.0 / 3, true);
    isSingleNoteInTupletAllowed(3, 1, 2.0 / 3, false);
    isSingleNoteInTupletAllowed(3, 1, 1.0, false);

    isSingleNoteInTupletAllowed(3, 2, 1.0 / 3, true);
    isSingleNoteInTupletAllowed(3, 2, 2.0 / 3, true);
    isSingleNoteInTupletAllowed(3, 2, 1.0, true);

    // quintuplets
    // too small note count - all these cases are not allowed
    isSingleNoteInTupletAllowed(5, 1, 1.0 / 5, false);
    isSingleNoteInTupletAllowed(5, 1, 2.0 / 5, false);
    isSingleNoteInTupletAllowed(5, 1, 1.0, false);

    isSingleNoteInTupletAllowed(5, 2, 1.0 / 5, false);
    isSingleNoteInTupletAllowed(5, 2, 2.0 / 5, false);
    isSingleNoteInTupletAllowed(5, 2, 1.0, false);

    // all tuplet cases
    //
    isChordCountInTupletAllowed(2, 1, true);
    isChordCountInTupletAllowed(2, 2, true);
    isChordCountInTupletAllowed(2, 3, true);   // special case with tuplet subdivision

    isChordCountInTupletAllowed(3, 1, true);
    isChordCountInTupletAllowed(3, 2, true);
    isChordCountInTupletAllowed(3, 3, true);
    isChordCountInTupletAllowed(3, 4, true);   // special case with tuplet subdivision

    isChordCountInTupletAllowed(4, 1, false);
    isChordCountInTupletAllowed(4, 2, false);
    isChordCountInTupletAllowed(4, 3, true);
    isChordCountInTupletAllowed(4, 4, true);
    isChordCountInTupletAllowed(4, 5, true);   // special case with tuplet subdivision

    isChordCountInTupletAllowed(5, 1, false);
    isChordCountInTupletAllowed(5, 2, false);
    isChordCountInTupletAllowed(5, 3, true);
    isChordCountInTupletAllowed(5, 4, true);
    isChordCountInTupletAllowed(5, 5, true);
    isChordCountInTupletAllowed(5, 6, true);   // special case with tuplet subdivision

    isChordCountInTupletAllowed(7, 1, false);
    isChordCountInTupletAllowed(7, 2, false);
    isChordCountInTupletAllowed(7, 3, false);
    isChordCountInTupletAllowed(7, 4, true);
    isChordCountInTupletAllowed(7, 5, true);
    isChordCountInTupletAllowed(7, 6, true);
    isChordCountInTupletAllowed(7, 7, true);
    isChordCountInTupletAllowed(7, 8, true);   // special case with tuplet subdivision

    // tuplet error should be less than regular error
    isTupletErrorAllowed(4, 5, true);
    isTupletErrorAllowed(5, 5, false);
    isTupletErrorAllowed(6, 5, false);
}

TEST_F(MidiImportTests, findTupletNumbers) {
    auto& opers = midiImportOperations;
    opers.addNewMidiFile("");
    MidiOperations::CurrentTrackSetter setCurrentTrack{ opers, 0 };
    {
        const ReducedFraction barFraction(4, 4);
        const ReducedFraction divLen = barFraction / 4;
        const auto numbers = MidiTuplet::findTupletNumbers(divLen, barFraction);
        EXPECT_EQ(numbers.size(), 4);
        EXPECT_EQ(numbers[0], 3);
        EXPECT_EQ(numbers[1], 5);
        EXPECT_EQ(numbers[2], 7);
        EXPECT_EQ(numbers[3], 9);
    }
    {
        const ReducedFraction barFraction(6, 8);
        const ReducedFraction divLen = barFraction / 2;
        const auto numbers = MidiTuplet::findTupletNumbers(divLen, barFraction);
        EXPECT_EQ(numbers.size(), 1);
        EXPECT_EQ(numbers[0], 4);
        // duplets are turned off by default
    }
}

TEST_F(MidiImportTests, findOnTimeRegularError) {
    ReducedFraction quantValue = ReducedFraction::fromTicks(engraving::Constants::DIVISION) / 4;    // 1/16
    MidiChord chord;
    MidiNote note;
    note.offTime = quantValue * 4;
    chord.notes.push_back(note);
    const auto onTime = quantValue + ReducedFraction::fromTicks(12);
    std::pair<const ReducedFraction, MidiChord> pair(onTime, chord);
    EXPECT_EQ(Quantize::findOnTimeQuantError(pair, quantValue),
              ReducedFraction::fromTicks(12));
}

TEST_F(MidiImportTests, findTupletApproximation) {
    const int tupletNumber = 3;
    const ReducedFraction tupletLen = ReducedFraction::fromTicks(engraving::Constants::DIVISION);
    const ReducedFraction quantValue = ReducedFraction::fromTicks(engraving::Constants::DIVISION) / 4;    // 1/16

    std::multimap<ReducedFraction, MidiChord> chords;
    MidiChord chord;
    MidiNote note;
    chord.notes.push_back(note);

    chord.notes.back().offTime = ReducedFraction::fromTicks(160);
    chords.insert({ ReducedFraction::fromTicks(0), chord });

    chord.notes.back().offTime = ReducedFraction::fromTicks(320);
    chords.insert({ ReducedFraction::fromTicks(160), chord });

    chord.notes.back().offTime = ReducedFraction::fromTicks(480);
    chords.insert({ ReducedFraction::fromTicks(320), chord });

    chord.notes.back().offTime = ReducedFraction::fromTicks(1480);
    chords.insert({ ReducedFraction::fromTicks(480), chord });

    chord.notes.back().offTime = ReducedFraction::fromTicks(2000);
    chords.insert({ ReducedFraction::fromTicks(1480), chord });

    chord.notes.back().offTime = ReducedFraction::fromTicks(3201);
    chords.insert({ ReducedFraction::fromTicks(2000), chord });

    chord.notes.back().offTime = ReducedFraction::fromTicks(4000);
    chords.insert({ ReducedFraction::fromTicks(3201), chord });

    {
        const ReducedFraction startTupletTime = ReducedFraction(0, 1);
        const MidiTuplet::TupletInfo tupletApprox = MidiTuplet::findTupletApproximation(
            tupletLen, tupletNumber, quantValue,
            startTupletTime, chords.begin(), chords.end()
            );
        EXPECT_EQ(tupletApprox.onTime, startTupletTime);
        EXPECT_EQ(tupletApprox.len, tupletLen);
        EXPECT_EQ(tupletApprox.tupletNumber, tupletNumber);
        EXPECT_EQ(tupletApprox.chords.size(), 3);
        EXPECT_EQ(tupletApprox.tupletSumError, ReducedFraction::fromTicks(0));
        EXPECT_EQ(tupletApprox.regularSumError, ReducedFraction::fromTicks(80));
        EXPECT_EQ(tupletApprox.chords.find(ReducedFraction::fromTicks(0))->second,
                  chords.find(ReducedFraction::fromTicks(0)));
        EXPECT_EQ(tupletApprox.chords.find(ReducedFraction::fromTicks(160))->second,
                  chords.find(ReducedFraction::fromTicks(160)));
        EXPECT_EQ(tupletApprox.chords.find(ReducedFraction::fromTicks(320))->second,
                  chords.find(ReducedFraction::fromTicks(320)));
    }
    {
        const ReducedFraction startTupletTime = ReducedFraction::fromTicks(960);
        const MidiTuplet::TupletInfo tupletApprox = MidiTuplet::findTupletApproximation(
            tupletLen, tupletNumber, quantValue,
            startTupletTime, chords.begin(), chords.end()
            );
        EXPECT_EQ(tupletApprox.chords.size(), 0);
    }
    {
        const ReducedFraction startTupletTime = ReducedFraction::fromTicks(1440);
        const MidiTuplet::TupletInfo tupletApprox = MidiTuplet::findTupletApproximation(
            tupletLen, tupletNumber, quantValue,
            startTupletTime, chords.begin(), chords.end()
            );
        EXPECT_EQ(tupletApprox.chords.size(), 1);
        EXPECT_EQ(tupletApprox.tupletSumError, ReducedFraction::fromTicks(40));
        EXPECT_EQ(tupletApprox.regularSumError, ReducedFraction::fromTicks(40));
        EXPECT_EQ(tupletApprox.chords.find(ReducedFraction::fromTicks(1480))->second,
                  chords.find(ReducedFraction::fromTicks(1480)));
    }
}

//--------------------------------------------------------------------------
// tuplet voice separation

static MidiNote noteFactory(const ReducedFraction& offTime, int pitch)
{
    MidiNote note;
    note.offTime = offTime;
    note.pitch = pitch;
    return note;
}

static MidiChord chordFactory(const ReducedFraction& offTime, const std::vector<int>& pitches)
{
    std::vector<MidiNote> notes;
    for (const auto& pitch: pitches) {
        notes.push_back(noteFactory(offTime, pitch));
    }
    MidiChord chord;
    if (notes.empty()) {
        return chord;
    }
    for (const auto& note: notes) {
        chord.notes.push_back(note);
    }

    return chord;
}

TEST_F(MidiImportTests, separateTupletVoices) {
    const ReducedFraction tupletLen = ReducedFraction::fromTicks(engraving::Constants::DIVISION);
    std::multimap<ReducedFraction, MidiChord> chords;
    // let's create 3 tuplets with the same first chord

    // triplet
    const ReducedFraction tripletLen = tupletLen;
    const int tripletNumber = 3;
    const ReducedFraction tripletNoteLen = tripletLen / tripletNumber;

    const MidiChord firstChord = chordFactory(tripletNoteLen, { 76, 71, 67 });
    chords.insert({ { 0, 1 }, firstChord });

    std::vector<int> pitches = { 74, 77 };
    for (int i = 1; i != tripletNumber; ++i) {
        chords.insert({ tripletNoteLen* i,
                        chordFactory(tripletNoteLen * (i + 1), { pitches[i - 1] }) });
    }
    // quintuplet
    const ReducedFraction quintupletLen = tupletLen;
    const int quintupletNumber = 5;
    const ReducedFraction quintupletNoteLen = quintupletLen / quintupletNumber;
    pitches = { 60, 62, 58, 60 };
    for (int i = 1; i != quintupletNumber; ++i) {
        chords.insert({ quintupletNoteLen* i,
                        chordFactory(quintupletNoteLen * (i + 1), { pitches[i - 1] }) });
    }
    // septuplet
    const ReducedFraction septupletLen = tupletLen * 2;
    const int septupletNumber = 7;
    const ReducedFraction septupletNoteLen = septupletLen / septupletNumber;
    pitches = { 50, 52, 48, 51, 47, 47 };
    for (int i = 1; i != septupletNumber; ++i) {
        chords.insert({ septupletNoteLen* i,
                        chordFactory(septupletNoteLen * (i + 1), { pitches[i - 1] }) });
    }

    MidiTuplet::TupletInfo tripletInfo;
    tripletInfo.onTime = ReducedFraction(0, 1);
    tripletInfo.len = tripletLen;
    tripletInfo.firstChordIndex = 0;
    for (int i = 0; i != 3; ++i) {
        const auto onTime = tripletNoteLen * i;
        tripletInfo.chords.insert({ onTime, chords.find(onTime) });
    }

    MidiTuplet::TupletInfo quintupletInfo;
    quintupletInfo.onTime = ReducedFraction(0, 1);
    quintupletInfo.len = quintupletLen;
    quintupletInfo.firstChordIndex = 0;
    for (int i = 0; i != 5; ++i) {
        const auto onTime = quintupletNoteLen * i;
        quintupletInfo.chords.insert({ onTime, chords.find(onTime) });
    }

    MidiTuplet::TupletInfo septupletInfo;
    septupletInfo.onTime = ReducedFraction(0, 1);
    septupletInfo.len = septupletLen;
    septupletInfo.firstChordIndex = 0;
    for (int i = 0; i != 7; ++i) {
        const auto onTime = septupletNoteLen * i;
        septupletInfo.chords.insert({ onTime, chords.find(onTime) });
    }

    std::vector<MidiTuplet::TupletInfo> tuplets;
    tuplets.push_back(tripletInfo);
    tuplets.push_back(quintupletInfo);
    tuplets.push_back(septupletInfo);

    EXPECT_EQ(chords.size(), 13);

    const auto firstChordTime = ReducedFraction(0, 1);

    auto tripletIt = tripletInfo.chords.find(firstChordTime);    // first chord in tuplet
    EXPECT_EQ(tripletIt->second->second.notes.size(), 3);
    EXPECT_EQ(tripletIt->second->second.notes[0].pitch, 76);
    EXPECT_EQ(tripletIt->second->second.notes[1].pitch, 71);
    EXPECT_EQ(tripletIt->second->second.notes[2].pitch, 67);

    auto quintupletIt = quintupletInfo.chords.find(firstChordTime);
    EXPECT_EQ(quintupletIt->second->second.notes.size(), 3);
    EXPECT_EQ(quintupletIt->second->second.notes[0].pitch, 76);
    EXPECT_EQ(quintupletIt->second->second.notes[1].pitch, 71);
    EXPECT_EQ(quintupletIt->second->second.notes[2].pitch, 67);

    auto septupletIt = septupletInfo.chords.find(firstChordTime);
    EXPECT_EQ(septupletIt->second->second.notes.size(), 3);
    EXPECT_EQ(septupletIt->second->second.notes[0].pitch, 76);
    EXPECT_EQ(septupletIt->second->second.notes[1].pitch, 71);
    EXPECT_EQ(septupletIt->second->second.notes[2].pitch, 67);

    MidiTuplet::splitFirstTupletChords(tuplets, chords);
    EXPECT_EQ(chords.size(), 15);

    tripletInfo = tuplets[0];
    quintupletInfo = tuplets[1];
    septupletInfo = tuplets[2];

    tripletIt = tripletInfo.chords.find(firstChordTime);
    EXPECT_EQ(tripletIt->second->second.notes.size(), 1);
    EXPECT_EQ(tripletIt->second->second.notes[0].pitch, 76);

    quintupletIt = quintupletInfo.chords.find(firstChordTime);
    EXPECT_EQ(quintupletIt->second->second.notes.size(), 1);
    EXPECT_EQ(quintupletIt->second->second.notes[0].pitch, 71);

    septupletIt = septupletInfo.chords.find(firstChordTime);
    EXPECT_EQ(septupletIt->second->second.notes.size(), 1);
    EXPECT_EQ(septupletIt->second->second.notes[0].pitch, 67);
}

TEST_F(MidiImportTests, findLongestUncommonGroup) {
    std::vector<MidiTuplet::TupletInfo> tuplets;
    MidiTuplet::TupletInfo info;
    const ReducedFraction basicQuant = ReducedFraction::fromTicks(engraving::Constants::DIVISION) / 4;    // 1/16
    // 0
    info.onTime = { 5, 8 };
    info.len = { 1, 8 };
    tuplets.push_back(info);
    // 1
    info.onTime = { 3, 4 };
    info.len = { 1, 8 };
    tuplets.push_back(info);
    // 2
    info.onTime = { 7, 8 };
    info.len = { 1, 8 };
    tuplets.push_back(info);
    // 3
    info.onTime = { 1, 2 };
    info.len = { 1, 4 };
    tuplets.push_back(info);
    // 4
    info.onTime = { 3, 4 };
    info.len = { 1, 4 };
    tuplets.push_back(info);
    // 5
    info.onTime = { 1, 2 };
    info.len = { 1, 2 };
    tuplets.push_back(info);
    // 6
    info.onTime = { 0, 1 };
    info.len = { 1, 1 };
    tuplets.push_back(info);

    std::set<int> result = MidiTuplet::findLongestUncommonGroup(tuplets, basicQuant);
    EXPECT_EQ(result.size(), 3);
    EXPECT_TRUE(result == std::set<int>({ 0, 1, 2 })
                || result == std::set<int>({ 1, 2, 3 }));

    // test unsuccessful case
    tuplets.clear();
    // 0
    info.onTime = { 5, 8 };
    info.len = { 1, 8 };
    tuplets.push_back(info);
    // 1
    info.onTime = { 1, 2 };
    info.len = { 1, 2 };
    tuplets.push_back(info);
    // 2
    info.onTime = { 0, 1 };
    info.len = { 1, 1 };
    tuplets.push_back(info);

    result = MidiTuplet::findLongestUncommonGroup(tuplets, basicQuant);
    EXPECT_EQ(result.size(), 1);
}

//---------------------------------------------------------
//  metric bar analysis
//---------------------------------------------------------

TEST_F(MidiImportTests, metricDivisionsOfTuplet) {
    MidiTuplet::TupletData tupletData;
    tupletData.voice = 0;
    tupletData.len = ReducedFraction::fromTicks(480);
    tupletData.onTime = ReducedFraction::fromTicks(480);
    tupletData.tupletNumber = 3;
    const int tupletStartLevel = -3;
    Meter::DivisionInfo tupletDivInfo = Meter::metricDivisionsOfTuplet(tupletData, tupletStartLevel);

    EXPECT_EQ(tupletDivInfo.isTuplet, true);
    EXPECT_EQ(tupletDivInfo.len, ReducedFraction::fromTicks(480));
    EXPECT_EQ(tupletDivInfo.onTime, ReducedFraction::fromTicks(480));
    EXPECT_EQ(tupletDivInfo.divLengths.size(), 5);

    EXPECT_EQ(tupletDivInfo.divLengths[0].len, tupletData.len);
    EXPECT_EQ(tupletDivInfo.divLengths[0].level, Meter::TUPLET_BOUNDARY_LEVEL);

    EXPECT_EQ(tupletDivInfo.divLengths[1].len, tupletData.len / tupletData.tupletNumber);
    EXPECT_EQ(tupletDivInfo.divLengths[1].level, tupletStartLevel);

    EXPECT_EQ(tupletDivInfo.divLengths[2].len, tupletData.len / (2 * tupletData.tupletNumber));
    EXPECT_EQ(tupletDivInfo.divLengths[2].level, tupletStartLevel - 1);

    EXPECT_EQ(tupletDivInfo.divLengths[3].len, tupletData.len / (4 * tupletData.tupletNumber));
    EXPECT_EQ(tupletDivInfo.divLengths[3].level, tupletStartLevel - 2);

    EXPECT_EQ(tupletDivInfo.divLengths[4].len, tupletData.len / (8 * tupletData.tupletNumber));
    EXPECT_EQ(tupletDivInfo.divLengths[4].level, tupletStartLevel - 3);
}

TEST_F(MidiImportTests, maxLevelBetween) {
    const ReducedFraction barFraction = ReducedFraction(4, 4);
    ReducedFraction startTickInBar = ReducedFraction::fromTicks(240);
    ReducedFraction endTickInBar = ReducedFraction::fromTicks(500);

    Meter::DivisionInfo regularDivInfo = Meter::metricDivisionsOfBar(barFraction);
    EXPECT_EQ(regularDivInfo.divLengths.size(), 8);

    Meter::MaxLevel level = Meter::maxLevelBetween(startTickInBar, endTickInBar, regularDivInfo);
    EXPECT_EQ(level.level, -2);
    EXPECT_EQ(level.pos, ReducedFraction::fromTicks(480));
    EXPECT_EQ(level.levelCount, 1);

    startTickInBar = ReducedFraction::fromTicks(499);
    level = Meter::maxLevelBetween(startTickInBar, endTickInBar, regularDivInfo);
    EXPECT_EQ(level.level, 0);
    EXPECT_EQ(level.pos, ReducedFraction(-1, 1));
    EXPECT_EQ(level.levelCount, 0);

    MidiTuplet::TupletData tupletData;
    tupletData.voice = 0;
    tupletData.len = ReducedFraction::fromTicks(480);
    tupletData.onTime = ReducedFraction::fromTicks(480);
    tupletData.tupletNumber = 3;

    const int tupletStartLevel = -3;
    const Meter::DivisionInfo tupletDivInfo = Meter::metricDivisionsOfTuplet(tupletData, tupletStartLevel);
    EXPECT_EQ(tupletDivInfo.divLengths.size(), 5);

    startTickInBar = tupletData.onTime;
    endTickInBar = startTickInBar + tupletData.len;
    level = Meter::maxLevelBetween(startTickInBar, endTickInBar, tupletDivInfo);
    EXPECT_EQ(level.level, tupletStartLevel);
    EXPECT_EQ(level.pos, tupletData.onTime + tupletData.len / tupletData.tupletNumber);
    EXPECT_EQ(level.levelCount, 2);

    std::vector<Meter::DivisionInfo> divInfo;
    // first elements of vector - all tuplets division info
    // last element - whole bar (regular) division info
    divInfo.push_back(tupletDivInfo);
    divInfo.push_back(regularDivInfo);

    // s - startTickInBar
    // e - endTickInBar
    // ts - tuplet onTime
    // te - tuplet onTime + tuplet len

    // s < e < ts < te
    startTickInBar = ReducedFraction::fromTicks(0);
    endTickInBar = (startTickInBar + tupletData.onTime) / 2;
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    EXPECT_EQ(level.level, -4);
    EXPECT_EQ(level.pos, (startTickInBar + endTickInBar) / 2);
    EXPECT_EQ(level.levelCount, 1);
    // s < e == ts < te
    startTickInBar = ReducedFraction::fromTicks(0);
    endTickInBar = tupletData.onTime;
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    EXPECT_EQ(level.level, -3);
    EXPECT_EQ(level.pos, (startTickInBar + tupletData.onTime) / 2);
    EXPECT_EQ(level.levelCount, 1);
    // s < ts < e < te
    startTickInBar = ReducedFraction::fromTicks(0);
    endTickInBar = tupletData.onTime + tupletData.len / 2;
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    EXPECT_EQ(level.level, Meter::TUPLET_BOUNDARY_LEVEL);
    EXPECT_EQ(level.pos, tupletData.onTime);
    EXPECT_EQ(level.levelCount, 1);
    // s < ts < e == te
    startTickInBar = ReducedFraction::fromTicks(0);
    endTickInBar = tupletData.onTime + tupletData.len;
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    EXPECT_EQ(level.level, Meter::TUPLET_BOUNDARY_LEVEL);
    EXPECT_EQ(level.pos, tupletData.onTime);
    EXPECT_EQ(level.levelCount, 1);
    // s < ts < te < e
    startTickInBar = ReducedFraction::fromTicks(0);
    endTickInBar = tupletData.onTime + tupletData.len + ReducedFraction(1, 3);
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    EXPECT_EQ(level.level, Meter::TUPLET_BOUNDARY_LEVEL);
    EXPECT_EQ(level.pos, tupletData.onTime + tupletData.len);
    EXPECT_EQ(level.levelCount, 1);
    // s == ts < e < te
    startTickInBar = tupletData.onTime;
    endTickInBar = tupletData.onTime + tupletData.len / 2;
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    EXPECT_EQ(level.level, tupletStartLevel);
    EXPECT_EQ(level.pos, tupletData.onTime + tupletData.len / tupletData.tupletNumber);
    EXPECT_EQ(level.levelCount, 1);
    // s == ts < e == te
    startTickInBar = tupletData.onTime;
    endTickInBar = tupletData.onTime + tupletData.len;
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    EXPECT_EQ(level.level, tupletStartLevel);
    EXPECT_EQ(level.pos, tupletData.onTime + tupletData.len / tupletData.tupletNumber);
    EXPECT_EQ(level.levelCount, 2);
    // s == ts < te < e
    startTickInBar = tupletData.onTime;
    endTickInBar = tupletData.onTime + tupletData.len * 2;
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    EXPECT_EQ(level.level, Meter::TUPLET_BOUNDARY_LEVEL);
    EXPECT_EQ(level.pos, tupletData.onTime + tupletData.len);
    EXPECT_EQ(level.levelCount, 1);
    // ts < s < e < te
    startTickInBar = tupletData.onTime + tupletData.len / tupletData.tupletNumber;
    endTickInBar = tupletData.onTime + tupletData.len * 2 / tupletData.tupletNumber;
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    EXPECT_EQ(level.level, tupletStartLevel - 1);
    EXPECT_EQ(level.pos, (startTickInBar + endTickInBar) / 2);
    EXPECT_EQ(level.levelCount, 1);
    // ts < s < e = te
    startTickInBar = tupletData.onTime + tupletData.len / tupletData.tupletNumber;
    endTickInBar = tupletData.onTime + tupletData.len;
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    EXPECT_EQ(level.level, tupletStartLevel);
    EXPECT_EQ(level.pos, tupletData.onTime + tupletData.len * 2 / tupletData.tupletNumber);
    EXPECT_EQ(level.levelCount, 1);
    // ts < s < te < e
    startTickInBar = tupletData.onTime + tupletData.len / 2;
    endTickInBar = tupletData.onTime + tupletData.len + ReducedFraction(1, 3);
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    EXPECT_EQ(level.level, Meter::TUPLET_BOUNDARY_LEVEL);
    EXPECT_EQ(level.pos, tupletData.onTime + tupletData.len);
    EXPECT_EQ(level.levelCount, 1);
    // ts < te = s < e
    startTickInBar = tupletData.onTime + tupletData.len;
    endTickInBar = tupletData.onTime + tupletData.len + ReducedFraction(1, 3);
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    EXPECT_NE(level.level, Meter::TUPLET_BOUNDARY_LEVEL);
    // ts < te < s < e
    startTickInBar = tupletData.onTime + tupletData.len + ReducedFraction(1, 3);
    endTickInBar = startTickInBar + ReducedFraction(1, 3);
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    EXPECT_NE(level.level, Meter::TUPLET_BOUNDARY_LEVEL);
}

TEST_F(MidiImportTests, isSimpleDuration) {
    EXPECT_TRUE(Meter::isSimpleNoteDuration({ 4, 2 }));
    EXPECT_TRUE(Meter::isSimpleNoteDuration({ 4, 1 }));
    EXPECT_TRUE(Meter::isSimpleNoteDuration({ 2, 2 }));
    EXPECT_TRUE(Meter::isSimpleNoteDuration({ 2, 1 }));
    EXPECT_TRUE(Meter::isSimpleNoteDuration({ 1, 1 }));
    EXPECT_TRUE(Meter::isSimpleNoteDuration({ 1, 2 }));
    EXPECT_TRUE(Meter::isSimpleNoteDuration({ 1, 4 }));
    EXPECT_TRUE(Meter::isSimpleNoteDuration({ 1, 8 }));
    EXPECT_TRUE(Meter::isSimpleNoteDuration({ 1, 16 }));
    EXPECT_TRUE(Meter::isSimpleNoteDuration({ 1, 32 }));
    EXPECT_TRUE(Meter::isSimpleNoteDuration({ 1, 64 }));
    EXPECT_TRUE(Meter::isSimpleNoteDuration({ 1, 128 }));

    EXPECT_TRUE(!Meter::isSimpleNoteDuration({ 1, 6 }));
    EXPECT_TRUE(!Meter::isSimpleNoteDuration({ 3, 2 }));
    EXPECT_TRUE(!Meter::isSimpleNoteDuration({ 12, 8 }));
    EXPECT_TRUE(!Meter::isSimpleNoteDuration({ 3, 16 }));
    EXPECT_TRUE(!Meter::isSimpleNoteDuration({ 3, 4 }));
    EXPECT_TRUE(!Meter::isSimpleNoteDuration({ 3, 8 }));
    EXPECT_TRUE(!Meter::isSimpleNoteDuration({ 1, 5 }));
}

// gui - tracks model

static int findColByHeader(const TracksModel& model, const char* colHeader)
{
    const int colCount = model.columnCount(QModelIndex());
    for (int i = 0; i != colCount; ++i) {
        const QString headerTitle = model.headerData(
            i, Qt::Horizontal, Qt::DisplayRole).toString();
        if (headerTitle == QObject::tr(colHeader)) {
            return i;
        }
    }
    return -1;
}

TEST_F(MidiImportTests, testGuiTracksModel) {
    QString midiFile("perc_drums");
    QString midiFileFullPath = midiFilePath(midiFile);
    auto& opers = midiImportOperations;
    opers.addNewMidiFile(midiFileFullPath);
    MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFileFullPath);

    std::unique_ptr<engraving::MasterScore> score = importMidi(midiFileFullPath);
    ASSERT_TRUE(score);

    TracksModel model;
    model.reset(opers.data()->trackOpers,
                MidiLyrics::makeLyricsListForUI(),
                opers.data()->trackCount,
                midiFileFullPath,
                !opers.data()->humanBeatData.beatSet.empty(),
                opers.data()->hasTempoText,
                !opers.data()->chordNames.empty());

    EXPECT_EQ(model.trackCount(), 1);

    Qt::ItemFlags notEditableFlags = Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

    const int clefChangeCol = findColByHeader(model, "Clef\nchanges");
    EXPECT_GE(clefChangeCol, 0);
    EXPECT_EQ(model.flags(model.index(0, clefChangeCol)), notEditableFlags);

    const int voiceCol = findColByHeader(model, "Max. voices");
    EXPECT_GE(voiceCol, 0);
    EXPECT_EQ(model.flags(model.index(0, voiceCol)), notEditableFlags);

    const int channelCol = findColByHeader(model, "Channel");
    EXPECT_GE(channelCol, 0);
    EXPECT_EQ(model.flags(model.index(0, channelCol)), notEditableFlags);
}
