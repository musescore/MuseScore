//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2009 Werner Schweer and others
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

#ifndef __ALSAMIDI_H__
#define __ALSAMIDI_H__

#include "mididriver.h"

namespace Ms {

struct PortName {
      Port port;
      QString name;
      };

//---------------------------------------------------------
//   AlsaMidiDriver
//---------------------------------------------------------

class AlsaMidiDriver : public MidiDriver {
      snd_seq_t* alsaSeq;

      bool putEvent(snd_seq_event_t* event);
      QList<PortName> outputPorts();
      QList<PortName> inputPorts();
      bool connect(Port src, Port dst);

   public:
      AlsaMidiDriver(Seq* s);
      virtual ~AlsaMidiDriver() {}
      virtual bool init();
      virtual Port registerOutPort(const QString& name);
      virtual Port registerInPort(const QString& name);
      virtual void getInputPollFd(struct pollfd**, int* n);
      virtual void getOutputPollFd(struct pollfd**, int* n);
      virtual void read();
      virtual void write(const Event&);
      virtual void updateInPortCount(int);
      };

}
#endif

