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
#ifndef MU_PLAYBACK_PLAYBACKCONFIGURATION_H
#define MU_PLAYBACK_PLAYBACKCONFIGURATION_H

#include "../iplaybackconfiguration.h"

namespace mu::playback {
class PlaybackConfiguration : public IPlaybackConfiguration
{
public:
    void init();

    bool playNotesWhenEditing() const override;
    void setPlayNotesWhenEditing(bool value) override;

    bool playChordWhenEditing() const override;
    void setPlayChordWhenEditing(bool value) override;

    bool playHarmonyWhenEditing() const override;
    void setPlayHarmonyWhenEditing(bool value) override;

    PlaybackCursorType cursorType() const override;
};
}

#endif // MU_PLAYBACK_PLAYBACKCONFIGURATION_H
