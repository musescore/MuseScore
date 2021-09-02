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

    void playElement(const notation::EngravingItem* e) override;
};
}

#endif // MU_PLAYBACK_PLAYBACKCONTROLLERSTUB_H
