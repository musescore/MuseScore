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

#ifndef MU_AUDIO_SOUNDTRACKWRITER_H
#define MU_AUDIO_SOUNDTRACKWRITER_H

#include <vector>
#include <cstdio>

#include "async/asyncable.h"
#include "modularity/ioc.h"

#include "audio/iaudioconfiguration.h"
#include "audiotypes.h"
#include "iaudiosource.h"
#include "internal/encoders/abstractaudioencoder.h"

namespace mu::audio::soundtrack {
class SoundTrackWriter : public async::Asyncable
{
    INJECT_STATIC(IAudioConfiguration, config)
public:
    SoundTrackWriter(const io::path_t& destination, const SoundTrackFormat& format, const msecs_t totalDuration, IAudioSourcePtr source);

    Ret write();
    void abort();

    framework::Progress progress();

private:
    encode::AbstractAudioEncoderPtr createEncoder(const SoundTrackType& type) const;
    Ret generateAudioData();

    void sendStepProgress(int step, int64_t current, int64_t total);

    IAudioSourcePtr m_source = nullptr;

    std::vector<float> m_inputBuffer;
    std::vector<float> m_intermBuffer;

    encode::AbstractAudioEncoderPtr m_encoderPtr = nullptr;

    framework::Progress m_progress;
    std::atomic<bool> m_isAborted = false;
};
}

#endif // MU_AUDIO_SOUNDTRACKWRITER_H
