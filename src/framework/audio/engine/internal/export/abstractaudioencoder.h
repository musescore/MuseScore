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

namespace muse::io {
class IODevice;
}

namespace muse::audio::encode {
class AbstractAudioEncoder
{
public:
    AbstractAudioEncoder() = default;

    virtual ~AbstractAudioEncoder() = default;

    virtual bool init(io::IODevice& dstDevice, const SoundTrackFormat& format, const samples_t totalSamplesNumber)
    {
        UNUSED(dstDevice);
        UNUSED(totalSamplesNumber);

        if (!format.isValid()) {
            return false;
        }

        m_format = format;

        return true;
    }

    virtual void deinit() = 0;

    const SoundTrackFormat& format() const
    {
        return m_format;
    }

    virtual size_t encode(samples_t samplesPerChannel, const float* input) = 0;
    virtual size_t flush() = 0;

    Progress progress()
    {
        return m_progress;
    }

protected:
    SoundTrackFormat m_format;
    Progress m_progress;
};

using AbstractAudioEncoderPtr = std::unique_ptr<AbstractAudioEncoder>;
}
