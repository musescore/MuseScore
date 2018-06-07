
//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
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

using namespace Ms;

//---------------------------------------------------------
//   TestOpcodes
//---------------------------------------------------------

class TestOpcodes : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void testOpcodes();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestOpcodes::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   testglobal
//---------------------------------------------------------

void TestOpcodes::testOpcodes()
      {
      Zerberus* synth = new Zerberus();
      preferences.setPreference(PREF_APP_PATHS_MYSOUNDFONTS, root);
      synth->loadInstrument("opcodeTest.sfz");

      std::list<Zone *>::iterator curZone = synth->instrument(0)->zones().begin();

      QCOMPARE(synth->instrument(0)->zones().size(), (size_t) 12);
      QCOMPARE((*curZone)->keyLo, (char) 60);
      QCOMPARE((*curZone)->keyHi, (char) 70);
      QCOMPARE((*curZone)->keyBase, (char) 40);
      QCOMPARE((*curZone)->veloLo, (char) 20);
      QCOMPARE((*curZone)->veloHi, (char) 40);
      QCOMPARE((*curZone)->locc[34], 40);
      QCOMPARE((*curZone)->hicc[34], 50);
      QCOMPARE((*curZone)->loRand, (double) 0.5f);
      QCOMPARE((*curZone)->hiRand, (double) 0.75f);
      QCOMPARE((*curZone)->seqLen, 2);
      QCOMPARE((*curZone)->seqPos, 2);
      QCOMPARE((*curZone)->group, 23);
      QCOMPARE((*curZone)->offBy, 42);
      QCOMPARE((*curZone)->offMode, OffMode::FAST);
      QCOMPARE((*curZone)->onLocc[35], 40);
      QCOMPARE((*curZone)->onHicc[35], 50);
      QCOMPARE((*curZone)->loopStart, 50);
      QCOMPARE((*curZone)->loopEnd, 100);
      QCOMPARE((*curZone)->loopMode, LoopMode::SUSTAIN);
      QCOMPARE((*curZone)->tune, 5*100 + 8);
      QCOMPARE((*curZone)->pitchKeytrack, 50.0f/100.0f);
      QCOMPARE((*curZone)->volume, (float) pow((float) 10.0f, (double) -6.0f/ (float) 20.0f));
      QCOMPARE((*curZone)->ampVeltrack, 50.0f);
      QCOMPARE((*curZone)->rtDecay, 10.0f);
      QCOMPARE((*curZone)->ampegDelay, 1.0f * 1000.0f);
      QCOMPARE((*curZone)->ampegStart, 20.0f / 100.0f);
      QCOMPARE((*curZone)->ampegAttack, 1.5f * 1000.0f);
      QCOMPARE((*curZone)->ampegHold, 2.0f * 1000.0f);
      QCOMPARE((*curZone)->ampegDecay, 2.5f * 1000.0f);
      QCOMPARE((*curZone)->ampegSustain, 50.0f / 100.0f);
      QCOMPARE((*curZone)->ampegRelease, 3.0f * 1000.0f);
      QCOMPARE((*curZone)->ampegVel2Delay  , 23.0f * 1000.0f);
      QCOMPARE((*curZone)->ampegVel2Attack , 24.0f * 1000.0f);
      QCOMPARE((*curZone)->ampegVel2Hold   , 25.0f * 1000.0f);
      QCOMPARE((*curZone)->ampegVel2Decay  , 26.0f * 1000.0f);
      QCOMPARE((*curZone)->ampegVel2Sustain, 27.0f / 100.0f);
      QCOMPARE((*curZone)->ampegVel2Release, 28.0f * 1000.0f);
      QCOMPARE((*curZone)->trigger, Trigger::CC);
      curZone++;
      QCOMPARE((*curZone)->keyLo, (char) 40);
      QCOMPARE((*curZone)->keyHi, (char) 40);
      QCOMPARE((*curZone)->keyBase, (char) 40);
      QCOMPARE((*curZone)->offMode, OffMode::NORMAL);
      QCOMPARE((*curZone)->loopMode, LoopMode::NO_LOOP);
      curZone++;
      QCOMPARE((*curZone)->loopMode, LoopMode::ONE_SHOT);
      QCOMPARE((*curZone)->trigger, Trigger::RELEASE);
      curZone++;
      QCOMPARE((*curZone)->loopMode, LoopMode::CONTINUOUS);
      QCOMPARE((*curZone)->trigger, Trigger::ATTACK);
      curZone++;
      QCOMPARE((*curZone)->trigger, Trigger::FIRST);
      curZone++;
      QCOMPARE((*curZone)->trigger, Trigger::LEGATO);
      curZone++;
      QCOMPARE((*curZone)->fil_type, FilterType::lpf_1p);
      QCOMPARE((*curZone)->isCutoffDefined, true);
      QCOMPARE((*curZone)->cutoff, 200.f);
      QCOMPARE((*curZone)->fil_veltrack, 4000);
      curZone++;
      QCOMPARE((*curZone)->fil_type, FilterType::hpf_1p);
      QCOMPARE((*curZone)->fil_keycenter, 60);
      QCOMPARE((*curZone)->fil_keytrack, 400);
      curZone++;
      QCOMPARE((*curZone)->fil_type, FilterType::hpf_2p);
      QCOMPARE((*curZone)->pan, 100);
      QCOMPARE((*curZone)->offset, 16178ll);
      curZone++;
      QCOMPARE((*curZone)->fil_type, FilterType::lpf_2p);
      QCOMPARE((*curZone)->delay, 2000.f); //ms
      curZone++;
      QCOMPARE((*curZone)->fil_type, FilterType::bpf_2p);
      curZone++;
      QCOMPARE((*curZone)->fil_type, FilterType::brf_2p);
      delete synth;
      }

QTEST_MAIN(TestOpcodes)

#include "tst_sfzopcodes.moc"


