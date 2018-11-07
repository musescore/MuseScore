//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2008-2009 Werner Schweer and others
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

#ifndef __MIDIDRIVER_H__
#define __MIDIDRIVER_H__

#if !defined(Q_OS_WIN)
#include <poll.h>
#endif

#include "config.h"
#include "driver.h"

namespace Ms {

class Event;
class Seq;

//---------------------------------------------------------
//    Port
//---------------------------------------------------------

class Port {
      enum { ALSA_TYPE, ZERO_TYPE } type;
      unsigned char _alsaPort;
      unsigned char _alsaClient;

   protected:
      unsigned char alsaPort() const   { return _alsaPort; }
      unsigned char alsaClient() const { return _alsaClient; }

   public:
      Port();
      Port(unsigned char client, unsigned char port);
      void setZero();
      bool isZero() const;
      bool operator==(const Port& p) const;
      bool operator<(const Port& p) const;
      friend class MidiDriver;
      friend class AlsaMidiDriver;
      friend class PortMidiDriver;
      };

//---------------------------------------------------------
//   MidiDriver
//---------------------------------------------------------

class MidiDriver {
   protected:
      Port midiInPort;
      QList<Port> midiOutPorts;
      Seq* seq;

   public:
      MidiDriver(Seq* s) { seq = s; }
      virtual ~MidiDriver() {}
      virtual bool init() = 0;
      virtual void getInputPollFd(struct pollfd**, int* n) = 0;
      virtual void getOutputPollFd(struct pollfd**, int* n) = 0;
      virtual void read() = 0;
      virtual void write(const Event&) = 0;
      };

#ifdef USE_ALSA

//---------------------------------------------------------
//   AlsaPort
//---------------------------------------------------------

struct AlsaPort {
      unsigned char _alsaPort;
      unsigned char _alsaClient;
      };

#endif


} // namespace Ms

#endif

