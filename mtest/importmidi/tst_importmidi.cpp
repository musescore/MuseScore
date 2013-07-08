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
      std::multimap<int, MidiChord> chords;
      chords.insert({10, MidiChord()});
      chords.insert({360, MidiChord()});
      chords.insert({480, MidiChord()});
      chords.insert({1480, MidiChord()});
      chords.insert({2000, MidiChord()});
      chords.insert({3201, MidiChord()});

      int startBarTick = 0;
      int endBarTick = 4 * MScore::division; // 4/4

      auto firstChordIt = findFirstChordInRange(startBarTick, endBarTick,
                                                chords.begin(), chords.end());
      QCOMPARE(firstChordIt, chords.begin());
      auto endChordIt = findEndChordInRange(endBarTick, firstChordIt, chords.end());
      QCOMPARE(endChordIt, chords.find(2000));

      endBarTick = 0;

      firstChordIt = findFirstChordInRange(startBarTick, endBarTick,
                                           chords.begin(), chords.end());
      QCOMPARE(firstChordIt, chords.end());
      endChordIt = findEndChordInRange(endBarTick, firstChordIt, chords.end());
      QCOMPARE(endChordIt, chords.end());

      startBarTick = 10;
      endBarTick = -100;

      firstChordIt = findFirstChordInRange(startBarTick, endBarTick,
                                           chords.begin(), chords.end());
      QCOMPARE(firstChordIt, chords.end());
      endChordIt = findEndChordInRange(endBarTick, firstChordIt, chords.end());
      QCOMPARE(endChordIt, chords.end());
      }

void TestImportMidi::bestChordForTupletNote()
      {
      Fraction tupletLen = Fraction::fromTicks(MScore::division);
      int quantValue = MScore::division / 4;
      int tupletNumber = 3;
      Fraction tupletNoteLen = tupletLen / tupletNumber;

      std::multimap<int, MidiChord> chords;
      chords.insert({10, MidiChord()});
      chords.insert({160, MidiChord()});
      chords.insert({360, MidiChord()});
      chords.insert({480, MidiChord()});
      chords.insert({1480, MidiChord()});
      chords.insert({2000, MidiChord()});
      chords.insert({3201, MidiChord()});

      int tupletNotePos = tupletNoteLen.ticks();
      auto bestChord = MidiTuplet::findBestChordForTupletNote(tupletNotePos, quantValue,
                                                              chords.begin(), chords.end());
      QCOMPARE(bestChord.first, chords.find(160));
      QCOMPARE(bestChord.second, 0);

      tupletNotePos = (tupletNoteLen * 2).ticks();
      bestChord = MidiTuplet::findBestChordForTupletNote(tupletNotePos, quantValue,
                                                         chords.begin(), chords.end());
      QCOMPARE(bestChord.first, chords.find(360));
      QCOMPARE(bestChord.second, 40);
      }

// tupletNoteNumber - number of note in tuplet (like index):
// i.e. for triplet notes can have numbers 1, 2, 3

void isSingleNoteInTupletAllowed(int tupletNumber,
                                 int tupletNoteNumber,
                                 double noteLenInTupletLen,
                                 bool expectedResult)
      {
      Fraction tupletLen = Fraction::fromTicks(MScore::division);
      int quantValue = MScore::division / 4;     // 1/16

      std::multimap<int, MidiChord> chords;
      MidiChord chord;
      MidiNote note;
      note.len = std::round(tupletLen.ticks() * noteLenInTupletLen);
      chord.notes.push_back(note);
      int onTime = 10;
      chords.insert({onTime, chord});

      std::map<int, std::multimap<int, MidiChord>::iterator> tupletChords;
      tupletChords.insert({tupletNoteNumber - 1, chords.begin()});
                  // tuplet error is less than regular error => allowed
      int tupletOnTimeSumError = 0;
      int regularSumError = 1;
      QCOMPARE(MidiTuplet::isTupletAllowed(tupletNumber, tupletLen, tupletOnTimeSumError,
                                           regularSumError, quantValue, tupletChords),
               expectedResult);
      }

