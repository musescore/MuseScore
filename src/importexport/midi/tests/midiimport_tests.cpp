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

#include "testbase.h"

#include "engraving/dom/mscore.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/durationtype.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/note.h"
#include "engraving/dom/keysig.h"

//#include "audio/exports/exportmidi.h"

#include "engraving/dom/mcursor.h"
#include "mtest/testutils.h"
#include "inner_func_decl.h"
#include "importexport/midiimport/internal/midiimport/importmidi_chord.h"
#include "importexport/midiimport/internal/midiimport/importmidi_tuplet.h"
#include "importexport/midiimport/internal/midiimport/importmidi_meter.h"
#include "importexport/midiimport/internal/midiimport/importmidi_inner.h"
#include "importexport/midiimport/internal/midiimport/importmidi_quant.h"
#include "importexport/midiimport/internal/midiimport/importmidi_fraction.h"
#include "importexport/midiimport/internal/midiimport/importmidi_operations.h"
#include "importexport/midiimport/internal/midiimport/importmidi_model.h"
#include "importexport/midiimport/internal/midiimport/importmidi_lyrics.h"

//#include "mscore/preferences.h"

namespace Ms {
extern Score::FileError importMidi(MasterScore*, const QString&);
}

using namespace Ms;

static const QString MIDIIMPORT_DIR("data/");

//---------------------------------------------------------
//   TestImportMidi
//---------------------------------------------------------

class TestImportMidi : public QObject, public MTest
{
    Q_OBJECT

    QString midiFilePath(const QString& fileName) const;
    QString midiFilePath(const char* fileName) const;
    void mf(const char* name) const;

    // functions that modify default settings
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
        mf(file);
    }

    void noTempoText(const char* file)
    {
        auto& opers = midiImportOperations;
        opers.addNewMidiFile(midiFilePath(file));
        MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(file));
        auto& data = *opers.data();

        data.trackOpers.showTempoText.setDefaultValue(false);
        mf(file);
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
        mf(file);
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
        mf(file);
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
        mf(file);
    }

