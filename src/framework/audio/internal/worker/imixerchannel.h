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
#ifndef MU_AUDIO_IMIXERCHANNEL_H
#define MU_AUDIO_IMIXERCHANNEL_H

#include <complex>
#include <memory>
#include "async/asyncable.h"
#include "iaudiosource.h"
#include "iaudioprocessor.h"

namespace mu::audio {
class IMixerChannel
{
public:
    virtual ~IMixerChannel() = default;

    virtual void setSource(std::shared_ptr<IAudioSource> source) = 0;

    virtual bool active() const = 0;
    virtual void setActive(bool active) = 0;

    virtual float level(unsigned int streamId) const = 0;
    virtual void setLevel(float level) = 0;
    virtual void setLevel(unsigned int streamId, float level) = 0;

    virtual std::complex<float> balance(unsigned int streamId) const = 0;
    virtual void setBalance(std::complex<float> value) = 0;
    virtual void setBalance(unsigned int streamId, std::complex<float> value) = 0;

    virtual IAudioProcessorPtr processor(unsigned int number) const = 0;
    virtual void setProcessor(unsigned int number, IAudioProcessorPtr proc) = 0;
};
}

#endif // MU_AUDIO_IMIXERCHANNEL_H
