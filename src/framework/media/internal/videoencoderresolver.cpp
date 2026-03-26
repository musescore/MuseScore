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

#include "global/timer.h"
#include "io/path.h"
#include "log.h"

#include "internal/ffmpeg/v8/videoencoder.h"
#include "internal/ffmpeg/v7/videoencoder.h"
#include "internal/ffmpeg/v6/videoencoder.h"
#include "internal/ffmpeg/v5/videoencoder.h"
#include "internal/ffmpeg/v4/videoencoder.h"

#include "mediatypes.h"

using namespace muse::media;

void VideoEncoderResolver::init()
{
    loadFFmpeg(configuration()->ffmpegLibsDir());

    m_reloadFfmpegTimer = std::make_shared<Timer>(std::chrono::seconds(1));
    m_reloadFfmpegTimer->onTimeout(this, [this](){
        loadFFmpeg(configuration()->ffmpegLibsDir());

        m_reloadFfmpegTimer->stop();
    });

    m_ffmpegLibWatcher.directoryChanged().onReceive(this, [this](const std::string&) {
        if (m_reloadFfmpegTimer->isActive()) {
            return;
        }

        m_reloadFfmpegTimer->start();
    });
}

void VideoEncoderResolver::deinit()
{
    m_ffmpegLibWatcher.stopWatching();
    m_reloadFfmpegTimer->stop();
    m_reloadFfmpegTimer.reset();
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

void VideoEncoderResolver::setIsSettingMode(bool arg)
{
    if (arg) {
        if (m_currentEncoderFFmpegVersion == FFMPEG_INVALID_VERION) {
            startWatchingFfmpegsDirs();
        }
    } else {
        m_ffmpegLibWatcher.stopWatching();
    }
}

void VideoEncoderResolver::startWatchingFfmpegsDirs()
{
    m_ffmpegLibWatcher.stopWatching();

    io::paths_t paths = defaultSearchPaths();
    paths.emplace_back(configuration()->ffmpegLibsDir());

    for (const io::path_t& p : paths) {
        m_ffmpegLibWatcher.startWatching(p.toStdString());
    }
}

muse::io::path_t VideoEncoderResolver::loadedFFmpegDir() const
{
    return configuration()->ffmpegLibsDir();
}

FFmpegVersion VideoEncoderResolver::loadedFFmpegVersion() const
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
    m_currentEncoderFFmpegVersion = FFMPEG_INVALID_VERION;
    configuration()->setFFmpegLibsDir({});

    m_loadedFFmpegChanged.notify();
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
