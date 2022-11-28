/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef MU_ENGRAVING_SYMBOLFONTSPROVIDER_H
#define MU_ENGRAVING_SYMBOLFONTSPROVIDER_H

#include <vector>

#include "isymbolfontsprovider.h"

#include "symbolfont.h"

namespace mu::engraving {
class SymbolFont;
class SymbolFontsProvider : public ISymbolFontsProvider
{
public:

    void addFont(const std::string& name, const std::string& family, const io::path_t& filePath) override;
    ISymbolFontPtr fontByName(const std::string& name) const override;
    std::vector<ISymbolFontPtr> fonts() const override;

    void setFallbackFont(const std::string& name) override;
    ISymbolFontPtr fallbackFont() const override;
    bool isFallbackFont(const ISymbolFont* f) const override;

private:

    std::shared_ptr<SymbolFont> doFontByName(const std::string& name) const;
    std::shared_ptr<SymbolFont> doFallbackFont() const;

    struct Fallback {
        std::string name;
        std::shared_ptr<SymbolFont> font;
    };

    mutable Fallback m_fallback;
    std::vector<std::shared_ptr<SymbolFont> > m_symbolFonts;
};
}

#endif // MU_ENGRAVING_SYMBOLFONTSPROVIDER_H
