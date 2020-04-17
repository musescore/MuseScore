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

#ifndef __PORTMIDI_H__
#define __PORTMIDI_H__

#include "config.h"
#include "mididriver.h"

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
  #include "portmidi/pm_common/portmidi.h"
#else
  #include <portmidi.h>
#endif

namespace Ms {

class Seq;

//---------------------------------------------------------
//   PortMidiDriver
//---------------------------------------------------------

class PortMidiDriver : public MidiDriver {
      int inputId;
      int outputId;
      QTimer* timer;
      PmStream* inputStream;
      PmStream* outputStream;

   public:
      PortMidiDriver(Seq*);
      virtual ~PortMidiDriver();
      virtual bool init();
      virtual Port registerOutPort(const QString& name);
      virtual Port registerInPort(const QString& name);
      virtual void getInputPollFd(struct pollfd**, int* n);
      virtual void getOutputPollFd(struct pollfd**, int* n);
      virtual void read();
      virtual void write(const Event&);
      QStringList deviceInList() const;
      QStringList deviceOutList() const;
      int getDeviceIn(const QString& interfaceAndName);
      int getDeviceOut(const QString& interfaceAndName);
      PmStream* getInputStream() { return inputStream; }
      PmStream* getOutputStream() { return outputStream; }
      bool canOutput() { return outputStream != 0; }
      bool isSameCoreMidiIacBus(const QString& inInterfaceAndName, const QString& outInterfaceAndName);
      };


} // namespace Ms
#endif


