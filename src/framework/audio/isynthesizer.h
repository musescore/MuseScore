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
#ifndef MU_AUDIO_ISYNTHESIZERR_H
#define MU_AUDIO_ISYNTHESIZERR_H

#include <memory>

#include "iaudiosource.h"

namespace mu::audio::synth {
class ISynthesizer : public IAudioSource
{
public:
    virtual ~ISynthesizer() = default;

    virtual std::string name() const = 0;
    virtual AudioSourceType type() const = 0;
    virtual bool isValid() const = 0;

    virtual void setup(const mpe::PlaybackData& playbackData) = 0;

    virtual const audio::AudioInputParams& params() const = 0;
    virtual async::Channel<audio::AudioInputParams> paramsChanged() const = 0;

    virtual msecs_t playbackPosition() const = 0;
    virtual void setPlaybackPosition(const msecs_t newPosition) = 0;

    virtual void revokePlayingNotes() = 0;
    virtual void flushSound() = 0;
};

using ISynthesizerPtr = std::shared_ptr<ISynthesizer>;
}

#endif // MU_AUDIO_ISYNTHESIZERR_H
