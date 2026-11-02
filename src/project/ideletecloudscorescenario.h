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

class QString;

namespace mu::project {
class IDeleteCloudScoreScenario : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IDeleteCloudScoreScenario)

public:
    virtual ~IDeleteCloudScoreScenario() = default;

    //! Confirms with the user, then deletes the score from MuseScore.com along with its local copy
    virtual void deleteScore(int scoreId, const QString& scoreName) = 0;

    //! Emitted once the score has been deleted from MuseScore.com
    virtual muse::async::Channel<int /*scoreId*/> scoreDeleted() const = 0;
};
}
