/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include "mediaconfiguration.h"

#include "settings.h"

using namespace muse::media;

static const muse::Settings::Key FFMPEG_LIBS_DIR("media", "application/ffmpegLibsDir");

void MediaConfiguration::init()
{
    settings()->setDefaultValue(FFMPEG_LIBS_DIR, Val(""));
    settings()->valueChanged(FFMPEG_LIBS_DIR).onReceive(this, [this](const Val& val) {
        m_ffmpegLibsDirChanged.send(val.toPath());
    });
}

muse::io::path_t MediaConfiguration::ffmpegLibsDir() const
{
    return settings()->value(FFMPEG_LIBS_DIR).toPath();
}

void MediaConfiguration::setFFmpegLibsDir(const io::path_t& path)
{
    settings()->setSharedValue(FFMPEG_LIBS_DIR, Val(path));
}

muse::async::Channel<muse::io::path_t> MediaConfiguration::ffmpegLibsDirChanged() const
{
    return m_ffmpegLibsDirChanged;
}
