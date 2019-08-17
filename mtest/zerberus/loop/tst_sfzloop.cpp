
//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>

#include "mtest/testutils.h"

#include "zerberus/instrument.h"
#include "zerberus/zerberus.h"
#include "zerberus/zone.h"
#include "mscore/preferences.h"
#include "synthesizer/event.h"
#include <sndfile.h>

using namespace Ms;

//---------------------------------------------------------
//   TestSfzEnvelopes
//---------------------------------------------------------

class TestSfzLoop : public QObject, public MTest
      {
      Q_OBJECT
      float samplerate = 44100;
      Zerberus* synth;

   private slots:
      void initTestCase();
      void testEnvelopesParsing();
      void testLoopAudio();
   public:
      ~TestSfzLoop();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestSfzLoop::initTestCase()
      {
      initMTest();
      synth = new Zerberus();
      synth->init(samplerate);
      preferences.setPreference(PREF_APP_PATHS_MYSOUNDFONTS, root);
      synth->loadInstrument("loopTest.sfz");
      }

//---------------------------------------------------------
//   testglobal
//---------------------------------------------------------

void TestSfzLoop::testEnvelopesParsing()
      {
      QCOMPARE(synth->instrument(0)->zones().size(), (size_t) 4);

      std::list<Zone *>::iterator curZone = synth->instrument(0)->zones().begin();
      QCOMPARE((*curZone)->keyLo, (char) 20);
      QCOMPARE((*curZone)->keyHi, (char) 20);
      QCOMPARE((*curZone)->keyBase, (char) 20);
      QCOMPARE((*curZone)->loopMode, LoopMode::NO_LOOP);
      curZone++;

      QCOMPARE((*curZone)->keyLo, (char) 21);
      QCOMPARE((*curZone)->keyHi, (char) 21);
      QCOMPARE((*curZone)->keyBase, (char) 21);
      QCOMPARE((*curZone)->loopMode, LoopMode::ONE_SHOT);
      curZone++;

      QCOMPARE((*curZone)->keyLo, (char) 22);
      QCOMPARE((*curZone)->keyHi, (char) 22);
      QCOMPARE((*curZone)->keyBase, (char) 22);
      QCOMPARE((*curZone)->loopMode, LoopMode::CONTINUOUS);
      curZone++;

      QCOMPARE((*curZone)->keyLo, (char) 23);
      QCOMPARE((*curZone)->keyHi, (char) 23);
      QCOMPARE((*curZone)->keyBase, (char) 23);
      QCOMPARE((*curZone)->loopMode, LoopMode::SUSTAIN);
      curZone++;
      }

void TestSfzLoop::testLoopAudio()
      {
      synth->play(Ms::PlayEvent(ME_PROGRAM, 0, 0, 0));
      float data[2 * 110 * 3]; // 2 Channel 110 Sample duration - enough space for at least one loop
      memset(data, 0, sizeof(data));

      // test no_loop
      synth->play(Ms::PlayEvent(ME_NOTEON, 0, 20, 127)); // play note - no_loop mode
      synth->process(50, data, nullptr, nullptr); // process - stop in middle of ones
      synth->play(Ms::PlayEvent(ME_NOTEON, 0, 20, 0)); // send note off and check whether voice is actually turned off
      synth->process(50 , data + (50*2), nullptr, nullptr); // process until end of sustain stage
      QCOMPARE(data[0], 0.0f);
      QCOMPARE(data[1], 0.0f);
      QCOMPARE(data[10 * 2] != 0.0f, true);
      QCOMPARE(data[10 * 2 + 1] != 0.0f, true);
      QCOMPARE(data[50 * 2], 0.0f);
      QCOMPARE(data[50 * 2 + 1], 0.0f);
      memset(data, 0, sizeof(data)); // clear data because each voice gets added to it!

      // test one_shot
      synth->play(Ms::PlayEvent(ME_NOTEON, 0, 21, 127)); // play note - one_shot
      synth->process(50, data, nullptr, nullptr); // process - stop in middle of ones
      synth->play(Ms::PlayEvent(ME_NOTEON, 0, 21, 0)); // send note off - sample should still play to end!
      synth->process(60 , data + (50*2), nullptr, nullptr); // process until end of sample
      QCOMPARE(data[0], 0.0f);
      QCOMPARE(data[1], 0.0f);
      QVERIFY(data[10 * 2] != 0.0f);
      QVERIFY(data[10 * 2 + 1] != 0.0f);
      QVERIFY(data[50 * 2] != 0.0f);
      QVERIFY(data[50 * 2 + 1] != 0.0f);
      QVERIFY(data[60 * 2] > pow(10, -30.0f/20.0f)); // it is not zero due to the filter - assume at least -30dB right after stop
      QVERIFY(data[60 * 2 + 1] > pow(10, -30.0f/20.0f));
      QVERIFY(data[70 * 2] < pow(10, -85.0f/20.0f)); // and -85dB 10 samples later
      QVERIFY(data[70 * 2 + 1] < pow(10, -85.0f/20.0f));
      memset(data, 0, sizeof(data)); // clear data because each voice gets added to it!

      // test loop_continuous
      synth->play(Ms::PlayEvent(ME_NOTEON, 0, 22, 127)); // play note
      synth->process(109, data, nullptr, nullptr); // process - it should loop (loop from 10 to 59)
      synth->play(Ms::PlayEvent(ME_NOTEON, 0, 22, 0)); // send note off - sample should not switch to the zero part
      synth->process(60 , data + (109*2), nullptr, nullptr); // process until end of release
      QCOMPARE(data[0], 0.0f);
      QCOMPARE(data[1], 0.0f);
      QVERIFY(data[10 * 2] != 0.0f);
      QVERIFY(data[10 * 2 + 1] != 0.0f);
      QVERIFY(data[50 * 2] != 0.0f);
      QVERIFY(data[50 * 2 + 1] != 0.0f);
      QVERIFY(data[60 * 2] > pow(10, -15.0f/20.0f)); // it should not stop make sure it is loud enough!
      QVERIFY(data[60 * 2 + 1] > pow(10, -15.0f/20.0f));
      QVERIFY(data[70 * 2] > pow(10, -15.0f/20.0f)); // it should not stop make sure it is loud enough!
      QVERIFY(data[70 * 2 + 1] > pow(10, -15.0f/20.0f));
      QVERIFY(data[110 * 2] > pow(10, -20.0f/20.0f)); // it should not stop make sure it is loud enough!
      QVERIFY(data[110 * 2 + 1] > pow(10, -20.0f/20.0f));
      QVERIFY(data[120 * 2] > pow(10, -35.0f/20.0f)); // it should not stop make sure it is loud enough!
      QVERIFY(data[120 * 2 + 1] > pow(10, -35.0f/20.0f));
      memset(data, 0, sizeof(data)); // clear data because each voice gets added to it!

      // test loop_sustain
      synth->play(Ms::PlayEvent(ME_NOTEON, 0, 23, 127)); // play note
      synth->process(109, data, nullptr, nullptr); // process - it should loop (loop from 10 to 59)
      synth->play(Ms::PlayEvent(ME_NOTEON, 0, 23, 0)); // send note off - sample should not switch to the zero part
      synth->process(60 , data + (109*2), nullptr, nullptr); // process until end of release
      QCOMPARE(data[0], 0.0f);
      QCOMPARE(data[1], 0.0f);
      QVERIFY(data[10 * 2] != 0.0f);
      QVERIFY(data[10 * 2 + 1] != 0.0f);
      QVERIFY(data[50 * 2] != 0.0f);
      QVERIFY(data[50 * 2 + 1] != 0.0f);
      QVERIFY(data[60 * 2] > pow(10, -15.0f/20.0f)); // it should not stop make sure it is loud enough!
      QVERIFY(data[60 * 2 + 1] > pow(10, -15.0f/20.0f));
      QVERIFY(data[70 * 2] > pow(10, -15.0f/20.0f)); // it should not stop make sure it is loud enough!
      QVERIFY(data[70 * 2 + 1] > pow(10, -15.0f/20.0f));
      QVERIFY(data[110 * 2] < pow(10, -20.0f/20.0f)); // it should play zeros after leaving sustain -> drastic volume reduce
      QVERIFY(data[110 * 2 + 1] < pow(10, -20.0f/20.0f));
      QVERIFY(data[120 * 2] < pow(10, -85.0f/20.0f));
      QVERIFY(data[120 * 2 + 1] < pow(10, -85.0f/20.0f));

      }

TestSfzLoop::~TestSfzLoop()
      {
      delete synth;
      }

QTEST_MAIN(TestSfzLoop)

#include "tst_sfzloop.moc"


