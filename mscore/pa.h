//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: pa.h 4147 2011-04-07 14:39:44Z wschweer $
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

#ifndef __PORTAUDIO_H__
#define __PORTAUDIO_H__

#include "config.h"

namespace Ms {

class Synth;
class Seq;
class MidiDriver;
class NPlayEvent;
enum class Transport : char;

//---------------------------------------------------------
//   Portaudio
//---------------------------------------------------------

class Portaudio : public Driver {
      bool initialized;
      int _sampleRate;

      Transport state;
      bool seekflag;
      unsigned pos;
      double startTime;

      MidiDriver* midiDriver;

   public:
      Portaudio(Seq*);
      virtual ~Portaudio();
      virtual bool init(bool hot = false);
      virtual bool start(bool hotPlug = false);
      virtual bool stop();
      virtual void startTransport();
      virtual void stopTransport();
      virtual Transport getState() override;
      virtual int sampleRate() const { return _sampleRate; }
      virtual void midiRead();
#ifdef USE_PORTMIDI
      virtual void putEvent(const NPlayEvent&, unsigned framePos);
#endif

      int framePos() const;
      float* getLBuffer(long n);
      float* getRBuffer(long n);
      virtual bool isRealtime() const   { return false; }

      QStringList apiList() const;
      QStringList deviceList(int apiIdx);
      int deviceIndex(int apiIdx, int apiDevIdx);
      int currentApi() const;
      int currentDevice() const;
      MidiDriver* mididriver() { return midiDriver; }
      };


} // namespace Ms
#endif


