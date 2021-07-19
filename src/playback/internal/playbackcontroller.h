/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MU_PLAYBACK_PLAYBACKCONTROLLER_H
#define MU_PLAYBACK_PLAYBACKCONTROLLER_H

#include <unordered_map>

#include "modularity/ioc.h"
#include "retval.h"
#include "async/asyncable.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "context/iglobalcontext.h"
#include "notation/notationtypes.h"
#include "notation/inotationconfiguration.h"
#include "notation/inotationplayback.h"
#include "audio/iplayer.h"
#include "audio/itracks.h"
#include "audio/iplayback.h"
#include "audio/audiotypes.h"

#include "../iplaybackcontroller.h"
#include "../iplaybackconfiguration.h"

namespace mu::playback {
class PlaybackController : public IPlaybackController, public actions::Actionable, public async::Asyncable
{
    INJECT(playback, actions::IActionsDispatcher, dispatcher)
    INJECT(playback, context::IGlobalContext, globalContext)
    INJECT(playback, IPlaybackConfiguration, configuration)
    INJECT(playback, notation::INotationConfiguration, notationConfiguration)
    INJECT(playback, audio::IPlayback, playback)

public:
    void init();

    bool isPlayAllowed() const override;
    async::Notification isPlayAllowedChanged() const override;

    bool isPlaying() const override;
    async::Notification isPlayingChanged() const override;

    void reset() override;

    void seek(const midi::tick_t tick) override;
    void seek(const audio::msecs_t msecs) override;

    async::Notification playbackPositionChanged() const override;
    async::Channel<uint32_t> midiTickPlayed() const override;
    float playbackPositionInSeconds() const override;

    audio::TrackSequenceId currentTrackSequenceId() const override;
    async::Notification currentTrackSequenceIdChanged() const override;

    void playElement(const notation::Element* element) override;

    bool actionChecked(const actions::ActionCode& actionCode) const override;
    async::Channel<actions::ActionCode> actionCheckedChanged() const override;

    QTime totalPlayTime() const override;

    notation::Tempo currentTempo() const override;
    notation::MeasureBeat currentBeat() const override;
    audio::msecs_t beatToMilliseconds(int measureIndex, int beatIndex) const override;

private:
    notation::INotationPlaybackPtr notationPlayback() const;
    notation::INotationSelectionPtr selection() const;

    int currentTick() const;
    bool isPaused() const;

    bool isLoopVisible() const;
    bool isPlaybackLooped() const;

    void onNotationChanged();
    void togglePlay();
    void rewind(const actions::ActionData& args);
    void play();
    void pause();
    void stop();
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

    void setCurrentSequence(const audio::TrackSequenceId sequenceId);
    void setCurrentTick(const midi::tick_t tick);
    void addTrack(const notation::INotationPlayback::InstrumentTrackId& id);
    void removeTrack(const notation::INotationPlayback::InstrumentTrackId& id);

    notation::INotationPtr m_notation;
    async::Notification m_isPlayAllowedChanged;
    async::Notification m_isPlayingChanged;
    async::Notification m_playbackPositionChanged;
    async::Channel<uint32_t> m_tickPlayed;
    async::Channel<actions::ActionCode> m_actionCheckedChanged;

    bool m_needRewindBeforePlay = false;

    bool m_isPlaying = false;

    audio::TrackSequenceId m_currentSequenceId = -1;
    async::Notification m_currentSequenceIdChanged;
    audio::PlaybackStatus m_currentPlaybackStatus = audio::PlaybackStatus::Stopped;
    midi::tick_t m_currentTick = 0;
    std::unordered_map<std::string /* score file path*/, audio::TrackSequenceId> m_sequenceIdMap;
    std::unordered_map<notation::INotationPlayback::InstrumentTrackId, audio::TrackId> m_trackIdMap;
};
}

#endif // MU_PLAYBACK_PLAYBACKCONTROLLER_H
