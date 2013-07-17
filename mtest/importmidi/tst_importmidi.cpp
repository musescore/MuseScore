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

      // test tuplet recognition functions
      void findChordInBar();
      void bestChordForTupletNote();
      void isTupletAllowed();
      void findTupletNumbers();
      void findOnTimeRegularError();
      void findTupletApproximation();
      void separateTupletVoices();

      // metric bar analysis
      void metricDivisionsOfTuplet();
      void maxLevelBetween();

      // test scores for meter (duration subdivision)
      void meterTimeSig4_4() { mf("meter_4-4"); }
      void metertimeSig9_8() { mf("meter_9-8"); }
      void metertimeSig12_8() { mf("meter_12-8"); }
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

      // test scores for tuplets
      void tuplet2Voices3_5Tuplets()
            {
                        // requires 1/32 quantization
            int defaultQuant = preferences.shortestNote;
            preferences.shortestNote = MScore::division / 8; // midi quantization: 1/32
            mf("tuplet_2_voices_3_5_tuplets");
            preferences.shortestNote = defaultQuant;
            }
      void tuplet2VoicesTupletNon() { mf("tuplet_2_voices_tuplet_non"); }
      void tuplet3_5_7tuplets() { mf("tuplet_3_5_7_tuplets"); }
      void tuplet5_5TupletsRests() { mf("tuplet_5_5_tuplets_rests"); }
      void tuplet3_4() { mf("tuplet_3-4"); }
      void tupletDuplet() { mf("tuplet_duplet"); }
      void tupletMars() { mf("tuplet_mars"); }
      void tupletNonuplet3_4()
            {
                        // requires 1/64 quantization
            int defaultQuant = preferences.shortestNote;
            preferences.shortestNote = MScore::division / 16; // midi quantization: 1/64
            mf("tuplet_nonuplet_3-4");
            preferences.shortestNote = defaultQuant;
            }
      void tupletNonuplet4_4()
            {
                        // requires 1/64 quantization
            int defaultQuant = preferences.shortestNote;
            preferences.shortestNote = MScore::division / 16; // midi quantization: 1/64
            mf("tuplet_nonuplet_4-4");
            preferences.shortestNote = defaultQuant;
            }
      void tupletQuadruplet() { mf("tuplet_quadruplet"); }
      void tupletSeptuplet() { mf("tuplet_septuplet"); }
      void tupletTripletsMixed() { mf("tuplet_triplets_mixed"); }
      void tupletTriplet() { mf("tuplet_triplet"); }
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
      QString midiname = QString(name) + ".mid";
      QString mscorename = QString(name) + ".mscx";
      QCOMPARE(importMidi(score,  TESTROOT "/mtest/" + DIR + midiname), Score::FILE_NO_ERROR);
      QVERIFY(saveCompareScore(score, mscorename, DIR + mscorename));
      delete score;
      }

//---------------------------------------------------------
//  tuplet recognition fuctions
//---------------------------------------------------------

void TestImportMidi::findChordInBar()
      {
      std::multimap<Fraction, MidiChord> chords;
      chords.insert({Fraction::fromTicks(10), MidiChord()});
      chords.insert({Fraction::fromTicks(360), MidiChord()});
      chords.insert({Fraction::fromTicks(480), MidiChord()});
      chords.insert({Fraction::fromTicks(1480), MidiChord()});
      chords.insert({Fraction::fromTicks(2000), MidiChord()});
      chords.insert({Fraction::fromTicks(3201), MidiChord()});

      Fraction startBarTick;
      Fraction endBarTick = Fraction::fromTicks(4 * MScore::division); // 4/4

      auto firstChordIt = findFirstChordInRange(startBarTick, endBarTick,
                                                chords.begin(), chords.end());
      QCOMPARE(firstChordIt, chords.begin());
      auto endChordIt = findEndChordInRange(endBarTick, firstChordIt, chords.end());
      QCOMPARE(endChordIt, chords.find(Fraction::fromTicks(2000)));

      endBarTick = Fraction(0);

      firstChordIt = findFirstChordInRange(startBarTick, endBarTick,
                                           chords.begin(), chords.end());
      QCOMPARE(firstChordIt, chords.end());
      endChordIt = findEndChordInRange(endBarTick, firstChordIt, chords.end());
      QCOMPARE(endChordIt, chords.end());

      startBarTick = Fraction::fromTicks(10);
      endBarTick = Fraction::fromTicks(-100);

      firstChordIt = findFirstChordInRange(startBarTick, endBarTick,
                                           chords.begin(), chords.end());
      QCOMPARE(firstChordIt, chords.end());
      endChordIt = findEndChordInRange(endBarTick, firstChordIt, chords.end());
      QCOMPARE(endChordIt, chords.end());
      }

