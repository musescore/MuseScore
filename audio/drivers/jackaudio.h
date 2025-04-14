//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2009 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __JACKAUDIO_H__
#define __JACKAUDIO_H__

#include <jack/jack.h>

#include "driver.h"

namespace Ms {

class Synth;
class Seq;
class MidiDriver;

//---------------------------------------------------------
//   JackAudio
//---------------------------------------------------------

class JackAudio : public Driver {
      unsigned _segmentSize;
                              // We use fakeState if preferences.useJackTransport = false to emulate JACK Transport.
                              // ALSA, PortAudio do the same thing because of they don't have any transport.
                              // Also this implements fake transport when we have to temporarily disconnect from JACK Transport.
                              // It may be useful when playing count in to let JACK Transport wait before playing score.
      Transport fakeState;

      jack_client_t* client;
      char _jackName[8];

      bool timeSigTempoChanged;
      QList<jack_port_t*> ports;
      QList<jack_port_t*> midiOutputPorts;
      QList<jack_port_t*> midiInputPorts;

      static int processAudio(jack_nframes_t, void*);
      static void timebase (jack_transport_state_t, jack_nframes_t, jack_position_t*, int, void *);
      void hotPlug();
      void setTimebaseCallback();
      void releaseTimebaseCallback();
      void rememberAudioConnections();
      void restoreAudioConnections();
      void rememberMidiConnections();
      void restoreMidiConnections();
      QList<QString> inputPorts();
   public:
      JackAudio(Seq*);
      virtual ~JackAudio();
      virtual bool init(bool hot = false) override;
      virtual bool start(bool hotPlug = false) override;
      virtual bool stop() override;
      int framePos() const;
      void connect(void*, void*);
      void connect(const char* src, const char* dst);
      void disconnect(void* src, void* dst);
      virtual bool isRealtime() const   { return jack_is_realtime(client); }
      virtual void startTransport() override;
      virtual void stopTransport() override;
      virtual Transport getState() override;
      virtual void seekTransport(int) override;
      virtual int sampleRate() const override { return jack_get_sample_rate(client); }
      virtual void putEvent(const NPlayEvent&, unsigned framePos) override;
      virtual void midiRead() override;

      virtual void registerPort(const QString& name, bool input, bool midi);
      virtual void unregisterPort(jack_port_t*);
      virtual void handleTimeSigTempoChanged() override;
      virtual void checkTransportSeek(int, int, bool) override;
      virtual int bufferSize() override {return _segmentSize;}
      void setBufferSize(int nframes) { _segmentSize = nframes;}
      void updateOutPortCount(int) override;
      };


} // namespace Ms
#endif

