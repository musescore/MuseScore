//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#include "winmidiinport.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include "log.h"
#include "midierrors.h"

struct mu::midi::WinMidiInPort::Win {
    HMIDIIN midiIn;
    int deviceID = -1;
};

using namespace mu::midi;

static std::string errorString(MMRESULT ret)
{
    switch (ret) {
    case MMSYSERR_NOERROR: return "MMSYSERR_NOERROR";
    case MIDIERR_NODEVICE: return "MIDIERR_NODEVICE";
    case MMSYSERR_ALLOCATED: return "MMSYSERR_ALLOCATED";
    case MMSYSERR_BADDEVICEID: return "MMSYSERR_BADDEVICEID";
    case MMSYSERR_INVALPARAM: return "MMSYSERR_INVALPARAM";
    case MMSYSERR_NOMEM: return "MMSYSERR_NOMEM";
    }

    return "UNKNOWN";
}

WinMidiInPort::WinMidiInPort()
{
    m_win = std::unique_ptr<Win>(new Win());
}

WinMidiInPort::~WinMidiInPort()
{
    if (isRunning()) {
        stop();
    }

    if (isConnected()) {
        disconnect();
    }
}

std::vector<MidiDevice> WinMidiInPort::devices() const
{
    std::vector<MidiDevice> ret;

    unsigned int numDevs = midiInGetNumDevs();
    if (numDevs == 0) {
        return ret;
    }

    for (unsigned int i = 0; i < numDevs; i++) {
        MIDIINCAPSW devCaps;
        midiInGetDevCapsW(i, &devCaps, sizeof(MIDIINCAPSW));

        std::wstring wstr(devCaps.szPname);
        std::string str(wstr.begin(), wstr.end());

        MidiDevice dev;
        dev.id = std::to_string(i);
        dev.name = str;

        ret.push_back(std::move(dev));
    }

    return ret;
}

static void CALLBACK proccess(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    UNUSED(hMidiIn);

    WinMidiInPort* self = reinterpret_cast<WinMidiInPort*>(dwInstance);
    switch (wMsg) {
    case MIM_OPEN:
    case MIM_CLOSE:
        break;
    case MIM_DATA:
        self->doProcess(static_cast<uint32_t>(dwParam1), static_cast<tick_t>(dwParam2));
        break;
    default:
        NOT_IMPLEMENTED << wMsg;
    }
}

void WinMidiInPort::doProcess(uint32_t message, tick_t timing)
{
    auto e = Event::fromMIDI10Package(message).toMIDI20();
    if (e) {
        m_eventReceived.send({ timing, e });
    }
}

mu::Ret WinMidiInPort::connect(const MidiDeviceID& deviceID)
{
    if (isConnected()) {
        disconnect();
    }

    m_win->deviceID = std::stoi(deviceID);
    MMRESULT ret = midiInOpen(&m_win->midiIn, m_win->deviceID,
                              reinterpret_cast<DWORD_PTR>(&proccess),
                              reinterpret_cast<DWORD_PTR>(this),
                              CALLBACK_FUNCTION | MIDI_IO_STATUS);

    if (ret != MMSYSERR_NOERROR) {
        return make_ret(Err::MidiFailedConnect, "failed open port, error: " + errorString(ret));
    }

    m_deviceID = deviceID;
    return Ret(true);
}

void WinMidiInPort::disconnect()
{
    if (!isConnected()) {
        return;
    }

    midiInClose(m_win->midiIn);

    m_win->midiIn = nullptr;
    m_win->deviceID = -1;

    m_deviceID.clear();
}

bool WinMidiInPort::isConnected() const
{
    return !m_deviceID.empty();
}

MidiDeviceID WinMidiInPort::deviceID() const
{
    return m_deviceID;
}

mu::Ret WinMidiInPort::run()
{
    if (!isConnected()) {
        return make_ret(Err::MidiNotConnected);
    }

    if (m_running) {
        LOGW() << "already started";
        return true;
    }

    midiInStart(m_win->midiIn);
    m_running = true;

    return Ret(true);
}

void WinMidiInPort::stop()
{
    if (!isConnected()) {
        return;
    }

    if (!m_running) {
        LOGW() << "already stoped";
        return;
    }

    midiInStop(m_win->midiIn);
    m_running = false;
}

bool WinMidiInPort::isRunning() const
{
    return m_running;
}

mu::async::Channel<std::pair<tick_t, Event> > WinMidiInPort::eventReceived() const
{
    return m_eventReceived;
}