void TestImportMidi::bestChordForTupletNote()
      {
      Fraction tupletLen = Fraction::fromTicks(MScore::division);
      Fraction quantValue = Fraction::fromTicks(MScore::division) / 4;
      int tupletNumber = 3;
      Fraction tupletNoteLen = tupletLen / tupletNumber;

      std::multimap<Fraction, MidiChord> chords;
      chords.insert({Fraction::fromTicks(10), MidiChord()});
      chords.insert({Fraction::fromTicks(160), MidiChord()});
      chords.insert({Fraction::fromTicks(360), MidiChord()});
      chords.insert({Fraction::fromTicks(480), MidiChord()});
      chords.insert({Fraction::fromTicks(1480), MidiChord()});
      chords.insert({Fraction::fromTicks(2000), MidiChord()});
      chords.insert({Fraction::fromTicks(3201), MidiChord()});

      Fraction tupletNotePos = tupletNoteLen;
      auto bestChord = MidiTuplet::findBestChordForTupletNote(tupletNotePos, quantValue,
                                                              chords.begin(), chords.end());
      QCOMPARE(bestChord.first, chords.find(Fraction::fromTicks(160)));
      QCOMPARE(bestChord.second, Fraction(0));

      tupletNotePos = tupletNoteLen * 2;
      bestChord = MidiTuplet::findBestChordForTupletNote(tupletNotePos, quantValue,
                                                         chords.begin(), chords.end());
      QCOMPARE(bestChord.first, chords.find(Fraction::fromTicks(360)));
      QCOMPARE(bestChord.second, Fraction::fromTicks(40));
      }

// tupletNoteNumber - number of note in tuplet (like index):
// i.e. for triplet notes can have numbers 1, 2, 3

void isSingleNoteInTupletAllowed(int tupletNumber,
                                 int tupletNoteNumber,
                                 double noteLenInTupletLen,
                                 bool expectedResult)
      {
      Fraction tupletLen = Fraction::fromTicks(MScore::division);
      Fraction quantValue = Fraction::fromTicks(MScore::division) / 4;     // 1/16

      std::multimap<Fraction, MidiChord> chords;
      MidiChord chord;
      MidiNote note;
      note.len = Fraction::fromTicks(std::round(tupletLen.ticks() * noteLenInTupletLen));
      chord.notes.push_back(note);
      Fraction onTime = Fraction::fromTicks(10);
      chords.insert({onTime, chord});

      std::map<int, std::multimap<Fraction, MidiChord>::iterator> tupletChords;
      tupletChords.insert({tupletNoteNumber - 1, chords.begin()});
                  // tuplet error is less than regular error => allowed
      Fraction tupletOnTimeSumError;
      Fraction regularSumError = Fraction::fromTicks(1);
      QCOMPARE(MidiTuplet::isTupletAllowed(tupletNumber, tupletLen, tupletOnTimeSumError,
                                           regularSumError, quantValue, tupletChords),
               expectedResult);
      }

