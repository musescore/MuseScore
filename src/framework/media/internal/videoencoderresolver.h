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

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "imediaconfiguration.h"

#include "../ivideoencoderresolver.h"

namespace muse::media {
class VideoEncoderResolver : public IVideoEncoderResolver, public muse::async::Asyncable
{
    GlobalInject<IMediaConfiguration> configuration;

public:
    void init();

    void loadFFmpeg(const muse::io::path_t& ffmpegLibsDir) override;
    io::path_t loadedFFmpegDir() const override;
    int loadedFFmpegVersion() const override;
    async::Notification loadedFFmpegChanged() const override;

    IVideoEncoderPtr currentVideoEncoder() const override;
    void setCurrentVideoEncoder(IVideoEncoderPtr encoder) override;

private:
    IVideoEncoderPtr m_encoder;
    int m_currentEncoderFFmpegVersion = -1;

    async::Notification m_loadedFFmpegChanged;
};
}
