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

#ifndef MU_AUDIO_ISYNTHESIZER_H
#define MU_AUDIO_ISYNTHESIZER_H

#include "async/channel.h"
#include "io/path.h"
#include "ret.h"
#include "midi/miditypes.h"

#include "synthtypes.h"
#include "audiotypes.h"
#include "iaudiosource.h"

namespace mu::audio::synth {
class ISynthesizer : public IAudioSource
{
public:
    virtual ~ISynthesizer() = default;

    virtual bool isValid() const = 0;

    virtual std::string name() const = 0;
    virtual AudioSourceType type() const = 0;
    virtual const audio::AudioInputParams& params() const = 0;
    virtual async::Channel<audio::AudioInputParams> paramsChanged() const = 0;

    virtual Ret init() = 0;

    virtual Ret setupSound(const std::vector<midi::Event>& events) = 0;
    virtual bool handleEvent(const midi::Event& e) = 0;
    virtual void flushSound() = 0;
};

using ISynthesizerPtr = std::shared_ptr<ISynthesizer>;
}

#endif // MU_AUDIO_ISYNTHESIZER_H
