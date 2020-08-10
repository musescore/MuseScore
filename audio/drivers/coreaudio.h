//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2020 Musescore BVBA
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

#ifndef __COREAUDIO_H__
#define __COREAUDIO_H__

#include "config.h"
#include "driver.h"

#include <CoreAudio/CoreAudio.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>

namespace Ms {
class Synth;
class Seq;
class MidiDriver;

enum class Transport : char;

//---------------------------------------------------------
//   CoreAudio
//---------------------------------------------------------

class CoreAudio : public Driver
{
    bool initialized;
    float _sampleRate;

    Transport state;
    bool seekflag;
    unsigned pos;
    double startTime;

    MidiDriver* midiDriver;

    UInt32 currentOutputDevice;

    static OSStatus onDefaultOutputDeviceChanged(AudioObjectID inObjectID, UInt32 inNumberAddresses,
                                                 const AudioObjectPropertyAddress inAddresses[], void* paVoid);
    static OSStatus onSampleRateChanged(AudioObjectID inObjectID, UInt32 inNumberAddresses, const AudioObjectPropertyAddress inAddresses[],
                                        void* paVoid);

public:
    CoreAudio(Seq*);
    virtual ~CoreAudio();
    virtual bool init(bool hot = false);
    virtual bool start(bool hotPlug = false);
    virtual bool stop();
    virtual void startTransport();
    virtual void stopTransport();
    virtual Transport getState() override;
    virtual int sampleRate() const { return static_cast<int>(_sampleRate); }
    virtual void midiRead();

    int framePos() const;
    float* getLBuffer(long n);
    float* getRBuffer(long n);
    virtual bool isRealtime() const { return false; }

    QStringList deviceList();
    int currentDevice() const;
    MidiDriver* mididriver() { return midiDriver; }
};
}

#endif // __COREAUDIO_H__
