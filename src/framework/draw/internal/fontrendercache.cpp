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
#include "fontrendercache.h"

#include <array>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string_view>

#include "global/io/dir.h"
#include "global/io/file.h"
#include "global/io/fileinfo.h"
#include "global/stringutils.h"

#include "log.h"

using namespace muse::draw;

void FontRenderCache::init()
{
    m_cache.clear();
    m_cacheInfoLoaded = false;
    m_cacheInfoMap.clear();

    if (!m_cacheDirPath.empty()) {
        muse::io::Dir::mkpath(m_cacheDirPath);
        updateCacheRevision();
    }
}

void FontRenderCache::setCacheDirPath(const muse::io::path_t& path, const std::string& revision)
{
    m_cacheDirPath = path;
    m_cacheRevision = revision;
    m_cache.clear();
    m_cacheInfoLoaded = false;
    m_cacheInfoMap.clear();

    if (!m_cacheDirPath.empty()) {
        muse::io::Dir::mkpath(m_cacheDirPath);
        updateCacheRevision();
    }
}

muse::io::path_t FontRenderCache::revisionFilePath() const
{
    return m_cacheDirPath.appendingComponent("revision");
}

void FontRenderCache::updateCacheRevision()
{
    if (m_cacheRevision.empty()) {
        return;
    }

    bool isNeedClear = true;
    const muse::io::path_t revisionPath = revisionFilePath();
    if (muse::io::File::exists(revisionPath)) {
        muse::io::File revisionFile(revisionPath);
        if (revisionFile.open(muse::io::IODevice::ReadOnly)) {
            const muse::ByteArray data = revisionFile.readAll();
            isNeedClear = std::string_view(data.constChar(), data.size()) != m_cacheRevision;
        }
    }

    if (isNeedClear) {
        muse::RetVal<muse::io::paths_t> files = muse::io::Dir::scanFiles(m_cacheDirPath, {}, muse::io::ScanMode::FilesInCurrentDir);
        if (!files.ret) {
            LOGE() << "failed scan font render cache dir: " << m_cacheDirPath << ", error: " << files.ret.toString();
            return;
        }

        bool isCleared = true;
        for (const muse::io::path_t& filePath : files.val) {
            if (!muse::io::File::remove(filePath)) {
                LOGE() << "failed remove font render cache file: " << filePath;
                isCleared = false;
            }
        }

        m_cacheInfoLoaded = false;
        m_cacheInfoMap.clear();

        if (!isCleared) {
            return;
        }
    }

    if (!muse::io::File::writeFile(revisionPath, muse::ByteArray(m_cacheRevision.c_str())).success()) {
        LOGE() << "failed write cache revision file: " << revisionPath;
    }
}

static std::string realToString(double n)
{
    std::ostringstream stream;
    stream << std::scientific << std::setprecision(std::numeric_limits<double>::max_digits10) << n;
    return stream.str();
}

enum class CacheInfoParam : size_t {
    Width = 0,
    Height,
    RectX,
    RectY,
    RectWidth,
    RectHeight,
    Range,
    Count
};

static constexpr size_t CACHE_INFO_PARAM_COUNT = static_cast<size_t>(CacheInfoParam::Count);

static constexpr size_t paramIndex(CacheInfoParam param)
{
    return static_cast<size_t>(param);
}

static std::string keyToString(const FaceKey& face, glyph_idx_t glyphIdx)
{
    std::string str;
    str.reserve(50);
    str += face.dataKey.family().id().toStdString();
    str += "_" + std::to_string(glyphIdx);
    str += "_" + std::to_string(face.dataKey.bold());
    str += "_" + std::to_string(face.dataKey.italic());
    str += "_" + std::to_string(face.pixelSize);
    return str;
}

static std::array<std::string, CACHE_INFO_PARAM_COUNT> cacheInfoParams(const GlyphImage& image)
{
    return {
        std::to_string(image.sdf.width),
        std::to_string(image.sdf.height),
        realToString(image.rect.x()),
        realToString(image.rect.y()),
        realToString(image.rect.width()),
        realToString(image.rect.height()),
        realToString(image.range),
    };
}

static std::string cacheInfoToString(const GlyphImage& image)
{
    std::string str;
    const std::array<std::string, CACHE_INFO_PARAM_COUNT> params = cacheInfoParams(image);
    for (size_t i = 0; i < params.size(); ++i) {
        if (i > 0) {
            str += "|";
        }
        str += params.at(i);
    }
    return str;
}

muse::io::path_t FontRenderCache::makeFilePath(const FaceKey& face, glyph_idx_t glyphIdx, const GlyphImage& image) const
{
    std::string str;
    str.reserve(100);
    str += keyToString(face, glyphIdx);
    str += "_[" + cacheInfoToString(image);
    str += "].sdf";

    return muse::io::path_t(str);
}

