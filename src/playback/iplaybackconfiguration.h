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
#ifndef MU_PLAYBACK_IPLAYBACKCONFIGURATION_H
#define MU_PLAYBACK_IPLAYBACKCONFIGURATION_H

#include "modularity/imoduleexport.h"
#include "retval.h"
#include "playbacktypes.h"

namespace mu::playback {
class IPlaybackConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IPlaybackConfiguration)

public:
    virtual ~IPlaybackConfiguration() = default;

    virtual bool playNotesWhenEditing() const = 0;
    virtual void setPlayNotesWhenEditing(bool value) = 0;

    virtual bool playChordWhenEditing() const = 0;
    virtual void setPlayChordWhenEditing(bool value) = 0;

    virtual bool playHarmonyWhenEditing() const = 0;
    virtual void setPlayHarmonyWhenEditing(bool value) = 0;

    virtual PlaybackCursorType cursorType() const = 0;
};
}

#endif // MU_PLAYBACK_IPLAYBACKCONFIGURATION_H
