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

#include <QImage>

#include "io/path.h"

#include "modularity/imoduleinterface.h"

namespace muse::media {
class IVideoEncoder : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IVideoEncoder)

public:
    virtual ~IVideoEncoder() = default;

    virtual bool open(const muse::io::path_t& fileName, unsigned width, unsigned height, unsigned bitrate, unsigned gop, unsigned fps) = 0;
    virtual void close() = 0;
    virtual bool encodeImage(const QImage& img) = 0;
    virtual bool muxAudioVideo(const muse::io::path_t& videoPath, const muse::io::path_t& audioPath, const muse::io::path_t& outputPath,
                               double audioOffsetSec) = 0;
};
using IVideoEncoderPtr = std::shared_ptr<IVideoEncoder>;
}