private slots:
    void initTestCase();
    void im1() { dontSimplify("m1"); }
    void im2() { dontSimplify("m2"); }       // tie across bar line
    void im3() { dontSimplify("m3"); }       // voices, typeA, resolve with tie
    void im4() { dontSimplify("m4"); }       // voices, typeB, resolve with tie
    void im5() { dontSimplify("m5"); }       // same as m1 with division 240

    // quantization
    void quantDotted4th()
    {
        QString midiFile("quant_dotted_4th");
        auto& opers = midiImportOperations;
        opers.addNewMidiFile(midiFilePath(midiFile));
        MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(midiFile));
        auto& data = *opers.data();

        // 1/4 quantization should preserve 4th dotted note
        data.trackOpers.quantValue.setDefaultValue(MidiOperations::QuantValue::Q_4, false);
        dontSimplify(midiFile.toStdString().c_str());
    }

    // human-performed (unaligned) files
    void human4_4()
    {
        QString midiFile("human_4-4");
        auto& opers = midiImportOperations;
        opers.addNewMidiFile(midiFilePath(midiFile));
        MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(midiFile));
        auto& data = *opers.data();

        data.trackOpers.doStaffSplit.setDefaultValue(false, false);
        data.trackOpers.simplifyDurations.setDefaultValue(false, false);
        data.trackOpers.maxVoiceCount.setDefaultValue(MidiOperations::VoiceCount::V_1, false);
        data.trackOpers.showTempoText.setDefaultValue(false);
        mf(midiFile.toStdString().c_str());
    }

    void humanTempo() { mf("human_tempo"); }

    // chord detection
    void chordSmallError() { noTempoText("chord_small_error"); }
    void chordBigError() { noTempoText("chord_big_error"); }
    void chordLegato() { noTempoText("chord_legato"); }
    void chordCollect() { noTempoText("chord_collect"); }
    // very short note - don't remove note but show it with min allowed duration (1/128)
    void chordVeryShort() { dontSimplify("chord_1_tick_long"); }

    // test tuplet recognition functions
    void findChordInBar();
    void isTupletAllowed();
    void findTupletNumbers();
    void findOnTimeRegularError();
    void findTupletApproximation();
    void separateTupletVoices();
    void findLongestUncommonGroup();

    // metric bar analysis
    void metricDivisionsOfTuplet();
    void maxLevelBetween();
    void isSimpleDuration();

    // test scores for meter (duration subdivision)
    void meterTimeSig4_4() { dontSimplify("meter_4-4"); }
    void metertimeSig9_8() { dontSimplify("meter_9-8"); }
    void metertimeSig12_8() { dontSimplify("meter_12-8"); }
    void metertimeSig15_8() { dontSimplify("meter_15-8"); }
    void meterCentralLongNote() { dontSimplify("meter_central_long_note"); }
    void meterCentralLongRest() { dontSimplify("meter_central_long_rest"); }
    void meterChordExample() { dontSimplify("meter_chord_example"); }
    void meterDotsExample1() { dontSimplify("meter_dots_example1"); }
    void meterDotsExample2() { dontSimplify("meter_dots_example2"); }
    void meterDotsExample3() { dontSimplify("meter_dots_example3"); }
    void meterHalfRest3_4() { dontSimplify("meter_half_rest_3-4"); }
    void meterFirst2_8thRestsCompound() { dontSimplify("meter_first_2_8th_rests_compound"); }
    void meterLastQuarterRestCompound() { dontSimplify("meter_last_quarter_rest_compound"); }
    void meterRests() { dontSimplify("meter_rests"); }
    void meterTwoBeatsOver() { dontSimplify("meter_two_beats_over"); }
    void meterDotTie() { dontSimplify("meter_dot_tie"); }

    // time sig
    void timesigChanges() { dontSimplify("timesig_changes"); }

    // test scores for tuplets
    void tuplet2Voices3_5Tuplets()
    {
        // requires 1/32 quantization
        QString midiFile("tuplet_2_voices_3_5_tuplets");
        auto& opers = midiImportOperations;
        opers.addNewMidiFile(midiFilePath(midiFile));
        MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(midiFile));
        auto& data = *opers.data();

        data.trackOpers.quantValue.setDefaultValue(MidiOperations::QuantValue::Q_32, false);
        data.trackOpers.showTempoText.setDefaultValue(false);
        mf(midiFile.toStdString().c_str());
    }

    void tuplet2VoicesTupletNon() { noTempoText("tuplet_2_voices_tuplet_non"); }
    void tuplet3_5_7tuplets()
    {
        QString midiFile("tuplet_3_5_7_tuplets");
        auto& opers = midiImportOperations;
        opers.addNewMidiFile(midiFilePath(midiFile));
        MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(midiFile));
        auto& data = *opers.data();

        data.trackOpers.changeClef.setDefaultValue(false, false);
        data.trackOpers.doStaffSplit.setDefaultValue(false, false);
        data.trackOpers.simplifyDurations.setDefaultValue(false, false);
        data.trackOpers.showTempoText.setDefaultValue(false);
        mf(midiFile.toStdString().c_str());
    }

    void tuplet5_5TupletsRests() { dontSimplify("tuplet_5_5_tuplets_rests"); }
    void tuplet3_4() { dontSimplify("tuplet_3-4"); }
    void tupletDuplet() { dontSimplify("tuplet_duplet"); }
    void tupletMars() { dontSimplify("tuplet_mars"); }
    void tupletNonuplet3_4()
    {
        // requires 1/64 quantization
        QString midiFile("tuplet_nonuplet_3-4");
        auto& opers = midiImportOperations;
        opers.addNewMidiFile(midiFilePath(midiFile));
        MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(midiFile));
        auto& data = *opers.data();

        data.trackOpers.quantValue.setDefaultValue(MidiOperations::QuantValue::Q_64, false);
        dontSimplify(midiFile.toStdString().c_str());
    }

    void tupletNonuplet4_4()
    {
        // requires 1/64 quantization
        QString midiFile("tuplet_nonuplet_4-4");
        auto& opers = midiImportOperations;
        opers.addNewMidiFile(midiFilePath(midiFile));
        MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(midiFile));
        auto& data = *opers.data();

        data.trackOpers.quantValue.setDefaultValue(MidiOperations::QuantValue::Q_64, false);
        dontSimplify(midiFile.toStdString().c_str());
    }

    void tupletQuadruplet() { dontSimplify("tuplet_quadruplet"); }
    void tupletSeptuplet() { dontSimplify("tuplet_septuplet"); }
    void tupletTripletsMixed() { dontSimplify("tuplet_triplets_mixed"); }
    void tupletTriplet() { dontSimplify("tuplet_triplet"); }
    void tupletTripletFirstTied() { dontSimplify("tuplet_triplet_first_tied"); }
    void tupletTripletFirstTied2() { dontSimplify("tuplet_triplet_first_tied2"); }
    void tupletTripletLastTied() { dontSimplify("tuplet_triplet_last_tied"); }
    void tupletTied3_5()
    {
        // requires 1/32 quantization
        QString midiFile("tuplet_tied_3_5_tuplets");
        auto& opers = midiImportOperations;
        opers.addNewMidiFile(midiFilePath(midiFile));
        MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(midiFile));
        auto& data = *opers.data();

        data.trackOpers.quantValue.setDefaultValue(MidiOperations::QuantValue::Q_32, false);
        dontSimplify(midiFile.toStdString().c_str());
    }

    void tupletTied3_5_2()
    {
        // requires 1/32 quantization
        QString midiFile("tuplet_tied_3_5_tuplets2");
        auto& opers = midiImportOperations;
        opers.addNewMidiFile(midiFilePath(midiFile));
        MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFilePath(midiFile));
        auto& data = *opers.data();

        data.trackOpers.quantValue.setDefaultValue(MidiOperations::QuantValue::Q_32, false);
        dontSimplify(midiFile.toStdString().c_str());
    }

    void tupletOffTimeOtherBar() { dontSimplify("tuplet_off_time_other_bar"); }
    void tupletOffTimeOtherBar2() { dontSimplify("tuplet_off_time_other_bar2"); }
    void tuplet16th8th() { dontSimplify("tuplet_16th_8th"); }
    void tuplet7Staccato() { noTempoText("tuplet_7_staccato"); }
    void minDuration() { dontSimplify("min_duration"); }

    void pickupMeasure() { dontSimplify("pickup"); }
    void pickupMeasureLong() { noTempoText("pickup_long"); }
    void pickupMeasureTurnOff() { noTempoText("pickup_turn_off"); }

    // LH/RH separation
    void LHRH_Nontuplet() { staffSplit("split_nontuplet"); }
    void LHRH_Acid() { staffSplit("split_acid"); }
    void LHRH_Tuplet() { staffSplit("split_tuplet"); }
    void LHRH_2melodies() { staffSplit("split_2_melodies"); }
    void LHRH_octave() { staffSplit("split_octave"); }

    // swing
    void swingTriplets()
    {
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
        mf(midiFile.toStdString().c_str());
    }

    void swingShuffle()
    {
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
        mf(midiFile.toStdString().c_str());
    }

    void swingClef()
    {
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
        mf(midiFile.toStdString().c_str());
    }

    // percussion
    void percDrums() { noTempoText("perc_drums"); }
    void percRemoveTies() { noTempoText("perc_remove_ties"); }
    void percNoGrandStaff() { noTempoText("perc_no_grand_staff"); }
    void percTriplet() { noTempoText("perc_triplet"); }
    void percRespectBeat() { noTempoText("perc_respect_beat"); }
    void percTupletVoice() { noTempoText("perc_tuplet_voice"); }
    void percTupletSimplify() { noTempoText("perc_tuplet_simplify"); }
    void percTupletSimplify2() { noTempoText("perc_tuplet_simplify2"); }
    void percShortNotes() { noTempoText("perc_short_notes"); }

    // clef changes along the score
    void clefTied() { dontSimplify("clef_tied"); }
    void clefMelody() { dontSimplify("clef_melody"); }
    void clefPrev() { dontSimplify("clef_prev"); }

    // duration simplification
    void simplify16thStaccato() { simplification("simplify_16th_staccato"); }
    void simplify8thDont() { simplification("simplify_8th_dont"); }
    void simplify32ndStaccato() { simplification("simplify_32nd_staccato"); }
    void simplify8thDottedNoStaccato() { simplification("simplify_8th_dotted_no_staccato"); }
    void simplify4thDottedTied() { simplification("simplify_4th_dotted_tied"); }
    void simplifyTripletStaccato() { simplification("simplify_triplet_staccato"); }
    void simplifyDotted3_4() { simplification("simplify_dotted_3-4"); }
    void simplifyStaccato9_8() { simplification("simplify_staccato_9-8"); }

    // voice separation
    void voiceSeparationAcid() { voiceSeparation("voice_acid"); }
    void voiceSeparationIntersect() { voiceSeparation("voice_intersect"); }
    void voiceSeparationTuplet() { voiceSeparation("voice_tuplet", true); }
    void voiceSeparationCentral() { voiceSeparation("voice_central"); }

    // division (fps and ticks per frame case)
    void division() { mf("division"); }

    // MIDI instruments and Grand Staff
    void instrumentGrand() { mf("instrument_grand"); }
    void instrumentGrand2() { mf("instrument_grand2"); }
    void instrumentChannels() { mf("instrument_channels"); }
    void instrument3StaffOrgan() { mf("instrument_3staff_organ"); }
    void instrumentClef() { noTempoText("instrument_clef"); }

    // lyrics
    void lyricsTime0() { noTempoText("lyrics_time_0"); }
    void lyricsVoice1() { noTempoText("lyrics_voice_1"); }

    // gui - tracks model
    void testGuiTracksModel();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestImportMidi::initTestCase()
{
    setRootDir(QString(iex_midiimport_tests_DATA_ROOT));
}

