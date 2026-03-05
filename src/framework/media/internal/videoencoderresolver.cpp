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

#include "ffmpegloader.h"
#include "videoencoder.h"

using namespace muse::media;

void VideoEncoderResolver::init()
{
    loadFFmpeg(configuration()->ffmpegLibsDir());
}

void VideoEncoderResolver::loadFFmpeg(const io::path_t& ffmpegLibsDir)
{
    if (m_encoder) {
        m_currentEncoderFFmpegVersion = -1;
        configuration()->setFFmpegLibsDir({});
    }

    auto ffmpegLibHandler = FFmpegLoader::load(ffmpegLibsDir);
    if (!ffmpegLibHandler) {
        return;
    }

    std::shared_ptr<VideoEncoder> encoder = std::make_shared<VideoEncoder>(ffmpegLibHandler);
    setCurrentVideoEncoder(encoder);

    configuration()->setFFmpegLibsDir(ffmpegLibHandler->dir());

    m_currentEncoderFFmpegVersion = ffmpegLibHandler->version();
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
