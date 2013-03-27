//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: driver.h 5660 2012-05-22 14:17:39Z wschweer $
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

class Seq;
class Event;

//---------------------------------------------------------
//   Driver
//---------------------------------------------------------

class Driver {

   protected:
      Seq* seq;

   public:
      Driver(Seq* s)    { seq = s; }
      virtual ~Driver() {}
      virtual bool init() = 0;
      virtual bool start() = 0;
      virtual bool stop() = 0;
      virtual QList<QString> inputPorts() { return QList<QString>(); }
      virtual void stopTransport() = 0;
      virtual void startTransport() = 0;
      virtual int getState() = 0;
      virtual int sampleRate() const = 0;
      virtual void registerPort(const QString& /*name*/, bool /*input*/, bool /*midi*/) {}
      virtual void unregisterPort(int) {}
      virtual void putEvent(const Event&, unsigned /*framePos*/) {}
      virtual void midiRead() {}
      };

#endif

