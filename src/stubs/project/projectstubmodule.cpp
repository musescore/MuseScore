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
#include "projectstubmodule.h"

#include "modularity/ioc.h"

#include "projectconfigurationstub.h"
#include "recentfilescontrollerstub.h"

using namespace mu::project;
using namespace muse::modularity;

std::string ProjectModule::moduleName() const
{
    return "notation_stub";
}

void ProjectModule::registerExports()
{
    ioc()->registerExport<IProjectConfiguration>(moduleName(), new ProjectConfigurationStub());
    ioc()->registerExport<IRecentFilesController>(moduleName(), new RecentFilesControllerStub());
}
