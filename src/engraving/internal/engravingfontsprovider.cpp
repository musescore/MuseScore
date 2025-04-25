/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "engravingfontsprovider.h"

#include "global/stringutils.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

EngravingFontsProvider::EngravingFontsProvider(const muse::modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx)
{
}

void EngravingFontsProvider::addInternalFont(const std::string& name, const std::string& family, const muse::io::path_t& filePath)
{
    muse::io::path_t basePath = muse::io::dirpath(filePath.toQString());
    muse::io::path_t metadataPath = basePath + "/metadata.json";
    std::shared_ptr<EngravingFont> f = std::make_shared<EngravingFont>(name, family, filePath, metadataPath, iocContext());
    m_symbolFonts.push_back(f);
    m_fallback.font = nullptr;
}

void EngravingFontsProvider::addExternalFont(const std::string& name, const std::string& family, const muse::io::path_t& filePath,
                                             const muse::io::path_t& metadataPath)
{
    std::shared_ptr<EngravingFont> f = std::make_shared<EngravingFont>(name, family, filePath, metadataPath, iocContext());
    m_externalSymbolFonts.emplace(muse::strings::toLower(name), f);
}

std::shared_ptr<EngravingFont> EngravingFontsProvider::doFontByName(const std::string& name) const
{
    // External fonts should have higher priority than internal fonts
    std::string name_lo = muse::strings::toLower(name);
    auto it = m_externalSymbolFonts.find(name_lo);
    if (it != m_externalSymbolFonts.end()) {
        return it->second;
    }
    for (const std::shared_ptr<EngravingFont>& f : m_symbolFonts) {
        if (muse::strings::toLower(f->name()) == name_lo) {
            return f;
        }
    }
    return nullptr;
}

IEngravingFontPtr EngravingFontsProvider::fontByName(const std::string& name) const
{
    std::shared_ptr<EngravingFont> font = doFontByName(name);
    if (!font) {
        font = doFallbackFont();
    }

    font->ensureLoad();
    return font;
}

std::vector<IEngravingFontPtr> EngravingFontsProvider::fonts() const
{
    std::vector<IEngravingFontPtr> fs;
    for (const std::shared_ptr<EngravingFont>& f : m_symbolFonts) {
        std::string name_lo = muse::strings::toLower(f->name());
        if (m_externalSymbolFonts.find(name_lo) == m_externalSymbolFonts.end()) {
            fs.push_back(f);
        }
    }
    for (const auto& [_, f] : m_externalSymbolFonts) {
        fs.push_back(f);
    }
    return fs;
}

void EngravingFontsProvider::setFallbackFont(const std::string& name)
{
    m_fallback.name = name;
    m_fallback.font = nullptr;
}

std::shared_ptr<EngravingFont> EngravingFontsProvider::doFallbackFont() const
{
    if (!m_fallback.font) {
        m_fallback.font = doFontByName(m_fallback.name);
        IF_ASSERT_FAILED(m_fallback.font) {
            return nullptr;
        }
    }

    return m_fallback.font;
}

IEngravingFontPtr EngravingFontsProvider::fallbackFont() const
{
    std::shared_ptr<EngravingFont> font = doFallbackFont();
    font->ensureLoad();
    return font;
}

bool EngravingFontsProvider::isFallbackFont(const IEngravingFont* f) const
{
    return doFallbackFont().get() == f;
}

void EngravingFontsProvider::clearExternalFonts()
{
    m_externalSymbolFonts.clear();
}

void EngravingFontsProvider::loadAllFonts()
{
    for (std::shared_ptr<EngravingFont>& f : m_symbolFonts) {
        f->ensureLoad();
    }
    for (const auto& [_, f] : m_externalSymbolFonts) {
        f->ensureLoad();
    }
}
