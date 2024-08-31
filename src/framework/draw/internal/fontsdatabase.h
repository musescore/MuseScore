/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

#include <vector>
#include <map>

#include "ifontsdatabase.h"

namespace muse::draw {
class FontsDatabase : public IFontsDatabase
{
public:
    FontsDatabase() = default;

    void setDefaultFont(Font::Type type, const FontDataKey& key) override;
    void insertSubstitution(const String& f1, const String& substituteName) override;

    int addFont(const FontDataKey& key, const io::path_t& path) override;

    FontDataKey actualFont(const FontDataKey& requireKey, Font::Type type) const override;
    std::vector<FontDataKey> substitutionFonts(Font::Type type) const override;
    FontData fontData(const FontDataKey& requireKey, Font::Type type) const override;
    io::path_t fontPath(const FontDataKey& requireKey, Font::Type type) const override;

    void addAdditionalFonts(const io::path_t& path) override;

private:

    struct FontInfo {
        int id = -1;
        FontDataKey key;
        io::path_t path;

        bool valid() const { return id > -1; }
    };

    const FontDataKey& defaultFont(Font::Type type) const;
    const FontInfo& fontInfo(const FontDataKey& key) const;

    std::map<Font::Type, FontDataKey> m_defaults;
    std::map<Font::Type, std::vector<FontDataKey> > m_substitutions;
    std::vector<FontInfo> m_fonts;
};
}
