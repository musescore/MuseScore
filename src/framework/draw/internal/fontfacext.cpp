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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "fontfacext.h"

#include <algorithm>
#include <cctype>
#include <string_view>

#include <lodepng.h>

#include "global/serialization/zipreader.h"
#include "global/serialization/msgpack.h"
#include "global/stringutils.h"
#include "global/io/buffer.h"
#include "global/io/fileinfo.h"
#include "global/containers.h"
#include "log.h"

using namespace muse::draw;

static bool isNumber(const std::string& s)
{
    return !s.empty()
           && (std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c); })
               || (s[0] == '-' && s.size() > 1 && std::all_of(s.begin() + 1, s.end(), [](unsigned char c) {
        return std::isdigit(c);
    })));
}

FontFaceXT::FontFaceXT()
{
}

FontFaceXT::~FontFaceXT()
{
}

bool FontFaceXT::load(const FaceKey& key, const FontData& fontData, bool isSymbolMode)
{
    m_key = key;
    m_isSymbolMode = isSymbolMode;

    if (!fontData.valid()) {
        LOGE() << "empty font data: " << key.dataKey.family().id();
        return false;
    }

    m_fileBuffer = std::make_unique<muse::io::Buffer>(muse::ByteArray(fontData.data));
    m_fileBuffer->open(muse::io::IODevice::ReadOnly);

    m_zip = std::make_unique<muse::ZipReader>(m_fileBuffer.get());

    // meta
    {
        muse::ByteArray metaData = m_zip->fileData("meta.txt");
        if (metaData.empty()) {
            LOGE() << "meta is empty";
            return false;
        }

        std::string meta(metaData.constChar(), metaData.size());
        std::vector<std::string> metas;
        muse::strings::split(meta, metas, "\n");

        std::string ver;
        std::string glyphs;
        for (const std::string& p : metas) {
            size_t i = p.find(':');
            if (i == std::string::npos) {
                LOGE() << "failed parse param: " << p;
                continue;
            }

            std::string name = p.substr(0, i);
            std::string valStr = p.substr(i + 1);

            if (name == "version") {
                ver = valStr;
            } else if (name == "glyphs") {
                glyphs = valStr;
            } else if (name == "leading") {
                m_leading = std::stol(valStr);
            } else if (name == "ascent") {
                m_ascent = std::stol(valStr);
            } else if (name == "descent") {
                m_descent = std::stol(valStr);
            } else if (name == "xHeight") {
                m_xHeight = std::stol(valStr);
            } else if (name == "capHeight") {
                m_capHeight = std::stol(valStr);
            } else if (name == "underlinePos") {
                m_underlinePos = std::stol(valStr);
            } else if (name == "lineWidth") {
                m_lineWidth = std::stol(valStr);
            } else {
                LOGW() << "unknown param: " << name;
            }
        }

        LOGI() << "fxt version: " << ver << ", glyphs: " << glyphs << ", family: " << key.dataKey.family().id();
    }

    // ligatures
    muse::ByteArray ligaturesData = m_zip->fileData("ligatures.txt");
    if (!ligaturesData.empty()) {
        std::string ligaturesStr(ligaturesData.constChar(), ligaturesData.size());
        std::vector<std::string> ligatureStrs;
        muse::strings::split(ligaturesStr, ligatureStrs, "\n");

        for (const std::string& str : ligatureStrs) {
            if (str.empty()) {
                continue;
            }

            size_t sepIdx = str.find('=');
            if (sepIdx == std::string::npos) {
                continue;
            }

            std::string valStr = str.substr(0, sepIdx);
            std::string keyStr = str.substr(sepIdx + 1);
            std::vector<std::string> keyStrs;
            muse::strings::split(keyStr, keyStrs, " ");

            Ligature l;
            l.first = std::stoi(valStr);
            for (const std::string& k : keyStrs) {
                if (k.empty()) {
                    continue;
                }
                l.second.push_back(std::stoi(k));
            }

            m_ligatures.push_back(l);
        }

        std::sort(m_ligatures.begin(), m_ligatures.end(), [](const Ligature& l1, const Ligature& l2) {
            return l1.second.size() > l2.second.size();
        });
    }

    // kernings
    muse::ByteArray kerningsData = m_zip->fileData("kernings.txt");
    if (!kerningsData.empty()) {
        std::string kerningsStr(kerningsData.constChar(), kerningsData.size());
        std::vector<std::string> kerningStrs;
        muse::strings::split(kerningsStr, kerningStrs, "\n");

        for (const std::string& str : kerningStrs) {
            if (str.empty()) {
                continue;
            }

            size_t sepIdx = str.find('=');
            if (sepIdx == std::string::npos) {
                continue;
            }

            std::string keyStr = str.substr(0, sepIdx);
            std::string valStr = str.substr(sepIdx + 1);
            std::vector<std::string> keyStrs;
            muse::strings::split(keyStr, keyStrs, " ");
            if (keyStrs.size() != 2) {
                //error in file
                continue;
            }

            f26dot6_t advance = std::stoi(valStr);
            std::pair<glyph_idx_t, glyph_idx_t> keyPair;
            keyPair.first = std::stoi(keyStrs[0]);
            keyPair.second = std::stoi(keyStrs[1]);

            m_kernings.insert({ keyPair, advance });
        }
    }

    {
        // glyphIndecies
        muse::ByteArray glyphIndeciesData = m_zip->fileData("glyphIndecies.txt");
        if (!glyphIndeciesData.empty()) {
            std::string glyphIndeciesStr(glyphIndeciesData.constChar(), glyphIndeciesData.size());
            std::vector<std::string> glyphIndexStrs;
            muse::strings::split(glyphIndeciesStr, glyphIndexStrs, "\n");

            for (const std::string& str : glyphIndexStrs) {
                if (str.empty()) {
                    continue;
                }

                std::vector<std::string> glyphIndexPair;
                muse::strings::split(str, glyphIndexPair, " ");

                if (glyphIndexPair.size() != 2) {
                    continue;
                }

                glyph_idx_t glyphIdx = static_cast<glyph_idx_t>(std::stoi(glyphIndexPair[1]));

                if (isNumber(glyphIndexPair[0])) {
                    m_unicodeGlyphs.insert({ static_cast<char32_t>(std::stoi(glyphIndexPair[0])), glyphIdx });
                } else {
                    m_nonUnicodeGlyphs.insert({ glyphIndexPair[0], glyphIdx });
                }
            }
        }
    }

    {
        // atlas texture
        muse::ByteArray metaData = m_zip->fileData("atlatInfo.txt");
        if (metaData.empty()) {
            LOGE() << "atlatInfo is empty";
            return false;
        }
        m_atlastexture.loadAtlasMetaInfo(metaData);

        muse::ByteArray atlasPng = m_zip->fileData("atlas.png");
        if (atlasPng.empty()) {
            LOGE() << "atlas.png is empty";
            return false;
        }
        if (!m_atlastexture.loadAtlas(atlasPng)) {
            return false;
        }
    }

    return true;
}

