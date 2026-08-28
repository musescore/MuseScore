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

#include "async/channel.h"
#include "io/path.h"
#include "types/ret.h"

#include "types/converttypes.h"

namespace mu::project {
class IConvertFileToScoreService : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IConvertFileToScoreService)

public:
    virtual ~IConvertFileToScoreService() = default;

    virtual const ConvertConfig& config() const = 0;

    virtual muse::Ret convert(const ConvertInput& input, const QString& convertedFileName) = 0;
    virtual muse::async::Channel<muse::Ret, muse::io::path_t> convertFinished() const = 0;

    virtual muse::async::Channel<int /*queueId*/, ConvertType> reviewRequested() const = 0;
    virtual void submitReview(int queueId, ReviewRating rating) = 0;
};

using IConvertFileToScoreServicePtr = std::shared_ptr<IConvertFileToScoreService>;
}
