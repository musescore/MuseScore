/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "modularity/imoduleinterface.h"

#include "async/promise.h"
#include "async/channel.h"
#include "io/path.h"
#include "types/retval.h"

#include "cloud/musescorecom/importtypes.h"

namespace mu::project {
struct ImportSelection {
    muse::cloud::ImportType type = muse::cloud::ImportType::Omr;
    muse::io::paths_t paths;
};

class IImportFileToScoreScenario : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IImportFileToScoreScenario)

public:
    virtual ~IImportFileToScoreScenario() = default;

    virtual muse::async::Promise<ImportSelection> selectFilesToImport() = 0;
    virtual muse::async::Promise<muse::RetVal<muse::cloud::ImportType> > validateFiles(const muse::io::paths_t& paths) = 0;
    virtual bool importFiles(muse::cloud::ImportType type, const muse::io::paths_t& files) = 0;

    virtual muse::async::Channel<muse::Ret, muse::io::path_t> importFinished() const = 0;
};

using IImportFileToScoreScenarioPtr = std::shared_ptr<IImportFileToScoreScenario>;
}
