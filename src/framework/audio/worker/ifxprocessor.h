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
#ifndef MUSE_AUDIO_IAUDIOPROCESSOR_H
#define MUSE_AUDIO_IAUDIOPROCESSOR_H

#include <memory>

#include "audio/common/audiotypes.h"

namespace muse::audio {
class IFxProcessor
{
public:
    virtual ~IFxProcessor() = default;

    virtual AudioFxType type() const = 0;
    virtual const AudioFxParams& params() const = 0;
    virtual async::Channel<audio::AudioFxParams> paramsChanged() const = 0;
    virtual void setSampleRate(unsigned int sampleRate) = 0;

    virtual bool active() const = 0;
    virtual void setActive(bool active) = 0;

    virtual void process(float* buffer, unsigned int sampleCount) = 0;
};

using IFxProcessorPtr = std::shared_ptr<IFxProcessor>;
}

#endif // MUSE_AUDIO_IAUDIOPROCESSOR_H
