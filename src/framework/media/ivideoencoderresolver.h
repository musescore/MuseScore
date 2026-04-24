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

#include "io/path.h"
#include "async/notification.h"

#include "modularity/imoduleinterface.h"

#include "mediatypes.h"

#include "ivideoencoder.h"

namespace muse::media {
class IVideoEncoderResolver : MODULE_GLOBAL_INTERFACE
{
    INTERFACE_ID(IVideoEncoderResolver)

public:
    virtual ~IVideoEncoderResolver() = default;

    virtual void loadFFmpeg(const muse::io::path_t& ffmpegLibsDir) = 0;
    virtual io::path_t loadedFFmpegDir() const = 0;
    virtual FFmpegVersion loadedFFmpegVersion() const = 0;
    virtual async::Notification loadedFFmpegChanged() const = 0;

    virtual IVideoEncoderPtr currentVideoEncoder() const = 0;
    virtual void setCurrentVideoEncoder(IVideoEncoderPtr encoder) = 0;

    virtual void setIsSettingMode(bool arg) = 0;
};
}