void isChordCountInTupletAllowed(int tupletNumber,
                                 int chordCount,
                                 bool expectedResult)
      {
      Fraction tupletLen = Fraction::fromTicks(MScore::division);
      Fraction quantValue = Fraction::fromTicks(MScore::division) / 4;     // 1/16

      std::map<int, std::multimap<Fraction, MidiChord>::iterator> tupletChords;
      std::multimap<Fraction, MidiChord> chords;
      for (int i = 0; i != chordCount; ++i) {
            MidiChord chord;
            MidiNote note;
            note.len = tupletLen / tupletNumber; // allowed
            chord.notes.push_back(note);
            Fraction onTime = tupletLen / tupletNumber * i;
            auto lastChordIt = chords.insert({onTime, chord});
            tupletChords.insert({i, lastChordIt});
            }
                  // tuplet error is less than regular error => allowed
      Fraction tupletOnTimeSumError;
      Fraction regularSumError = Fraction::fromTicks(1);
      QCOMPARE(MidiTuplet::isTupletAllowed(tupletNumber, tupletLen, tupletOnTimeSumError,
                                           regularSumError, quantValue, tupletChords),
               expectedResult);
      }

void isTupletErrorAllowed(int tupletOnTimeSumError,
                          int regularSumError,
                          bool expectedResult)
      {
      int tupletNumber = 3;
      Fraction tupletLen = Fraction::fromTicks(MScore::division);
      int quantValue = MScore::division / 4;     // 1/16

      std::multimap<Fraction, MidiChord> chords;
      MidiChord chord;
      MidiNote note;
      note.len = tupletLen / tupletNumber;  // allowed
      chord.notes.push_back(note);
      Fraction onTime = Fraction::fromTicks(10);
      chords.insert({onTime, chord});

      std::map<int, std::multimap<Fraction, MidiChord>::iterator> tupletChords;
      tupletChords.insert({0, chords.begin()});
      QCOMPARE(MidiTuplet::isTupletAllowed(tupletNumber, tupletLen, tupletOnTimeSumError,
                                           regularSumError, quantValue, tupletChords),
               expectedResult);
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
      Fraction barFraction(4, 4);
      Fraction divLen = barFraction / 4;
      auto numbers = MidiTuplet::findTupletNumbers(divLen, barFraction);
      QVERIFY(numbers.size() == 4);
      QCOMPARE(numbers[0], 3);
      QCOMPARE(numbers[1], 5);
      QCOMPARE(numbers[2], 7);
      QCOMPARE(numbers[3], 9);
      }
      {
      Fraction barFraction(6, 8);
      Fraction divLen = barFraction / 2;
      auto numbers = MidiTuplet::findTupletNumbers(divLen, barFraction);
      QVERIFY(numbers.size() == 1);
      QCOMPARE(numbers[0], 4);
                  // duplets are turned off by default
      }
      }

void TestImportMidi::findOnTimeRegularError()
      {
      Fraction quantValue = Fraction::fromTicks(MScore::division) / 4;  // 1/16
      Fraction onTime = quantValue + Fraction::fromTicks(12);
      QCOMPARE(MidiTuplet::findOnTimeRegularError(onTime, quantValue),
               Fraction::fromTicks(12));
      }

