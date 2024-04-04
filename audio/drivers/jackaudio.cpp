//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#if (defined (_MSCVER) || defined (_MSC_VER))
// Include stdint.h and #define _STDINT_H to prevent <systemdeps.h> from redefining types
// #undef UNICODE to force LoadLibrary to use the char-based implementation instead of the wchar_t one.
#include <stdint.h>
#define _STDINT_H 1
#endif

#include "jackaudio.h"

#include <jack/midiport.h>

#include "libmscore/mscore.h"
#include "libmscore/sig.h"
#include "libmscore/score.h"
#include "libmscore/repeatlist.h"

#include "mscore/musescore.h"
#include "mscore/preferences.h"
#include "mscore/seq.h"
#include "mscore/iplaypanel.h"

// Prevent killing sequencer with wrong data
#define less128(__less) ((__less >=0 && __less <= 127) ? __less : 0)

namespace Ms {

//---------------------------------------------------------
//   JackAudio
//---------------------------------------------------------

JackAudio::JackAudio(Seq* s)
   : Driver(s)
      {
      client = 0;
      }

//---------------------------------------------------------
//   ~JackAudio
//---------------------------------------------------------

JackAudio::~JackAudio()
      {
      if (client) {
            stop();
            if (jack_client_close(client)) {
                  qDebug("jack_client_close() failed: %s",
                     strerror(errno));
                  }
            }
      }

//---------------------------------------------------------
//   updateOutPortCount
//   Add/remove JACK MIDI Out ports
//---------------------------------------------------------

void JackAudio::updateOutPortCount(int maxport)
      {
      if (!preferences.getBool(PREF_IO_JACK_USEJACKMIDI) || maxport == midiOutputPorts.size())
            return;
      if (MScore::debugMode)
            qDebug()<<"JACK number of ports:"<<midiOutputPorts.size()<<", change to:"<<maxport;

      bool oldremember = preferences.getBool(PREF_IO_JACK_REMEMBERLASTCONNECTIONS);
      preferences.setPreference(PREF_IO_JACK_REMEMBERLASTCONNECTIONS, true);

      if (maxport > midiOutputPorts.size()) {
            for (int i = midiOutputPorts.size(); i < maxport; ++i)
                  registerPort(QString("mscore-midi-%1").arg(i+1), false, true);
            restoreMidiConnections();
            }
      else if (maxport < midiOutputPorts.size()) {
            rememberMidiConnections();
            for(int i = midiOutputPorts.size() - 1; i >= maxport; --i) {
                  unregisterPort(midiOutputPorts[i]);
                  midiOutputPorts.removeAt(i);
                  }
            }
      preferences.setPreference(PREF_IO_JACK_REMEMBERLASTCONNECTIONS, oldremember);

      }

//---------------------------------------------------------
//   registerPort
//---------------------------------------------------------

void JackAudio::registerPort(const QString& name, bool input, bool midi)
      {
      int portFlag         = input ? JackPortIsInput : JackPortIsOutput;
      const char* portType = midi ? JACK_DEFAULT_MIDI_TYPE : JACK_DEFAULT_AUDIO_TYPE;

      jack_port_t* port = jack_port_register(client, qPrintable(name), portType, portFlag, 0);
      if (port == 0) {
            qDebug("JackAudio:registerPort(%s) failed", qPrintable(name));
            return;
            }
      if (midi) {
            if (input)
                  midiInputPorts.append(port);
            else
                  midiOutputPorts.append(port);
            }
      else
            ports.append(port);
      }

//---------------------------------------------------------
//   unregisterPort
//---------------------------------------------------------

void JackAudio::unregisterPort(jack_port_t* port)
      {
      if (jack_port_is_mine(client,port)) {
            jack_port_unregister(client, port);
            port = 0;
            }
      else
            qDebug("Trying to unregister port that is not my!");
      }

//---------------------------------------------------------
//   inputPorts
//---------------------------------------------------------

QList<QString> JackAudio::inputPorts()
      {
      const char** prts = jack_get_ports(client, 0, 0, 0);
      QList<QString> clientList;
      for (const char** p = prts; p && *p; ++p) {
            jack_port_t* port = jack_port_by_name(client, *p);
            int flags = jack_port_flags(port);
            if (!(flags & JackPortIsInput))
                  continue;
            char buffer[128];
            strncpy(buffer, *p, sizeof(buffer) - 1);
            buffer[sizeof(buffer) - 1] = 0;
            if (strncmp(buffer, "Mscore", 6) == 0)
                  continue;
            clientList.append(QString(buffer));
            }
      return clientList;
      }

//---------------------------------------------------------
//   connect
//---------------------------------------------------------

void JackAudio::connect(void* src, void* dst)
      {
      const char* sn = jack_port_name((jack_port_t*) src);
      const char* dn = jack_port_name((jack_port_t*) dst);

      if (sn == 0 || dn == 0) {
            qDebug("JackAudio::connect: unknown jack ports");
            return;
            }
      if (jack_connect(client, sn, dn)) {
            qDebug("jack connect <%s>%p - <%s>%p failed",
               sn, src, dn, dst);
            }
      }

//---------------------------------------------------------
//   connect
//---------------------------------------------------------

void JackAudio::connect(const char* src, const char* dst)
      {
      if (src == 0 || dst == 0) {
            qDebug("JackAudio::connect: unknown jack ports");
            return;
            }
      qDebug("JackAudio::connect <%s> <%s>", src, dst);
      int rv = jack_connect(client, src, dst);
      if (rv)
            qDebug("jack connect port <%s> - <%s> failed: %d", src, dst, rv);
      }

//---------------------------------------------------------
//   disconnect
//---------------------------------------------------------

void JackAudio::disconnect(void* src, void* dst)
      {
      const char* sn = jack_port_name((jack_port_t*) src);
      const char* dn = jack_port_name((jack_port_t*) dst);
      if (sn == 0 || dn == 0) {
            qDebug("JackAudio::disconnect: unknown jack ports");
            return;
            }
      if (jack_disconnect(client, sn, dn)) {
            qDebug("jack disconnect <%s> - <%s> failed", sn, dn);
            }
      }

//---------------------------------------------------------
//   start
//    return false on error
//---------------------------------------------------------

bool JackAudio::start(bool hotPlug)
      {
      bool oldremember = preferences.getBool(PREF_IO_JACK_REMEMBERLASTCONNECTIONS);
      if (hotPlug)
            preferences.setPreference(PREF_IO_JACK_REMEMBERLASTCONNECTIONS, true);

      if (jack_activate(client)) {
            qDebug("JACK: cannot activate client");
            return false;
            }
      /* connect the ports. Note: you can't do this before
         the client is activated, because we can't allow
         connections to be made to clients that aren't
         running.
       */
      if (preferences.getBool(PREF_IO_JACK_USEJACKAUDIO))
            restoreAudioConnections();
      if (preferences.getBool(PREF_IO_JACK_USEJACKMIDI))
            restoreMidiConnections();

      if (hotPlug)
            preferences.setPreference(PREF_IO_JACK_REMEMBERLASTCONNECTIONS, oldremember);
      return true;
      }

//---------------------------------------------------------
//   stop
//    return false on error
//---------------------------------------------------------

bool JackAudio::stop()
      {
      if (preferences.getBool(PREF_IO_JACK_USEJACKMIDI))
            rememberMidiConnections();
      if (preferences.getBool(PREF_IO_JACK_USEJACKAUDIO))
            rememberAudioConnections();

      if (jack_deactivate(client)) {
            qDebug("cannot deactivate client");
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   framePos
//---------------------------------------------------------

int JackAudio::framePos() const
      {
      jack_nframes_t n = jack_frame_time(client);
      return (int)n;
      }

//---------------------------------------------------------
//   freewheel_callback
//---------------------------------------------------------

static void freewheel_callback(int /*starting*/, void*)
      {
      }

//---------------------------------------------------------
//   sampleRateCallback
//---------------------------------------------------------

int sampleRateCallback(jack_nframes_t sampleRate, void*)
      {
      qDebug("JACK: sample rate changed: %d", sampleRate);
      MScore::sampleRate = sampleRate;
      return 0;
      }

//---------------------------------------------------------
//   bufferSizeCallback called if JACK buffer changed
//---------------------------------------------------------

int bufferSizeCallback(jack_nframes_t nframes, void *arg)
      {
      JackAudio* audio = (JackAudio*)arg;
      audio->setBufferSize(nframes);
      return 0;
      }

static void registration_callback(jack_port_id_t, int, void*)
      {
//      qDebug("JACK: registration changed");
      }

static int graph_callback(void*)
      {
//      qDebug("JACK: graph changed");
      return 0;
      }

//---------------------------------------------------------
//   timebase
//---------------------------------------------------------

void JackAudio::timebase(jack_transport_state_t state, jack_nframes_t /*nframes*/, jack_position_t *pos, int /*new_pos*/, void *arg)
      {
      JackAudio* audio = (JackAudio*)arg;
      if (!audio->seq->score()) {
            if (state==JackTransportLooping || state==JackTransportRolling)
                  audio->stopTransport();
            }
      else if (audio->seq->isRunning()) {
            if (!audio->seq->score()->masterScore())
                  return;

            pos->valid = JackPositionBBT;
            int curTick = audio->seq->score()->repeatList().utick2tick(audio->seq->getCurTick());
            int bar,beat,tick;
            audio->seq->score()->sigmap()->tickValues(curTick, &bar, &beat, &tick);
            // Providing the final tempo
            pos->beats_per_minute = 60 * audio->seq->curTempo() * audio->seq->score()->tempomap()->relTempo();
            pos->ticks_per_beat   = MScore::division;
            pos->tick             = tick;
            pos->bar              = bar+1;
            pos->beat             = beat+1;

            if (audio->timeSigTempoChanged) {
                  Fraction timeSig = audio->seq->score()->sigmap()->timesig(curTick).nominal();
                  pos->beats_per_bar =  timeSig.numerator();
                  pos->beat_type = timeSig.denominator();
                  audio->timeSigTempoChanged = false;
                  qDebug()<<"Time signature changed: "<< pos->beats_per_minute<<", bar: "<< pos->bar<<",beat: "<<pos->beat<<", tick:"<<pos->tick<<", time sig: "<<pos->beats_per_bar<<"/"<<pos->beat_type;
                  }
            }
      // TODO: Handle new_pos
      }
//---------------------------------------------------------
//   processAudio
//    JACK callback
//---------------------------------------------------------

int JackAudio::processAudio(jack_nframes_t frames, void* p)
      {
      JackAudio* audio = (JackAudio*)p;
      // Prevent from crash if score not opened yet
      if(!audio->seq->score())
            return 0;

      float* l;
      float* r;
      if (preferences.getBool(PREF_IO_JACK_USEJACKAUDIO) && audio->ports.size() == 2) {
            l = (float*)jack_port_get_buffer(audio->ports[0], frames);
            r = (float*)jack_port_get_buffer(audio->ports[1], frames);
            }
      else {
            l = 0;
            r = 0;
            }
      if (preferences.getBool(PREF_IO_JACK_USEJACKMIDI)) {
            foreach(jack_port_t* port, audio->midiOutputPorts) {
                  void* portBuffer = jack_port_get_buffer(port, frames);
                  jack_midi_clear_buffer(portBuffer);
                  }
            foreach(jack_port_t* port, audio->midiInputPorts) {
                  void* portBuffer = jack_port_get_buffer(port, frames);
                  if (portBuffer) {
                        jack_nframes_t n = jack_midi_get_event_count(portBuffer);
                        for (jack_nframes_t i = 0; i < n; ++i) {
                              jack_midi_event_t event;
                              if (jack_midi_event_get(&event, portBuffer, i) != 0)
                                    continue;
                              size_t nn = event.size;
                              int type = event.buffer[0];
                              if (nn && (type == ME_CLOCK || type == ME_SENSE))
                                    continue;
                              Event e;
                              e.setChannel(type & 0xf);
                              type &= 0xf0;
                              e.setType(type);
                              if (type == ME_NOTEON || type == ME_NOTEOFF) {
                                    e.setPitch(event.buffer[1]);
                                    e.setVelo(event.buffer[2]);
                                    audio->seq->eventToGui(e);
                                    }
                              else if (type == ME_CONTROLLER) {
                                    e.setController(event.buffer[1]);
                                    e.setValue(event.buffer[2]);
                                    audio->seq->eventToGui(e);
                                    }
                              }
                        }
                  }
            }
      if (l && r) {
#if (!defined (_MSCVER) && !defined (_MSC_VER))
         float buffer[frames * 2];
#else
         // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
         //    heap allocation is slow, an optimization might be used.
         std::vector<float> vBuffer(frames * 2);
         float* buffer = vBuffer.data();
#endif
            audio->seq->process((unsigned)frames, buffer);
            float* sp = buffer;
            for (unsigned i = 0; i < frames; ++i) {
                  *l++ = *sp++;
                  *r++ = *sp++;
                  }
            }
      else {
            // JACK MIDI only
#if (!defined (_MSCVER) && !defined (_MSC_VER))
         float buffer[frames * 2];
#else
            // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
            //    heap allocation is slow, an optimization might be used.
         std::vector<float> vBuffer(frames * 2);
         float* buffer = vBuffer.data();
#endif
            audio->seq->process((unsigned)frames, buffer);
            }
      return 0;
      }

//---------------------------------------------------------
//   jackError
//---------------------------------------------------------

static void jackError(const char *s)
      {
      qDebug("JACK ERROR: %s", s);
      }

//---------------------------------------------------------
//   noJackError
//---------------------------------------------------------

static void noJackError(const char*  s )
      {
      qDebug("noJACK ERROR: %s", s);
      }

//---------------------------------------------------------
//   init
//    return false on error
//---------------------------------------------------------

bool JackAudio::init(bool hot)
      {
      if (hot) {
            hotPlug();
            return true;
            }
      jack_set_error_function(noJackError);

      client = 0;
      timeSigTempoChanged = false;
      fakeState = Transport::STOP;
      strcpy(_jackName, "mscore");

      jack_options_t options = (jack_options_t)0;
      jack_status_t status;
      client = jack_client_open(_jackName, options, &status);

      if (client == 0) {
            qDebug("JackAudio()::init(): failed, status 0x%0x", status);
            return false;
            }

      jack_set_error_function(jackError);
      jack_set_process_callback(client, processAudio, this);
      //jack_on_shutdown(client, processShutdown, this);
      jack_set_sample_rate_callback(client, sampleRateCallback, this);
      jack_set_port_registration_callback(client, registration_callback, this);
      jack_set_graph_order_callback(client, graph_callback, this);
      jack_set_freewheel_callback (client, freewheel_callback, this);
      if (preferences.getBool(PREF_IO_JACK_TIMEBASEMASTER))
            setTimebaseCallback();
      if (jack_set_buffer_size_callback (client, bufferSizeCallback, this) != 0)
            qDebug("Can not set bufferSizeCallback");
      _segmentSize  = jack_get_buffer_size(client);

      MScore::sampleRate = sampleRate();
      // register mscore left/right output ports
      if (preferences.getBool(PREF_IO_JACK_USEJACKAUDIO)) {
            registerPort("left", false, false);
            registerPort("right", false, false);
            }

      if (preferences.getBool(PREF_IO_JACK_USEJACKMIDI)) {
            registerPort(QString("mscore-midi-1"), false, true);
            registerPort(QString("mscore-midiin-1"), true, true);
            }
      return true;
      }

//---------------------------------------------------------
//   startTransport
//---------------------------------------------------------

void JackAudio::startTransport()
      {
      if (preferences.getBool(PREF_IO_JACK_USEJACKTRANSPORT))
            jack_transport_start(client);
      else
            fakeState = Transport::PLAY;
      }

//---------------------------------------------------------
//   stopTrasnport
//---------------------------------------------------------

void JackAudio::stopTransport()
      {
      if (preferences.getBool(PREF_IO_JACK_USEJACKTRANSPORT))
            jack_transport_stop(client);
      else
            fakeState = Transport::STOP;
      }

//---------------------------------------------------------
//   getState
//---------------------------------------------------------

Transport JackAudio::getState()
      {
      if (!preferences.getBool(PREF_IO_JACK_USEJACKTRANSPORT))
            return fakeState;
      int transportState = jack_transport_query(client, NULL);
      switch (transportState) {
            case JackTransportStopped:  return Transport::STOP;
            case JackTransportLooping:
            case JackTransportRolling:  return Transport::PLAY;
            case JackTransportStarting: return seq->isPlaying()?Transport::PLAY:Transport::STOP;// Keep current state
            default:
                  return Transport::STOP;
            }
      }

//---------------------------------------------------------
//   putEvent
//---------------------------------------------------------

void JackAudio::putEvent(const NPlayEvent& e, unsigned framePos)
      {
      if (!preferences.getBool(PREF_IO_JACK_USEJACKMIDI))
            return;

      int portIdx = seq->score()->midiPort(e.channel());
      int chan    = seq->score()->midiChannel(e.channel());

// qDebug("JackAudio::putEvent %d:%d  pos %d(%d)", portIdx, chan, framePos, _segmentSize);

      if (portIdx < 0 || portIdx >= midiOutputPorts.size()) {
            qDebug("JackAudio::putEvent: invalid port %d", portIdx);
            return;
            }
      jack_port_t* port = midiOutputPorts[portIdx];
      if (midiOutputTrace) {
            const char* portName = jack_port_name(port);
            int a     = e.dataA();
            int b     = e.dataB();
            qDebug("MidiOut<%s>: jackMidi: %02x %02x %02x, chan: %i", portName, e.type(), a, b, chan);
            // e.dump();
            }
      void* pb = jack_port_get_buffer(port, _segmentSize);

      if (pb == NULL) {
            qDebug()<<"jack_port_get_buffer failed, cannot send anything";
            }

      if (framePos >= _segmentSize) {
            qDebug("JackAudio::putEvent: time out of range %d(seg=%d)", framePos, _segmentSize);
            if (framePos > _segmentSize)
                  framePos = _segmentSize - 1;
            }

      switch(e.type()) {
            case ME_NOTEON:
            case ME_NOTEOFF:
            case ME_POLYAFTER:
            case ME_CONTROLLER:
                  // Catch CTRL_PROGRAM and let other ME_CONTROLLER events to go
                  if (e.dataA() == CTRL_PROGRAM) {
                        // Convert CTRL_PROGRAM event to ME_PROGRAM
                        unsigned char* p = jack_midi_event_reserve(pb, framePos, 2);
                        if (p == 0) {
                              qDebug("JackMidi: buffer overflow, event lost");
                              return;
                              }
                        p[0] = ME_PROGRAM | chan;
                        p[1] = less128(e.dataB());
                        break;
                        }
                  //fall through
            case ME_PITCHBEND:
                  {
                  unsigned char* p = jack_midi_event_reserve(pb, framePos, 3);
                  if (p == 0) {
                        qDebug("JackMidi: buffer overflow, event lost");
                        return;
                        }
                  p[0] = e.type() | chan;
                  p[1] = less128(e.dataA());
                  p[2] = less128(e.dataB());
                  }
                  break;

            case ME_PROGRAM:
            case ME_AFTERTOUCH:
                  {
                  unsigned char* p = jack_midi_event_reserve(pb, framePos, 2);
                  if (p == 0) {
                        qDebug("JackMidi: buffer overflow, event lost");
                        return;
                        }
                  p[0] = e.type() | chan;
                  p[1] = less128(e.dataA());
                  }
                  break;
          // Do we really need to handle ME_SYSEX?
          /*  case ME_SYSEX:
                  {
                  const unsigned char* data = e.edata();
                  int len = e.len();
                  unsigned char* p = jack_midi_event_reserve(pb, framePos, len+2);
                  if (p == 0) {
                        qDebug("JackMidi: buffer overflow, event lost");
                        return;
                        }
                  p[0]     = 0xf0;
                  p[len+1] = 0xf7;
                  memcpy(p+1, data, len);
                  }
                  break;*/
            case ME_SONGPOS:
            case ME_CLOCK:
            case ME_START:
            case ME_CONTINUE:
            case ME_STOP:
                  qDebug("JackMidi: event type %x not supported", e.type());
                  break;
            }
      }

//---------------------------------------------------------
//   midiRead
//---------------------------------------------------------

void JackAudio::midiRead()
      {
//      midiDriver->read();
      }

//---------------------------------------------------------
//   handleTimeSigTempoChanged
//   Called after tempo or time signature
//   changed while playback
//---------------------------------------------------------

void JackAudio::handleTimeSigTempoChanged()
      {
      timeSigTempoChanged = true;
      }

//---------------------------------------------------------
//   checkTransportSeek
//   The opposite of Timebase master:
//   check JACK Transport for a new position or tempo.
//---------------------------------------------------------

void JackAudio::checkTransportSeek(int cur_frame, int nframes, bool inCountIn)
      {
      if (!seq || !seq->score() || inCountIn)
            return;

      // Obtaining the current JACK Transport position
      jack_position_t pos;
      jack_transport_query(client, &pos);

      if (preferences.getBool(PREF_IO_JACK_USEJACKTRANSPORT)) {
            if (mscore->playPanelInterface() && mscore->playPanelInterface()->isSpeedSliderPressed())
                  return;
            int cur_utick = seq->score()->utime2utick((qreal)cur_frame / MScore::sampleRate);
            int utick     = seq->score()->utime2utick((qreal)pos.frame / MScore::sampleRate);

            // Conversion is not precise, should check frames and uticks
            if (labs((long int)cur_frame - (long int)pos.frame)>nframes + 1 && abs(utick - cur_utick)> seq->score()->utime2utick((qreal)nframes / MScore::sampleRate) + 1) {
                  if (MScore::debugMode)
                        qDebug()<<"JACK Transport position changed, cur_frame: "<<cur_frame<<",pos.frame: "<<pos.frame<<", frame diff: "<<labs((long int)cur_frame - (long int)pos.frame)<<"cur utick:"<<cur_utick<<",seek to utick: "<<utick<<", tick diff: "<<abs(utick - cur_utick);
                  seq->seekRT(utick);
                  }
            }

      // Tempo
      if (!preferences.getBool(PREF_IO_JACK_TIMEBASEMASTER)  && (pos.valid & JackPositionBBT)) {
            if (!seq->score()->tempomap())
                  return;

            if (int(pos.beats_per_minute) != int(60 * seq->curTempo() * seq->score()->tempomap()->relTempo())) {
                  if (MScore::debugMode)
                        qDebug()<<"JACK Transport tempo changed! JACK bpm: "<<(int)pos.beats_per_minute<<", current bpm: "<<int(60 * seq->curTempo() * seq->score()->tempomap()->relTempo());
                  if (qFuzzyIsNull(60 * seq->curTempo()))
                        return;
                  qreal newRelTempo = pos.beats_per_minute / (60* seq->curTempo());
                  seq->setRelTempo(newRelTempo);
                  // Update UI
                  if (mscore->getPlayPanel()) {
                        mscore->playPanelInterface()->setSpeed(newRelTempo);
                        mscore->playPanelInterface()->setTempo(seq->curTempo() * newRelTempo);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   seekTransport
//---------------------------------------------------------

void JackAudio::seekTransport(int utick)
      {
      if (MScore::debugMode)
            qDebug()<<"jack locate to utick: "<<utick<<", frame: "<<int(seq->score()->utick2utime(utick) * MScore::sampleRate);
      jack_transport_locate(client, seq->score()->utick2utime(utick) * MScore::sampleRate);
      }

//---------------------------------------------------------
//   setTimebaseCallback
//---------------------------------------------------------

void JackAudio::setTimebaseCallback()
      {
      int errCode = jack_set_timebase_callback(client, 0, timebase, this); // 0: force set timebase
      if (errCode == 0) {
            if (MScore::debugMode)
                  qDebug("Registered as JACK Timebase Master.");
            }
      else {
            preferences.setPreference(PREF_IO_JACK_TIMEBASEMASTER, false);
            qDebug("Unable to take over JACK Timebase, error code: %i",errCode);
            }
      }

//---------------------------------------------------------
//   releaseTimebaseCallback
//---------------------------------------------------------

void JackAudio::releaseTimebaseCallback()
      {
      int errCode = jack_release_timebase(client);
      if (errCode == 0)
            qDebug("Unregistered as JACK Timebase Master");
      else
            qDebug("Unable to unregister as JACK Timebase Master (not a Timebase Master?), error code: %i", errCode);
      }

//---------------------------------------------------------
//   rememberAudioConnections
//---------------------------------------------------------

void JackAudio::rememberAudioConnections()
      {
      if (!preferences.getBool(PREF_IO_JACK_REMEMBERLASTCONNECTIONS))
            return;
      if (MScore::debugMode)
            qDebug("Saving audio connections...");
      QSettings settings;
      settings.setValue(QString("audio-0-connections"), 0);
      settings.setValue(QString("audio-1-connections"), 0);
      int port = 0;
      foreach(jack_port_t* mp, ports) {
            const char** cc = jack_port_get_connections(mp);
            const char** c = cc;
            int idx = 0;
            while (c) {
                  const char* p = *c++;
                  if (p == 0)
                        break;
                  settings.setValue(QString("audio-%1-%2").arg(port).arg(idx), p);
                  ++idx;
                  }
            settings.setValue(QString("audio-%1-connections").arg(port), idx);
            free((void*)cc);
            ++port;
            }
      }

//---------------------------------------------------------
//   restoreAudioConnections
//   Connect to the ports in Preferences->I/O
//---------------------------------------------------------

void JackAudio::restoreAudioConnections()
      {
      for (auto p : qAsConst(ports))
            jack_port_disconnect(client, p);

      QList<QString> portList = inputPorts();
      QList<QString>::iterator pi = portList.begin();

      QSettings settings;
      // Number of saved ports
      int n = settings.value(QString("audio-0-connections"), 0).toInt() + settings.value(QString("audio-1-connections"), 0).toInt();

      // Connecting to system ports
      if (!preferences.getBool(PREF_IO_JACK_REMEMBERLASTCONNECTIONS) || n == 0) {
            if (MScore::debugMode)
                  qDebug("Connecting to system ports...");
            for (auto p : qAsConst(ports)) {
                  const char* src = jack_port_name(p);
                  if (pi != portList.end()) {
                        connect(src, qPrintable(*pi));
                        ++pi;
                        }
                  }
            return;
            }
      if (MScore::debugMode)
            qDebug("Restoring audio connections...");
      // Connecting to saved ports
      int nPorts = ports.size();
      for (int i = 0; i < nPorts; ++i) {
            int j = settings.value(QString("audio-%1-connections").arg(i), 0).toInt();
            const char* src = jack_port_name(ports[i]);
            for (int k = 0; k < j; ++k) {
                  QString dst = settings.value(QString("audio-%1-%2").arg(i).arg(k), "").toString();
                  if (!dst.isEmpty()) {
                        if (jack_port_connected_to(ports[i], qPrintable(dst)))
                              qDebug()<<"Audio port "<<src<<" ("<<i<<") already connected to "<<qPrintable(dst);
                        else
                              connect(src, qPrintable(dst));
                        }
                  }
            }
      }

//---------------------------------------------------------
//   rememberMidiConnections
//---------------------------------------------------------

void JackAudio::rememberMidiConnections()
      {
      if (!preferences.getBool(PREF_IO_JACK_REMEMBERLASTCONNECTIONS))
            return;
      if (MScore::debugMode)
            qDebug("Saving midi connections...");
      QSettings settings;
      int port = 0;
      foreach(jack_port_t* mp, midiOutputPorts) {
            const char** cc = jack_port_get_connections(mp);
            const char** c = cc;
            int idx = 0;
            while (c) {
                  const char* p = *c++;
                  if (p == 0)
                        break;
                  settings.setValue(QString("midi-%1-%2").arg(port).arg(idx), p);
                  ++idx;
                  }
            settings.setValue(QString("midi-%1-connections").arg(port), idx);
            free((void*)cc);
            ++port;
            }

      port = 0;
      foreach(jack_port_t* mp, midiInputPorts) {
            const char** cc = jack_port_get_connections(mp);
            const char** c = cc;
            int idx = 0;
            while (c) {
                  const char* p = *c++;
                  if (p == 0)
                        break;
                  settings.setValue(QString("midiin-%1-%2").arg(idx).arg(port), p);
                  ++idx;
                  }
            settings.setValue(QString("midiin-%1-connections").arg(port), idx);
            free((void*)cc);
            ++port;
            }
      }

//---------------------------------------------------------
//   restoreMidiConnections
//   Connects to the ports from previous connection
//---------------------------------------------------------

void JackAudio::restoreMidiConnections()
      {
      if (!preferences.getBool(PREF_IO_JACK_REMEMBERLASTCONNECTIONS))
            return;
      if (MScore::debugMode)
            qDebug("Restoring midi connections...");
      QSettings settings;
      int nPorts = midiOutputPorts.size();
      for (int i = 0; i < nPorts; ++i) {
            int n = settings.value(QString("midi-%1-connections").arg(i), 0).toInt();
            const char* src = jack_port_name(midiOutputPorts[i]);
            for (int k = 0; k < n; ++k) {
                  QString dst = settings.value(QString("midi-%1-%2").arg(i).arg(k), "").toString();
                  if (!dst.isEmpty()) {
                        if (jack_port_connected_to(midiOutputPorts[i], qPrintable(dst)))
                              continue;
                        connect(src, qPrintable(dst));
                        }
                  }
            }
      nPorts = midiInputPorts.size();
      for (int i = 0; i < nPorts; ++i) {
            int n = settings.value(QString("midiin-%1-connections").arg(i), 0).toInt();
            const char* dst = jack_port_name(midiInputPorts[i]);
            for (int k = 0; k < n; ++k) {
                  QString src = settings.value(QString("midiin-%1-%2").arg(k).arg(i), "").toString();
                  if (!src.isEmpty()) {
                        if (jack_port_connected_to(midiInputPorts[i], qPrintable(src)))
                              continue;
                        connect(qPrintable(src), dst);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   hotPlug
//   Change driver settings without unload
//---------------------------------------------------------

void JackAudio::hotPlug()
      {
      bool oldremember = preferences.getBool(PREF_IO_JACK_REMEMBERLASTCONNECTIONS);
      preferences.setPreference(PREF_IO_JACK_REMEMBERLASTCONNECTIONS, true);
      // Remember connections before calling jack_deactivate() - it disconnects all ports
      rememberMidiConnections();
      if (ports.size() != 0)
            rememberAudioConnections();

      // We must set callbacks only on inactive client
      if (jack_deactivate(client))
            qDebug("cannot deactivate client");

      // Audio connections
      if (preferences.getBool(PREF_IO_JACK_USEJACKAUDIO)) {
            if (ports.size() == 0) {
                  registerPort("left", false, false);
                  registerPort("right", false, false);
                  }
            }
      else if (!preferences.getBool(PREF_IO_JACK_USEJACKAUDIO)) {
            foreach(jack_port_t* p, ports) {
                  unregisterPort(p);
                  ports.removeOne(p);
                  }
            }

      // Midi connections
      if (preferences.getBool(PREF_IO_JACK_USEJACKMIDI)) {
            if (midiInputPorts.size() == 0)
                  registerPort(QString("mscore-midiin-1"), true, true);
            }
      else { // No midi
            updateOutPortCount(0);
            if (midiInputPorts.size() != 0) {
                  unregisterPort(midiInputPorts[0]);
                  midiInputPorts.removeOne(midiInputPorts[0]);
                  }
            }

      // Timebase Master callback
      if (preferences.getBool(PREF_IO_JACK_TIMEBASEMASTER))
            setTimebaseCallback();
      else
            releaseTimebaseCallback();

      preferences.setPreference(PREF_IO_JACK_REMEMBERLASTCONNECTIONS, oldremember);
      }
}
