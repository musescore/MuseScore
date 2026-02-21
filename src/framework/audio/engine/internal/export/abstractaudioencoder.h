/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <cstddef>
#include <memory>

#include "global/progress.h"

#include "audio/common/audiotypes.h"

namespace muse::audio::encode {
class AbstractAudioEncoder
{
public:
    AbstractAudioEncoder(const AbstractAudioEncoder&) = delete;
    AbstractAudioEncoder(AbstractAudioEncoder&&) = delete;

    virtual ~AbstractAudioEncoder() noexcept = default;

    AbstractAudioEncoder& operator=(const AbstractAudioEncoder&) = delete;
    AbstractAudioEncoder& operator=(AbstractAudioEncoder&&) = delete;

    virtual bool begin(samples_t totalSamplesNumber) = 0;
    virtual size_t encode(samples_t samplesPerChannel, const float* input) = 0;
    virtual size_t end() = 0;

    const SoundTrackFormat& format() const
    {
        return m_format;
    }

    Progress progress()
    {
        return m_progress;
    }

protected:
    explicit AbstractAudioEncoder(const SoundTrackFormat& format)
        : m_format(format)
    {
    }

    SoundTrackFormat m_format;
    Progress m_progress;
};

using AbstractAudioEncoderPtr = std::unique_ptr<AbstractAudioEncoder>;
}
