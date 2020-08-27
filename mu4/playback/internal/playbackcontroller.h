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
#ifndef MU_PLAYBACK_PLAYBACKCONTROLLER_H
#define MU_PLAYBACK_PLAYBACKCONTROLLER_H

#include "../iplaybackcontroller.h"
#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "context/iglobalcontext.h"
#include "audio/iaudioplayer.h"
#include "../iplaybackconfiguration.h"
#include "retval.h"
#include "async/asyncable.h"

namespace mu {
namespace playback {
class PlaybackController : public IPlaybackController, public actions::Actionable, public async::Asyncable
{
    INJECT(playback, actions::IActionsDispatcher, dispatcher)
    INJECT(playback, context::IGlobalContext, globalContext)
    INJECT(playback, audio::IAudioPlayer, audioPlayer)
    INJECT(playback, IPlaybackConfiguration, configuration)

public:

    void init();

    bool isPlayAllowed() const override;
    async::Notification isPlayAllowedChanged() const override;

    bool isPlaying() const override;
    async::Notification isPlayingChanged() const override;

    float playbackPosition() const override;
    async::Channel<uint32_t> midiTickPlayed() const override;

    void playElementOnClick(const notation::Element* e) override;

private:

    void onNotationChanged();
    void togglePlay();
    void play();
    void seek(int tick);
    void stop();

    notation::INotationPtr m_notation;
    async::Notification m_isPlayAllowedChanged;
    async::Notification m_isPlayingChanged;
};
}
}

#endif // MU_PLAYBACK_PLAYBACKCONTROLLER_H
