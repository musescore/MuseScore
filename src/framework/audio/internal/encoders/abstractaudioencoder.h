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

#ifndef MUSE_AUDIO_ABSTRACTAUDIOENCODER_H
#define MUSE_AUDIO_ABSTRACTAUDIOENCODER_H

#include <cstdio>
#include <vector>
#include <memory>

#include "global/progress.h"
#include "global/io/path.h"

#include "audiotypes.h"

namespace muse::audio::encode {
class AbstractAudioEncoder
{
public:
    AbstractAudioEncoder() = default;

    virtual ~AbstractAudioEncoder() = default;

    virtual bool init(const io::path_t& path, const SoundTrackFormat& format, const samples_t totalSamplesNumber)
    {
        if (!format.isValid()) {
            return false;
        }

        m_format = format;

        if (!openDestination(path)) {
            return false;
        }

        prepareOutputBuffer(totalSamplesNumber);

        return true;
    }

    void deinit()
    {
        closeDestination();
    }

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
    virtual size_t requiredOutputBufferSize(samples_t totalSamplesNumber) const = 0;

    virtual void prepareWriting()
    {
#ifdef Q_OS_WIN
        //!Note See https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/setlocale-wsetlocale
        //!     UTF-8 support
        m_locale = setlocale(LC_ALL, nullptr);
        setlocale(LC_ALL, ".UTF8");
#endif
    }

    virtual void completeWriting()
    {
#ifdef Q_OS_WIN
        setlocale(LC_ALL, m_locale.c_str());
#endif
    }

    virtual bool openDestination(const io::path_t& path)
    {
        prepareWriting();
        m_fileStream = std::fopen(path.c_str(), "wb+");

        if (!m_fileStream) {
            return false;
        }

        return true;
    }

    virtual void prepareOutputBuffer(const samples_t totalSamplesNumber)
    {
        m_outputBuffer.resize(requiredOutputBufferSize(totalSamplesNumber));
    }

    virtual void closeDestination()
    {
        if (m_fileStream) {
            std::fclose(m_fileStream);
        }

        completeWriting();
    }

    std::FILE* m_fileStream = nullptr;
    std::vector<unsigned char> m_outputBuffer;

    SoundTrackFormat m_format;
    Progress m_progress;

    std::string m_locale;
};

using AbstractAudioEncoderPtr = std::unique_ptr<AbstractAudioEncoder>;
}

#endif // MUSE_AUDIO_ABSTRACTAUDIOENCODER_H
