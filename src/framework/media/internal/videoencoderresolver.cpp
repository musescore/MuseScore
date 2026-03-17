/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "videoencoderresolver.h"

#include "internal/ffmpeg/v8/videoencoder.h"
#include "internal/ffmpeg/v7/videoencoder.h"
#include "internal/ffmpeg/v6/videoencoder.h"
#include "internal/ffmpeg/v5/videoencoder.h"
#include "internal/ffmpeg/v4/videoencoder.h"

#include "io/path.h"
#include "log.h"

using namespace muse::media;

void VideoEncoderResolver::init()
{
    loadFFmpeg(configuration()->ffmpegLibsDir());
}

void VideoEncoderResolver::loadFFmpeg(const io::path_t& ffmpegLibsDir)
{
    const FFmpegLibPaths paths = findLibraryPaths(ffmpegLibsDir);
    if (paths.avFormatPath.empty()) {
        resetFFmpegSettings();
        return;
    }

    EncoderInfo encoderInfo = makeEncoder(paths);
    if (!encoderInfo.encoder) {
        resetFFmpegSettings();
        return;
    }

    setCurrentVideoEncoder(encoderInfo.encoder);

    configuration()->setFFmpegLibsDir(encoderInfo.ffmpegLibsDir);

    m_currentEncoderFFmpegVersion = encoderInfo.ffmpegVersion;
    m_loadedFFmpegChanged.notify();
}

muse::io::path_t VideoEncoderResolver::loadedFFmpegDir() const
{
    return configuration()->ffmpegLibsDir();
}

int VideoEncoderResolver::loadedFFmpegVersion() const
{
    return m_currentEncoderFFmpegVersion;
}

muse::async::Notification VideoEncoderResolver::loadedFFmpegChanged() const
{
    return m_loadedFFmpegChanged;
}

IVideoEncoderPtr VideoEncoderResolver::currentVideoEncoder() const
{
    return m_encoder;
}

void VideoEncoderResolver::setCurrentVideoEncoder(IVideoEncoderPtr encoder)
{
    m_encoder = std::move(encoder);
}

void VideoEncoderResolver::resetFFmpegSettings()
{
    m_currentEncoderFFmpegVersion = -1;
    configuration()->setFFmpegLibsDir({});
}

template<typename Encoder>
static IVideoEncoderPtr tryCreateEncoder(const FFmpegLibPaths& paths)
{
    auto encoder = std::make_shared<Encoder>();
    if (encoder->load(paths)) {
        return encoder;
    }
    return nullptr;
}

VideoEncoderResolver::EncoderInfo VideoEncoderResolver::makeEncoder(const FFmpegLibPaths& ffmpegLibsPaths) const
{
    EncoderInfo result;

    const FFmpegVersion version = versionFromAVFormatPath(ffmpegLibsPaths.avFormatPath);
    switch (version) {
    case FFMPEG_V8:
        result.encoder = tryCreateEncoder<ffmpeg::v8::VideoEncoder>(ffmpegLibsPaths);
        break;
    case FFMPEG_V7:
        result.encoder = tryCreateEncoder<ffmpeg::v7::VideoEncoder>(ffmpegLibsPaths);
        break;
    case FFMPEG_V6:
        result.encoder = tryCreateEncoder<ffmpeg::v6::VideoEncoder>(ffmpegLibsPaths);
        break;
    case FFMPEG_V5:
        result.encoder = tryCreateEncoder<ffmpeg::v5::VideoEncoder>(ffmpegLibsPaths);
        break;
    case FFMPEG_V4:
        result.encoder = tryCreateEncoder<ffmpeg::v4::VideoEncoder>(ffmpegLibsPaths);
        break;
    default:
        break;
    }

    if (result.encoder) {
        result.ffmpegLibsDir = io::dirpath(ffmpegLibsPaths.avFormatPath);
        result.ffmpegVersion = version;

        LOGD() << "FFmpeg loaded, version: " << version;
    }

    return result;
}
