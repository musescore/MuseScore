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
#include "project/iconvertfiletoscorescenario.h"

namespace mu::project {
class ConvertFileToScoreScenario : public IConvertFileToScoreScenario, public muse::async::Asyncable, public muse::Contextable
{
    muse::ContextInject<muse::cloud::IMuseScoreComService> museScoreComService = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::GlobalInject<muse::IGlobalConfiguration> globalConfiguration;
    muse::GlobalInject<IProjectConfiguration> configuration;
    muse::GlobalInject<muse::io::IFileSystem> fileSystem;

public:
    explicit ConvertFileToScoreScenario(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    void init();
    void resumeConvert();

    muse::async::Promise<ConvertSelection> selectFilesToConvert() override;
    muse::async::Promise<muse::RetVal<muse::cloud::ConvertType> > validateFiles(const muse::io::paths_t& paths) override;
    bool convertFiles(muse::cloud::ConvertType type, const muse::io::paths_t& files) override;

    muse::async::Channel<muse::Ret, muse::io::path_t> convertFinished() const override;

private:
    enum class DownloadStatus {
        NotStarted,
        Downloading
    };

    struct WatchedItem {
        muse::cloud::ConvertType type = muse::cloud::ConvertType::Omr;
        muse::io::paths_t filePaths;
        DownloadStatus downloadStatus = DownloadStatus::NotStarted;
        muse::cloud::ConvertStatus lastHandledStatus = muse::cloud::ConvertStatus::Unknown;
    };

    muse::Ret ensureAuthorization();

    bool validateAgainstConfig(muse::cloud::ConvertType type, const muse::io::paths_t& paths, const muse::cloud::ConvertConfig& config);
    void showFileTooLargeError(qint64 maxFileSizeBytes);
    void showUnsupportedFormatError(const QStringList& allowedExtensions);
    void showFileValidationError(const std::string& title, const std::string& text);

    void openFilesAndUpload(muse::cloud::ConvertType type, const muse::io::paths_t& paths);
    void upload(muse::cloud::ConvertType type, const muse::cloud::ConvertFileList& files, const muse::io::paths_t& filePaths);

    void watch(int queueId, muse::cloud::ConvertType type, const muse::io::paths_t& filePaths);
    void poll();

    muse::io::path_t pendingConvertsJsonPath() const;

    void loadWatchedItems();
    void saveWatchedItems();

    muse::Ret attachFailedFiles(muse::Ret ret, int queueId) const;

    void onStatusChanged(const muse::cloud::ConvertQueueItem& item);
    bool shouldHandle(int queueId, muse::cloud::ConvertStatus status);

    void askReviewRating(muse::cloud::ConvertType type, int queueId);
    void downloadIfNotAlready(muse::cloud::ConvertType type, int queueId);
    void fetchScoreUrlAndDownload(muse::cloud::ConvertType type, int queueId);
    void downloadScoreAndFinish(const muse::cloud::SignedMsczUrl& urlInfo);
    void markDownloaded(int queueId);
    void clearDownloading(int queueId);
    void finishConvert(const muse::Ret& ret, const muse::io::path_t& path = muse::io::path_t());

    QTimer m_timer;
    std::unordered_map<int /*queueId*/, WatchedItem> m_watchedItems;
    bool m_pollInProgress = false;
    int m_pollFailureCount = 0;
    muse::async::Channel<muse::Ret, muse::io::path_t> m_convertFinished;
};
}
