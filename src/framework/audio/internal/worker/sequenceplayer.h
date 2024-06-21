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

#ifndef MUSE_AUDIO_SEQUENCEPLAYER_H
#define MUSE_AUDIO_SEQUENCEPLAYER_H

#include "global/async/asyncable.h"

#include "modularity/ioc.h"
#include "iaudioengine.h"

#include "isequenceplayer.h"
#include "igettracks.h"
#include "iclock.h"

namespace muse::audio {
class SequencePlayer : public ISequencePlayer, public Injectable, public async::Asyncable
{
    Inject<IAudioEngine> audioEngine = { this };

public:
    explicit SequencePlayer(IGetTracks* getTracks, IClockPtr clock, const modularity::ContextPtr& iocCtx);

    void play() override;
    void seek(const secs_t newPosition) override;
    void stop() override;
    void pause() override;
    void resume() override;

    PlaybackStatus playbackStatus() const override;
    async::Channel<PlaybackStatus> playbackStatusChanged() const override;

    msecs_t duration() const override;
    void setDuration(const msecs_t duration) override;
    Ret setLoop(const msecs_t fromMsec, const msecs_t toMsec) override;
    void resetLoop() override;

    secs_t playbackPosition() const override;
    async::Channel<secs_t> playbackPositionChanged() const override;

private:
    void setAllTracksActive(bool active);
    void seekAllTracks(const msecs_t newPositionMsecs);

    IGetTracks* m_getTracks = nullptr;
    IClockPtr m_clock = nullptr;
};
}

#endif // MUSE_AUDIO_SEQUENCEPLAYER_H
