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
#include "audiodriverstub.h"

using namespace mu::audio;

std::string AudioDriverStub::name() const
{
    return std::string();
}

bool AudioDriverStub::open(const IAudioDriver::Spec&, IAudioDriver::Spec*)
{
    return false;
}

void AudioDriverStub::close()
{
}

bool AudioDriverStub::isOpened() const
{
    return false;
}

std::string AudioDriverStub::outputDevice() const
{
    return std::string();
}

bool AudioDriverStub::selectOutputDevice(const std::string&)
{
    return false;
}

std::vector<std::string> AudioDriverStub::availableOutputDevices() const
{
    return {};
}

mu::async::Notification AudioDriverStub::availableOutputDevicesChanged() const
{
    return async::Notification();
}

void AudioDriverStub::resume()
{
}

void AudioDriverStub::suspend()
{
}
