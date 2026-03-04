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

#pragma once

#include "../ivideoencoder.h"

#include "ffmpegfunctions.h"

#include "ffmpeglibhandler.h"

namespace muse::media {
struct FFmpeg;
class VideoEncoder : public IVideoEncoder
{
public:
    explicit VideoEncoder(const std::shared_ptr<FFmpegLibHandler>& handler);
    ~VideoEncoder() override;

    bool open(const muse::io::path_t& fileName, unsigned width, unsigned height, unsigned bitrate, unsigned gop, unsigned fps) override;
    void close() override;

    bool encodeImage(const QImage& img) override;
    bool muxAudioVideo(const muse::io::path_t& videoPath, const muse::io::path_t& audioPath, const muse::io::path_t& outputPath,
                       double audioOffsetSec) override;

private:
    bool convertImage_sws(const QImage& img);
    const FFmpegFunctions* ffmpegFunctions() const;

    std::shared_ptr<FFmpegLibHandler> m_ffmpegHandler = nullptr;
    FFmpeg* m_ffmpeg = nullptr;
};
}
