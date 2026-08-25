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

#include "cloud/musescorecom/converttypes.h"

namespace mu::project {
struct ConvertSelection {
    muse::cloud::ConvertType type = muse::cloud::ConvertType::Omr;
    muse::io::paths_t paths;
};

class IConvertFileToScoreScenario : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IConvertFileToScoreScenario)

public:
    virtual ~IConvertFileToScoreScenario() = default;

    virtual const muse::cloud::ConvertConfig& convertConfig() const = 0;

    virtual muse::async::Promise<ConvertSelection> selectFilesToConvert() = 0;
    virtual muse::async::Promise<muse::RetVal<muse::cloud::ConvertType> > validateFiles(const muse::io::paths_t& paths) = 0;
    virtual bool convertFiles(muse::cloud::ConvertType type, const muse::io::paths_t& files) = 0;

    virtual muse::async::Channel<muse::Ret, muse::io::path_t> convertFinished() const = 0;
};

using IConvertFileToScoreScenarioPtr = std::shared_ptr<IConvertFileToScoreScenario>;
}
