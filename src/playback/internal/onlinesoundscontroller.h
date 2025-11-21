/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#pragma once

#include "actions/actionable.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"

#include "playback/iplaybackconfiguration.h"
#include "actions/iactionsdispatcher.h"
#include "global/iinteractive.h"
#include "audio/main/iplayback.h"

namespace mu::playback {
class OnlineSoundsController : public muse::actions::Actionable, public muse::async::Asyncable
{
    muse::Inject<IPlaybackConfiguration> configuration;
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher;
    muse::Inject<muse::IInteractive> interactive;
    muse::Inject<muse::audio::IPlayback> playback;

public:
    OnlineSoundsController();

    void regActions();

    void setCurrentSequence(muse::audio::TrackSequenceId seqId);
    void resetCurrentSequence();

    void addOnlineTrack(const muse::audio::TrackId trackId, const muse::audio::AudioResourceMeta& meta);
    void removeOnlineTrack(const muse::audio::TrackId trackId);

    bool shouldShowOnlineSoundsProcessingError(bool isPlaying) const;
    void showOnlineSoundsProcessingError(const std::function<void()>& onShown);

    const std::map<muse::audio::TrackId, muse::audio::AudioResourceMeta>& onlineSounds() const;
    muse::async::Notification onlineSoundsChanged() const;
    muse::Progress onlineSoundsProcessingProgress() const;

private:
    void listenProcessingProgress(const muse::audio::TrackId trackId);
    void showLimitReachedErrorIfNeed(const muse::audio::InputProcessingProgress::StatusInfo& status);

    void processOnlineSounds();
    void clearOnlineSoundsCache();

    muse::audio::TrackSequenceId m_currentSequenceId = -1;
    std::map<muse::audio::TrackId, muse::audio::AudioResourceMeta> m_onlineSounds;
    std::unordered_set<muse::audio::TrackId> m_onlineSoundsBeingProcessed;
    std::unordered_set<muse::String> m_onlineLibrariesWithExceededLimit;
    muse::async::Notification m_onlineSoundsChanged;
    muse::Progress m_onlineSoundsProcessingProgress;
    muse::Ret m_onlineSoundsProcessingRet;
};
}