void isChordCountInTupletAllowed(int tupletNumber,
                                 int chordCount,
                                 bool expectedResult)
      {
      Fraction tupletLen = Fraction::fromTicks(MScore::division);
      int quantValue = MScore::division / 4;     // 1/16

      std::map<int, std::multimap<int, MidiChord>::iterator> tupletChords;
      std::multimap<int, MidiChord> chords;
      for (int i = 0; i != chordCount; ++i) {
            MidiChord chord;
            MidiNote note;
            note.len = (tupletLen / tupletNumber).ticks(); // allowed
            chord.notes.push_back(note);
            int onTime = (tupletLen / tupletNumber * i).ticks();
            auto lastChordIt = chords.insert({onTime, chord});
            tupletChords.insert({i, lastChordIt});
            }
                  // tuplet error is less than regular error => allowed
      int tupletOnTimeSumError = 0;
      int regularSumError = 1;
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

      std::multimap<int, MidiChord> chords;
      MidiChord chord;
      MidiNote note;
      note.len = (tupletLen / tupletNumber).ticks();  // allowed
      chord.notes.push_back(note);
      int onTime = 10;
      chords.insert({onTime, chord});

      std::map<int, std::multimap<int, MidiChord>::iterator> tupletChords;
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
      QVERIFY(numbers.size() == 3);
      QCOMPARE(numbers[0], 3);
      QCOMPARE(numbers[1], 5);
      QCOMPARE(numbers[2], 7);
      }
      {
      Fraction barFraction(6, 8);
      Fraction divLen = barFraction / 2;
      auto numbers = MidiTuplet::findTupletNumbers(divLen, barFraction);
      QVERIFY(numbers.size() == 2);
      QCOMPARE(numbers[0], 2);
      QCOMPARE(numbers[1], 4);
      }
      }

void TestImportMidi::findOnTimeRegularError()
      {
      int quantValue = MScore::division / 4;  // 1/16
      int onTime = quantValue + 12;
      QCOMPARE(MidiTuplet::findOnTimeRegularError(onTime, quantValue), 12);
      }

void TestImportMidi::findTupletApproximation()
      {
      int tupletNumber = 3;
      Fraction tupletLen = Fraction::fromTicks(MScore::division);
      int quantValue = MScore::division / 4;  // 1/16

      std::multimap<int, MidiChord> chords;
      chords.insert({0, MidiChord()});
      chords.insert({160, MidiChord()});
      chords.insert({320, MidiChord()});
      chords.insert({480, MidiChord()});
      chords.insert({1480, MidiChord()});
      chords.insert({2000, MidiChord()});
      chords.insert({3201, MidiChord()});

      {
      int startTupletTime = 0;
      MidiTuplet::TupletInfo tupletApprox = MidiTuplet::findTupletApproximation(
                        tupletLen, tupletNumber, quantValue,
                        startTupletTime, chords.begin(), chords.end()
                        );
      QCOMPARE(tupletApprox.onTime, startTupletTime);
      QCOMPARE(tupletApprox.len.ticks(), tupletLen.ticks());
      QCOMPARE(tupletApprox.tupletNumber, tupletNumber);
      QCOMPARE(tupletApprox.tupletQuantValue.ticks(), MScore::division / 3);
      QCOMPARE(tupletApprox.regularQuantValue.ticks(), quantValue);
      QVERIFY(tupletApprox.chords.size() == 3);
      QCOMPARE(tupletApprox.tupletSumError, 0);
      QCOMPARE(tupletApprox.regularSumError, 80);
      QCOMPARE(tupletApprox.chords.find(0)->second, chords.find(0));
      QCOMPARE(tupletApprox.chords.find(1)->second, chords.find(160));
      QCOMPARE(tupletApprox.chords.find(2)->second, chords.find(320));
      }
      {
      int startTupletTime = 960;
      MidiTuplet::TupletInfo tupletApprox = MidiTuplet::findTupletApproximation(
                        tupletLen, tupletNumber, quantValue,
                        startTupletTime, chords.begin(), chords.end()
                        );
      QVERIFY(tupletApprox.chords.size() == 0);
      }
      {
      int startTupletTime = 1440;
      MidiTuplet::TupletInfo tupletApprox = MidiTuplet::findTupletApproximation(
                        tupletLen, tupletNumber, quantValue,
                        startTupletTime, chords.begin(), chords.end()
                        );
      QVERIFY(tupletApprox.chords.size() == 1);
      QCOMPARE(tupletApprox.tupletSumError, 40);
      QCOMPARE(tupletApprox.regularSumError, 40);
      QCOMPARE(tupletApprox.chords.find(0)->second, chords.find(1480));
      }
      }

