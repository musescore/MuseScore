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

#ifndef __DRIVER_H__
#define __DRIVER_H__

namespace Ms {

class Seq;
class NPlayEvent;
enum class Transport : char;

//---------------------------------------------------------
//   Driver
//---------------------------------------------------------

class Driver {

   protected:
      Seq* seq;

   public:
      Driver(Seq* s)    { seq = s; }
      virtual ~Driver() {}
      virtual bool init(bool hot = false) = 0;
      virtual bool start(bool hotPlug = false) = 0;
      virtual bool stop() = 0;
      virtual void stopTransport() = 0;
      virtual void startTransport() = 0;
      virtual Transport getState() = 0;
      virtual void seekTransport(int) {}
      virtual int sampleRate() const = 0;
      virtual void putEvent(const NPlayEvent&, unsigned /*framePos*/) {}
      virtual void midiRead() {}
      virtual void handleTimeSigTempoChanged() {}
      virtual void checkTransportSeek(int, int, bool) {}
      virtual int bufferSize() {return 0;}
      virtual void updateOutPortCount(int) {}
      };

extern bool alsaIsUsed, jackIsUsed, portAudioIsUsed, pulseAudioIsUsed;

} // namespace Ms
#endif

