/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <map>
#include <string>
#include <unordered_map>

#include "global/io/path.h"

#include "types/fontstypes.h"

namespace muse::draw {
class FontRenderCache
{
public:
    void init();
    void setCacheDirPath(const io::path_t& path, const std::string& revision = std::string());

    void store(const FaceKey& face, glyph_idx_t glyphIdx, const GlyphImage& image);
    GlyphImage load(const FaceKey& face, glyph_idx_t glyphIdx) const;

private:
    io::path_t revisionFilePath() const;
    io::path_t makeFilePath(const FaceKey& face, glyph_idx_t glyphIdx, const GlyphImage& image) const;
    void updateCacheRevision();

    using GlyphImages = std::unordered_map<glyph_idx_t, GlyphImage>;

    mutable std::map<FaceKey, GlyphImages> m_cache;
    io::path_t m_cacheDirPath;
    std::string m_cacheRevision;

    struct CacheInfo {
        io::path_t filePath;
        uint32_t width = 0;
        uint32_t height = 0;
        RectF rect;
        float range = 0.0f;
    };

    using CacheInfoMap = std::map<std::string, CacheInfo>;

    void loadCachedInfo(CacheInfoMap& map, const io::path_t& dir) const;

    mutable bool m_cacheInfoLoaded = false;
    mutable CacheInfoMap m_cacheInfoMap;
};
}