//--------------------------------------------------------------------------
      // tuplet voice separation

MidiNote noteFactory(int len, int pitch)
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
            notes.push_back(noteFactory(len.ticks(), pitch));
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
      int endBarTick = (tupletLen * 4).ticks();
      std::multimap<int, MidiChord> chords;
                  // let's create 3 tuplets with the same first chord

                  // triplet
      Fraction tripletLen = tupletLen;
      const int tripletNumber = 3;
      Fraction tripletNoteLen = tripletLen / tripletNumber;

      MidiChord firstChord = chordFactory(tripletNoteLen, {76, 71, 67});
      chords.insert({0, firstChord});

      std::vector<int> pitches = {74, 77};
      for (int i = 1; i != tripletNumber; ++i) {
            chords.insert({(tripletNoteLen * i).ticks(),
                           chordFactory(tripletNoteLen, {pitches[i]})});
            }
                  // quintuplet
      Fraction quintupletLen = tupletLen;
      const int quintupletNumber = 5;
      Fraction quintupletNoteLen = quintupletLen / quintupletNumber;
      pitches = {60, 62, 58, 60};
      for (int i = 1; i != quintupletNumber; ++i) {
            chords.insert({(quintupletNoteLen * i).ticks(),
                           chordFactory(quintupletNoteLen, {pitches[i]})});
            }
                  // septuplet
      Fraction septupletLen = tupletLen * 2;
      const int septupletNumber = 7;
      Fraction septupletNoteLen = septupletLen / septupletNumber;
      pitches = {50, 52, 48, 51, 47, 47};
      for (int i = 1; i != septupletNumber; ++i) {
            chords.insert({(septupletNoteLen * i).ticks(),
                           chordFactory(septupletNoteLen, {pitches[i]})});
            }

      MidiTuplet::TupletInfo tripletInfo;
      tripletInfo.onTime = 0;
      tripletInfo.len = tripletLen;
      for (int i = 0; i != 3; ++i)
            tripletInfo.chords.insert({i, chords.find((tripletNoteLen * i).ticks())});

      MidiTuplet::TupletInfo quintupletInfo;
      quintupletInfo.onTime = 0;
      quintupletInfo.len = quintupletLen;
      for (int i = 0; i != 5; ++i)
            quintupletInfo.chords.insert({i, chords.find((quintupletNoteLen * i).ticks())});

      MidiTuplet::TupletInfo septupletInfo;
      septupletInfo.onTime = 0;
      septupletInfo.len = septupletLen;
      for (int i = 0; i != 7; ++i)
            septupletInfo.chords.insert({i, chords.find((septupletNoteLen * i).ticks())});

      std::vector<MidiTuplet::TupletInfo> tuplets;
      tuplets.push_back(tripletInfo);
      tuplets.push_back(quintupletInfo);
      tuplets.push_back(septupletInfo);

      QVERIFY(chords.size() == 13);

      auto tripletIt = tripletInfo.chords.find(0);
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


QTEST_MAIN(TestImportMidi)

#include "tst_importmidi.moc"

