
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
#include "synthesizer/event.h"
#include <sndfile.h>

using namespace Ms;

//---------------------------------------------------------
//   TestSfzEnvelopes
//---------------------------------------------------------

class TestSfzEnvelopes : public QObject, public MTest
      {
      Q_OBJECT
      float samplerate = 44100;
      Zerberus* synth;

   private slots:
      void initTestCase();
      void testEnvelopesParsing();
      void testEnvelopesAudio();
   public:
      ~TestSfzEnvelopes();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestSfzEnvelopes::initTestCase()
      {
      initMTest();
      synth = new Zerberus();
      synth->init(samplerate);
      Ms::preferences.mySoundfontsPath += ";" + root;
      synth->loadInstrument("envelopesTest.sfz");
      }

//---------------------------------------------------------
//   testglobal
//---------------------------------------------------------

void TestSfzEnvelopes::testEnvelopesParsing()
      {
      QCOMPARE(synth->instrument(0)->zones().size(), (unsigned long) 1);
      QCOMPARE(synth->instrument(0)->zones().front()->ampegDelay, 0.01f * 1000.0f);
      QCOMPARE(synth->instrument(0)->zones().front()->ampegStart, 20.0f / 100.0f); // 20 percent
      QCOMPARE(synth->instrument(0)->zones().front()->ampegAttack, 0.01f * 1000.0f);
      QCOMPARE(synth->instrument(0)->zones().front()->ampegHold, 0.01f * 1000.0f);
      QCOMPARE(synth->instrument(0)->zones().front()->ampegDecay, 0.01f * 1000.0f);
      QCOMPARE(synth->instrument(0)->zones().front()->ampegSustain, 50.0f / 100.0f); // 50 percent
      QCOMPARE(synth->instrument(0)->zones().front()->ampegRelease, 0.01f * 1000.0f);
      }

void TestSfzEnvelopes::testEnvelopesAudio()
      {
      synth->play(Ms::PlayEvent(ME_PROGRAM, 0, 0, 0));
      float data[6 * 441 * 2]; // 6 envelope stages with duration of 441 Samples and 2 Channels
      memset(data, 0, sizeof(data));
      synth->play(Ms::PlayEvent(ME_NOTEON, 0, 60, 127)); // play a note
      synth->process(5*441 , data, nullptr, nullptr); // process until end of sustain stage
      synth->play(Ms::PlayEvent(ME_NOTEON, 0, 60, 0)); // send note off
      synth->process(441 , data + (5*441*2), nullptr, nullptr); // process until end of sustain stage

      /* To generate wav file
      SF_INFO sf_info;
      sf_info.channels = 2;
      sf_info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
      sf_info.frames = 6 *441;
      sf_info.samplerate = samplerate;
      SNDFILE* sf = sf_open("result.wav", SFM_WRITE, &sf_info);
      sf_writef_float(sf, data, 441*6);
      sf_close(sf); */

      // read wav file
      QString filename = root + "/zerberus/envelopes/result.wav";
      SF_INFO sf_info;
      SNDFILE *sf = sf_open(filename.toLocal8Bit().constData(), SFM_READ, &sf_info);
      float compare_data[6 * 441 * 2];
      sf_readf_float(sf, compare_data, 6 * 441);
      sf_close(sf);

      for (int i = 0; i < 6 * 441 * 2; i++)
            QCOMPARE(data[i], compare_data[i]);

      }

TestSfzEnvelopes::~TestSfzEnvelopes()
      {
      delete synth;
      }

QTEST_MAIN(TestSfzEnvelopes)

#include "tst_sfzenvelopes.moc"


