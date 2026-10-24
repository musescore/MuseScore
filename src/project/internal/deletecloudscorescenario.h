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

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "interactive/iinteractive.h"
#include "io/ifilesystem.h"

#include "cloud/musescorecom/imusescorecomservice.h"

#include "project/iprojectconfiguration.h"
#include "project/irecentfilescontroller.h"
#include "project/ideletecloudscorescenario.h"

namespace mu::project {
class DeleteCloudScoreScenario : public IDeleteCloudScoreScenario, public muse::async::Asyncable, public muse::Contextable
{
public:
    muse::GlobalInject<IProjectConfiguration> configuration;
    muse::GlobalInject<muse::io::IFileSystem> fileSystem;
    muse::GlobalInject<muse::cloud::IMuseScoreComService> museScoreComService;
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<IRecentFilesController> recentFilesController = { this };

public:
    explicit DeleteCloudScoreScenario(const muse::modularity::ContextPtr& iocCtx);

    void deleteScore(int scoreId, const QString& scoreName) override;
    muse::async::Channel<int /*scoreId*/> scoreDeleted() const override;

private:
    void doDeleteScore(int scoreId);

    muse::async::Channel<int /*scoreId*/> m_scoreDeleted;
};
}
