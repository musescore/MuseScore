//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: mididriver.cpp 5568 2012-04-22 10:08:43Z wschweer $
//
//  Copyright (C) 2008 Werner Schweer and others
//
//  AlsaDriver based on code from Fons Adriaensen (clalsadr.cc)
//  Copyright (C) 2003 Fons Adriaensen
//  partly based on original work from Paul Davis
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

#include <time.h>

#include "config.h"
#include "mididriver.h"
#include "preferences.h"
#include "musescore.h"
#include "midi/midifile.h"
#include "globals.h"
#include "seq.h"
#include "libmscore/utils.h"
#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   Port
//---------------------------------------------------------

Port::Port()
      {
      type = ZERO_TYPE;
      }

Port::Port(unsigned char client, unsigned char port)
      {
      _alsaPort = port;
      _alsaClient = client;
      type = ALSA_TYPE;
      }

//---------------------------------------------------------
//   setZero
//---------------------------------------------------------

void Port::setZero()
      {
      type = ZERO_TYPE;
      }

//---------------------------------------------------------
//   isZero
//---------------------------------------------------------

bool Port::isZero()  const
      {
      return type == ZERO_TYPE;
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool Port::operator==(const Port& p) const
      {
      if (type == ALSA_TYPE)
            return _alsaPort == p._alsaPort && _alsaClient == p._alsaClient;
      else
            return true;
      }

//---------------------------------------------------------
//   operator<
//---------------------------------------------------------

bool Port::operator<(const Port& p) const
      {
      if (type == ALSA_TYPE) {
            if (_alsaPort != p._alsaPort)
                  return _alsaPort < p._alsaPort;
            return _alsaClient < p._alsaClient;
            }
      return false;
      }
}

#ifdef USE_ALSA
#include "alsa.h"
#include "alsamidi.h"

