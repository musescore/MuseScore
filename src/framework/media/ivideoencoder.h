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

#include <string>

#include <QImage>

#include "global/types/bytearray.h"
#include "io/path.h"

#include "modularity/imoduleinterface.h"

namespace muse::media {
class IVideoEncoder : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IVideoEncoder)

public:
    virtual ~IVideoEncoder() = default;

    struct Options {
        std::string format = "";
        unsigned width = static_cast<unsigned>(-1);
        unsigned height = static_cast<unsigned>(-1);
        unsigned bitrate = static_cast<unsigned>(-1);
        unsigned gop = static_cast<unsigned>(-1);
        unsigned fps = static_cast<unsigned>(-1);
    };

    virtual bool open(const muse::io::path_t& fileName, const Options& options) = 0;
    virtual void close() = 0;

    virtual bool encodeImage(const QImage& img) = 0;
    virtual void finishEncode() = 0;

    virtual bool encodeVideo(const muse::ByteArray& videoData, int maxFrames = -1) = 0;

    virtual bool addAudio(const muse::io::path_t& audioPath) = 0;
};
using IVideoEncoderPtr = std::shared_ptr<IVideoEncoder>;
}
