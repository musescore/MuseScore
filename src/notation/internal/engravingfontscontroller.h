/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "inotationconfiguration.h"
#include "draw/internal/ifontsdatabase.h"
#include "ui/iuiconfiguration.h"

namespace mu::notation {
class EngravingFontsController : public muse::async::Asyncable, muse::Injectable
{
    muse::Inject<mu::notation::INotationConfiguration> configuration = { this };
    muse::Inject<mu::engraving::IEngravingFontsProvider> engravingFonts = { this };
    muse::Inject<muse::draw::IFontsDatabase> fontsDatabase = { this };
    muse::Inject<muse::ui::IUiConfiguration> uiConfiguration = { this };

public:
    void init();

private:
    void scanAllDirectories() const;
    void scanDirectory(const muse::io::path_t& path, bool isPrivate) const;
    muse::io::path_t findFontPathGlobal(const QString& fontName) const;
    muse::io::path_t findFontPathPrivate(const QString& metadataDir, const QString& fontName) const;
};
}
