/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "winmidiinport.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#ifdef ERROR
#undef ERROR
#endif

#include "log.h"
#include "midierrors.h"

struct mu::midi::WinMidiInPort::Win {
    HMIDIIN midiIn;
    int deviceID = -1;
};

using namespace mu::midi;

namespace wmidi_prv {
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
}

WinMidiInPort::~WinMidiInPort()
{
    if (isConnected()) {
        disconnect();
    }
}

void WinMidiInPort::init()
{
    m_win = std::make_shared<Win>();

    m_devicesListener.startWithCallback([this]() {
        return devices();
    });

    m_devicesListener.devicesChanged().onNotify(this, [this]() {
        m_devicesChanged.notify();
    });
}

MidiDeviceList WinMidiInPort::devices() const
{
    std::lock_guard lock(m_devicesMutex);
    MidiDeviceList ret;

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

mu::async::Notification WinMidiInPort::devicesChanged() const
{
    return m_devicesChanged;
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
        m_eventReceived.send(timing, e);
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
        return make_ret(Err::MidiFailedConnect, "failed open port, error: " + wmidi_prv::errorString(ret));
    }

    m_deviceID = deviceID;
    return run();
}

void WinMidiInPort::disconnect()
{
    if (!isConnected()) {
        return;
    }

    midiInClose(m_win->midiIn);

    stop();

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

mu::async::Channel<tick_t, Event> WinMidiInPort::eventReceived() const
{
    return m_eventReceived;
}