//---------------------------------------------------------
//   midifile
//---------------------------------------------------------

void TestImportMidi::mf(const char* name) const
{
    MasterScore* score = new MasterScore(mscore->baseStyle());
    score->setName(name);
    const QString mscorename = QString(name) + ".mscx";
    QCOMPARE(importMidi(score,  midiFilePath(name)), Score::FileError::FILE_NO_ERROR);
    QVERIFY(saveCompareScore(score, mscorename, MIDIIMPORT_DIR + mscorename));
    delete score;
}

QString TestImportMidi::midiFilePath(const QString& fileName) const
{
    const QString nameWithExtension = fileName + ".mid";
    return QString(QString(iex_midiimport_tests_DATA_ROOT) + "/" + MIDIIMPORT_DIR + nameWithExtension);
}

QString TestImportMidi::midiFilePath(const char* fileName) const
{
    return midiFilePath(QString(fileName));
}

//---------------------------------------------------------
//  tuplet recognition functions
//---------------------------------------------------------

void TestImportMidi::findChordInBar()
{
    std::multimap<ReducedFraction, MidiChord> chords;
    chords.insert({ ReducedFraction::fromTicks(10), MidiChord() });
    chords.insert({ ReducedFraction::fromTicks(360), MidiChord() });
    chords.insert({ ReducedFraction::fromTicks(480), MidiChord() });
    chords.insert({ ReducedFraction::fromTicks(1480), MidiChord() });
    chords.insert({ ReducedFraction::fromTicks(2000), MidiChord() });
    chords.insert({ ReducedFraction::fromTicks(3201), MidiChord() });

    ReducedFraction startBarTick;
    ReducedFraction endBarTick = ReducedFraction::fromTicks(4 * MScore::division);   // 4/4

    auto firstChordIt = MChord::findFirstChordInRange(startBarTick, endBarTick,
                                                      chords.begin(), chords.end());
    QCOMPARE(firstChordIt, chords.begin());
    firstChordIt = MChord::findFirstChordInRange(chords, startBarTick, endBarTick);
    QCOMPARE(firstChordIt, chords.begin());

    auto endChordIt = MChord::findEndChordInRange(endBarTick, firstChordIt, chords.end());
    QCOMPARE(endChordIt, chords.find(ReducedFraction::fromTicks(2000)));
    endChordIt = chords.lower_bound(endBarTick);
    QCOMPARE(endChordIt, chords.find(ReducedFraction::fromTicks(2000)));

    endBarTick = ReducedFraction(0, 1);

    firstChordIt = MChord::findFirstChordInRange(startBarTick, endBarTick,
                                                 chords.begin(), chords.end());
    QCOMPARE(firstChordIt, chords.end());
    firstChordIt = MChord::findFirstChordInRange(chords, startBarTick, endBarTick);
    QCOMPARE(firstChordIt, chords.end());

    endChordIt = MChord::findEndChordInRange(endBarTick, firstChordIt, chords.end());
    QCOMPARE(endChordIt, chords.end());

    startBarTick = ReducedFraction::fromTicks(10);
    endBarTick = ReducedFraction::fromTicks(-100);

    firstChordIt = MChord::findFirstChordInRange(startBarTick, endBarTick,
                                                 chords.begin(), chords.end());
    QCOMPARE(firstChordIt, chords.end());
    firstChordIt = MChord::findFirstChordInRange(chords, startBarTick, endBarTick);
    QCOMPARE(firstChordIt, chords.end());

    endChordIt = MChord::findEndChordInRange(endBarTick, firstChordIt, chords.end());
    QCOMPARE(endChordIt, chords.end());
}

