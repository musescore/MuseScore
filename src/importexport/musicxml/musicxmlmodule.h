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
#ifndef MU_IMPORTEXPORT_MUSICXMLMODULE_H
#define MU_IMPORTEXPORT_MUSICXMLMODULE_H

#include <memory>

#include "modularity/imodulesetup.h"

namespace mu::iex::musicxml {
class MusicXmlConfiguration;
class MusicXmlModule : public modularity::IModuleSetup
{
public:

    std::string moduleName() const override;
    void registerResources() override;
    void registerExports() override;
    void resolveImports() override;
    void onInit(const framework::IApplication::RunMode&) override;

private:
    std::shared_ptr<MusicXmlConfiguration> m_configuration;
};
}

#endif // MU_IMPORTEXPORT_MUSICXMLMODULE_H
