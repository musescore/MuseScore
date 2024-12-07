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

#ifndef MU_ENGRAVING_ENGRAVINGFONTSPROVIDER_H
#define MU_ENGRAVING_ENGRAVINGFONTSPROVIDER_H

#include <vector>

#include "iengravingfontsprovider.h"
#include "modularity/ioc.h"

#include "engravingfont.h"

namespace mu::engraving {
class EngravingFont;
class EngravingFontsProvider : public IEngravingFontsProvider, public muse::Injectable
{
public:

    EngravingFontsProvider(const muse::modularity::ContextPtr& iocCtx);

    void addFont(const std::string& name, const std::string& family, const muse::io::path_t& filePath) override;
    IEngravingFontPtr fontByName(const std::string& name) const override;
    std::vector<IEngravingFontPtr> fonts() const override;

    void clearUserFonts() override;
    void addUserFont(const std::string& name, const std::string& family, const muse::io::path_t& filePath) override;

    void setFallbackFont(const std::string& name) override;
    IEngravingFontPtr fallbackFont() const override;
    bool isFallbackFont(const IEngravingFont* f) const override;

    void loadAllFonts() override;

private:

    std::shared_ptr<EngravingFont> doFontByName(const std::string& name) const;
    std::shared_ptr<EngravingFont> doFallbackFont() const;

    struct Fallback {
        std::string name;
        std::shared_ptr<EngravingFont> font;
    };

    mutable Fallback m_fallback;
    std::vector<std::shared_ptr<EngravingFont> > m_symbolFonts;
    std::vector<std::shared_ptr<EngravingFont> > m_userSymbolFonts;
};
}

#endif // MU_ENGRAVING_ENGRAVINGFONTSPROVIDER_H
