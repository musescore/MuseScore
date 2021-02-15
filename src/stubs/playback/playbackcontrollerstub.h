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
#ifndef MU_PLAYBACK_PLAYBACKCONTROLLERSTUB_H
#define MU_PLAYBACK_PLAYBACKCONTROLLERSTUB_H

#include "playback/iplaybackcontroller.h"

namespace mu::playback {
class PlaybackControllerStub : public IPlaybackController
{
public:
    bool isPlayAllowed() const override;
    async::Notification isPlayAllowedChanged() const override;

    bool isPlaying() const override;
    async::Notification isPlayingChanged() const override;

    float playbackPosition() const override;
    async::Channel<uint32_t> midiTickPlayed() const override;

    void playElementOnClick(const notation::Element* e) override;
};
}

#endif // MU_PLAYBACK_PLAYBACKCONTROLLERSTUB_H
