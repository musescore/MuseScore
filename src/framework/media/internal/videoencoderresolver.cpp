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

#include "internal/ffmpeg/v8/ffmpeglibhandler.h"
#include "internal/ffmpeg/v8/videoencoder.h"

#include "internal/ffmpeg/v7/ffmpeglibhandler.h"
#include "internal/ffmpeg/v7/videoencoder.h"

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

VideoEncoderResolver::EncoderInfo VideoEncoderResolver::makeEncoder(const FFmpegLibPaths& ffmpegLibsPaths) const
{
    EncoderInfo result;

    const FFmpegVersion version = versionFromAVFormatPath(ffmpegLibsPaths.avFormatPath);
    switch (version) {
    case FFMPEG_V8: {
        auto ffmpegLibHandler = std::make_shared<ffmpeg::v8::FFmpegLibHandler>();
        if (ffmpegLibHandler->loadLib(ffmpegLibsPaths.avUtilPath, ffmpegLibsPaths.avCodecPath, ffmpegLibsPaths.avFormatPath,
                                      ffmpegLibsPaths.swScalePath, ffmpegLibsPaths.swResamplePath)
            && ffmpegLibHandler->loadApi()) {
            ffmpegLibHandler->setVersion(version);
            ffmpegLibHandler->setDir(io::dirpath(ffmpegLibsPaths.avFormatPath));

            LOGD() << "FFmpeg loaded, version: " << ffmpegLibHandler->version();

            result.encoder = std::make_shared<ffmpeg::v8::VideoEncoder>(ffmpegLibHandler);
            result.ffmpegLibsDir = ffmpegLibHandler->dir();
            result.ffmpegVersion = ffmpegLibHandler->version();
        } else {
            LOGW() << "FFmpeg libraries not found";
        }
    } break;
    case FFMPEG_V7: {
        auto ffmpegLibHandler = std::make_shared<ffmpeg::v7::FFmpegLibHandler>();
        if (ffmpegLibHandler->loadLib(ffmpegLibsPaths.avUtilPath, ffmpegLibsPaths.avCodecPath, ffmpegLibsPaths.avFormatPath,
                                      ffmpegLibsPaths.swScalePath, ffmpegLibsPaths.swResamplePath)
            && ffmpegLibHandler->loadApi()) {
            ffmpegLibHandler->setVersion(version);
            ffmpegLibHandler->setDir(io::dirpath(ffmpegLibsPaths.avFormatPath));

            LOGD() << "FFmpeg loaded, version: " << ffmpegLibHandler->version();

            result.encoder = std::make_shared<ffmpeg::v7::VideoEncoder>(ffmpegLibHandler);
            result.ffmpegLibsDir = ffmpegLibHandler->dir();
            result.ffmpegVersion = ffmpegLibHandler->version();
        } else {
            LOGW() << "FFmpeg libraries not found";
        }
    } break;
    default:
        break;
    }

    return result;
}
