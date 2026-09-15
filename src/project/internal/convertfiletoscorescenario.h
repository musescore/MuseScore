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

#include <map>

#include <QObject>

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "global/iinteractive.h"
#include "actions/iactionsdispatcher.h"
#include "toast/itoastservice.h"

#include "context/iglobalcontext.h"

#include "cloud/musescorecom/imusescorecomservice.h"

#include "project/iprojectconfiguration.h"
#include "project/iconvertfiletoscorescenario.h"
#include "project/iconvertfiletoscoreservice.h"

namespace mu::project {
class ConvertFileToScoreScenario : public QObject, public IConvertFileToScoreScenario, public muse::async::Asyncable,
    public muse::Contextable
{
    Q_OBJECT

public:
    muse::ContextInject<muse::cloud::IMuseScoreComService> museScoreComService = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::GlobalInject<muse::toast::IToastService> toastService;
    muse::ContextInject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::ContextInject<context::IGlobalContext> globalContext = { this };
    muse::GlobalInject<IProjectConfiguration> configuration;
    muse::ContextInject<IConvertFileToScoreService> service = { this };

public:
    explicit ConvertFileToScoreScenario(const muse::modularity::ContextPtr& iocCtx, QObject* parent = nullptr);

    void init();

    const ConvertConfig& config() const override;

    bool isFileSupported(const muse::io::path_t& path) const override;
    muse::RetVal<ConvertFilesValidation> validateFiles(const muse::io::paths_t& paths) override;
    muse::Ret validateLink(const QUrl& link) override;

    void convertFiles(const muse::io::paths_t& paths = {}) override;
    muse::async::Channel<muse::Ret, ScoreInfo> convertFinished() const override;

private:
    muse::async::Promise<muse::Ret> checkConvertIsAllowed();
    muse::async::Promise<muse::Ret> ensureAuthorization();

    muse::async::Promise<ConvertSelection> selectFilesToConvert(const muse::io::paths_t& paths = {}, ConvertType type = ConvertType::Omr);

    void confirmConvert(const muse::io::paths_t& paths, ConvertType type);

    muse::Ret startConvert(const ConvertInput& input, const muse::String& convertedScoreName);

    void showValidationError(const muse::Ret& ret);

    void showCloudIsNotAvailableError();
    void showUnknownError();
    void showFileTooLargeError(qint64 maxFileSizeBytes);
    void showCombinedImageSizeTooLargeError(qint64 maxFileSizeBytes);
    void showUnsupportedFormatError();
    void showUnsupportedLinkError();
    void showMixedFileTypesError();
    void showMultiplePdfFilesError();
    void showTooManyAudioFilesError(int maxFiles);
    void showTooManyImagesError(int maxImages);

    void showFileProcessingDialog();
    void showScoreReadyNotification(const ScoreInfo& scoreInfo);
    void showConvertFailedNotification(const muse::Ret& ret);
    void showPollingFailureNotification();

    void askReviewRating(int scoreId);
    void checkPendingReview();

    muse::async::Channel<muse::Ret, ScoreInfo> m_convertFinished;
    std::map<muse::io::path_t, int /*scoreId*/> m_pendingReviews;

    bool m_retryToastShown = false;
};
}
