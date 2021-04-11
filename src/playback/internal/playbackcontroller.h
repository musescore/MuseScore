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
#include "../iplaybackconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "retval.h"
#include "async/asyncable.h"
#include "audio/isequencer.h"

namespace mu::playback {
class PlaybackController : public IPlaybackController, public actions::Actionable, public async::Asyncable
{
    INJECT(playback, actions::IActionsDispatcher, dispatcher)
    INJECT(playback, context::IGlobalContext, globalContext)
    INJECT(playback, IPlaybackConfiguration, configuration)
    INJECT(playback, notation::INotationConfiguration, notationConfiguration)
    INJECT(playback, audio::ISequencer, sequencer)

public:
    void init();

    bool isPlayAllowed() const override;
    async::Notification isPlayAllowedChanged() const override;

    bool isPlaying() const override;
    async::Notification isPlayingChanged() const override;

    async::Notification playbackPositionChanged() const override;
    async::Channel<uint32_t> midiTickPlayed() const override;
    float playbackPositionInSeconds() const override;

    void playElementOnClick(const notation::Element* element) override;

    bool actionChecked(const actions::ActionCode& actionCode) const override;
    async::Channel<actions::ActionCode> actionCheckedChanged() const override;

    QTime totalPlayTime() const override;

    notation::Tempo currentTempo() const override;
    notation::MeasureBeat currentBeat() const override;
    uint64_t beatToMilliseconds(int measureIndex, int beatIndex) const override;

private:
    static constexpr unsigned int MIDI_TRACK = 0;

    notation::INotationPlaybackPtr playback() const;
    notation::INotationSelectionPtr selection() const;

    int currentTick() const;
    bool isPaused() const;

    bool isLoopVisible() const;
    bool isPlaybackLooped() const;

    void onNotationChanged();
    void togglePlay();
    void rewind(const actions::ActionData& args);
    void play();
    void seek(int tick);
    void pause();
    void resume();

    void togglePlayRepeats();
    void toggleAutomaticallyPan();
    void toggleMetronome();
    void toggleMidiInput();
    void toggleCountIn();
    void toggleLoopPlayback();

    void addLoopBoundary(notation::LoopBoundaryType type);
    void addLoopBoundaryToTick(notation::LoopBoundaryType type, int tick);

    void setLoop(const notation::LoopBoundaries& boundaries);

    void showLoop();
    void hideLoop();

    void notifyActionCheckedChanged(const actions::ActionCode& actionCode);

    notation::INotationPtr m_notation;
    async::Notification m_isPlayAllowedChanged;
    async::Notification m_isPlayingChanged;
    async::Notification m_playbackPositionChanged;
    async::Channel<uint32_t> m_tickPlayed;
    async::Channel<actions::ActionCode> m_actionCheckedChanged;

    bool m_needRewindBeforePlay = false;
};
}

#endif // MU_PLAYBACK_PLAYBACKCONTROLLER_H
