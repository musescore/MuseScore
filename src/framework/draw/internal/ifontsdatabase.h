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

#include "global/modularity/imoduleinterface.h"
#include "global/io/path.h"

#include "../types/fontstypes.h"

namespace muse::draw {
class IFontsDatabase : public modularity::IModuleExportInterface
{
    INTERFACE_ID(muse::draw::IFontsDatabase)
public:
    virtual ~IFontsDatabase() = default;

    virtual void setDefaultFont(Font::Type type, const FontDataKey& key) = 0;
    //! NOTE Used for Qt font provider
    virtual void insertSubstitution(const String& f1, const String& substituteName) = 0;

    virtual int addFont(const FontDataKey& key, const io::path_t& path) = 0;

    virtual FontDataKey actualFont(const FontDataKey& requireKey, Font::Type type) const = 0;
    virtual std::vector<FontDataKey> substitutionFonts(Font::Type type) const = 0;
    virtual FontData fontData(const FontDataKey& requireKey, Font::Type type) const = 0;
    virtual io::path_t fontPath(const FontDataKey& requireKey, Font::Type type) const = 0;

    virtual void addAdditionalFonts(const io::path_t& path) = 0;
};
}
