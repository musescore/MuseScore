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
#include "global/iinteractive.h"
#include "actions/iactionsdispatcher.h"

#include "cloud/musescorecom/imusescorecomservice.h"

#include "project/iprojectconfiguration.h"
#include "project/iconvertfiletoscorescenario.h"
#include "project/iconvertfiletoscoreservice.h"

namespace mu::project {
class ConvertFileToScoreScenario : public IConvertFileToScoreScenario, public muse::async::Asyncable, public muse::Contextable
{
    muse::ContextInject<muse::cloud::IMuseScoreComService> museScoreComService = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::GlobalInject<IProjectConfiguration> configuration;
    muse::ContextInject<IConvertFileToScoreService> service = { this };

public:
    explicit ConvertFileToScoreScenario(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    void init();

    const muse::cloud::ConvertConfig& convertConfig() const override;

    muse::async::Promise<ConvertSelection> selectFilesToConvert() override;
    muse::async::Promise<muse::RetVal<muse::cloud::ConvertType> > validateFiles(const muse::io::paths_t& paths) override;
    bool convertFiles(muse::cloud::ConvertType type, const muse::io::paths_t& files) override;

    muse::async::Channel<muse::Ret, muse::io::path_t> convertFinished() const override;

private:
    muse::Ret ensureAuthorization();

    bool validateAgainstConfig(muse::cloud::ConvertType type, const muse::io::paths_t& paths, const muse::cloud::ConvertConfig& config);
    void showFileTooLargeError(qint64 maxFileSizeBytes);
    void showUnsupportedFormatError(const QStringList& allowedExtensions);
    void showFileValidationError(const std::string& title, const std::string& text);

    void openFilesAndUpload(muse::cloud::ConvertType type, const muse::io::paths_t& paths);
    void showFileProcessingDialog();
    void showScoreReadyNotification(const muse::io::path_t& path);

    void askReviewRating(int queueId);

    muse::async::Channel<muse::Ret, muse::io::path_t> m_convertFinished;
};
}
