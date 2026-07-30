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

#include <unordered_map>

#include <QTimer>

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "global/iinteractive.h"
#include "global/iglobalconfiguration.h"
#include "io/ifilesystem.h"

#include "cloud/musescorecom/imusescorecomservice.h"

#include "project/iprojectconfiguration.h"
#include "project/iimportfiletoscorescenario.h"

namespace mu::project {
class ImportFileToScoreScenario : public IImportFileToScoreScenario, public muse::async::Asyncable, public muse::Contextable
{
    muse::ContextInject<muse::cloud::IMuseScoreComService> museScoreComService = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::GlobalInject<muse::IGlobalConfiguration> globalConfiguration;
    muse::GlobalInject<IProjectConfiguration> configuration;
    muse::GlobalInject<muse::io::IFileSystem> fileSystem;

public:
    explicit ImportFileToScoreScenario(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    void init();
    void resumeImport();

    muse::async::Promise<ImportSelection> selectFilesToImport() override;
    muse::async::Promise<muse::RetVal<muse::cloud::ImportType> > validateFiles(const muse::io::paths_t& paths) override;
    bool importFiles(muse::cloud::ImportType type, const muse::io::paths_t& files) override;

    muse::async::Channel<muse::Ret, muse::io::path_t> importFinished() const override;

private:
    enum class DownloadStatus {
        NotStarted,
        Downloading
    };

    struct WatchedItem {
        muse::cloud::ImportType type = muse::cloud::ImportType::Omr;
        DownloadStatus downloadStatus = DownloadStatus::NotStarted;
        muse::cloud::ImportStatus lastHandledStatus = muse::cloud::ImportStatus::Unknown;
    };

    muse::Ret ensureAuthorization();

    bool validateAgainstConfig(muse::cloud::ImportType type, const muse::io::paths_t& paths, const muse::cloud::ImportConfig& config);
    void showFileTooLargeError(qint64 maxFileSizeBytes);
    void showUnsupportedFormatError(const QStringList& allowedExtensions);
    void showFileValidationError(const std::string& title, const std::string& text);

    void openFilesAndUpload(muse::cloud::ImportType type, const muse::io::paths_t& paths);
    void upload(muse::cloud::ImportType type, const muse::cloud::ImportFileList& files);

    void watch(int queueId, muse::cloud::ImportType type);
    void poll();

    muse::io::path_t pendingImportsJsonPath() const;

    void loadWatchedItems();
    void saveWatchedItems();

    void onStatusChanged(const muse::cloud::ImportQueueItem& item);
    bool shouldHandle(int queueId, muse::cloud::ImportStatus status);

    void submitMeta(int queueId);
    void askReviewRating(int queueId);
    void submitReview(int queueId, muse::cloud::OmrReviewRating rating);
    void downloadIfNotAlready(muse::cloud::ImportType type, int queueId);
    void fetchScoreUrlAndDownload(muse::cloud::ImportType type, int queueId);
    void downloadScoreAndFinish(const muse::cloud::SignedMsczUrl& urlInfo);
    void markDownloaded(int queueId);
    void clearDownloading(int queueId);
    void finishImport(const muse::Ret& ret, const muse::io::path_t& path = muse::io::path_t());

    QTimer m_timer;
    std::unordered_map<int /*queueId*/, WatchedItem> m_watchedItems;
    bool m_pollInProgress = false;
    int m_pollFailureCount = 0;
    muse::async::Channel<muse::Ret, muse::io::path_t> m_importFinished;
};
}