namespace Ms {
static const unsigned int inCap  = SND_SEQ_PORT_CAP_SUBS_READ;
static const unsigned int outCap = SND_SEQ_PORT_CAP_SUBS_WRITE;

//---------------------------------------------------------
//   AlsaMidiDriver
//---------------------------------------------------------

AlsaMidiDriver::AlsaMidiDriver(Seq* s)
   : MidiDriver(s)
      {
      }

//---------------------------------------------------------
//   init
//    return false on error
//---------------------------------------------------------

bool AlsaMidiDriver::init()
      {
      int error = snd_seq_open(&alsaSeq, "hw", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
      if (error < 0) {
            if (error == ENOENT)
                  qDebug("open ALSA sequencer failed: %s", snd_strerror(error));
            return false;
            }

      snd_seq_set_client_name(alsaSeq, "MuseScore");

      //-----------------------------------------
      //    subscribe to "Announce"
      //    this enables callbacks for any
      //    alsa port changes
      //-----------------------------------------

      snd_seq_addr_t src, dst;
      int rv = snd_seq_create_simple_port(alsaSeq, "MuseScore Port 0",
         inCap | outCap | SND_SEQ_PORT_CAP_READ
         | SND_SEQ_PORT_CAP_WRITE
         | SND_SEQ_PORT_CAP_NO_EXPORT,
         SND_SEQ_PORT_TYPE_APPLICATION);
      if (rv < 0) {
            qDebug("Alsa: create MuseScore port failed: %s", snd_strerror(error));
            return false;
            }
      dst.port   = rv;
      dst.client = snd_seq_client_id(alsaSeq);
      src.port   = SND_SEQ_PORT_SYSTEM_ANNOUNCE;
      src.client = SND_SEQ_CLIENT_SYSTEM;

      snd_seq_port_subscribe_t* subs;
      snd_seq_port_subscribe_alloca(&subs);
      snd_seq_port_subscribe_set_dest(subs, &dst);
      snd_seq_port_subscribe_set_sender(subs, &src);
      error = snd_seq_subscribe_port(alsaSeq, subs);
      if (error < 0) {
            qDebug("Alsa: Subscribe System failed: %s", snd_strerror(error));
            return false;
            }
      midiInPort   = registerOutPort("MuseScore Port-0");
      midiOutPorts.append(registerInPort("MuseScore Port-0"));

      struct pollfd* pfd;
      int npfd;
      getInputPollFd(&pfd, &npfd);
      for (int i = 0; i < npfd; ++i) {
            int fd = pfd[i].fd;
            if (fd != -1) {
                  QSocketNotifier* s = new QSocketNotifier(fd, QSocketNotifier::Read,  mscore);
                  s->connect(s, SIGNAL(activated(int)), seq, SLOT(midiInputReady()));
                  }
            }
#if 0
      // TODO: autoconnect all output ports
      QList<PortName> ol = outputPorts();
      foreach(PortName pn, ol) {
            if (MScore::debugMode)
                  qDebug("connect to midi output <%s>", qPrintable(pn.name));
            qDebug("Output <%s>", qPrintable(pn.name));
            }
#endif

      // connect all midi sources to mscore
      QList<PortName> il = inputPorts();
      foreach(PortName pn, il) {
            if (MScore::debugMode)
                  qDebug("connect to midi input <%s>", qPrintable(pn.name));
            connect(pn.port, midiInPort);
            }
      return true;
      }

//---------------------------------------------------------
//   outputPorts
//---------------------------------------------------------

QList<PortName> AlsaMidiDriver::outputPorts()
      {
      QList<PortName> clientList;
      snd_seq_client_info_t* cinfo;
      snd_seq_client_info_alloca(&cinfo);
      snd_seq_client_info_set_client(cinfo, 0);

      while (snd_seq_query_next_client(alsaSeq, cinfo) >= 0) {
            snd_seq_port_info_t *pinfo;
            snd_seq_port_info_alloca(&pinfo);
            snd_seq_port_info_set_client(pinfo, snd_seq_client_info_get_client(cinfo));
            snd_seq_port_info_set_port(pinfo, -1);
            while (snd_seq_query_next_port(alsaSeq, pinfo) >= 0) {
                  unsigned int capability = snd_seq_port_info_get_capability(pinfo);
                  if (((capability & outCap) == outCap)
                     && !(capability & SND_SEQ_PORT_CAP_NO_EXPORT)) {
                        int client = snd_seq_port_info_get_client(pinfo);
                        if (client != snd_seq_client_id(alsaSeq)) {
                              PortName pn;
                              pn.name = QString(snd_seq_port_info_get_name(pinfo));
                              pn.port = Port(client, snd_seq_port_info_get_port(pinfo));
                              clientList.append(pn);
                              }
                        }
                  }
            }
      return clientList;
      }

//---------------------------------------------------------
//   inputPorts
//---------------------------------------------------------

QList<PortName> AlsaMidiDriver::inputPorts()
      {
      QList<PortName> clientList;

      snd_seq_client_info_t* cinfo;
      snd_seq_client_info_alloca(&cinfo);
      snd_seq_client_info_set_client(cinfo, 0);

      while (snd_seq_query_next_client(alsaSeq, cinfo) >= 0) {
            snd_seq_port_info_t *pinfo;
            snd_seq_port_info_alloca(&pinfo);
            snd_seq_port_info_set_client(pinfo, snd_seq_client_info_get_client(cinfo));
            snd_seq_port_info_set_port(pinfo, -1);
            while (snd_seq_query_next_port(alsaSeq, pinfo) >= 0) {
                  unsigned int capability = snd_seq_port_info_get_capability(pinfo);
                  if (((capability & inCap) == inCap)
                     && !(capability & SND_SEQ_PORT_CAP_NO_EXPORT)) {
                        int client = snd_seq_port_info_get_client(pinfo);
                        if (client != snd_seq_client_id(alsaSeq)) {
                              PortName pn;
                              pn.name = QString(snd_seq_port_info_get_name(pinfo));
                              pn.port = Port(client, snd_seq_port_info_get_port(pinfo));
                              clientList.append(pn);
                              }
                        }
                  }
            }
      return clientList;
      }

//---------------------------------------------------------
//   connect
//    return false if connect fails
//---------------------------------------------------------

bool AlsaMidiDriver::connect(Port src, Port dst)
      {
      snd_seq_port_subscribe_t* sub;
      snd_seq_port_subscribe_alloca(&sub);

      snd_seq_addr_t s, d;
      s.port   = src.alsaPort();
      s.client = src.alsaClient();
      d.port   = dst.alsaPort();
      d.client = dst.alsaClient();
      snd_seq_port_subscribe_set_sender(sub, &s);
      snd_seq_port_subscribe_set_dest(sub, &d);

      int rv = snd_seq_subscribe_port(alsaSeq, sub);
      if (rv < 0) {
            qDebug("AlsaMidi::connect(%d:%d, %d:%d) failed: %s",
               src.alsaClient(), src.alsaPort(),
               dst.alsaClient(), dst.alsaPort(),
               snd_strerror(rv));
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   registerOutPort
//---------------------------------------------------------

Port AlsaMidiDriver::registerOutPort(const QString& name)
      {
      int alsaPort  = snd_seq_create_simple_port(alsaSeq, name.toLatin1().data(),
         outCap | SND_SEQ_PORT_CAP_WRITE, SND_SEQ_PORT_TYPE_APPLICATION);
      if (alsaPort < 0) {
            perror("cannot create alsa out port");
            return Port();
            }
      return Port(snd_seq_client_id(alsaSeq), alsaPort);
      }

//---------------------------------------------------------
//   registerInPort
//---------------------------------------------------------

Port AlsaMidiDriver::registerInPort(const QString& name)
      {
      int alsaPort  = snd_seq_create_simple_port(alsaSeq, name.toLatin1().data(),
         inCap | SND_SEQ_PORT_CAP_READ, SND_SEQ_PORT_TYPE_APPLICATION);
      if (alsaPort < 0) {
            perror("cannot create alsa in port");
            return Port();
            }
      return Port(snd_seq_client_id(alsaSeq), alsaPort);
      }

//---------------------------------------------------------
//   updateInPortCount
//---------------------------------------------------------

void AlsaMidiDriver::updateInPortCount(int maxport)
      {
      int ports = midiOutPorts.size();
      if (maxport == ports)
            return;
      if (MScore::debugMode)
            qDebug()<<"ALSA number of ports:"<<ports<<", change to:"<<maxport;
      if (maxport > ports) {
            for (int i = ports; i < maxport; ++i)
                  midiOutPorts.append(registerInPort(QString("MuseScore Port-%1").arg(i)));
            }
      else if (maxport < ports) {
            for(int i = ports - 1; i >= maxport; --i) {
                  if (snd_seq_delete_simple_port(alsaSeq, midiOutPorts[i].alsaPort()) < 0)
                        qDebug("Can not delete ALSA port");
                  else
                        midiOutPorts.removeAt(i);
                  }
            }
      }

//---------------------------------------------------------
//   getInputPollFd
//---------------------------------------------------------

void AlsaMidiDriver::getInputPollFd(struct pollfd** p, int* n)
      {
      int npfdi = snd_seq_poll_descriptors_count(alsaSeq, POLLIN);
      struct pollfd* pfdi  = new struct pollfd[npfdi];
      snd_seq_poll_descriptors(alsaSeq, pfdi, npfdi, POLLIN);
      *p = pfdi;
      *n = npfdi;
      }

//---------------------------------------------------------
//   getOutputPollFd
//---------------------------------------------------------

void AlsaMidiDriver::getOutputPollFd(struct pollfd** p, int* n)
      {
      int npfdo = snd_seq_poll_descriptors_count(alsaSeq, POLLOUT);
      struct pollfd* pfdo  = new struct pollfd[npfdo];
      snd_seq_poll_descriptors(alsaSeq, pfdo, npfdo, POLLOUT);
      *p = pfdo;
      *n = npfdo;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void AlsaMidiDriver::read()
      {
      snd_seq_event_t* ev;
      for (;;) {
            int rv = snd_seq_event_input(alsaSeq, &ev);
            if (rv < 0)
                  return;

            if (!mscore || !mscore->midiinEnabled()) {
                  snd_seq_free_event(ev);
                  return;
                  }

            if (ev->type == SND_SEQ_EVENT_NOTEON) {
                  int pitch = ev->data.note.note;
                  int velo  = ev->data.note.velocity;
                  mscore->midiNoteReceived(ev->data.note.channel, pitch, velo);
                  }
            else if (ev->type == SND_SEQ_EVENT_NOTEOFF) {    // "Virtual Keyboard" sends this
                  int pitch = ev->data.note.note;
                  mscore->midiNoteReceived(ev->data.note.channel, pitch, 0);
                  }
            else if (ev->type == SND_SEQ_EVENT_CONTROLLER) {
                  mscore->midiCtrlReceived(ev->data.control.param,
                     ev->data.control.value);
                  }

            if (midiInputTrace) {
                  qDebug("MidiIn: ");
                  switch(ev->type) {
                        case SND_SEQ_EVENT_NOTEON:
                              qDebug("noteOn ch:%2d 0x%02x 0x%02x",
                                 ev->data.note.channel,
                                 ev->data.note.note,
                                 ev->data.note.velocity);
                              break;
                        case SND_SEQ_EVENT_SYSEX:
                              qDebug("sysEx len %d", ev->data.ext.len);
                              break;
                        case SND_SEQ_EVENT_CONTROLLER:
                              qDebug("ctrl 0x%02x 0x%02x",
                                 ev->data.control.param,
                                 ev->data.control.value);
                              break;
                        case SND_SEQ_EVENT_PITCHBEND:
                              qDebug("pitchbend 0x%04x", ev->data.control.value);
                              break;
                        case SND_SEQ_EVENT_PGMCHANGE:
                              qDebug("pgmChange 0x%02x", ev->data.control.value);
                              break;
                        case SND_SEQ_EVENT_CHANPRESS:
                              qDebug("channelPress 0x%02x", ev->data.control.value);
                              break;
                        case SND_SEQ_EVENT_START:
                              qDebug("start");
                              break;
                        case SND_SEQ_EVENT_CONTINUE:
                              qDebug("continue");
                              break;
                        case SND_SEQ_EVENT_STOP:
                              qDebug("stop");
                              break;
                        case SND_SEQ_EVENT_SONGPOS:
                              qDebug("songpos");
                              break;
                        default:
                              qDebug("type 0x%02x", ev->type);
                              break;
                        case SND_SEQ_EVENT_PORT_SUBSCRIBED:
                        case SND_SEQ_EVENT_SENSING:
                              break;
                        }
                  }
            snd_seq_free_event(ev);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void AlsaMidiDriver::write(const Event& e)
      {
      MasterScore* cs = mscore->currentScore()->masterScore();
      int port  = cs->midiPort(e.channel());
      int chn   = cs->midiChannel(e.channel());
      int a     = e.dataA();
      int b     = e.dataB();

      if (midiOutputTrace)
            qDebug("midiOut: %2d %02x %02x %02x", port, e.type(), a, b);

      snd_seq_event_t event;
      memset(&event, 0, sizeof(event));
      snd_seq_ev_set_direct(&event);
      if (port >= midiOutPorts.size())
            port = 0;
      snd_seq_ev_set_source(&event, midiOutPorts[port].alsaPort());
      snd_seq_ev_set_dest(&event, SND_SEQ_ADDRESS_SUBSCRIBERS, 0);

      switch(e.type()) {
            case ME_NOTEON:
                  snd_seq_ev_set_noteon(&event, chn, a, b);
                  break;
            case ME_NOTEOFF:
                  snd_seq_ev_set_noteoff(&event, chn, a, 0);
                  break;
            case ME_PROGRAM:
                  snd_seq_ev_set_pgmchange(&event, chn, a);
                  break;
            case ME_CONTROLLER:
                  snd_seq_ev_set_controller(&event, chn, a, b);
                  break;
            case ME_PITCHBEND:
                  snd_seq_ev_set_pitchbend(&event, chn, a);
                  break;
            case ME_POLYAFTER:
                  // chnEvent2(chn, 0xa0, a, b);
                  break;
            case ME_AFTERTOUCH:
                  snd_seq_ev_set_chanpress(&event, chn, a);
                  break;
            case ME_SYSEX:
                  {
#if 0
                  SysexEvent* se         = (SysexEvent*) e;
                  const unsigned char* p = se->data();
                  int n                  = se->len();
                  int len                = n + sizeof(event) + 2;
                  char buf[len];
                  event.type             = SND_SEQ_EVENT_SYSEX;
                  event.flags            = SND_SEQ_EVENT_LENGTH_VARIABLE;
                  event.data.ext.len     = n + 2;
                  event.data.ext.ptr  = (void*)(buf + sizeof(event));
                  memcpy(buf, &event, sizeof(event));
                  char* pp = buf + sizeof(event);
                  *pp++ = 0xf0;
                  memcpy(pp, p, n);
                  pp += n;
                  *pp = 0xf7;
                  putEvent(&event);
#endif
                  return;
                  }
            default:
                  qDebug("MidiAlsaDriver::putEvent(): event type 0x%02x not implemented", e.type());
                  return;
            }
      putEvent(&event);
      }

//---------------------------------------------------------
//   putEvent
//    return false if event is delivered
//---------------------------------------------------------

bool AlsaMidiDriver::putEvent(snd_seq_event_t* event)
      {
      int error;

      do {
            error   = snd_seq_event_output_direct(alsaSeq, event);
            int len = snd_seq_event_length(event);
            if (error == len) {
                  return false;
                  }
            if (error < 0) {
                  if (error == -12) {
                        return true;
                        }
                  else {
                        qDebug("MidiAlsaDevice::%p putEvent(): midi write error: %s",
                           this, snd_strerror(error));
                        //exit(-1);
                        }
                  }
            else
                  qDebug("MidiAlsaDevice::putEvent(): midi write returns %d, expected %d: %s",
                     error, len, snd_strerror(error));
            } while (error == -12);
      return true;
      }
}

#endif /* USE_ALSA */


