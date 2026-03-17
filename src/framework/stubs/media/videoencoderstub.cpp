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
#include "videoencoderstub.h"

using namespace muse::media;

bool VideoEncoderStub::open(const muse::io::path_t&, const IVideoEncoder::Options&)
{
    return false;
}

void VideoEncoderStub::close()
{
}

bool VideoEncoderStub::encodeImage(const QImage&)
{
    return false;
}

void VideoEncoderStub::finishEncode()
{
}

bool VideoEncoderStub::addAudio(const muse::io::path_t&, double)
{
    return false;
}
