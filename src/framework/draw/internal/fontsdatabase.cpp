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
#include "global/io/fileinfo.h"

#include "log.h"

using namespace muse;
using namespace muse::draw;

#ifndef MUSE_MODULE_DRAW_USE_QTFONTMETRICS
static int s_fontID = -1;
#endif

void FontsDatabase::setDefaultFont(Font::Type type, const FontDataKey& key)
{
    m_defaults[type] = key;
    m_changed.notify();
}

void FontsDatabase::insertSubstitution(const String& familyName, const String& substituteName)
{
    FontDataKey familyKey(familyName);
    FontDataKey substituteKey(substituteName);
    m_familySubstitutions[familyKey].push_back(substituteKey);

#ifdef MUSE_MODULE_DRAW_USE_QTFONTMETRICS
    QFont::insertSubstitution(familyName, substituteName);
#endif

    m_changed.notify();
}

void FontsDatabase::removeSubstitutions(const String& familyName, const std::vector<String>& substituteNames)
{
    auto it = m_familySubstitutions.find(FontDataKey(familyName));
    if (it == m_familySubstitutions.end()) {
        return;
    }

    std::vector<FontDataKey>& substitutes = it->second;
    size_t removed = 0;
    for (const String& substituteName : substituteNames) {
        removed += std::erase(substitutes, FontDataKey(substituteName));
    }

    if (removed == 0) {
        return;
    }

#ifdef MUSE_MODULE_DRAW_USE_QTFONTMETRICS
    QFont::removeSubstitutions(familyName);
    for (const FontDataKey& key : substitutes) {
        QFont::insertSubstitution(familyName, key.family().id().toQString());
    }
#endif

    if (substitutes.empty()) {
        m_familySubstitutions.erase(it);
    }

    m_changed.notify();
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
#ifdef MUSE_MODULE_DRAW_USE_QTFONTMETRICS
    const int id = QFontDatabase::addApplicationFont(path.toQString());
    if (id < 0) {
        LOGW() << "failed register font file: " << path;
        return id;
    }
#else
    const int id = ++s_fontID;
#endif

    auto it = m_fonts.find(key);
    if (it != m_fonts.end()) {
        const FontInfo replaced = it->second;
        m_fonts.erase(it);
        release(replaced);
    }

    m_fonts.insert({ key, FontInfo { id, key, path } });
    m_changed.notify();

    return id;
}

void FontsDatabase::removeFont(const FontDataKey& key)
{
    auto it = m_fonts.find(key);
    if (it == m_fonts.end()) {
        return;
    }

    const FontInfo removed = it->second;
    m_fonts.erase(it);
    release(removed);
    m_changed.notify();
}

void FontsDatabase::release(const FontInfo& fi)
{
#ifdef MUSE_MODULE_DRAW_USE_QTFONTMETRICS
    if (fi.valid()) {
        QFontDatabase::removeApplicationFont(fi.id);
    }
#endif

    for (const auto& it : m_fonts) {
        if (it.second.path == fi.path) {
            return;
        }
    }

    m_fileDataCache.erase(fi.path.toStdString());
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

    std::string pathStr = path.toStdString();
    auto it = m_fileDataCache.find(pathStr);
    if (it != m_fileDataCache.end()) {
        return FontData { key, it->second };
    }

    ByteArray data;
    if (!io::File::readFile(path, data)) {
        LOGE() << "failed read font file: " << path;
        return FontData();
    }

    m_fileDataCache.insert({ pathStr, data });
    return FontData { key, data };
}

bool FontsDatabase::isFtxFont(const FontDataKey& requireKey, Font::Type type) const
{
    FontDataKey key = actualFont(requireKey, type);
    io::path_t path = fontInfo(key).path;
    return io::FileInfo::suffix(path).toLower() == u"ftx";
}

async::Notification FontsDatabase::changed() const
{
    return m_changed;
}

const FontsDatabase::FontInfo& FontsDatabase::fontInfo(const FontDataKey& key) const
{
    auto it = m_fonts.find(key);
    if (it != m_fonts.end()) {
        return it->second;
    }

    static FontInfo null;
    return null;
}
