/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#ifndef MU_BRAILLE_BRAILLEMODULE_H
#define MU_BRAILLE_BRAILLEMODULE_H

#include "modularity/imodulesetup.h"

namespace mu::engraving {
class NotationBraille;
class BrailleConverter;
class BrailleConfiguration;
}

namespace mu::braille {
class BrailleModule : public muse::modularity::IModuleSetup
{
public:
    std::string moduleName() const override;
    void resolveImports() override;
    void registerExports() override;
    void registerUiTypes() override;
    void registerResources() override;
    void onInit(const muse::IApplication::RunMode& mode) override;

private:
    std::shared_ptr<engraving::BrailleConfiguration> m_brailleConfiguration;
    std::shared_ptr<engraving::BrailleConverter> m_brailleConverter;
    std::shared_ptr<engraving::NotationBraille> m_notationBraille;
};
}

#endif // MU_BRAILLE_BRAILLEMODULE_H