void TestImportMidi::findTupletApproximation()
      {
      int tupletNumber = 3;
      Fraction tupletLen = Fraction::fromTicks(MScore::division);
      Fraction quantValue = Fraction::fromTicks(MScore::division) / 4;  // 1/16

      std::multimap<Fraction, MidiChord> chords;
      chords.insert({Fraction::fromTicks(0), MidiChord()});
      chords.insert({Fraction::fromTicks(160), MidiChord()});
      chords.insert({Fraction::fromTicks(320), MidiChord()});
      chords.insert({Fraction::fromTicks(480), MidiChord()});
      chords.insert({Fraction::fromTicks(1480), MidiChord()});
      chords.insert({Fraction::fromTicks(2000), MidiChord()});
      chords.insert({Fraction::fromTicks(3201), MidiChord()});

      {
      Fraction startTupletTime = Fraction(0);
      MidiTuplet::TupletInfo tupletApprox = MidiTuplet::findTupletApproximation(
                        tupletLen, tupletNumber, quantValue,
                        startTupletTime, chords.begin(), chords.end()
                        );
      QCOMPARE(tupletApprox.onTime, startTupletTime);
      QCOMPARE(tupletApprox.len, tupletLen);
      QCOMPARE(tupletApprox.tupletNumber, tupletNumber);
      QCOMPARE(tupletApprox.tupletQuantValue, Fraction::fromTicks(MScore::division) / 3);
      QCOMPARE(tupletApprox.regularQuantValue, quantValue);
      QVERIFY(tupletApprox.chords.size() == 3);
      QCOMPARE(tupletApprox.tupletSumError, Fraction::fromTicks(0));
      QCOMPARE(tupletApprox.regularSumError, Fraction::fromTicks(80));
      QCOMPARE(tupletApprox.chords.find(0)->second, chords.find(Fraction::fromTicks(0)));
      QCOMPARE(tupletApprox.chords.find(1)->second, chords.find(Fraction::fromTicks(160)));
      QCOMPARE(tupletApprox.chords.find(2)->second, chords.find(Fraction::fromTicks(320)));
      }
      {
      Fraction startTupletTime = Fraction::fromTicks(960);
      MidiTuplet::TupletInfo tupletApprox = MidiTuplet::findTupletApproximation(
                        tupletLen, tupletNumber, quantValue,
                        startTupletTime, chords.begin(), chords.end()
                        );
      QVERIFY(tupletApprox.chords.size() == 0);
      }
      {
      Fraction startTupletTime = Fraction::fromTicks(1440);
      MidiTuplet::TupletInfo tupletApprox = MidiTuplet::findTupletApproximation(
                        tupletLen, tupletNumber, quantValue,
                        startTupletTime, chords.begin(), chords.end()
                        );
      QVERIFY(tupletApprox.chords.size() == 1);
      QCOMPARE(tupletApprox.tupletSumError, Fraction::fromTicks(40));
      QCOMPARE(tupletApprox.regularSumError, Fraction::fromTicks(40));
      QCOMPARE(tupletApprox.chords.find(0)->second, chords.find(Fraction::fromTicks(1480)));
      }
      }

//--------------------------------------------------------------------------
      // tuplet voice separation

MidiNote noteFactory(const Fraction &len, int pitch)
      {
      MidiNote note;
      note.len = len;
      note.pitch = pitch;
      return note;
      }

