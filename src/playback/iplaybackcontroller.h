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
#ifndef MU_PLAYBACK_IPLAYBACKCONTROLLER_H
#define MU_PLAYBACK_IPLAYBACKCONTROLLER_H

#include "modularity/imoduleexport.h"
#include "async/notification.h"
#include "async/channel.h"

#include "notation/notationtypes.h"

namespace mu {
namespace playback {
class IPlaybackController : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IPlaybackController)

public:
    virtual ~IPlaybackController() = default;

    virtual bool isPlayAllowed() const = 0;
    virtual async::Notification isPlayAllowedChanged() const = 0;

    virtual bool isPlaying() const = 0;
    virtual async::Notification isPlayingChanged() const = 0;

    virtual float playbackPosition() const = 0;
    virtual async::Channel<uint32_t> midiTickPlayed() const = 0;

    virtual void playElementOnClick(const notation::Element* e) = 0;
};
}
}

#endif // MU_PLAYBACK_IPLAYBACKCONTROLLER_H