const FaceKey& FontFaceXT::key() const
{
    return m_key;
}

bool FontFaceXT::isSymbolMode() const
{
    return m_isSymbolMode;
}

f26dot6_t FontFaceXT::leading() const
{
    return m_leading;
}

f26dot6_t FontFaceXT::ascent() const
{
    return m_ascent;
}

f26dot6_t FontFaceXT::descent() const
{
    return m_descent;
}

f26dot6_t FontFaceXT::xHeight() const
{
    return m_xHeight;
}

f26dot6_t FontFaceXT::capHeight() const
{
    return m_capHeight;
}

f26dot6_t FontFaceXT::underlinePos() const
{
    return m_underlinePos;
}

f26dot6_t FontFaceXT::lineWidth() const
{
    return m_lineWidth;
}

void FontFaceXT::applyLigatures(std::vector<glyph_idx_t>& glyphs, const Ligatures& ls)
{
    for (const Ligature& l : ls) {
        if (l.second.empty() || l.second.size() > glyphs.size() || (l.second.size() == 1 && l.second.front() == l.first)) {
            continue;
        }

        while (1) {
            auto it = std::search(glyphs.begin(), glyphs.end(), l.second.begin(), l.second.end());
            if (it != glyphs.end()) {
                *it = l.first;
                for (size_t i = 1; i < l.second.size(); ++i) {
                    ++it;
                    *it = 0;
                }
            } else {
                break;
            }
        }
    }
}

