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
#ifndef MUSE_DRAW_DRAWMODULE_H
#define MUSE_DRAW_DRAWMODULE_H

#include <memory>

#include "global/modularity/imodulesetup.h"

namespace muse::draw {
class FontsEngine;
class DrawModule : public modularity::IModuleSetup
{
public:
    std::string moduleName() const override;
    void registerExports() override;
    void onInit(const IApplication::RunMode& mode) override;

private:

    std::shared_ptr<FontsEngine> m_fontsEngine;
};
}

#endif // MUSE_DRAW_DRAWMODULE_H