void FontRenderCache::store(const FaceKey& face, glyph_idx_t glyphIdx, const GlyphImage& image)
{
    m_cache[face][glyphIdx] = image;

    if (m_cacheDirPath.empty()) {
        return;
    }

    muse::io::path_t fileFullPath = m_cacheDirPath.appendingComponent(makeFilePath(face, glyphIdx, image));
    muse::io::File file(fileFullPath);
    if (!file.open(muse::io::IODevice::WriteOnly)) {
        LOGE() << "failed open to write file: " << fileFullPath;
        return;
    }

    file.write(image.sdf.bitmap);
}

GlyphImage FontRenderCache::load(const FaceKey& face, glyph_idx_t glyphIdx) const
{
    auto faceIt = m_cache.find(face);
    if (faceIt != m_cache.end()) {
        const GlyphImages& images = faceIt->second;
        auto imageIt = images.find(glyphIdx);
        if (imageIt != images.end()) {
            return imageIt->second;
        }
    }

    if (!m_cacheInfoLoaded) {
        if (!m_cacheDirPath.empty()) {
            loadCachedInfo(m_cacheInfoMap, m_cacheDirPath);
        }
        m_cacheInfoLoaded = true;
    }

    auto cacheInfoIt = m_cacheInfoMap.find(keyToString(face, glyphIdx));
    if (cacheInfoIt == m_cacheInfoMap.end()) {
        return GlyphImage();
    }

    const CacheInfo& info = cacheInfoIt->second;
    muse::io::File file(info.filePath);
    if (!file.open(muse::io::IODevice::ReadOnly)) {
        LOGE() << "failed read file: " << file.filePath();
        return GlyphImage();
    }

    GlyphImage image;
    image.sdf.bitmap = file.readAll();
    image.sdf.width = info.width;
    image.sdf.hash = std::hash<std::string_view> {}({ reinterpret_cast<const char*>(image.sdf.bitmap.data()),
                                                      image.sdf.bitmap.size() });
    image.sdf.height = info.height;
    image.rect = info.rect;
    image.range = info.range;

    m_cache[face][glyphIdx] = image;

    return image;
}

void FontRenderCache::loadCachedInfo(CacheInfoMap& map, const muse::io::path_t& dir) const
{
    muse::RetVal<muse::io::paths_t> files = muse::io::Dir::scanFiles(dir, {}, muse::io::ScanMode::FilesInCurrentDir);
    for (const muse::io::path_t& p : files.val) {
        std::string str = muse::io::FileInfo(p).completeBaseName().toStdString();

        size_t startInfoIdx = str.find('[');
        if (startInfoIdx == std::string::npos) {
            continue;
        }

        size_t endInfoIdx = str.find(']');
        if (endInfoIdx == std::string::npos) {
            continue;
        }

        std::string key = str.substr(0, startInfoIdx - 1);
        std::string data = str.substr(startInfoIdx + 1, endInfoIdx - startInfoIdx - 1);
        std::vector<std::string> params;
        muse::strings::split(data, params, "|");
        if (params.size() != CACHE_INFO_PARAM_COUNT) {
            continue;
        }

        bool ok = false;

        int width = muse::AsciiStringView(params.at(paramIndex(CacheInfoParam::Width))).toInt(&ok);
        if (!ok || width < 0) {
            LOGW() << "failed parse font render cache info from file: " << p;
            continue;
        }

        int height = muse::AsciiStringView(params.at(paramIndex(CacheInfoParam::Height))).toInt(&ok);
        if (!ok || height < 0) {
            LOGW() << "failed parse font render cache info from file: " << p;
            continue;
        }

        double rectX = muse::AsciiStringView(params.at(paramIndex(CacheInfoParam::RectX))).toDouble(&ok);
        bool isParsed = ok;
        double rectY = muse::AsciiStringView(params.at(paramIndex(CacheInfoParam::RectY))).toDouble(&ok);
        isParsed = isParsed && ok;
        double rectWidth = muse::AsciiStringView(params.at(paramIndex(CacheInfoParam::RectWidth))).toDouble(&ok);
        isParsed = isParsed && ok;
        double rectHeight = muse::AsciiStringView(params.at(paramIndex(CacheInfoParam::RectHeight))).toDouble(&ok);
        isParsed = isParsed && ok;
        double range = muse::AsciiStringView(params.at(paramIndex(CacheInfoParam::Range))).toDouble(&ok);
        isParsed = isParsed && ok;

        if (!isParsed) {
            LOGW() << "failed parse font render cache info from file: " << p;
            continue;
        }

        CacheInfo info;
        info.filePath = p;
        info.width = static_cast<uint32_t>(width);
        info.height = static_cast<uint32_t>(height);
        info.rect = RectF(rectX, rectY, rectWidth, rectHeight);
        info.range = range;

        map[key] = info;
    }
}
