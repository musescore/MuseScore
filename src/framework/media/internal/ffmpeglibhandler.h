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

#pragma once

#include "io/path.h"
#include "ffmpegfunctions.h"

namespace muse::media {
class FFmpegLibHandler
{
public:
    FFmpegLibHandler() = default;

    bool loadLib(const io::path_t& avUtilPath, const io::path_t& avCodecPath, const io::path_t& avFormatPath,
                 const io::path_t& swScalePath);
    bool loadApi();
    void unload();

    const FFmpegFunctions* functions() const { return &m_functions; }

    int version() const { return m_version; }
    io::path_t dir() const { return m_dir; }

private:
    void* getSymbol(void* lib, const char* name) const;
    bool tryLoadPath(void*& lib, const io::path_t& fullPath);
    void closeLib(void*& lib);

    void* m_avUtilLibrary = nullptr;
    void* m_avCodecLibrary = nullptr;
    void* m_avFormatLibrary = nullptr;
    void* m_swsScaleLibrary = nullptr;

    FFmpegFunctions m_functions;

    int m_version = 0;
    io::path_t m_dir;
};
}
