
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
#include "zerberus/channel.h"
#include "zerberus/zone.h"
#include "mscore/preferences.h"
#include "synthesizer/event.h"

using namespace Ms;

//---------------------------------------------------------
//   TestSfzGlobal
//---------------------------------------------------------

class TestSfzInputControls : public QObject, public MTest
      {
      Q_OBJECT

      Zerberus* synth;
      Channel* channel;
      std::list<Zone *>::iterator zoneIterator;

   private slots:
      void initTestCase();
      void testInputKeys();
      void testInputVolume();
      void testInputCCFilter();
      void testInputRand();
      void testInputSeq();
      void testInputTrigger();
      void testInputGroupOffBy();
      void testInputCCTrigger();
      void testInputRtDecay();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestSfzInputControls::initTestCase()
      {
      initMTest();
      synth = new Zerberus();
      Ms::preferences.mySoundfontsPath += ";" + root;
      synth->init(44100.0f);
      synth->loadInstrument("inputControls.sfz");
      channel = synth->channel(0);
      zoneIterator = synth->instrument(0)->zones().begin();
      }

void TestSfzInputControls::testInputKeys()
      {
      QCOMPARE(false, (*zoneIterator)->match(channel, 18, 127, Trigger::ATTACK, 0.0f, 0, 0));
      QCOMPARE(true, (*zoneIterator)->match(channel, 19, 127, Trigger::ATTACK, 0.0f, 0, 0));
      QCOMPARE(false, (*zoneIterator)->match(channel, 20, 127, Trigger::ATTACK, 0.0f, 0, 0));
      zoneIterator++;

      QCOMPARE(false, (*zoneIterator)->match(channel, 19, 127, Trigger::ATTACK, 0.0f, 0, 0));
      for (int i = 20; i <= 30; i++)
            QCOMPARE(true, (*zoneIterator)->match(channel,i, 127, Trigger::ATTACK, 0.0f, 0, 0));
      QCOMPARE(false, (*zoneIterator)->match(channel, 31, 127, Trigger::ATTACK, 0.0f, 0, 0));
      zoneIterator++;
      }

void TestSfzInputControls::testInputVolume()
      {
      QCOMPARE(false, (*zoneIterator)->match(channel, 40, 29, Trigger::ATTACK, 0.0f, 0, 0));
      for (int i = 30; i <= 40; i++)
            QCOMPARE(true, (*zoneIterator)->match(channel,40, i, Trigger::ATTACK, 0.0f, 0, 0));
      QCOMPARE(false, (*zoneIterator)->match(channel, 40, 41, Trigger::ATTACK, 0.0f, 0, 0));
      zoneIterator++;
      }

void TestSfzInputControls::testInputCCFilter()
      {
      QCOMPARE(false, (*zoneIterator)->match(channel, 41, 127, Trigger::ATTACK, 0.0f, 0, 0));
      for (int i = 10; i <= 15; i++) {
            channel->controller(23, i);
            QCOMPARE(true, (*zoneIterator)->match(channel, 41, 127, Trigger::ATTACK, 0.0f, 0, 0));
            }
      channel->controller(23, 16);
      QCOMPARE(false, (*zoneIterator)->match(channel, 41, 127, Trigger::ATTACK, 0.0f, 0, 0));
      zoneIterator++;
      }

void TestSfzInputControls::testInputRand()
      {
      QCOMPARE(false, (*zoneIterator)->match(channel, 42, 127, Trigger::ATTACK, 0.0f, 0, 0));
      QCOMPARE(true, (*zoneIterator)->match(channel, 42, 127, Trigger::ATTACK, 0.5f, 0, 0));
      QCOMPARE(true, (*zoneIterator)->match(channel, 42, 127, Trigger::ATTACK, 0.6f, 0, 0));
      QCOMPARE(true, (*zoneIterator)->match(channel, 42, 127, Trigger::ATTACK, 0.74f, 0, 0));
      QCOMPARE(false, (*zoneIterator)->match(channel, 42, 127, Trigger::ATTACK, 0.75f, 0, 0));
      zoneIterator++;
      }

void TestSfzInputControls::testInputSeq()
      {
      std::list<Zone *>::iterator beforeIterator = zoneIterator;
      zoneIterator++;
      QCOMPARE(true, (*beforeIterator)->match(channel, 43, 127, Trigger::ATTACK, 0.0f, 0, 0));
      QCOMPARE(false, (*zoneIterator)->match(channel, 43, 127, Trigger::ATTACK, 0.0f, 0, 0));
      QCOMPARE(false, (*beforeIterator)->match(channel, 43, 127, Trigger::ATTACK, 0.0f, 0, 0));
      QCOMPARE(true, (*zoneIterator)->match(channel, 43, 127, Trigger::ATTACK, 0.0f, 0, 0));
      zoneIterator++;
      }

void TestSfzInputControls::testInputTrigger()
      {
      QCOMPARE(true, (*zoneIterator)->match(channel, 44, 127, Trigger::ATTACK, 0.0f, 0, 0));
      QCOMPARE(false, (*zoneIterator)->match(channel, 44, 127, Trigger::RELEASE, 0.0f, 0, 0));
      QCOMPARE(false, (*zoneIterator)->match(channel, 44, 127, Trigger::FIRST, 0.0f, 0, 0));
      QCOMPARE(false, (*zoneIterator)->match(channel, 44, 127, Trigger::LEGATO, 0.0f, 0, 0));

      zoneIterator++;
      QCOMPARE(false, (*zoneIterator)->match(channel, 44, 127, Trigger::ATTACK, 0.0f, 0, 0));
      QCOMPARE(true, (*zoneIterator)->match(channel, 44, 127, Trigger::RELEASE, 0.0f, 0, 0));
      QCOMPARE(false, (*zoneIterator)->match(channel, 44, 127, Trigger::FIRST, 0.0f, 0, 0));
      QCOMPARE(false, (*zoneIterator)->match(channel, 44, 127, Trigger::LEGATO, 0.0f, 0, 0));

      zoneIterator++;
      QCOMPARE(false, (*zoneIterator)->match(channel, 44, 127, Trigger::ATTACK, 0.0f, 0, 0));
      QCOMPARE(false, (*zoneIterator)->match(channel, 44, 127, Trigger::RELEASE, 0.0f, 0, 0));
      QCOMPARE(true, (*zoneIterator)->match(channel, 44, 127, Trigger::FIRST, 0.0f, 0, 0));
      QCOMPARE(false, (*zoneIterator)->match(channel, 44, 127, Trigger::LEGATO, 0.0f, 0, 0));

      zoneIterator++;
      QCOMPARE(false, (*zoneIterator)->match(channel, 44, 127, Trigger::ATTACK, 0.0f, 0, 0));
      QCOMPARE(false, (*zoneIterator)->match(channel, 44, 127, Trigger::RELEASE, 0.0f, 0, 0));
      QCOMPARE(false, (*zoneIterator)->match(channel, 44, 127, Trigger::FIRST, 0.0f, 0, 0));
      QCOMPARE(true, (*zoneIterator)->match(channel, 44, 127, Trigger::LEGATO, 0.0f, 0, 0));
      zoneIterator++;
      }


void TestSfzInputControls::testInputGroupOffBy()
      {
      synth->play(Ms::PlayEvent(ME_NOTEON, 0, 45, 127));
      QCOMPARE(true, synth->getActiveVoices() != 0);
      QCOMPARE(true, synth->getActiveVoices()->isPlaying());
      QCOMPARE(true, synth->getActiveVoices()->next() != 0);
      QCOMPARE(true, synth->getActiveVoices()->next()->isPlaying());
      QCOMPARE(true, synth->getActiveVoices()->next()->next() == 0);

      synth->play(Ms::PlayEvent(ME_NOTEON, 0, 46, 127));
      // in our list the newest voice is at the beginning
      QCOMPARE(true, synth->getActiveVoices()->isPlaying());
      QCOMPARE(46, synth->getActiveVoices()->key());
      QCOMPARE(true, synth->getActiveVoices()->next()->isStopped());
      QCOMPARE(45, synth->getActiveVoices()->next()->key());
      QCOMPARE(true, synth->getActiveVoices()->next()->next() != 0);
      QCOMPARE(true, synth->getActiveVoices()->next()->next()->isStopped());
      QCOMPARE(45, synth->getActiveVoices()->next()->next()->key());

      zoneIterator++;
      zoneIterator++;
      zoneIterator++;
      }

void TestSfzInputControls::testInputCCTrigger()
      {
      QCOMPARE(false, (*zoneIterator)->match(channel, -1, -1, Trigger::ATTACK, 0.0f, 23, 60));
      QCOMPARE(true, (*zoneIterator)->match(channel, -1, -1, Trigger::CC, 0.0f, 23, 60));
      QCOMPARE(false, (*zoneIterator)->match(channel, -1, -1, Trigger::CC, 0.0f, 23, 59));
      for (int i = 60; i < 65; i++) {
            QCOMPARE(true, (*zoneIterator)->match(channel, -1, -1, Trigger::CC, 0.0f, 23, i));
            }
      QCOMPARE(false, (*zoneIterator)->match(channel, -1, -1, Trigger::CC, 0.0f, 23, 66));
      zoneIterator++;
      }

void TestSfzInputControls::testInputRtDecay()
      { 
      float buffer[44100 * 2];
      synth->play(Ms::PlayEvent(ME_NOTEON, 0, 50, 127));
      synth->process(44100, buffer, nullptr, nullptr);
      synth->play(Ms::PlayEvent(ME_NOTEOFF, 0, 50, 0));
      double rt_decay_value = pow(10, -6 * (double) 1.0f / 20);
      float gain = (*zoneIterator)->volume * 100 * .005 * channel->gain() * rt_decay_value;
      QCOMPARE(synth->getActiveVoices()->getGain(), gain);
      }

QTEST_MAIN(TestSfzInputControls)

#include "tst_sfzinputcontrols.moc"