std::vector<GlyphPos> FontFaceXT::glyphs(const char32_t* text, int text_length) const
{
    if (text_length < 1) {
        return std::vector<GlyphPos>();
    }

    std::vector<GlyphPos> result;

    std::vector<char32_t> data(text, text + text_length);

    std::vector<glyph_idx_t> glyphs;
    glyphs.reserve(text_length);

    for (char32_t ch : data) {
        if (ch == 0) {
            continue;
        }

        glyph_idx_t idx = glyphIndex(ch);
        if (idx == 0) {
            LOGE() << "font: " << m_key.dataKey.family().id() << " not found glyph: " << (int)ch;
            continue;
        }

        glyphs.push_back(idx);
    }

    if (!m_isSymbolMode) {
        applyLigatures(glyphs, m_ligatures);
    }

    muse::remove(glyphs, (glyph_idx_t)0);

    for (auto it = glyphs.begin(); it != glyphs.end(); ++it) {
        GlyphPos p;
        if (auto nextIt = std::next(it); nextIt != glyphs.end()) {
            p.x_advance = glyphAdvance(*it, *nextIt);
        } else {
            p.x_advance = glyphAdvance(*it);
        }
        p.idx = *it;

        result.push_back(std::move(p));
    }

    return result;
}

glyph_idx_t FontFaceXT::glyphIndex(char32_t ucs4) const
{
    return muse::value(m_unicodeGlyphs, ucs4);
}

glyph_idx_t FontFaceXT::glyphIndex(const std::string& glyphName) const
{
    return muse::value(m_nonUnicodeGlyphs, glyphName);
}

char32_t FontFaceXT::findCharCode(glyph_idx_t idx) const
{
    return muse::key(m_unicodeGlyphs, idx);
}

FBBox FontFaceXT::glyphBbox(glyph_idx_t idx) const
{
    return m_isSymbolMode ? glyphData(idx).symBbox : glyphData(idx).textBbox;
}

f26dot6_t FontFaceXT::glyphAdvance(glyph_idx_t idx) const
{
    return m_isSymbolMode ? glyphData(idx).symAdvance : glyphData(idx).textAdvance;
}

const GlyphImage& FontFaceXT::glyphImage(glyph_idx_t idx) const
{
    return glyphData(idx).glyphImage;
}

const FontFaceXT::GlyphData& FontFaceXT::glyphData(glyph_idx_t idx) const
{
    auto it = m_cache.find(idx);
    if (it != m_cache.end()) {
        return it->second;
    }

    muse::ByteArray data = m_zip->fileData(std::to_string(idx));
    muse::io::Buffer buf(&data);
    buf.open(muse::io::IODevice::ReadOnly);

    std::pair<glyph_idx_t, GlyphData> v;
    v.first = idx;
    v.second.read(&buf);
    v.second.glyphImage.sdf = m_atlastexture.getGlyphImage(idx);

    return m_cache.insert(std::move(v)).first->second;
}

f26dot6_t FontFaceXT::glyphAdvance(glyph_idx_t idx, glyph_idx_t nextIdx) const
{
    auto kerningPair = std::make_pair(idx, nextIdx);

    if (auto it = m_kernings.find(kerningPair); it != m_kernings.end()) {
        return it->second;
    }

    return glyphAdvance(idx);
}

void FontFaceXT::GlyphData::write(muse::io::IODevice* d) const
{
    muse::ByteArray data = muse::msgpack::pack(static_cast<int64_t>(textBbox.x()),
                                               static_cast<int64_t>(textBbox.y()),
                                               static_cast<int64_t>(textBbox.width()),
                                               static_cast<int64_t>(textBbox.height()),
                                               static_cast<int64_t>(textAdvance),
                                               static_cast<int64_t>(symBbox.x()),
                                               static_cast<int64_t>(symBbox.y()),
                                               static_cast<int64_t>(symBbox.width()),
                                               static_cast<int64_t>(symBbox.height()),
                                               static_cast<int64_t>(symAdvance),
                                               glyphImage.rect.x(), glyphImage.rect.y(),
                                               glyphImage.rect.width(), glyphImage.rect.height(),
                                               glyphImage.range);
    d->write(data);
}

