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
#include "webaudiodriver.h"
#include "log.h"
using namespace mu::audio;

WebAudioDriver::WebAudioDriver()
{
}

std::string WebAudioDriver::name() const
{
    return "MUAUDIO(WEB)";
}

bool WebAudioDriver::open(const Spec& spec, Spec* activeSpec)
{
    //TODO:
    m_opened = true;
    return m_opened;
}

void WebAudioDriver::close()
{
    //TODO:
}

bool WebAudioDriver::isOpened() const
{
    return m_opened;
}

std::string WebAudioDriver::device() const
{
    NOT_IMPLEMENTED;
    return "default";
}

bool WebAudioDriver::selectDevice(std::string name)
{
    NOT_IMPLEMENTED;
    return false;
}

std::vector<std::string> WebAudioDriver::availableDevices() const
{
    NOT_IMPLEMENTED;
    return { "default" };
}

mu::async::Notification WebAudioDriver::deviceListChanged() const
{
    NOT_IMPLEMENTED;
    return mu::async::Notification();
}
