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
#include "mscore/importmidi_tupletdata.h"


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
      void findTupletCandidatesOfBar();
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

      auto firstChordIt = Quantize::findFirstChordInRange(startBarTick, endBarTick,
                                                          chords.begin(), chords.end());
      QCOMPARE(firstChordIt, chords.begin());
      auto endChordIt = Quantize::findEndChordInRange(endBarTick, firstChordIt, chords.end());
      QCOMPARE(endChordIt, chords.find(2000));

      endBarTick = 0;

      firstChordIt = Quantize::findFirstChordInRange(startBarTick, endBarTick,
                                                     chords.begin(), chords.end());
      QCOMPARE(firstChordIt, chords.end());
      endChordIt = Quantize::findEndChordInRange(endBarTick, firstChordIt, chords.end());
      QCOMPARE(endChordIt, chords.end());

      startBarTick = 10;
      endBarTick = -100;

      firstChordIt = Quantize::findFirstChordInRange(startBarTick, endBarTick,
                                                     chords.begin(), chords.end());
      QCOMPARE(firstChordIt, chords.end());
      endChordIt = Quantize::findEndChordInRange(endBarTick, firstChordIt, chords.end());
      QCOMPARE(endChordIt, chords.end());
      }

void TestImportMidi::bestChordForTupletNote()
      {
      int tupletLen = MScore::division;
      int quantValue = MScore::division / 4;
      int tupletNumber = 3;
      int tupletNoteLen = tupletLen / tupletNumber;

      std::multimap<int, MidiChord> chords;
      chords.insert({10, MidiChord()});
      chords.insert({160, MidiChord()});
      chords.insert({360, MidiChord()});
      chords.insert({480, MidiChord()});
      chords.insert({1480, MidiChord()});
      chords.insert({2000, MidiChord()});
      chords.insert({3201, MidiChord()});

      int tupletNotePos = 1 * tupletNoteLen;
      auto bestChord = Quantize::findBestChordForTupletNote(tupletNotePos, quantValue,
                                                            chords.begin(), chords.end());
      QCOMPARE(bestChord.first, chords.find(160));
      QCOMPARE(bestChord.second, 0);

      tupletNotePos = 2 * tupletNoteLen;
      bestChord = Quantize::findBestChordForTupletNote(tupletNotePos, quantValue,
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
      int tupletLen = MScore::division;
      int quantValue = MScore::division / 4;     // 1/16

      std::multimap<int, MidiChord> chords;
      MidiChord chord;
      MidiNote note;
      note.len = std::round(tupletLen * noteLenInTupletLen);
      chord.notes.push_back(note);
      chord.onTime = 10;
      chords.insert({chord.onTime, chord});

      std::map<int, std::multimap<int, MidiChord>::iterator> tupletChords;
      tupletChords.insert({tupletNoteNumber - 1, chords.begin()});
                  // tuplet error is less than regular error => allowed
      int tupletOnTimeSumError = 0;
      int regularSumError = 1;
      QCOMPARE(Quantize::isTupletAllowed(tupletNumber, tupletLen, tupletOnTimeSumError,
                                         regularSumError, quantValue, tupletChords),
               expectedResult);
      }

void isChordCountInTupletAllowed(int tupletNumber,
                                 int chordCount,
                                 bool expectedResult)
      {
      int tupletLen = MScore::division;
      int quantValue = MScore::division / 4;     // 1/16

      std::map<int, std::multimap<int, MidiChord>::iterator> tupletChords;
      std::multimap<int, MidiChord> chords;
      for (int i = 0; i != chordCount; ++i) {
            MidiChord chord;
            MidiNote note;
            note.len = tupletLen / tupletNumber; // allowed
            chord.notes.push_back(note);
            chord.onTime = note.len * i;
            auto lastChordIt = chords.insert({chord.onTime, chord});
            tupletChords.insert({i, lastChordIt});
            }
                  // tuplet error is less than regular error => allowed
      int tupletOnTimeSumError = 0;
      int regularSumError = 1;
      QCOMPARE(Quantize::isTupletAllowed(tupletNumber, tupletLen, tupletOnTimeSumError,
                                         regularSumError, quantValue, tupletChords),
               expectedResult);
      }

void isTupletErrorAllowed(int tupletOnTimeSumError,
                          int regularSumError,
                          bool expectedResult)
      {
      int tupletNumber = 3;
      int tupletLen = MScore::division;
      int quantValue = MScore::division / 4;     // 1/16

      std::multimap<int, MidiChord> chords;
      MidiChord chord;
      MidiNote note;
      note.len = tupletLen / tupletNumber;  // allowed
      chord.notes.push_back(note);
      chord.onTime = 10;
      chords.insert({chord.onTime, chord});

      std::map<int, std::multimap<int, MidiChord>::iterator> tupletChords;
      tupletChords.insert({0, chords.begin()});
      QCOMPARE(Quantize::isTupletAllowed(tupletNumber, tupletLen, tupletOnTimeSumError,
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
      int divLen = barFraction.ticks() / 4;
      auto numbers = Quantize::findTupletNumbers(divLen, barFraction);
      QVERIFY(numbers.size() == 3);
      QCOMPARE(numbers[0], 3);
      QCOMPARE(numbers[1], 5);
      QCOMPARE(numbers[2], 7);
      }
      {
      Fraction barFraction(6, 8);
      int divLen = barFraction.ticks() / 2;
      auto numbers = Quantize::findTupletNumbers(divLen, barFraction);
      QVERIFY(numbers.size() == 2);
      QCOMPARE(numbers[0], 2);
      QCOMPARE(numbers[1], 4);
      }
      }

void TestImportMidi::findOnTimeRegularError()
      {
      int quantValue = MScore::division / 4;  // 1/16
      int onTime = quantValue + 12;
      QCOMPARE(Quantize::findOnTimeRegularError(onTime, quantValue), 12);
      }

void TestImportMidi::findTupletApproximation()
      {
      int tupletNumber = 3;
      int tupletLen = MScore::division;
      int tupletNoteLen = tupletLen / tupletNumber;
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
      Ms::Quantize::TupletInfo tupletApprox = Quantize::findTupletApproximation(
                        tupletNumber, tupletNoteLen, quantValue,
                        startTupletTime, chords.begin(), chords.end()
                        );
      QCOMPARE(tupletApprox.onTime, startTupletTime);
      QCOMPARE(tupletApprox.len, tupletLen);
      QCOMPARE(tupletApprox.tupletNumber, tupletNumber);
      QCOMPARE(tupletApprox.tupletQuantValue, MScore::division / 3);
      QCOMPARE(tupletApprox.regularQuantValue, quantValue);
      QVERIFY(tupletApprox.chords.size() == 3);
      QCOMPARE(tupletApprox.tupletOnTimeSumError, 0);
      QCOMPARE(tupletApprox.regularSumError, 80);
      QCOMPARE(tupletApprox.chords.find(0)->second, chords.find(0));
      QCOMPARE(tupletApprox.chords.find(1)->second, chords.find(160));
      QCOMPARE(tupletApprox.chords.find(2)->second, chords.find(320));
      }
      {
      int startTupletTime = 960;
      Ms::Quantize::TupletInfo tupletApprox = Quantize::findTupletApproximation(
                        tupletNumber, tupletNoteLen, quantValue,
                        startTupletTime, chords.begin(), chords.end()
                        );
      QVERIFY(tupletApprox.chords.size() == 0);
      }
      {
      int startTupletTime = 1440;
      Ms::Quantize::TupletInfo tupletApprox = Quantize::findTupletApproximation(
                        tupletNumber, tupletNoteLen, quantValue,
                        startTupletTime, chords.begin(), chords.end()
                        );
      QVERIFY(tupletApprox.chords.size() == 1);
      QCOMPARE(tupletApprox.tupletOnTimeSumError, 40);
      QCOMPARE(tupletApprox.regularSumError, 40);
      QCOMPARE(tupletApprox.chords.find(0)->second, chords.find(1480));
      }
      }

void TestImportMidi::findTupletCandidatesOfBar()
      {
//      int startBarTick = 0;
//      int endBarTick = 1920;
//      Fraction barFraction(4, 4);

//      std::multimap<int, MidiChord> chords;
//      chords.insert({0, MidiChord()});
//      chords.insert({160, MidiChord()});
//      chords.insert({320, MidiChord()});
//      chords.insert({480, MidiChord()});
//      chords.insert({1480, MidiChord()});
//      chords.insert({2000, MidiChord()});
//      chords.insert({3201, MidiChord()});

//      std::multimap<double, TupletInfo> candidates
//                  = Quantize::findTupletCandidatesOfBar(startBarTick, endBarTick,
//                                                        barFraction, chords);
//      QVERIFY(candidates.size() == 2);
      }


QTEST_MAIN(TestImportMidi)

#include "tst_importmidi.moc"

