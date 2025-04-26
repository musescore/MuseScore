/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include <string>

#include "global/modularity/imoduleinterface.h"
#include "global/io/path.h"

#include "iengravingfont.h"

namespace mu::engraving {
class IEngravingFontsProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IEngravingFontsProvider)

public:
    virtual ~IEngravingFontsProvider() = default;

    virtual void addInternalFont(const std::string& name, const std::string& family, const muse::io::path_t& filePath) = 0;
    virtual void addExternalFont(const std::string& name, const std::string& family, const muse::io::path_t& filePath,
                                 const muse::io::path_t& metadataPath) = 0;
    virtual IEngravingFontPtr fontByName(const std::string& name) const = 0;
    virtual std::vector<IEngravingFontPtr> fonts() const = 0;

    virtual void setFallbackFont(const std::string& name) = 0;
    virtual IEngravingFontPtr fallbackFont() const = 0;
    virtual bool isFallbackFont(const IEngravingFont* f) const = 0;

    virtual void clearExternalFonts() = 0;

    virtual void loadAllFonts() = 0;
};
}