void FontFaceXT::GlyphData::read(muse::io::IODevice* d)
{
    int64_t textBboxX = 0;
    int64_t textBboxY = 0;
    int64_t textBboxW = 0;
    int64_t textBboxH = 0;
    int64_t textAdvanceValue = 0;
    int64_t symBboxX = 0;
    int64_t symBboxY = 0;
    int64_t symBboxW = 0;
    int64_t symBboxH = 0;
    int64_t symAdvanceValue = 0;
    double rectX = 0.0;
    double rectY = 0.0;
    double rectW = 0.0;
    double rectH = 0.0;
    float range = 0.f;

    const bool ok = muse::msgpack::unpack(d->readAll(), textBboxX, textBboxY, textBboxW, textBboxH, textAdvanceValue,
                                          symBboxX, symBboxY, symBboxW, symBboxH, symAdvanceValue,
                                          rectX, rectY, rectW, rectH, range);
    if (!ok) {
        LOGE() << "failed read glyph data";
        return;
    }

    textBbox = FBBox(textBboxX, textBboxY, textBboxW, textBboxH);
    textAdvance = textAdvanceValue;
    symBbox = FBBox(symBboxX, symBboxY, symBboxW, symBboxH);
    symAdvance = symAdvanceValue;
    glyphImage.rect = muse::RectF(rectX, rectY, rectW, rectH);
    glyphImage.range = range;
}

bool AtlasTexture::loadAtlas(const muse::ByteArray& atlas)
{
    const unsigned error = lodepng::decode(m_atlas, m_atlasWidth, m_atlasHeight,
                                           atlas.constData(), atlas.size(), LCT_GREY);
    if (error != 0) {
        LOGE() << "failed to decode atlas: " << lodepng_error_text(error);
        m_atlas.clear();
        m_atlasWidth = 0;
        m_atlasHeight = 0;
        return false;
    }

    return true;
}

void AtlasTexture::loadAtlasMetaInfo(const muse::ByteArray& metaData)
{
    std::string meta(metaData.constChar(), metaData.size());
    std::vector<std::string> glyphsInfos;
    muse::strings::split(meta, glyphsInfos, "\n");

    for (const std::string& info : glyphsInfos) {
        if (info.empty()) {
            continue;
        }
        GlyphInfo glyphInfo;
        size_t i = info.find('=');
        if (i == std::string::npos) {
            LOGE() << "failed parse param: " << info;
            continue;
        }
        glyphInfo.gidx = std::stoi(info.substr(0, i));

        std::string valStr = info.substr(i + 1);
        std::vector<std::string> infos;
        muse::strings::split(valStr, infos, ":");
        if (infos.size() != 5) {
            continue;
        }

        glyphInfo.rect.setX(std::stoi(infos[0]));
        glyphInfo.rect.setY(std::stoi(infos[1]));
        glyphInfo.rect.setWidth(std::stoi(infos[2]));
        glyphInfo.rect.setHeight(std::stoi(infos[3]));
        glyphInfo.rotated = std::stoi(infos[4]);
        m_glyphInfos.insert({ glyphInfo.gidx, glyphInfo });
    }
}

Sdf AtlasTexture::getGlyphImage(glyph_idx_t idx)
{
    Sdf result;
    auto it = m_glyphInfos.find(idx);
    if (it == m_glyphInfos.end()) {
        return result;
    }

    auto width = it->second.rect.width();
    auto height = it->second.rect.height();
    auto posX = it->second.rect.x();
    auto posY = it->second.rect.y();

    if (width <= 0 || height <= 0 || posX < 0 || posY < 0
        || posX + width > static_cast<int>(m_atlasWidth)
        || posY + height > static_cast<int>(m_atlasHeight)) {
        LOGE() << "invalid atlas glyph rect: " << idx;
        return result;
    }

    result.bitmap.reserve(width * height);

    if (it->second.rotated) {
        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y) {
                auto px = m_atlas.at((posY + y) * m_atlasWidth + posX + x);
                result.bitmap.push_back(px);
            }
        }
    } else {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                result.bitmap.push_back(m_atlas.at((posY + y) * m_atlasWidth + posX + x));
            }
        }
    }

    result.width = it->second.rotated ? height : width;
    result.height = it->second.rotated ? width : height;
    result.hash = std::hash<std::string_view> {}({ reinterpret_cast<const char*>(result.bitmap.data()), result.bitmap.size() });

    return result;
}
