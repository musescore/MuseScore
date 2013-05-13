//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: jackaudio.cpp 5660 2012-05-22 14:17:39Z wschweer $
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

#include "jackaudio.h"

#include "libmscore/mscore.h"
#include "preferences.h"
// #include "msynth/synti.h"
#include "seq.h"

#include <jack/midiport.h>

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
            if (jack_client_close(client)) {
                  qDebug("jack_client_close() failed: %s\n",
                     strerror(errno));
                  }
            }
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
            qDebug("JackAudio:registerPort(%s) failed\n", qPrintable(name));
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

void JackAudio::unregisterPort(int port)
      {
      jack_port_unregister(client, ports[port]);
      ports[port] = 0;
      }

//---------------------------------------------------------
//   inputPorts
//---------------------------------------------------------

QList<QString> JackAudio::inputPorts()
      {
      const char** ports = jack_get_ports(client, 0, 0, 0);
      QList<QString> clientList;
      for (const char** p = ports; p && *p; ++p) {
            jack_port_t* port = jack_port_by_name(client, *p);
            int flags = jack_port_flags(port);
            if (!(flags & JackPortIsInput))
                  continue;
            char buffer[128];
            strncpy(buffer, *p, 128);
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
            qDebug("JackAudio::connect: unknown jack ports\n");
            return;
            }
      if (jack_connect(client, sn, dn)) {
            qDebug("jack connect <%s>%p - <%s>%p failed\n",
               sn, src, dn, dst);
            }
      }

//---------------------------------------------------------
//   disconnect
//---------------------------------------------------------

void JackAudio::disconnect(void* src, void* dst)
      {
      const char* sn = jack_port_name((jack_port_t*) src);
      const char* dn = jack_port_name((jack_port_t*) dst);
      if (sn == 0 || dn == 0) {
            qDebug("JackAudio::disconnect: unknown jack ports\n");
            return;
            }
      if (jack_disconnect(client, sn, dn)) {
            qDebug("jack disconnect <%s> - <%s> failed\n", sn, dn);
            }
      }

//---------------------------------------------------------
//   start
//    return false on error
//---------------------------------------------------------

