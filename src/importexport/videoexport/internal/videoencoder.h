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
#ifndef MU_IMPORTEXPORT_VIDEOENCODER_H
#define MU_IMPORTEXPORT_VIDEOENCODER_H

#include <QImage>
#include "io/path.h"

namespace mu::iex::videoexport {
struct FFmpeg;
class VideoEncoder
{
public:
    VideoEncoder();
    ~VideoEncoder();

    bool open(const io::path_t& fileName, unsigned width, unsigned height, unsigned bitrate, unsigned gop, unsigned fps);
    void close();

    bool encodeImage(const QImage& img);

private:

    bool convertImage_sws(const QImage& img);

    FFmpeg* m_ffmpeg = nullptr;
};
}

#endif // MU_IMPORTEXPORT_VIDEOENCODER_H
