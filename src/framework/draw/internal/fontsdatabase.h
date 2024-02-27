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
#ifndef MU_DRAW_FONTSDATABASE_H
#define MU_DRAW_FONTSDATABASE_H

#include <vector>
#include <map>

#include "ifontsdatabase.h"

namespace mu::draw {
class FontsDatabase : public IFontsDatabase
{
public:
    FontsDatabase() = default;

    void setDefaultFont(mu::draw::Font::Type type, const FontDataKey& key) override;

    int addFont(const FontDataKey& key, const mu::io::path_t& path) override;

    FontDataKey actualFont(const FontDataKey& requireKey, mu::draw::Font::Type type) const override;
    std::vector<FontDataKey> substitutionFonts(mu::draw::Font::Type type) const override;
    FontData fontData(const FontDataKey& requireKey, mu::draw::Font::Type type) const override;
    mu::io::path_t fontPath(const FontDataKey& requireKey, mu::draw::Font::Type type) const override;

    void addAdditionalFonts(const mu::io::path_t& path) override;

private:

    struct FontInfo {
        int id = -1;
        FontDataKey key;
        mu::io::path_t path;

        bool valid() const { return id > -1; }
    };

    const FontDataKey& defaultFont(mu::draw::Font::Type type) const;
    const FontInfo& fontInfo(const FontDataKey& key) const;

    std::map<mu::draw::Font::Type, FontDataKey> m_defaults;
    std::map<mu::draw::Font::Type, std::vector<FontDataKey> > m_substitutions;
    std::vector<FontInfo> m_fonts;
};
}
#endif // MU_DRAW_FONTSDATABASE_H