// tupletNoteNumber - number of note in tuplet (like index):
// i.e. for triplet notes can have numbers 1, 2, 3

void isSingleNoteInTupletAllowed(int tupletNumber,
                                 int tupletNoteNumber,
                                 double noteLenInTupletLen,
                                 bool expectedResult)
{
    MidiTuplet::TupletInfo tupletInfo;
    tupletInfo.len = ReducedFraction::fromTicks(MScore::division);

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
    QCOMPARE(MidiTuplet::isTupletAllowed(tupletInfo), expectedResult);
}

void isChordCountInTupletAllowed(int tupletNumber,
                                 int chordCount,
                                 bool expectedResult)
{
    MidiTuplet::TupletInfo tupletInfo;
    tupletInfo.len = ReducedFraction::fromTicks(MScore::division);

    std::multimap<ReducedFraction, MidiChord> chords;
    tupletInfo.firstChordIndex = 0;
    for (int i = 0; i != chordCount; ++i) {
        MidiChord chord;
        MidiNote note;
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
    QCOMPARE(MidiTuplet::isTupletAllowed(tupletInfo), expectedResult);
}

void isTupletErrorAllowed(int tupletSumError,
                          int regularSumError,
                          bool expectedResult)
{
    MidiTuplet::TupletInfo tupletInfo;
    tupletInfo.tupletNumber = 3;
    tupletInfo.len = ReducedFraction::fromTicks(MScore::division);

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
    QCOMPARE(MidiTuplet::isTupletAllowed(tupletInfo), expectedResult);
}

void TestImportMidi::isTupletAllowed()
{
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

void TestImportMidi::findTupletNumbers()
{
    auto& opers = midiImportOperations;
    opers.addNewMidiFile("");
    MidiOperations::CurrentTrackSetter setCurrentTrack{ opers, 0 };
    {
        const ReducedFraction barFraction(4, 4);
        const ReducedFraction divLen = barFraction / 4;
        const auto numbers = MidiTuplet::findTupletNumbers(divLen, barFraction);
        QVERIFY(numbers.size() == 4);
        QCOMPARE(numbers[0], 3);
        QCOMPARE(numbers[1], 5);
        QCOMPARE(numbers[2], 7);
        QCOMPARE(numbers[3], 9);
    }
    {
        const ReducedFraction barFraction(6, 8);
        const ReducedFraction divLen = barFraction / 2;
        const auto numbers = MidiTuplet::findTupletNumbers(divLen, barFraction);
        QVERIFY(numbers.size() == 1);
        QCOMPARE(numbers[0], 4);
        // duplets are turned off by default
    }
}

void TestImportMidi::findOnTimeRegularError()
{
    ReducedFraction quantValue = ReducedFraction::fromTicks(MScore::division) / 4;    // 1/16
    MidiChord chord;
    MidiNote note;
    note.offTime = quantValue * 4;
    chord.notes.push_back(note);
    const auto onTime = quantValue + ReducedFraction::fromTicks(12);
    std::pair<const ReducedFraction, MidiChord> pair(onTime, chord);
    QCOMPARE(Quantize::findOnTimeQuantError(pair, quantValue),
             ReducedFraction::fromTicks(12));
}

void TestImportMidi::findTupletApproximation()
{
    const int tupletNumber = 3;
    const ReducedFraction tupletLen = ReducedFraction::fromTicks(MScore::division);
    const ReducedFraction quantValue = ReducedFraction::fromTicks(MScore::division) / 4;    // 1/16

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
        QCOMPARE(tupletApprox.onTime, startTupletTime);
        QCOMPARE(tupletApprox.len, tupletLen);
        QCOMPARE(tupletApprox.tupletNumber, tupletNumber);
        QVERIFY(tupletApprox.chords.size() == 3);
        QCOMPARE(tupletApprox.tupletSumError, ReducedFraction::fromTicks(0));
        QCOMPARE(tupletApprox.regularSumError, ReducedFraction::fromTicks(80));
        QCOMPARE(tupletApprox.chords.find(ReducedFraction::fromTicks(0))->second,
                 chords.find(ReducedFraction::fromTicks(0)));
        QCOMPARE(tupletApprox.chords.find(ReducedFraction::fromTicks(160))->second,
                 chords.find(ReducedFraction::fromTicks(160)));
        QCOMPARE(tupletApprox.chords.find(ReducedFraction::fromTicks(320))->second,
                 chords.find(ReducedFraction::fromTicks(320)));
    }
    {
        const ReducedFraction startTupletTime = ReducedFraction::fromTicks(960);
        const MidiTuplet::TupletInfo tupletApprox = MidiTuplet::findTupletApproximation(
            tupletLen, tupletNumber, quantValue,
            startTupletTime, chords.begin(), chords.end()
            );
        QVERIFY(tupletApprox.chords.size() == 0);
    }
    {
        const ReducedFraction startTupletTime = ReducedFraction::fromTicks(1440);
        const MidiTuplet::TupletInfo tupletApprox = MidiTuplet::findTupletApproximation(
            tupletLen, tupletNumber, quantValue,
            startTupletTime, chords.begin(), chords.end()
            );
        QVERIFY(tupletApprox.chords.size() == 1);
        QCOMPARE(tupletApprox.tupletSumError, ReducedFraction::fromTicks(40));
        QCOMPARE(tupletApprox.regularSumError, ReducedFraction::fromTicks(40));
        QCOMPARE(tupletApprox.chords.find(ReducedFraction::fromTicks(1480))->second,
                 chords.find(ReducedFraction::fromTicks(1480)));
    }
}

void TestImportMidi::findLongestUncommonGroup()
{
    std::vector<MidiTuplet::TupletInfo> tuplets;
    MidiTuplet::TupletInfo info;
    const ReducedFraction basicQuant = ReducedFraction::fromTicks(MScore::division) / 4;    // 1/16
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
    QVERIFY(result.size() == 3);
    QVERIFY(result == std::set<int>({ 0, 1, 2 })
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
    QVERIFY(result.size() == 1);
}

//--------------------------------------------------------------------------
// tuplet voice separation

MidiNote noteFactory(const ReducedFraction& offTime, int pitch)
{
    MidiNote note;
    note.offTime = offTime;
    note.pitch = pitch;
    return note;
}

MidiChord chordFactory(const ReducedFraction& offTime, const std::vector<int>& pitches)
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

void TestImportMidi::separateTupletVoices()
{
    const ReducedFraction tupletLen = ReducedFraction::fromTicks(MScore::division);
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

    QVERIFY(chords.size() == 13);

    const auto firstChordTime = ReducedFraction(0, 1);

    auto tripletIt = tripletInfo.chords.find(firstChordTime);    // first chord in tuplet
    QCOMPARE(tripletIt->second->second.notes.size(), 3);
    QCOMPARE(tripletIt->second->second.notes[0].pitch, 76);
    QCOMPARE(tripletIt->second->second.notes[1].pitch, 71);
    QCOMPARE(tripletIt->second->second.notes[2].pitch, 67);

    auto quintupletIt = quintupletInfo.chords.find(firstChordTime);
    QCOMPARE(quintupletIt->second->second.notes.size(), 3);
    QCOMPARE(quintupletIt->second->second.notes[0].pitch, 76);
    QCOMPARE(quintupletIt->second->second.notes[1].pitch, 71);
    QCOMPARE(quintupletIt->second->second.notes[2].pitch, 67);

    auto septupletIt = septupletInfo.chords.find(firstChordTime);
    QCOMPARE(septupletIt->second->second.notes.size(), 3);
    QCOMPARE(septupletIt->second->second.notes[0].pitch, 76);
    QCOMPARE(septupletIt->second->second.notes[1].pitch, 71);
    QCOMPARE(septupletIt->second->second.notes[2].pitch, 67);

    MidiTuplet::splitFirstTupletChords(tuplets, chords);
    QVERIFY(chords.size() == 15);

    tripletInfo = tuplets[0];
    quintupletInfo = tuplets[1];
    septupletInfo = tuplets[2];

    tripletIt = tripletInfo.chords.find(firstChordTime);
    QCOMPARE(tripletIt->second->second.notes.size(), 1);
    QCOMPARE(tripletIt->second->second.notes[0].pitch, 76);

    quintupletIt = quintupletInfo.chords.find(firstChordTime);
    QCOMPARE(quintupletIt->second->second.notes.size(), 1);
    QCOMPARE(quintupletIt->second->second.notes[0].pitch, 71);

    septupletIt = septupletInfo.chords.find(firstChordTime);
    QCOMPARE(septupletIt->second->second.notes.size(), 1);
    QCOMPARE(septupletIt->second->second.notes[0].pitch, 67);
}

//---------------------------------------------------------
//  metric bar analysis
//---------------------------------------------------------

void TestImportMidi::metricDivisionsOfTuplet()
{
    MidiTuplet::TupletData tupletData;
    tupletData.voice = 0;
    tupletData.len = ReducedFraction::fromTicks(480);
    tupletData.onTime = ReducedFraction::fromTicks(480);
    tupletData.tupletNumber = 3;
    const int tupletStartLevel = -3;
    Meter::DivisionInfo tupletDivInfo = Meter::metricDivisionsOfTuplet(tupletData, tupletStartLevel);

    QCOMPARE(tupletDivInfo.isTuplet, true);
    QCOMPARE(tupletDivInfo.len, ReducedFraction::fromTicks(480));
    QCOMPARE(tupletDivInfo.onTime, ReducedFraction::fromTicks(480));
    QVERIFY(tupletDivInfo.divLengths.size() == 5);

    QCOMPARE(tupletDivInfo.divLengths[0].len, tupletData.len);
    QCOMPARE(tupletDivInfo.divLengths[0].level, Meter::TUPLET_BOUNDARY_LEVEL);

    QCOMPARE(tupletDivInfo.divLengths[1].len, tupletData.len / tupletData.tupletNumber);
    QCOMPARE(tupletDivInfo.divLengths[1].level, tupletStartLevel);

    QCOMPARE(tupletDivInfo.divLengths[2].len, tupletData.len / (2 * tupletData.tupletNumber));
    QCOMPARE(tupletDivInfo.divLengths[2].level, tupletStartLevel - 1);

    QCOMPARE(tupletDivInfo.divLengths[3].len, tupletData.len / (4 * tupletData.tupletNumber));
    QCOMPARE(tupletDivInfo.divLengths[3].level, tupletStartLevel - 2);

    QCOMPARE(tupletDivInfo.divLengths[4].len, tupletData.len / (8 * tupletData.tupletNumber));
    QCOMPARE(tupletDivInfo.divLengths[4].level, tupletStartLevel - 3);
}

void TestImportMidi::maxLevelBetween()
{
    const ReducedFraction barFraction = ReducedFraction(4, 4);
    ReducedFraction startTickInBar = ReducedFraction::fromTicks(240);
    ReducedFraction endTickInBar = ReducedFraction::fromTicks(500);

    Meter::DivisionInfo regularDivInfo = Meter::metricDivisionsOfBar(barFraction);
    QVERIFY(regularDivInfo.divLengths.size() == 8);

    Meter::MaxLevel level = Meter::maxLevelBetween(startTickInBar, endTickInBar, regularDivInfo);
    QCOMPARE(level.level, -2);
    QCOMPARE(level.pos, ReducedFraction::fromTicks(480));
    QCOMPARE(level.levelCount, 1);

    startTickInBar = ReducedFraction::fromTicks(499);
    level = Meter::maxLevelBetween(startTickInBar, endTickInBar, regularDivInfo);
    QCOMPARE(level.level, 0);
    QCOMPARE(level.pos, ReducedFraction(-1, 1));
    QCOMPARE(level.levelCount, 0);

    MidiTuplet::TupletData tupletData;
    tupletData.voice = 0;
    tupletData.len = ReducedFraction::fromTicks(480);
    tupletData.onTime = ReducedFraction::fromTicks(480);
    tupletData.tupletNumber = 3;

    const int tupletStartLevel = -3;
    const Meter::DivisionInfo tupletDivInfo = Meter::metricDivisionsOfTuplet(tupletData, tupletStartLevel);
    QVERIFY(tupletDivInfo.divLengths.size() == 5);

    startTickInBar = tupletData.onTime;
    endTickInBar = startTickInBar + tupletData.len;
    level = Meter::maxLevelBetween(startTickInBar, endTickInBar, tupletDivInfo);
    QCOMPARE(level.level, tupletStartLevel);
    QCOMPARE(level.pos, tupletData.onTime + tupletData.len / tupletData.tupletNumber);
    QCOMPARE(level.levelCount, 2);

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
    QVERIFY(level.level == -4);
    QCOMPARE(level.pos, (startTickInBar + endTickInBar) / 2);
    QCOMPARE(level.levelCount, 1);
    // s < e == ts < te
    startTickInBar = ReducedFraction::fromTicks(0);
    endTickInBar = tupletData.onTime;
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    QVERIFY(level.level == -3);
    QCOMPARE(level.pos, (startTickInBar + tupletData.onTime) / 2);
    QCOMPARE(level.levelCount, 1);
    // s < ts < e < te
    startTickInBar = ReducedFraction::fromTicks(0);
    endTickInBar = tupletData.onTime + tupletData.len / 2;
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    QVERIFY(level.level == Meter::TUPLET_BOUNDARY_LEVEL);
    QCOMPARE(level.pos, tupletData.onTime);
    QCOMPARE(level.levelCount, 1);
    // s < ts < e == te
    startTickInBar = ReducedFraction::fromTicks(0);
    endTickInBar = tupletData.onTime + tupletData.len;
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    QVERIFY(level.level == Meter::TUPLET_BOUNDARY_LEVEL);
    QCOMPARE(level.pos, tupletData.onTime);
    QCOMPARE(level.levelCount, 1);
    // s < ts < te < e
    startTickInBar = ReducedFraction::fromTicks(0);
    endTickInBar = tupletData.onTime + tupletData.len + ReducedFraction(1, 3);
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    QVERIFY(level.level == Meter::TUPLET_BOUNDARY_LEVEL);
    QCOMPARE(level.pos, tupletData.onTime + tupletData.len);
    QCOMPARE(level.levelCount, 1);
    // s == ts < e < te
    startTickInBar = tupletData.onTime;
    endTickInBar = tupletData.onTime + tupletData.len / 2;
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    QCOMPARE(level.level, tupletStartLevel);
    QCOMPARE(level.pos, tupletData.onTime + tupletData.len / tupletData.tupletNumber);
    QCOMPARE(level.levelCount, 1);
    // s == ts < e == te
    startTickInBar = tupletData.onTime;
    endTickInBar = tupletData.onTime + tupletData.len;
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    QCOMPARE(level.level, tupletStartLevel);
    QCOMPARE(level.pos, tupletData.onTime + tupletData.len / tupletData.tupletNumber);
    QCOMPARE(level.levelCount, 2);
    // s == ts < te < e
    startTickInBar = tupletData.onTime;
    endTickInBar = tupletData.onTime + tupletData.len * 2;
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    QCOMPARE(level.level, Meter::TUPLET_BOUNDARY_LEVEL);
    QCOMPARE(level.pos, tupletData.onTime + tupletData.len);
    QCOMPARE(level.levelCount, 1);
    // ts < s < e < te
    startTickInBar = tupletData.onTime + tupletData.len / tupletData.tupletNumber;
    endTickInBar = tupletData.onTime + tupletData.len * 2 / tupletData.tupletNumber;
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    QVERIFY(level.level == tupletStartLevel - 1);
    QCOMPARE(level.pos, (startTickInBar + endTickInBar) / 2);
    QCOMPARE(level.levelCount, 1);
    // ts < s < e = te
    startTickInBar = tupletData.onTime + tupletData.len / tupletData.tupletNumber;
    endTickInBar = tupletData.onTime + tupletData.len;
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    QVERIFY(level.level == tupletStartLevel);
    QCOMPARE(level.pos, tupletData.onTime + tupletData.len * 2 / tupletData.tupletNumber);
    QCOMPARE(level.levelCount, 1);
    // ts < s < te < e
    startTickInBar = tupletData.onTime + tupletData.len / 2;
    endTickInBar = tupletData.onTime + tupletData.len + ReducedFraction(1, 3);
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    QVERIFY(level.level == Meter::TUPLET_BOUNDARY_LEVEL);
    QCOMPARE(level.pos, tupletData.onTime + tupletData.len);
    QCOMPARE(level.levelCount, 1);
    // ts < te = s < e
    startTickInBar = tupletData.onTime + tupletData.len;
    endTickInBar = tupletData.onTime + tupletData.len + ReducedFraction(1, 3);
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    QVERIFY(level.level != Meter::TUPLET_BOUNDARY_LEVEL);
    // ts < te < s < e
    startTickInBar = tupletData.onTime + tupletData.len + ReducedFraction(1, 3);
    endTickInBar = startTickInBar + ReducedFraction(1, 3);
    level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
    QVERIFY(level.level != Meter::TUPLET_BOUNDARY_LEVEL);
}

void TestImportMidi::isSimpleDuration()
{
    QVERIFY(Meter::isSimpleNoteDuration({ 4, 2 }));
    QVERIFY(Meter::isSimpleNoteDuration({ 4, 1 }));
    QVERIFY(Meter::isSimpleNoteDuration({ 2, 2 }));
    QVERIFY(Meter::isSimpleNoteDuration({ 2, 1 }));
    QVERIFY(Meter::isSimpleNoteDuration({ 1, 1 }));
    QVERIFY(Meter::isSimpleNoteDuration({ 1, 2 }));
    QVERIFY(Meter::isSimpleNoteDuration({ 1, 4 }));
    QVERIFY(Meter::isSimpleNoteDuration({ 1, 8 }));
    QVERIFY(Meter::isSimpleNoteDuration({ 1, 16 }));
    QVERIFY(Meter::isSimpleNoteDuration({ 1, 32 }));
    QVERIFY(Meter::isSimpleNoteDuration({ 1, 64 }));
    QVERIFY(Meter::isSimpleNoteDuration({ 1, 128 }));

    QVERIFY(!Meter::isSimpleNoteDuration({ 1, 6 }));
    QVERIFY(!Meter::isSimpleNoteDuration({ 3, 2 }));
    QVERIFY(!Meter::isSimpleNoteDuration({ 12, 8 }));
    QVERIFY(!Meter::isSimpleNoteDuration({ 3, 16 }));
    QVERIFY(!Meter::isSimpleNoteDuration({ 3, 4 }));
    QVERIFY(!Meter::isSimpleNoteDuration({ 3, 8 }));
    QVERIFY(!Meter::isSimpleNoteDuration({ 1, 5 }));
}

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

void TestImportMidi::testGuiTracksModel()
{
    QString midiFile("perc_drums");
    QString midiFileFullPath = midiFilePath(midiFile);
    auto& opers = midiImportOperations;
    opers.addNewMidiFile(midiFileFullPath);
    MidiOperations::CurrentMidiFileSetter setCurrentMidiFile(opers, midiFileFullPath);

    MasterScore score(mscore->baseStyle());
    score.setName(midiFile);
    QCOMPARE(importMidi(&score, midiFileFullPath), Score::FileError::FILE_NO_ERROR);

    TracksModel model;
    model.reset(opers.data()->trackOpers,
                MidiLyrics::makeLyricsListForUI(),
                opers.data()->trackCount,
                midiFileFullPath,
                !opers.data()->humanBeatData.beatSet.empty(),
                opers.data()->hasTempoText,
                !opers.data()->chordNames.empty());

    QVERIFY(model.trackCount() == 1);

    Qt::ItemFlags notEditableFlags = Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

    const int clefChangeCol = findColByHeader(model, "Clef\nchanges");
    QVERIFY(clefChangeCol >= 0);
    QCOMPARE(model.flags(model.index(0, clefChangeCol)), notEditableFlags);

    const int voiceCol = findColByHeader(model, "Max. voices");
    QVERIFY(voiceCol >= 0);
    QCOMPARE(model.flags(model.index(0, voiceCol)), notEditableFlags);

    const int channelCol = findColByHeader(model, "Channel");
    QVERIFY(channelCol >= 0);
    QCOMPARE(model.flags(model.index(0, channelCol)), notEditableFlags);
}

QTEST_MAIN(TestImportMidi)

#include "tst_importmidi.moc"
