//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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
      inputId = -1;
      outputId = -1;
      timer = 0;
      inputStream = 0;
      outputStream = 0;
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
      inputId = getDeviceIn(preferences.getString(PREF_IO_PORTMIDI_INPUTDEVICE));
      if (inputId == -1)
            inputId  = Pm_GetDefaultInputDeviceID();

      if (inputId == pmNoDevice)
            return false;

      outputId = getDeviceOut(preferences.getString(PREF_IO_PORTMIDI_OUTPUTDEVICE)); // Note: allow init even if outputId == pmNoDevice, since input is more important than output.

      static const int DRIVER_INFO = 0;
      static const int TIME_INFO = 0;

      Pt_Start(20, 0, 0);      // timer started, 20 millisecond accuracy

      PmError error = Pm_OpenInput(&inputStream,
         inputId,
         (void*)DRIVER_INFO,
         preferences.getInt(PREF_IO_PORTMIDI_INPUTBUFFERCOUNT),
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

      if (outputId != pmNoDevice) {
            error = Pm_OpenOutput(&outputStream,
               outputId,
               (void*)DRIVER_INFO,
               preferences.getInt(PREF_IO_PORTMIDI_OUTPUTBUFFERCOUNT),
               ((PmTimeProcPtr) Pt_Time),
               (void*)TIME_INFO,
               preferences.getInt(PREF_IO_PORTMIDI_OUTPUTLATENCYMILLISECONDS));
            if (error != pmNoError) {
                  const char* p = Pm_GetErrorText(error);
                  qDebug("PortMidi: open output (id=%d) failed: %s", int(outputId), p);
                  Pt_Stop();
                  return false;
                  }
            }

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
      if (!inputStream)
            return;
      PmEvent buffer[1];
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
                il.append(QString(info->interf) + "," + QString(info->name));
            }
      return il;
      }

//---------------------------------------------------------
//   deviceOutList
//---------------------------------------------------------

QStringList PortMidiDriver::deviceOutList() const
      {
      QStringList ol;
      int interf = Pm_CountDevices();
      for (PmDeviceID id = 0; id < interf; id++) {
            const PmDeviceInfo* info = Pm_GetDeviceInfo((PmDeviceID)id);
            if(info->output)
                ol.append(QString(info->interf) + "," + QString(info->name));
            }
      return ol;
      }

//---------------------------------------------------------
//   getDeviceIn
//---------------------------------------------------------

int PortMidiDriver::getDeviceIn(const QString& interfaceAndName)
      {
      int interf = Pm_CountDevices();
      for (int id = 0; id < interf; id++) {
            const PmDeviceInfo* info = Pm_GetDeviceInfo((PmDeviceID)id);
            if (info->input) {
                  if (QString(info->interf) + "," + QString(info->name) == interfaceAndName)
                        return id;
                  }
            }
      return -1;
      }

//---------------------------------------------------------
//   getDeviceOut
//---------------------------------------------------------

int PortMidiDriver::getDeviceOut(const QString& interfaceAndName)
      {
      int interf = Pm_CountDevices();
      for (int id = 0; id < interf; id++) {
            const PmDeviceInfo* info = Pm_GetDeviceInfo((PmDeviceID)id);
            if (info->output) {
                  if (QString(info->interf) + "," + QString(info->name) == interfaceAndName)
                        return id;
                  }
            }
      return -1;
      }

//---------------------------------------------------------
//   isSameCoreMidiIacBus
//    determines if both the input and output devices are the same shared CoreMIDI "IAC" bus
//---------------------------------------------------------

bool PortMidiDriver::isSameCoreMidiIacBus(const QString& inInterfaceAndName, const QString& outInterfaceAndName)
      {
      int interf = Pm_CountDevices();
      const PmDeviceInfo* inInfo = 0;
      const PmDeviceInfo* outInfo = 0;
      for (PmDeviceID id = 0; id < interf; id++) {
            const PmDeviceInfo* info = Pm_GetDeviceInfo((PmDeviceID)id);
            if (info->input && inInterfaceAndName.contains(info->name))
                  inInfo = info;
            if (info->output && outInterfaceAndName.contains(info->name))
                  outInfo = info;
            }

      if (inInfo && outInfo &&
          QString(inInfo->interf) == "CoreMIDI" && QString(outInfo->interf) == "CoreMIDI" &&
          inInterfaceAndName.contains("IAC") && outInterfaceAndName == inInterfaceAndName)
            return true;
      else
            return false;
      }
}

