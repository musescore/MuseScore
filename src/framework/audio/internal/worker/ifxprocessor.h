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
#ifndef MU_AUDIO_IAUDIOPROCESSOR_H
#define MU_AUDIO_IAUDIOPROCESSOR_H

#include <memory>

#include "audiotypes.h"

namespace mu::audio {
class IFxProcessor
{
public:
    virtual ~IFxProcessor() = default;

    virtual FxProcessorId id() const = 0;

    virtual void setSampleRate(unsigned int sampleRate) = 0;

    virtual bool active() const = 0;
    virtual void setActive(bool active) = 0;

    //! return streams count for this insert: 1 for mono, 2 for stereo
    virtual unsigned int streamCount() const = 0;

    virtual void process(float* input, float* output, unsigned int sampleCount) = 0;
};

using IFxProcessorPtr = std::shared_ptr<IFxProcessor>;
}

#endif // MU_AUDIO_IAUDIOPROCESSOR_H
