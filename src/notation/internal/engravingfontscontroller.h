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

#ifndef MU_NOTATION_ENGRAVINGFONTSCONTROLLER_H
#define MU_NOTATION_ENGRAVINGFONTSCONTROLLER_H

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "inotationconfiguration.h"
#include "modularity/imodulesetup.h"

namespace mu::notation {
class EngravingFontsController : public muse::async::Asyncable, muse::modularity::IModuleSetup
{
    INJECT(mu::notation::INotationConfiguration, configuration)
    INJECT(mu::engraving::IEngravingFontsProvider, engravingFonts)

public:
    std::string moduleName() const override;
    void init();

private:
    void scanDirectory(const muse::io::path_t& path, bool isPrivate) const;
};
}

#endif // MU_NOTATION_ENGRAVINGFONTSCONTROLLER_H
