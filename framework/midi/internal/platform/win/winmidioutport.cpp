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
#include "winmidioutport.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include "log.h"
#include "../../midiparser.h"

struct mu::midi::WinMidiOutPort::Win {
    HMIDIOUT midiOut;
    int deviceID;
};

using namespace mu::midi;

WinMidiOutPort::WinMidiOutPort()
{
    m_win = new Win();
}

WinMidiOutPort::~WinMidiOutPort()
{
    delete m_win;
}

std::vector<IMidiOutPort::Device> WinMidiOutPort::devices() const
{
    std::vector<Device> ret;

    int numDevs = midiOutGetNumDevs();
    if (numDevs == 0) {
        return ret;
    }

    for (int i = 0; i < numDevs; i++) {
        Device dev;
        MIDIOUTCAPSW devCaps;
        midiOutGetDevCapsW(i, &devCaps, sizeof(MIDIOUTCAPSW));

        std::wstring wstr(devCaps.szPname);
        std::string str(wstr.begin(), wstr.end());

        dev.id = std::to_string(i);
        dev.name = str;

        ret.push_back(std::move(dev));
    }

    return ret;
}

bool WinMidiOutPort::connect(const std::string& deviceID)
{
    if (m_isConnected) {
        disconnect();
    }

    auto errorString = [](MMRESULT ret) {
        switch (ret) {
        case MMSYSERR_NOERROR: return "MMSYSERR_NOERROR";
        case MIDIERR_NODEVICE: return "MIDIERR_NODEVICE";
        case MMSYSERR_ALLOCATED: return "MMSYSERR_ALLOCATED";
        case MMSYSERR_BADDEVICEID: return "MMSYSERR_BADDEVICEID";
        case MMSYSERR_INVALPARAM: return "MMSYSERR_INVALPARAM";
        case MMSYSERR_NOMEM: return "MMSYSERR_NOMEM";
        }

        return  "UNKNOWN";
    };

    m_win->deviceID = std::stoi(deviceID);
    MMRESULT ret = midiOutOpen(&m_win->midiOut, m_win->deviceID, 0, 0, CALLBACK_NULL);
    if (ret != MMSYSERR_NOERROR) {
        LOGE() << "failed open port, error: " << errorString(ret);
        return false;
    }

    m_isConnected = true;
    return true;
}

void WinMidiOutPort::disconnect()
{
    if (!m_isConnected) {
        return;
    }

    midiOutClose(m_win->midiOut);
    m_isConnected = false;
    m_win->deviceID = -1;
}

void WinMidiOutPort::sendEvent(const Event& e)
{
    if (!m_isConnected) {
        return;
    }

    uint32_t msg = MidiParser::message(e);
    midiOutShortMsg(m_win->midiOut, (DWORD)msg);
}

