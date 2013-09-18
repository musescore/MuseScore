//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>

#include "libmscore/mscore.h"
#include "libmscore/score.h"
#include "libmscore/durationtype.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/keysig.h"
#include "mscore/exportmidi.h"

#include "libmscore/mcursor.h"
#include "mtest/testutils.h"
#include "inner_func_decl.h"
#include "mscore/importmidi_chord.h"
#include "mscore/importmidi_tuplet.h"
#include "mscore/importmidi_meter.h"
#include "mscore/importmidi_inner.h"
#include "mscore/importmidi_fraction.h"
#include "mscore/preferences.h"


namespace Ms {
      extern Score::FileError importMidi(Score*, const QString&);
      }

using namespace Ms;

#define DIR QString("importmidi/")

//---------------------------------------------------------
//   TestImportMidi
//---------------------------------------------------------

class TestImportMidi : public QObject, public MTest
      {
      Q_OBJECT

      void mf(const char* name);

   private slots:
      void initTestCase();
      void im1() { mf("m1"); }
      void im2() { mf("m2"); }     // tie across bar line
      void im3() { mf("m3"); }     // voices, typeA, resolve with tie
      void im4() { mf("m4"); }     // voices, typeB, resolve with tie
      void im5() { mf("m5"); }     // same as m1 with division 240

      // quantization
      void quantDotted4th()
            {
                        // 1/4 quantization should preserve 4th dotted note
            const int defaultQuant = preferences.shortestNote;
            preferences.shortestNote = MScore::division; // midi quantization: 1/4
            TrackOperations opers;
            opers.quantize.reduceToShorterNotesInBar = false;
            preferences.midiImportOperations.appendTrackOperations(opers);
            mf("quant_dotted_4th");
            preferences.shortestNote = defaultQuant;
            preferences.midiImportOperations.clear();
            }

      // test tuplet recognition functions
      void findChordInBar();
      void bestChordForTupletNote();
      void isTupletAllowed();
      void findTupletNumbers();
      void findOnTimeRegularError();
      void findTupletApproximation();
      void separateTupletVoices();
      void findTupletsWithCommonChords();

      // metric bar analysis
      void metricDivisionsOfTuplet();
      void maxLevelBetween();
      void isSimpleDuration();

      // test scores for meter (duration subdivision)
      void meterTimeSig4_4() { mf("meter_4-4"); }
      void metertimeSig9_8() { mf("meter_9-8"); }
      void metertimeSig12_8() { mf("meter_12-8"); }
      void metertimeSig15_8() { mf("meter_15-8"); }
      void meterCentralLongNote() { mf("meter_central_long_note"); }
      void meterCentralLongRest() { mf("meter_central_long_rest"); }
      void meterChordExample() { mf("meter_chord_example"); }
      void meterDotsExample1() { mf("meter_dots_example1"); }
      void meterDotsExample2() { mf("meter_dots_example2"); }
      void meterDotsExample3() { mf("meter_dots_example3"); }
      void meterHalfRest3_4() { mf("meter_half_rest_3-4"); }
      void meterFirst2_8thRestsCompound() { mf("meter_first_2_8th_rests_compound"); }
      void meterLastQuarterRestCompound() { mf("meter_last_quarter_rest_compound"); }
      void meterRests() { mf("meter_rests"); }
      void meterTwoBeatsOver() { mf("meter_two_beats_over"); }
      void meterDotTie() { mf("meter_dot_tie"); }

      // time sig
      void timesigChanges() { mf("timesig_changes"); }

      // test scores for tuplets
      void tuplet2Voices3_5Tuplets()
            {
                        // requires 1/32 quantization
            const int defaultQuant = preferences.shortestNote;
            preferences.shortestNote = MScore::division / 8; // midi quantization: 1/32
            mf("tuplet_2_voices_3_5_tuplets");
            preferences.shortestNote = defaultQuant;
            }
      void tuplet2VoicesTupletNon() { mf("tuplet_2_voices_tuplet_non"); }
      void tuplet3_5_7tuplets()
            {
            TrackOperations opers;
            opers.changeClef = false;
            preferences.midiImportOperations.appendTrackOperations(opers);
            mf("tuplet_3_5_7_tuplets");
            preferences.midiImportOperations.clear();
            }
      void tuplet5_5TupletsRests() { mf("tuplet_5_5_tuplets_rests"); }
      void tuplet3_4() { mf("tuplet_3-4"); }
      void tupletDuplet() { mf("tuplet_duplet"); }
      void tupletMars() { mf("tuplet_mars"); }
      void tupletNonuplet3_4()
            {
                        // requires 1/64 quantization
            const int defaultQuant = preferences.shortestNote;
            preferences.shortestNote = MScore::division / 16; // midi quantization: 1/64
            mf("tuplet_nonuplet_3-4");
            preferences.shortestNote = defaultQuant;
            }
      void tupletNonuplet4_4()
            {
                        // requires 1/64 quantization
            const int defaultQuant = preferences.shortestNote;
            preferences.shortestNote = MScore::division / 16; // midi quantization: 1/64
            mf("tuplet_nonuplet_4-4");
            preferences.shortestNote = defaultQuant;
            }
      void tupletQuadruplet() { mf("tuplet_quadruplet"); }
      void tupletSeptuplet() { mf("tuplet_septuplet"); }
      void tupletTripletsMixed() { mf("tuplet_triplets_mixed"); }
      void tupletTriplet() { mf("tuplet_triplet"); }
      void tupletTripletFirstTied() { mf("tuplet_triplet_first_tied"); }
      void tupletTripletFirstTied2() { mf("tuplet_triplet_first_tied2"); }
      void tupletTripletLastTied() { mf("tuplet_triplet_last_tied"); }
      void tupletTied3_5()
            {
                        // requires 1/32 quantization
            const int defaultQuant = preferences.shortestNote;
            preferences.shortestNote = MScore::division / 8; // midi quantization: 1/32
            mf("tuplet_tied_3_5_tuplets");
            preferences.shortestNote = defaultQuant;
            }
      void tupletTied3_5_2()
            {
            // requires 1/32 quantization
            const int defaultQuant = preferences.shortestNote;
            preferences.shortestNote = MScore::division / 8; // midi quantization: 1/32
            mf("tuplet_tied_3_5_tuplets2");
            preferences.shortestNote = defaultQuant;
            }
      void tupletOffTimeOtherBar() { mf("tuplet_off_time_other_bar"); }
      void tupletOffTimeOtherBar2() { mf("tuplet_off_time_other_bar2"); }
      void minDuration() { mf("min_duration"); }
      void minDurationNoReduce()
            {
            TrackOperations opers;
            opers.quantize.reduceToShorterNotesInBar = false;
            preferences.midiImportOperations.appendTrackOperations(opers);
            mf("min_duration_no_reduce");
            preferences.midiImportOperations.clear();
            }

      void pickupMeasure() { mf("pickup"); }

      // LH/RH separation
      void LHRH_Nontuplet()
            {
            TrackOperations opers;
            opers.LHRH.doIt = true;
            preferences.midiImportOperations.appendTrackOperations(opers);
            mf("split_nontuplet");
            preferences.midiImportOperations.clear();
            }
      void LHRH_Tuplet()
            {
            TrackOperations opers;
            opers.LHRH.doIt = true;
            preferences.midiImportOperations.appendTrackOperations(opers);
            mf("split_tuplet");
            preferences.midiImportOperations.clear();
            }

      // swing
      void swingTriplets()
            {
            TrackOperations opers;
            opers.swing = MidiOperation::Swing::SWING;
            preferences.midiImportOperations.appendTrackOperations(opers);
            mf("swing_triplets");
            preferences.midiImportOperations.clear();
            }
      void swingShuffle()
            {
            TrackOperations opers;
            opers.swing = MidiOperation::Swing::SHUFFLE;
            preferences.midiImportOperations.appendTrackOperations(opers);
            mf("swing_shuffle");
            preferences.midiImportOperations.clear();
            }

      // percussion
      void percDrums() { mf("perc_drums"); }

      // clef changes along the score
      void clefTied() { mf("clef_tied"); }
      void clefMelody() { mf("clef_melody"); }
      void clefPrev() { mf("clef_prev"); }
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestImportMidi::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   midifile
//---------------------------------------------------------

void TestImportMidi::mf(const char* name)
      {
      Score* score = new Score(mscore->baseStyle());
      score->setName(name);
      const QString midiname = QString(name) + ".mid";
      const QString mscorename = QString(name) + ".mscx";
      QCOMPARE(importMidi(score,  TESTROOT "/mtest/" + DIR + midiname), Score::FILE_NO_ERROR);
      QVERIFY(saveCompareScore(score, mscorename, DIR + mscorename));
      delete score;
      }

//---------------------------------------------------------
//  tuplet recognition fuctions
//---------------------------------------------------------

void TestImportMidi::findChordInBar()
      {
      std::multimap<ReducedFraction, MidiChord> chords;
      chords.insert({ReducedFraction::fromTicks(10), MidiChord()});
      chords.insert({ReducedFraction::fromTicks(360), MidiChord()});
      chords.insert({ReducedFraction::fromTicks(480), MidiChord()});
      chords.insert({ReducedFraction::fromTicks(1480), MidiChord()});
      chords.insert({ReducedFraction::fromTicks(2000), MidiChord()});
      chords.insert({ReducedFraction::fromTicks(3201), MidiChord()});

      ReducedFraction startBarTick;
      ReducedFraction endBarTick = ReducedFraction::fromTicks(4 * MScore::division); // 4/4

      auto firstChordIt = MChord::findFirstChordInRange(startBarTick, endBarTick,
                                                        chords.begin(), chords.end());
      QCOMPARE(firstChordIt, chords.begin());
      auto endChordIt = MChord::findEndChordInRange(endBarTick, firstChordIt, chords.end());
      QCOMPARE(endChordIt, chords.find(ReducedFraction::fromTicks(2000)));

      endBarTick = ReducedFraction(0, 1);

      firstChordIt = MChord::findFirstChordInRange(startBarTick, endBarTick,
                                                   chords.begin(), chords.end());
      QCOMPARE(firstChordIt, chords.end());
      endChordIt = MChord::findEndChordInRange(endBarTick, firstChordIt, chords.end());
      QCOMPARE(endChordIt, chords.end());

      startBarTick = ReducedFraction::fromTicks(10);
      endBarTick = ReducedFraction::fromTicks(-100);

      firstChordIt = MChord::findFirstChordInRange(startBarTick, endBarTick,
                                                   chords.begin(), chords.end());
      QCOMPARE(firstChordIt, chords.end());
      endChordIt = MChord::findEndChordInRange(endBarTick, firstChordIt, chords.end());
      QCOMPARE(endChordIt, chords.end());
      }

void TestImportMidi::bestChordForTupletNote()
      {
      const ReducedFraction tupletLen = ReducedFraction::fromTicks(MScore::division);
      const ReducedFraction quantValue = ReducedFraction::fromTicks(MScore::division) / 4;
      const int tupletNumber = 3;
      const ReducedFraction tupletNoteLen = tupletLen / tupletNumber;

      std::multimap<ReducedFraction, MidiChord> chords;
      chords.insert({ReducedFraction::fromTicks(10), MidiChord()});
      chords.insert({ReducedFraction::fromTicks(160), MidiChord()});
      chords.insert({ReducedFraction::fromTicks(360), MidiChord()});
      chords.insert({ReducedFraction::fromTicks(480), MidiChord()});
      chords.insert({ReducedFraction::fromTicks(1480), MidiChord()});
      chords.insert({ReducedFraction::fromTicks(2000), MidiChord()});
      chords.insert({ReducedFraction::fromTicks(3201), MidiChord()});

      ReducedFraction tupletNotePos = tupletNoteLen;
      auto bestChord = MidiTuplet::findBestChordForTupletNote(tupletNotePos, quantValue,
                                                              chords.begin(), chords.end());
      QCOMPARE(bestChord.first, chords.find(ReducedFraction::fromTicks(160)));
      QCOMPARE(bestChord.second, ReducedFraction(0, 1));

      tupletNotePos = tupletNoteLen * 2;
      bestChord = MidiTuplet::findBestChordForTupletNote(tupletNotePos, quantValue,
                                                         chords.begin(), chords.end());
      QCOMPARE(bestChord.first, chords.find(ReducedFraction::fromTicks(360)));
      QCOMPARE(bestChord.second, ReducedFraction::fromTicks(40));
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
      tupletInfo.regularQuant = ReducedFraction::fromTicks(MScore::division) / 4;     // 1/16

      std::multimap<ReducedFraction, MidiChord> chords;
      MidiChord chord;
      MidiNote note;
      note.len = ReducedFraction::fromTicks(std::round(tupletInfo.len.ticks()
                                                       * noteLenInTupletLen));
      chord.notes.push_back(note);
      const ReducedFraction onTime = ReducedFraction::fromTicks(10);
      chords.insert({onTime, chord});

      tupletInfo.chords.insert({onTime, chords.begin()});
                  // tuplet error is less than regular error => allowed
      tupletInfo.tupletSumError = {0, 1};
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
      tupletInfo.regularQuant = ReducedFraction::fromTicks(MScore::division) / 4;     // 1/16

      std::multimap<ReducedFraction, MidiChord> chords;
      tupletInfo.firstChordIndex = 0;
      for (int i = 0; i != chordCount; ++i) {
            MidiChord chord;
            MidiNote note;
            note.len = tupletInfo.len / tupletNumber; // allowed
            chord.notes.push_back(note);
            const ReducedFraction onTime = tupletInfo.len / tupletNumber * i;
            const auto lastChordIt = chords.insert({onTime, chord});
            tupletInfo.chords.insert({onTime, lastChordIt});
            }
                  // tuplet error is less than regular error => allowed
      tupletInfo.tupletSumError = {0, 1};
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
      tupletInfo.regularQuant = ReducedFraction::fromTicks(MScore::division) / 4;     // 1/16

      std::multimap<ReducedFraction, MidiChord> chords;
      MidiChord chord;
      MidiNote note;
      note.len = tupletInfo.len / tupletInfo.tupletNumber;  // allowed
      chord.notes.push_back(note);
      const ReducedFraction onTime = ReducedFraction::fromTicks(10);
      chords.insert({onTime, chord});

      tupletInfo.chords.insert({onTime, chords.begin()});
      tupletInfo.tupletSumError = {tupletSumError, 1};
      tupletInfo.regularSumError = {regularSumError, 1};
      QCOMPARE(MidiTuplet::isTupletAllowed(tupletInfo), expectedResult);
      }

void TestImportMidi::isTupletAllowed()
      {
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
      isChordCountInTupletAllowed(2, 3, true); // special case with tuplet subdivision

      isChordCountInTupletAllowed(3, 1, true);
      isChordCountInTupletAllowed(3, 2, true);
      isChordCountInTupletAllowed(3, 3, true);
      isChordCountInTupletAllowed(3, 4, true); // special case with tuplet subdivision

      isChordCountInTupletAllowed(4, 1, false);
      isChordCountInTupletAllowed(4, 2, false);
      isChordCountInTupletAllowed(4, 3, true);
      isChordCountInTupletAllowed(4, 4, true);
      isChordCountInTupletAllowed(4, 5, true); // special case with tuplet subdivision

      isChordCountInTupletAllowed(5, 1, false);
      isChordCountInTupletAllowed(5, 2, false);
      isChordCountInTupletAllowed(5, 3, true);
      isChordCountInTupletAllowed(5, 4, true);
      isChordCountInTupletAllowed(5, 5, true);
      isChordCountInTupletAllowed(5, 6, true); // special case with tuplet subdivision

      isChordCountInTupletAllowed(7, 1, false);
      isChordCountInTupletAllowed(7, 2, false);
      isChordCountInTupletAllowed(7, 3, false);
      isChordCountInTupletAllowed(7, 4, true);
      isChordCountInTupletAllowed(7, 5, true);
      isChordCountInTupletAllowed(7, 6, true);
      isChordCountInTupletAllowed(7, 7, true);
      isChordCountInTupletAllowed(7, 8, true); // special case with tuplet subdivision

      // tuplet error should be less than regular error
      isTupletErrorAllowed(4, 5, true);
      isTupletErrorAllowed(5, 5, false);
      isTupletErrorAllowed(6, 5, false);
      }

void TestImportMidi::findTupletNumbers()
      {
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
      ReducedFraction quantValue = ReducedFraction::fromTicks(MScore::division) / 4;  // 1/16
      ReducedFraction onTime = quantValue + ReducedFraction::fromTicks(12);
      QCOMPARE(MidiTuplet::findQuantizationError(onTime, quantValue),
               ReducedFraction::fromTicks(12));
      }

void TestImportMidi::findTupletApproximation()
      {
      const int tupletNumber = 3;
      const ReducedFraction tupletLen = ReducedFraction::fromTicks(MScore::division);
      const ReducedFraction quantValue = ReducedFraction::fromTicks(MScore::division) / 4;  // 1/16

      std::multimap<ReducedFraction, MidiChord> chords;
      chords.insert({ReducedFraction::fromTicks(0), MidiChord()});
      chords.insert({ReducedFraction::fromTicks(160), MidiChord()});
      chords.insert({ReducedFraction::fromTicks(320), MidiChord()});
      chords.insert({ReducedFraction::fromTicks(480), MidiChord()});
      chords.insert({ReducedFraction::fromTicks(1480), MidiChord()});
      chords.insert({ReducedFraction::fromTicks(2000), MidiChord()});
      chords.insert({ReducedFraction::fromTicks(3201), MidiChord()});

      {
      const ReducedFraction startTupletTime = ReducedFraction(0, 1);
      const MidiTuplet::TupletInfo tupletApprox = MidiTuplet::findTupletApproximation(
                        tupletLen, tupletNumber, quantValue,
                        startTupletTime, chords.begin(), chords.end()
                        );
      QCOMPARE(tupletApprox.onTime, startTupletTime);
      QCOMPARE(tupletApprox.len, tupletLen);
      QCOMPARE(tupletApprox.tupletNumber, tupletNumber);
      QCOMPARE(tupletApprox.tupletQuant, ReducedFraction::fromTicks(MScore::division) / 3);
      QCOMPARE(tupletApprox.regularQuant, quantValue);
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

//--------------------------------------------------------------------------
      // tuplet voice separation

MidiNote noteFactory(const ReducedFraction &len, int pitch)
      {
      MidiNote note;
      note.len = len;
      note.pitch = pitch;
      return note;
      }

MidiChord chordFactory(const ReducedFraction &len, const std::vector<int> &pitches)
      {
      std::vector<MidiNote> notes;
      for (const auto &pitch: pitches)
            notes.push_back(noteFactory(len, pitch));
      MidiChord chord;
      if (notes.empty())
            return chord;
      for (const auto &note: notes)
            chord.notes.push_back(note);

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

      const MidiChord firstChord = chordFactory(tripletNoteLen, {76, 71, 67});
      chords.insert({{0, 1}, firstChord});

      std::vector<int> pitches = {74, 77};
      for (int i = 1; i != tripletNumber; ++i) {
            chords.insert({tripletNoteLen * i,
                           chordFactory(tripletNoteLen, {pitches[i]})});
            }
                  // quintuplet
      const ReducedFraction quintupletLen = tupletLen;
      const int quintupletNumber = 5;
      const ReducedFraction quintupletNoteLen = quintupletLen / quintupletNumber;
      pitches = {60, 62, 58, 60};
      for (int i = 1; i != quintupletNumber; ++i) {
            chords.insert({quintupletNoteLen * i,
                           chordFactory(quintupletNoteLen, {pitches[i]})});
            }
                  // septuplet
      const ReducedFraction septupletLen = tupletLen * 2;
      const int septupletNumber = 7;
      const ReducedFraction septupletNoteLen = septupletLen / septupletNumber;
      pitches = {50, 52, 48, 51, 47, 47};
      for (int i = 1; i != septupletNumber; ++i) {
            chords.insert({septupletNoteLen * i,
                           chordFactory(septupletNoteLen, {pitches[i]})});
            }

      MidiTuplet::TupletInfo tripletInfo;
      tripletInfo.onTime = ReducedFraction(0, 1);
      tripletInfo.len = tripletLen;
      tripletInfo.firstChordIndex = 0;
      for (int i = 0; i != 3; ++i) {
            const auto onTime = tripletNoteLen * i;
            tripletInfo.chords.insert({onTime, chords.find(onTime)});
            }

      MidiTuplet::TupletInfo quintupletInfo;
      quintupletInfo.onTime = ReducedFraction(0, 1);
      quintupletInfo.len = quintupletLen;
      quintupletInfo.firstChordIndex = 0;
      for (int i = 0; i != 5; ++i) {
            const auto onTime = quintupletNoteLen * i;
            quintupletInfo.chords.insert({onTime, chords.find(onTime)});
            }

      MidiTuplet::TupletInfo septupletInfo;
      septupletInfo.onTime = ReducedFraction(0, 1);
      septupletInfo.len = septupletLen;
      septupletInfo.firstChordIndex = 0;
      for (int i = 0; i != 7; ++i) {
            const auto onTime = septupletNoteLen * i;
            septupletInfo.chords.insert({onTime, chords.find(onTime)});
            }

      std::vector<MidiTuplet::TupletInfo> tuplets;
      tuplets.push_back(tripletInfo);
      tuplets.push_back(quintupletInfo);
      tuplets.push_back(septupletInfo);

      QVERIFY(chords.size() == 13);

      const auto firstChordTime = ReducedFraction(0, 1);

      auto tripletIt = tripletInfo.chords.find(firstChordTime);  // first chord in tuplet
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

void TestImportMidi::findTupletsWithCommonChords()
      {
      std::vector<MidiTuplet::TupletInfo> tuplets;

      std::multimap<ReducedFraction, MidiChord> chords;
      chords.insert({ReducedFraction(0, 1), MidiChord()});
      chords.insert({ReducedFraction(1, 12), MidiChord()});
      chords.insert({ReducedFraction(1, 6), MidiChord()});
      chords.insert({ReducedFraction(3, 10), MidiChord()});
      chords.insert({ReducedFraction(7, 20), MidiChord()});
      chords.insert({ReducedFraction(2, 5), MidiChord()});
      chords.insert({ReducedFraction(9, 20), MidiChord()});

      MidiTuplet::TupletInfo tupletData;
                  // triplet, total len = 1/8
      tupletData.chords.clear();
      tupletData.chords.insert({ReducedFraction(0, 1), chords.find(ReducedFraction(0, 1))});
      tupletData.chords.insert({ReducedFraction(1, 12), chords.find(ReducedFraction(1, 12))});
      tuplets.push_back(tupletData);
                  // second triplet, total len = 1/8
      tupletData.chords.clear();
      tupletData.chords.insert({ReducedFraction(1, 6), chords.find(ReducedFraction(1, 6))});
      tuplets.push_back(tupletData);
                  // third triplet, total len = 1/4
      tupletData.chords.clear();
      tupletData.chords.insert({ReducedFraction(0, 1), chords.find(ReducedFraction(0, 1))});
      tupletData.chords.insert({ReducedFraction(1, 12), chords.find(ReducedFraction(1, 12))});
      tupletData.chords.insert({ReducedFraction(1, 6), chords.find(ReducedFraction(1, 6))});
      tuplets.push_back(tupletData);
                  // quintuplet, total len = 1/4
      tupletData.chords.clear();
      tupletData.chords.insert({ReducedFraction(3, 10), chords.find(ReducedFraction(3, 10))});
      tupletData.chords.insert({ReducedFraction(7, 20), chords.find(ReducedFraction(7, 20))});
      tupletData.chords.insert({ReducedFraction(2, 5), chords.find(ReducedFraction(2, 5))});
      tupletData.chords.insert({ReducedFraction(9, 20), chords.find(ReducedFraction(9, 20))});
      tuplets.push_back(tupletData);
                  // second quintuplet, total len = 1/2
      tupletData.chords.clear();
      tupletData.chords.insert({ReducedFraction(0, 1), chords.find(ReducedFraction(0, 1))});
      tupletData.chords.insert({ReducedFraction(1, 12), chords.find(ReducedFraction(1, 12))});
      tupletData.chords.insert({ReducedFraction(3, 10), chords.find(ReducedFraction(3, 10))});
      tupletData.chords.insert({ReducedFraction(2, 5), chords.find(ReducedFraction(2, 5))});
      tuplets.push_back(tupletData);

      QVERIFY(tuplets.size() == 5);
      std::list<int> restTupletIndexes = {0, 1, 2, 3, 4};

      std::list<int> commonTuplets = MidiTuplet::findTupletsWithCommonChords(restTupletIndexes, tuplets);
      commonTuplets.sort();
      QVERIFY(restTupletIndexes.empty());
      QVERIFY(commonTuplets == std::list<int>({0, 1, 2, 3, 4}));

      std::vector<int> uncommonTuplets = MidiTuplet::findTupletsWithNoCommonChords(commonTuplets, tuplets);
      std::sort(uncommonTuplets.begin(), uncommonTuplets.end());
      QVERIFY(uncommonTuplets == std::vector<int>({0, 1, 3}));
      QVERIFY(commonTuplets == std::list<int>({2, 4}));
                  // process the rest tuplets with common chords
      uncommonTuplets = MidiTuplet::findTupletsWithNoCommonChords(commonTuplets, tuplets);
      std::sort(uncommonTuplets.begin(), uncommonTuplets.end());
      QVERIFY(uncommonTuplets == std::vector<int>({2}));
      QVERIFY(commonTuplets == std::list<int>({4}));
                  // process the rest tuplets with common chords
      uncommonTuplets = MidiTuplet::findTupletsWithNoCommonChords(commonTuplets, tuplets);
      std::sort(uncommonTuplets.begin(), uncommonTuplets.end());
      QVERIFY(uncommonTuplets == std::vector<int>({4}));
      QVERIFY(commonTuplets.empty());
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
      QVERIFY(Meter::isSimpleNoteDuration({4, 2}));
      QVERIFY(Meter::isSimpleNoteDuration({4, 1}));
      QVERIFY(Meter::isSimpleNoteDuration({2, 2}));
      QVERIFY(Meter::isSimpleNoteDuration({2, 1}));
      QVERIFY(Meter::isSimpleNoteDuration({1, 1}));
      QVERIFY(Meter::isSimpleNoteDuration({1, 2}));
      QVERIFY(Meter::isSimpleNoteDuration({1, 4}));
      QVERIFY(Meter::isSimpleNoteDuration({1, 8}));
      QVERIFY(Meter::isSimpleNoteDuration({1, 16}));
      QVERIFY(Meter::isSimpleNoteDuration({1, 32}));
      QVERIFY(Meter::isSimpleNoteDuration({1, 64}));
      QVERIFY(Meter::isSimpleNoteDuration({1, 128}));

      QVERIFY(!Meter::isSimpleNoteDuration({1, 6}));
      QVERIFY(!Meter::isSimpleNoteDuration({3, 2}));
      QVERIFY(!Meter::isSimpleNoteDuration({12, 8}));
      QVERIFY(!Meter::isSimpleNoteDuration({3, 16}));
      QVERIFY(!Meter::isSimpleNoteDuration({3, 4}));
      QVERIFY(!Meter::isSimpleNoteDuration({3, 8}));
      QVERIFY(!Meter::isSimpleNoteDuration({1, 5}));
      }


QTEST_MAIN(TestImportMidi)

#include "tst_importmidi.moc"