bool JackAudio::start()
      {
      if (jack_activate(client)) {
            qDebug("JACK: cannot activate client\n");
            return false;
            }
      if (preferences.useJackAudio) {
            /* connect the ports. Note: you can't do this before
               the client is activated, because we can't allow
               connections to be made to clients that aren't
               running.
             */
            QString lport = preferences.lPort;
            QString rport = preferences.rPort;

            const char* src = jack_port_name(ports[0]);
            int rv;
            rv = jack_connect(client, src, qPrintable(lport));
            if (rv) {
                  qDebug("jack connect <%s> - <%s> failed: %d\n",
                     src, qPrintable(lport), rv);
                  }
            src = jack_port_name(ports[1]);
            if (!rport.isEmpty()) {
                  if (jack_connect(client, src, qPrintable(rport))) {
                        qDebug("jack connect <%s> - <%s> failed: %d\n",
                           src, qPrintable(rport), rv);
                        }
                  }

            }
      if (preferences.useJackMidi && preferences.rememberLastMidiConnections) {
            QSettings settings;
            int nPorts = midiOutputPorts.size();
            for (int i = 0; i < nPorts; ++i) {
                  int n = settings.value(QString("midi-%1-connections").arg(i), 0).toInt();
                  const char* src = jack_port_name(midiOutputPorts[i]);
                  for (int k = 0; k < n; ++k) {
                        QString dst = settings.value(QString("midi-%1-%2").arg(i).arg(k), "").toString();
                        if (!dst.isEmpty()) {
                              int rv = jack_connect(client, src, qPrintable(dst));
                              if (rv) {
                                    qDebug("jack connect midi output <%s> - <%s> failed: %d\n",
                                       src, qPrintable(dst), rv);
                                    }
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
                              int rv = jack_connect(client, qPrintable(src), dst);
                              if (rv) {
                                    qDebug("jack connect midi input <%s> - <%s> failed: %d\n",
                                       qPrintable(src), dst, rv);
                                    }
                              }
                        }
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   stop
//    return false on error
//---------------------------------------------------------

bool JackAudio::stop()
      {
      if (preferences.useJackMidi && preferences.rememberLastMidiConnections) {
            QSettings settings;
            settings.setValue("midiPorts", midiOutputPorts.size());
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
            settings.setValue("midiInputPorts", midiInputPorts.size());
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


static int bufsize_callback(jack_nframes_t /*n*/, void*)
      {
//      qDebug("JACK: buffersize changed %d\n", n);
      return 0;
      }

//---------------------------------------------------------
//   freewheel_callback
//---------------------------------------------------------

static void freewheel_callback(int /*starting*/, void*)
      {
      }

static int srate_callback(jack_nframes_t, void*)
      {
//      qDebug("JACK: sample rate changed: %d\n", n);
      return 0;
      }

static void registration_callback(jack_port_id_t, int, void*)
      {
//      qDebug("JACK: registration changed\n");
      }

static int graph_callback(void*)
      {
//      qDebug("JACK: graph changed\n");
      return 0;
      }

//---------------------------------------------------------
//   processAudio
//    JACK callback
//---------------------------------------------------------

int JackAudio::processAudio(jack_nframes_t frames, void* p)
      {
      JackAudio* audio = (JackAudio*)p;
      float* l;
      float* r;
      if (preferences.useJackAudio) {
            l = (float*)jack_port_get_buffer(audio->ports[0], frames);
            r = (float*)jack_port_get_buffer(audio->ports[1], frames);
            }
      else {
            l = 0;
            r = 0;
            }
      if (preferences.useJackMidi) {
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
                              int r = jack_midi_event_get(&event, portBuffer, i);
                              if (r != 0)
                                    continue;
                              int nn = event.size;
                              int type = event.buffer[0];
                              if (nn && (type == ME_CLOCK || type == ME_SENSE))
                                    continue;
                              Event e;
                              e.setType(type);
                              e.setChannel(type & 0xf);
                              type &= 0xf0;
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
            float buffer[frames * 2];
            audio->seq->process((unsigned)frames, buffer);
            float* sp = buffer;
            for (unsigned i = 0; i < frames; ++i) {
                  *l++ = *sp++;
                  *r++ = *sp++;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   jackError
//---------------------------------------------------------

static void jackError(const char *s)
      {
      qDebug("JACK ERROR: %s\n", s);
      }

//---------------------------------------------------------
//   noJackError
//---------------------------------------------------------

static void noJackError(const char* /* s */)
      {
      }

//---------------------------------------------------------
//   init
//    return false on error
//---------------------------------------------------------

bool JackAudio::init()
      {
      jack_set_error_function(noJackError);

      client = 0;
      strcpy(_jackName, "mscore");

      jack_options_t options = (jack_options_t)0;
      jack_status_t status;
      client = jack_client_open(_jackName, options, &status);
      if (client == 0) {
            qDebug("JackAudio()::init(): failed, status 0x%0x\n", status);
            return false;
            }

      jack_set_error_function(jackError);
      jack_set_process_callback(client, processAudio, this);
      //jack_on_shutdown(client, processShutdown, this);
      jack_set_buffer_size_callback(client, bufsize_callback, this);
      jack_set_sample_rate_callback(client, srate_callback, this);
      jack_set_port_registration_callback(client, registration_callback, this);
      jack_set_graph_order_callback(client, graph_callback, this);
      jack_set_freewheel_callback (client, freewheel_callback, this);
      _segmentSize  = jack_get_buffer_size(client);

      MScore::sampleRate = sampleRate();
      // register mscore left/right output ports
      if (preferences.useJackAudio) {
            registerPort("left", false, false);
            registerPort("right", false, false);

            // connect mscore output ports to jack input ports
            QString lport = preferences.lPort;
            QString rport = preferences.rPort;
            QList<QString> ports = inputPorts();
            QList<QString>::iterator pi = ports.begin();
            if (lport.isEmpty()) {
                  if (pi != ports.end()) {
                        preferences.lPort = *pi;
                        ++pi;
                        }
                  else {
                        qDebug("no jack ports found\n");
                        jack_client_close(client);
                        client = 0;
                        return false;
                        }
                  }
            if (rport.isEmpty()) {
                  if (pi != ports.end()) {
                        preferences.rPort = *pi;
                        }
                  else {
                        qDebug("no jack port for right channel found!\n");
                        }
                  }
            }

      if (preferences.useJackMidi) {
            for (int i = 0; i < preferences.midiPorts; ++i)
                  registerPort(QString("mscore-midi-%1").arg(i+1), false, true);
            registerPort(QString("mscore-midiin-1"), true, true);
            }
      return true;
      }

//---------------------------------------------------------
//   startTransport
//---------------------------------------------------------

void JackAudio::startTransport()
      {
      jack_transport_start(client);
      }

//---------------------------------------------------------
//   stopTrasnport
//---------------------------------------------------------

void JackAudio::stopTransport()
      {
      jack_transport_stop(client);
      }

//---------------------------------------------------------
//   getState
//---------------------------------------------------------

int JackAudio::getState()
      {
      jack_position_t pos;
      int transportState = jack_transport_query(client, &pos);
      switch (transportState) {
            case JackTransportStopped:  return Seq::TRANSPORT_STOP;
            case JackTransportLooping:
            case JackTransportRolling:  return Seq::TRANSPORT_PLAY;
            default:
                  return Seq::TRANSPORT_STOP;
            }
      }

//---------------------------------------------------------
//   putEvent
//---------------------------------------------------------

void JackAudio::putEvent(const Event& e, unsigned framePos)
      {
      if (!preferences.useJackMidi)
            return;

      int portIdx = e.channel() / 16;
      int chan    = e.channel() % 16;

// qDebug("JackAudio::putEvent %d:%d  pos %d(%d)\n", portIdx, chan, framePos, _segmentSize);

      if (portIdx < 0 || portIdx >= midiOutputPorts.size()) {
            qDebug("JackAudio::putEvent: invalid port %d\n", portIdx);
            return;
            }
      jack_port_t* port = midiOutputPorts[portIdx];
      if (midiOutputTrace) {
            const char* portName = jack_port_name(port);
            qDebug("MidiOut<%s>: jackMidi: ", portName);
            // e.dump();
            }
      void* pb = jack_port_get_buffer(port, _segmentSize);

      if (framePos >= _segmentSize) {
            qDebug("JackAudio::putEvent: time out of range %d(seg=%d)\n", framePos, _segmentSize);
            if (framePos > _segmentSize)
                  framePos = _segmentSize - 1;
            }

      switch(e.type()) {
            case ME_NOTEON:
            case ME_NOTEOFF:
            case ME_POLYAFTER:
            case ME_CONTROLLER:
            case ME_PITCHBEND:
                  {
                  unsigned char* p = jack_midi_event_reserve(pb, framePos, 3);
                  if (p == 0) {
                        qDebug("JackMidi: buffer overflow, event lost\n");
                        return;
                        }
                  p[0] = e.type() | chan;
                  p[1] = e.dataA();
                  p[2] = e.dataB();
                  }
                  break;

            case ME_PROGRAM:
            case ME_AFTERTOUCH:
                  {
                  unsigned char* p = jack_midi_event_reserve(pb, framePos, 2);
                  if (p == 0) {
                        qDebug("JackMidi: buffer overflow, event lost\n");
                        return;
                        }
                  p[0] = e.type() | chan;
                  p[1] = e.dataA();
                  }
                  break;
            case ME_SYSEX:
                  {
                  const unsigned char* data = e.edata();
                  int len = e.len();
                  unsigned char* p = jack_midi_event_reserve(pb, framePos, len+2);
                  if (p == 0) {
                        qDebug("JackMidi: buffer overflow, event lost\n");
                        return;
                        }
                  p[0]     = 0xf0;
                  p[len+1] = 0xf7;
                  memcpy(p+1, data, len);
                  }
                  break;
            case ME_SONGPOS:
            case ME_CLOCK:
            case ME_START:
            case ME_CONTINUE:
            case ME_STOP:
                  qDebug("JackMidi: event type %x not supported\n", e.type());
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
}

