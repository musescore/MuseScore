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
#include <functional>
#include <cstdio>

#include "audiotypes.h"
#include "iaudiosource.h"
#include "internal/encoders/abstractaudioencoder.h"

namespace mu::audio::soundtrack {
class SoundTrackWriter
{
public:
    SoundTrackWriter(const io::path &destination, const SoundTrackFormat& format, const msecs_t totalDuration, IAudioSourcePtr source);
    ~SoundTrackWriter();

    bool write();

private:
    using EncodeFunc = std::function<samples_t(const SoundTrackFormat&, samples_t, float*, char*)>;

    EncodeFunc encodeHandler() const;
    size_t requiredOutputBufferSize(const SoundTrackType type, const samples_t samplesPerChannel) const;
    bool prepareInputBuffer();
    bool writeEncodedOutput();
    void completeOutput();

    SoundTrackFormat m_format;
    IAudioSourcePtr m_source = nullptr;
    std::FILE* m_fileStream = nullptr;

    std::vector<float> m_inputBuffer;
    std::vector<float> m_intermBuffer;
    std::vector<char> m_outputBuffer;
};
}

#endif // MU_AUDIO_SOUNDTRACKWRITER_H
