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
    return addFont(key, std::make_shared<io::File>(path));
}

int FontsDatabase::addFont(const FontDataKey& key, std::shared_ptr<io::IODevice> device)
{
    if (!device) {
        LOGE() << "no device for font: " << key.family().id();
        return -1;
    }

    const std::string filePath = device->meta("file_path");
    ByteArray data;
    if (filePath.empty()) {
        if (device->open(io::IODevice::ReadOnly)) {
            data = device->readAll();
        }
        if (data.empty()) {
            LOGE() << "failed read font: " << key.family().id();
            return -1;
        }
    } else if (!io::File::exists(filePath)) {
        LOGW() << "font file not found: " << filePath;
        return -1;
    }

#ifdef MUSE_MODULE_DRAW_USE_QTFONTMETRICS
    const int id = filePath.empty()
                   ? QFontDatabase::addApplicationFontFromData(data.toQByteArray())
                   : QFontDatabase::addApplicationFont(QString::fromStdString(filePath));
    if (id < 0) {
        LOGW() << "failed register font: " << key.family().id();
        return id;
    }
#else
    const int id = ++s_fontID;
#endif

    auto it = m_fonts.find(key);
    if (it != m_fonts.end()) {
        release(it->second);
        m_fonts.erase(it);
    }

    m_fonts.insert({ key, FontInfo { id, key, std::move(device) } });
    m_changed.notify();

    return id;
}

void FontsDatabase::removeFont(const FontDataKey& key)
{
    auto it = m_fonts.find(key);
    if (it == m_fonts.end()) {
        return;
    }

    release(it->second);
    m_fonts.erase(it);
    m_changed.notify();
}

void FontsDatabase::release(const FontInfo& fi)
{
#ifdef MUSE_MODULE_DRAW_USE_QTFONTMETRICS
    if (fi.valid()) {
        QFontDatabase::removeApplicationFont(fi.id);
    }
#else
    UNUSED(fi);
#endif
}

FontDataKey FontsDatabase::actualFont(const FontDataKey& requireKey, Font::Type type) const
{
    if (fontInfo(requireKey).valid()) {
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
    const FontInfo& info = fontInfo(key);
    if (!info.valid()) {
        return FontData();
    }

    if (!info.device->isOpen() && !info.device->open(io::IODevice::ReadOnly)) {
        LOGE() << "failed open font: " << key.family().id();
        return FontData();
    }

    info.device->seek(0);
    ByteArray data = info.device->readAll();
    if (data.empty()) {
        LOGE() << "failed read font: " << key.family().id();
        return FontData();
    }

    return FontData { key, data };
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
