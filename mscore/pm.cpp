//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: pm.cpp 4874 2011-10-21 12:18:42Z wschweer $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#if defined(Q_OS_WIN)
  #include <windows.h>
  #include <mmsystem.h>
#endif

#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
  #include "portmidi/porttime/porttime.h"
#else
  #include <porttime.h>
#endif

#include "preferences.h"
#include "pm.h"
#include "musescore.h"
#include "seq.h"

namespace Ms {

//---------------------------------------------------------
//   PortMidiDriver
//---------------------------------------------------------

PortMidiDriver::PortMidiDriver(Seq* s)
  : MidiDriver(s)
      {
      inputStream = 0;
      timer = 0;
      }

PortMidiDriver::~PortMidiDriver()
      {
      if (inputStream) {
            Pt_Stop();
            Pm_Close(inputStream);
            }
      }

//---------------------------------------------------------
//   init
//    return false on error
//---------------------------------------------------------

bool PortMidiDriver::init()
      {
      inputId = getDeviceIn(preferences.portMidiInput);
      if(inputId == -1)
        inputId  = Pm_GetDefaultInputDeviceID();
      outputId = Pm_GetDefaultOutputDeviceID();

      if (inputId == pmNoDevice)
            return false;

      static const int INPUT_BUFFER_SIZE = 100;
      static const int DRIVER_INFO = 0;
      static const int TIME_INFO = 0;

      Pt_Start(20, 0, 0);      // timer started, 20 millisecond accuracy

      PmError error = Pm_OpenInput(&inputStream,
         inputId,
         (void*)DRIVER_INFO, INPUT_BUFFER_SIZE,
         ((PmTimeProcPtr) Pt_Time),
         (void*)TIME_INFO);
      if (error != pmNoError) {
            const char* p = Pm_GetErrorText(error);
            qDebug("PortMidi: open input (id=%d) failed: %s", int(inputId), p);
            Pt_Stop();
            return false;
            }

      Pm_SetFilter(inputStream, PM_FILT_ACTIVE | PM_FILT_CLOCK | PM_FILT_SYSEX);

      PmEvent buffer[1];
      while (Pm_Poll(inputStream))
            Pm_Read(inputStream, buffer, 1);

      timer = new QTimer();
      timer->setInterval(20);       // 20 msec
      timer->start();
      timer->connect(timer, SIGNAL(timeout()), seq, SLOT(midiInputReady()));
      return true;
      }

//---------------------------------------------------------
//   registerOutPort
//---------------------------------------------------------

Port PortMidiDriver::registerOutPort(const QString&)
      {
      return Port();
      }

//---------------------------------------------------------
//   registerInPort
//---------------------------------------------------------

Port PortMidiDriver::registerInPort(const QString&)
      {
      return Port();
      }

//---------------------------------------------------------
//   getInputPollFd
//---------------------------------------------------------

void PortMidiDriver::getInputPollFd(struct pollfd**, int* n)
      {
      *n = 0;
      }

//---------------------------------------------------------
//   getOutputPollFd
//---------------------------------------------------------

void PortMidiDriver::getOutputPollFd(struct pollfd**, int* n)
      {
      *n = 0;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void PortMidiDriver::read()
      {
      static const int THRESHOLD = 3; // iterations required before consecutive drum notes are not considered part of a chord
      static int active = 0;
      static int iter = 0;
      //for some reason, some users could have "active" blocked in < 0
      if (active < 0)
        active = 0;
      if (!inputStream)
            return;
      PmEvent buffer[1];
      iter = (iter >= THRESHOLD) ? iter : (iter+1);
      while (Pm_Poll(inputStream)) {
            int n = Pm_Read(inputStream, buffer, 1);
            if (n > 0) {
                  int status  = Pm_MessageStatus(buffer[0].message);
                  int type    = status & 0xF0;
                  int channel = status & 0x0F;
                  if (type == ME_NOTEON) {
                        int pitch = Pm_MessageData1(buffer[0].message);
                        int velo = Pm_MessageData2(buffer[0].message);
                        mscore->midiNoteReceived(channel, pitch, velo);
                        }
                  else if (type == ME_NOTEOFF) {
                        int pitch = Pm_MessageData1(buffer[0].message);
                        (void)Pm_MessageData2(buffer[0].message); // read but ignore
                        mscore->midiNoteReceived(channel, pitch, 0);
                        }
                  else if (type == ME_CONTROLLER) {
                        int param = Pm_MessageData1(buffer[0].message);
                        int value = Pm_MessageData2(buffer[0].message);
                        mscore->midiCtrlReceived(param, value);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void PortMidiDriver::write(const Event&)
      {
      }

//---------------------------------------------------------
//   deviceInList
//---------------------------------------------------------

QStringList PortMidiDriver::deviceInList() const
      {
      QStringList il;
      int interf = Pm_CountDevices();
      for (PmDeviceID id = 0; id < interf; id++) {
            const PmDeviceInfo* info = Pm_GetDeviceInfo((PmDeviceID)id);
            if(info->input)
                il.append(QString(info->interf) +","+ QString(info->name));
            }
      return il;
      }

//---------------------------------------------------------
//   getDeviceIn
//---------------------------------------------------------

int PortMidiDriver::getDeviceIn(const QString& name)
      {
      int interf = Pm_CountDevices();
      for (int id = 0; id < interf; id++) {
            const PmDeviceInfo* info = Pm_GetDeviceInfo((PmDeviceID)id);
            if (info->input) {
                  QString n = QString(info->interf) + "," + QString(info->name);
                  if (n == name)
                        return id;
                  }
            }
      return -1;
      }
}

