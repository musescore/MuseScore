/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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
#include "fontsdatabase.h"

#include "muse_framework_config.h"

#ifdef MUSE_MODULE_DRAW_USE_QTFONTMETRICS
#include <QFontDatabase>
#include <QFont>
#endif

#include "global/io/file.h"

#include "log.h"

using namespace muse;
using namespace muse::draw;

static int s_fontID = -1;

void FontsDatabase::setDefaultFont(Font::Type type, const FontDataKey& key)
{
    m_defaults[type] = key;
}

void FontsDatabase::insertSubstitution(const String& familyName, const String& substituteName)
{
    FontDataKey familyKey(familyName);
    FontDataKey substituteKey(substituteName);
    m_familySubstitutions[familyKey].push_back(substituteKey);

#ifdef MUSE_MODULE_DRAW_USE_QTFONTMETRICS
    QFont::insertSubstitution(familyName, substituteName);
#endif
}

const FontDataKey& FontsDatabase::defaultFont(Font::Type type) const
{
    auto it = m_defaults.find(type);
    if (it != m_defaults.end()) {
        return it->second;
    }

    it = m_defaults.find(Font::Type::Unknown);
    IF_ASSERT_FAILED(it != m_defaults.end()) {
        static FontDataKey null;
        return null;
    }
    return it->second;
}

int FontsDatabase::addFont(const FontDataKey& key, const io::path_t& path)
{
    s_fontID++;
    m_fonts.push_back(FontInfo { s_fontID, key, path });

#ifdef MUSE_MODULE_DRAW_USE_QTFONTMETRICS
    QFontDatabase::addApplicationFont(path.toQString());
#endif

    return s_fontID;
}

FontDataKey FontsDatabase::actualFont(const FontDataKey& requireKey, Font::Type type) const
{
    io::path_t path = fontInfo(requireKey).path;
    if (!path.empty() && io::File::exists(path)) {
        return requireKey;
    }

    FontDataKey def = defaultFont(type);
    LOGW() << "not found required font: " << requireKey.family().id() << ", will be using default: " << def.family().id();
    return def;
}

std::vector<FontDataKey> FontsDatabase::substitutionFonts(const FontDataKey& requireKey) const
{
    auto familyIt = m_familySubstitutions.find(FontDataKey(requireKey.family()));
    if (familyIt != m_familySubstitutions.end()) {
        return familyIt->second;
    }

    static std::vector<FontDataKey> null;
    return null;
}

FontData FontsDatabase::fontData(const FontDataKey& requireKey, Font::Type type) const
{
    FontDataKey key = actualFont(requireKey, type);
    io::path_t path = fontInfo(key).path;
    IF_ASSERT_FAILED(io::File::exists(path)) {
        return FontData();
    }

    io::File file(path);
    if (!file.open()) {
        LOGE() << "failed open font file: " << path;
        return FontData();
    }

    FontData fd;
    fd.key = key;
    fd.data = file.readAll();
    return fd;
}

io::path_t FontsDatabase::fontPath(const FontDataKey& requireKey, Font::Type type) const
{
    FontDataKey key = actualFont(requireKey, type);
    io::path_t path = fontInfo(key).path;
    if (!io::File::exists(path)) {
        LOGE() << "not exists font: " << path;
        DO_ASSERT(io::File::exists(path));
        return io::path_t();
    }
    return path;
}

const FontsDatabase::FontInfo& FontsDatabase::fontInfo(const FontDataKey& key) const
{
    for (const FontInfo& fi : m_fonts) {
        if (fi.key == key) {
            return fi;
        }
    }

    static FontInfo null;
    return null;
}