MidiChord chordFactory(const Fraction &len, const std::vector<int> &pitches)
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
      Fraction tupletLen = Fraction::fromTicks(MScore::division);
      Fraction endBarTick = tupletLen * 4;
      std::multimap<Fraction, MidiChord> chords;
                  // let's create 3 tuplets with the same first chord

                  // triplet
      Fraction tripletLen = tupletLen;
      const int tripletNumber = 3;
      Fraction tripletNoteLen = tripletLen / tripletNumber;

      MidiChord firstChord = chordFactory(tripletNoteLen, {76, 71, 67});
      chords.insert({0, firstChord});

      std::vector<int> pitches = {74, 77};
      for (int i = 1; i != tripletNumber; ++i) {
            chords.insert({tripletNoteLen * i,
                           chordFactory(tripletNoteLen, {pitches[i]})});
            }
                  // quintuplet
      Fraction quintupletLen = tupletLen;
      const int quintupletNumber = 5;
      Fraction quintupletNoteLen = quintupletLen / quintupletNumber;
      pitches = {60, 62, 58, 60};
      for (int i = 1; i != quintupletNumber; ++i) {
            chords.insert({quintupletNoteLen * i,
                           chordFactory(quintupletNoteLen, {pitches[i]})});
            }
                  // septuplet
      Fraction septupletLen = tupletLen * 2;
      const int septupletNumber = 7;
      Fraction septupletNoteLen = septupletLen / septupletNumber;
      pitches = {50, 52, 48, 51, 47, 47};
      for (int i = 1; i != septupletNumber; ++i) {
            chords.insert({septupletNoteLen * i,
                           chordFactory(septupletNoteLen, {pitches[i]})});
            }

      MidiTuplet::TupletInfo tripletInfo;
      tripletInfo.onTime = Fraction(0);
      tripletInfo.len = tripletLen;
      for (int i = 0; i != 3; ++i)
            tripletInfo.chords.insert({i, chords.find(tripletNoteLen * i)});

      MidiTuplet::TupletInfo quintupletInfo;
      quintupletInfo.onTime = Fraction(0);
      quintupletInfo.len = quintupletLen;
      for (int i = 0; i != 5; ++i)
            quintupletInfo.chords.insert({i, chords.find(quintupletNoteLen * i)});

      MidiTuplet::TupletInfo septupletInfo;
      septupletInfo.onTime = Fraction(0);
      septupletInfo.len = septupletLen;
      for (int i = 0; i != 7; ++i)
            septupletInfo.chords.insert({i, chords.find(septupletNoteLen * i)});

      std::vector<MidiTuplet::TupletInfo> tuplets;
      tuplets.push_back(tripletInfo);
      tuplets.push_back(quintupletInfo);
      tuplets.push_back(septupletInfo);

      QVERIFY(chords.size() == 13);

      auto tripletIt = tripletInfo.chords.find(0);  // first chord in tuplet
      QCOMPARE(tripletIt->second->second.notes.size(), 3);
      QCOMPARE(tripletIt->second->second.notes[0].pitch, 76);
      QCOMPARE(tripletIt->second->second.notes[1].pitch, 71);
      QCOMPARE(tripletIt->second->second.notes[2].pitch, 67);

      auto quintupletIt = quintupletInfo.chords.find(0);
      QCOMPARE(quintupletIt->second->second.notes.size(), 3);
      QCOMPARE(quintupletIt->second->second.notes[0].pitch, 76);
      QCOMPARE(quintupletIt->second->second.notes[1].pitch, 71);
      QCOMPARE(quintupletIt->second->second.notes[2].pitch, 67);

      auto septupletIt = septupletInfo.chords.find(0);
      QCOMPARE(septupletIt->second->second.notes.size(), 3);
      QCOMPARE(septupletIt->second->second.notes[0].pitch, 76);
      QCOMPARE(septupletIt->second->second.notes[1].pitch, 71);
      QCOMPARE(septupletIt->second->second.notes[2].pitch, 67);

      MidiTuplet::separateTupletVoices(tuplets, chords.begin(), chords.end(),
                                       chords, endBarTick);
      QVERIFY(chords.size() == 15);

      tripletInfo = tuplets[0];
      quintupletInfo = tuplets[1];
      septupletInfo = tuplets[2];

      tripletIt = tripletInfo.chords.find(0);
      QCOMPARE(tripletIt->second->second.notes.size(), 1);
      QCOMPARE(tripletIt->second->second.notes[0].pitch, 76);

      quintupletIt = quintupletInfo.chords.find(0);
      QCOMPARE(quintupletIt->second->second.notes.size(), 1);
      QCOMPARE(quintupletIt->second->second.notes[0].pitch, 71);

      septupletIt = septupletInfo.chords.find(0);
      QCOMPARE(septupletIt->second->second.notes.size(), 1);
      QCOMPARE(septupletIt->second->second.notes[0].pitch, 67);

      for (const auto &chord: tripletInfo.chords) {
            const MidiChord &midiChord = chord.second->second;
            QCOMPARE(midiChord.voice, 0);
            }
      for (const auto &chord: quintupletInfo.chords) {
            const MidiChord &midiChord = chord.second->second;
            QCOMPARE(midiChord.voice, 1);
            }
      for (const auto &chord: septupletInfo.chords) {
            const MidiChord &midiChord = chord.second->second;
            QCOMPARE(midiChord.voice, 2);
            }
      }


