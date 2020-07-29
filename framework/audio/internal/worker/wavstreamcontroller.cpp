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

#include "wavstreamcontroller.h"

#include "log.h"

using namespace mu::audio;
using namespace mu::audio::worker;

std::shared_ptr<IAudioSource> WavStreamController::makeSource(const StreamID& id, const std::string& name) const
{
    NOT_IMPLEMENTED;
    UNUSED(id);
    UNUSED(name);
    return nullptr;
}

void WavStreamController::load(const StreamID& id, const std::string& url, uint16_t trackNum,
                               const std::function<void(bool success)>& onLoaded)
{
    NOT_IMPLEMENTED;
    UNUSED(id);
    UNUSED(url);
    UNUSED(trackNum);
    UNUSED(onLoaded);
}
