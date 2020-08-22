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
#include "dummymidioutport.h"

#include "log.h"
#include "midierrors.h"

using namespace mu::midi;

std::vector<MidiDevice> DummyMidiOutPort::devices() const
{
    MidiDevice d;
    d.id = "dummy";
    d.name = "Dummy";
    return { d };
}

mu::Ret DummyMidiOutPort::connect(const std::string& deviceID)
{
    LOGI() << "deviceID: " << deviceID;
    m_connectedDeviceID = deviceID;
    return Ret(true);
}

void DummyMidiOutPort::disconnect()
{
    LOGI() << "disconnect";
    m_connectedDeviceID.clear();
}

bool DummyMidiOutPort::isConnected() const
{
    return !m_connectedDeviceID.empty();
}

std::string DummyMidiOutPort::deviceID() const
{
    return m_connectedDeviceID;
}

mu::Ret DummyMidiOutPort::sendEvent(const Event& e)
{
    if (!isConnected()) {
        return make_ret(Err::MidiNotConnected);
    }
    LOGI() << e.to_string();
    return Ret(true);
}