//---------------------------------------------------------
//  metric bar analysis
//---------------------------------------------------------

void TestImportMidi::metricDivisionsOfTuplet()
      {
      MidiTuplet::TupletData tupletData;
      tupletData.voice = 0;
      tupletData.len = Fraction::fromTicks(480);
      tupletData.onTime = Fraction::fromTicks(480);
      tupletData.tupletNumber = 3;
      int tupletStartLevel = -3;
      Meter::DivisionInfo tupletDivInfo = Meter::metricDivisionsOfTuplet(tupletData, tupletStartLevel);

      QCOMPARE(tupletDivInfo.isTuplet, true);
      QCOMPARE(tupletDivInfo.len, Fraction::fromTicks(480));
      QCOMPARE(tupletDivInfo.onTime, Fraction::fromTicks(480));
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
      Fraction barFraction = Fraction(4, 4);
      Fraction startTickInBar = Fraction::fromTicks(240);
      Fraction endTickInBar = Fraction::fromTicks(500);

      Meter::DivisionInfo regularDivInfo = Meter::metricDivisionsOfBar(barFraction);
      QVERIFY(regularDivInfo.divLengths.size() == 8);

      Meter::MaxLevel level = Meter::maxLevelBetween(startTickInBar, endTickInBar, regularDivInfo);
      QCOMPARE(level.level, -2);
      QCOMPARE(level.lastPos, Fraction::fromTicks(480));
      QCOMPARE(level.levelCount, 1);

      startTickInBar = Fraction::fromTicks(499);
      level = Meter::maxLevelBetween(startTickInBar, endTickInBar, regularDivInfo);
      QCOMPARE(level.level, 0);
      QCOMPARE(level.lastPos, Fraction(-1, 1));
      QCOMPARE(level.levelCount, 0);

      MidiTuplet::TupletData tupletData;
      tupletData.voice = 0;
      tupletData.len = Fraction::fromTicks(480);
      tupletData.onTime = Fraction::fromTicks(480);
      tupletData.tupletNumber = 3;

      int tupletStartLevel = -3;
      Meter::DivisionInfo tupletDivInfo = Meter::metricDivisionsOfTuplet(tupletData, tupletStartLevel);
      QVERIFY(tupletDivInfo.divLengths.size() == 5);

      startTickInBar = tupletData.onTime;
      endTickInBar = startTickInBar + tupletData.len;
      level = Meter::maxLevelBetween(startTickInBar, endTickInBar, tupletDivInfo);
      QCOMPARE(level.level, tupletStartLevel);
      QCOMPARE(level.lastPos, tupletData.onTime + tupletData.len * 2 / tupletData.tupletNumber);
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
      startTickInBar = Fraction::fromTicks(0);
      endTickInBar = (startTickInBar + tupletData.onTime) / 2;
      level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
      QVERIFY(level.level == -4);
      QCOMPARE(level.lastPos, (startTickInBar + endTickInBar) / 2);
      QCOMPARE(level.levelCount, 1);
                  // s < e == ts < te
      startTickInBar = Fraction::fromTicks(0);
      endTickInBar = tupletData.onTime;
      level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
      qDebug() << level.level;
      QVERIFY(level.level == -3);
      QCOMPARE(level.lastPos, (startTickInBar + tupletData.onTime) / 2);
      QCOMPARE(level.levelCount, 1);
                  // s < ts < e < te
      startTickInBar = Fraction::fromTicks(0);
      endTickInBar = tupletData.onTime + tupletData.len / 2;
      level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
      QVERIFY(level.level == Meter::TUPLET_BOUNDARY_LEVEL);
      QCOMPARE(level.lastPos, tupletData.onTime);
      QCOMPARE(level.levelCount, 1);
                  // s < ts < e == te
      startTickInBar = Fraction::fromTicks(0);
      endTickInBar = tupletData.onTime + tupletData.len;
      level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
      QVERIFY(level.level == Meter::TUPLET_BOUNDARY_LEVEL);
      QCOMPARE(level.lastPos, tupletData.onTime);
      QCOMPARE(level.levelCount, 1);
                  // s < ts < te < e
      startTickInBar = Fraction::fromTicks(0);
      endTickInBar = tupletData.onTime + tupletData.len + Fraction(1, 3);
      level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
      QVERIFY(level.level == Meter::TUPLET_BOUNDARY_LEVEL);
      QCOMPARE(level.lastPos, tupletData.onTime + tupletData.len);
      QCOMPARE(level.levelCount, 1);
                  // s == ts < e < te
      startTickInBar = tupletData.onTime;
      endTickInBar = tupletData.onTime + tupletData.len / 2;
      level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
      QCOMPARE(level.level, tupletStartLevel);
      QCOMPARE(level.lastPos, tupletData.onTime + tupletData.len / tupletData.tupletNumber);
      QCOMPARE(level.levelCount, 1);
                  // s == ts < e == te
      startTickInBar = tupletData.onTime;
      endTickInBar = tupletData.onTime + tupletData.len;
      level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
      QCOMPARE(level.level, tupletStartLevel);
      QCOMPARE(level.lastPos, tupletData.onTime + tupletData.len * 2 / tupletData.tupletNumber);
      QCOMPARE(level.levelCount, 2);
                  // s == ts < te < e
      startTickInBar = tupletData.onTime;
      endTickInBar = tupletData.onTime + tupletData.len * 2;
      level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
      QCOMPARE(level.level, Meter::TUPLET_BOUNDARY_LEVEL);
      QCOMPARE(level.lastPos, tupletData.onTime + tupletData.len);
      QCOMPARE(level.levelCount, 1);
                  // ts < s < e < te
      startTickInBar = tupletData.onTime + tupletData.len / tupletData.tupletNumber;
      endTickInBar = tupletData.onTime + tupletData.len * 2 / tupletData.tupletNumber;
      level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
      QVERIFY(level.level == tupletStartLevel - 1);
      QCOMPARE(level.lastPos, (startTickInBar + endTickInBar) / 2);
      QCOMPARE(level.levelCount, 1);
                  // ts < s < e = te
      startTickInBar = tupletData.onTime + tupletData.len / tupletData.tupletNumber;
      endTickInBar = tupletData.onTime + tupletData.len;
      level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
      QVERIFY(level.level == tupletStartLevel);
      QCOMPARE(level.lastPos, tupletData.onTime + tupletData.len * 2 / tupletData.tupletNumber);
      QCOMPARE(level.levelCount, 1);
                  // ts < s < te < e
      startTickInBar = tupletData.onTime + tupletData.len / 2;
      endTickInBar = tupletData.onTime + tupletData.len + Fraction(1, 3);
      level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
      QVERIFY(level.level == Meter::TUPLET_BOUNDARY_LEVEL);
      QCOMPARE(level.lastPos, tupletData.onTime + tupletData.len);
      QCOMPARE(level.levelCount, 1);
                  // ts < te = s < e
      startTickInBar = tupletData.onTime + tupletData.len;
      endTickInBar = tupletData.onTime + tupletData.len + Fraction(1, 3);
      level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
      QVERIFY(level.level != Meter::TUPLET_BOUNDARY_LEVEL);
                  // ts < te < s < e
      startTickInBar = tupletData.onTime + tupletData.len + Fraction(1, 3);
      endTickInBar = startTickInBar + Fraction(1, 3);
      level = Meter::findMaxLevelBetween(startTickInBar, endTickInBar, divInfo);
      QVERIFY(level.level != Meter::TUPLET_BOUNDARY_LEVEL);
      }


QTEST_MAIN(TestImportMidi)

#include "tst_importmidi.moc"

