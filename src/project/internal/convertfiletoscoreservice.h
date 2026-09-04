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
#include "io/ifilesystem.h"

#include "cloud/musescorecom/imusescorecomservice.h"

#include "project/iconvertfiletoscoreservice.h"
#include "project/iprojectconfiguration.h"

namespace mu::project {
class ConvertFileToScoreService : public IConvertFileToScoreService, public muse::async::Asyncable, public muse::Contextable
{
public:
    muse::ContextInject<muse::cloud::IMuseScoreComService> museScoreComService = { this };
    muse::GlobalInject<muse::io::IFileSystem> fileSystem;
    muse::GlobalInject<IProjectConfiguration> configuration;

    explicit ConvertFileToScoreService(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    void init();
    void resumeConvert();

    const ConvertConfig& config() const override;

    bool isFileSupported(const muse::io::path_t& path) const override;
    muse::RetVal<ConvertFilesValidation> validateFiles(const muse::io::paths_t& paths) const override;
    muse::Ret validateLink(const QUrl& link) const override;

    muse::Ret startConvert(const ConvertInput& input, const muse::String& convertedFileName) override;
    muse::async::Channel<muse::Ret, muse::io::path_t> convertFinished() const override;

    muse::StringList fileNamesBeingConverted() const override;
    muse::async::Notification fileNamesBeingConvertedChanged() const override;

    muse::async::Channel<ConvertType, int> reviewRequested() const override;
    void submitReview(ConvertType type, int queueId, ReviewRating rating, const QString& comment = QString()) override;
    void submitReviewComment(ConvertType type, int queueId, const QString& comment) override;

private:
    static constexpr int MIN_RETRY_INTERVAL_MS = 60000;
    static constexpr int MAX_RETRY_INTERVAL_MS = 30 * 60000;
    static constexpr int MAX_CONSECUTIVE_POLL_FAILURES = 7; // gives up after ~1 hour

    enum class DownloadStatus {
        NotStarted,
        Downloading
    };

    struct WatchedItem {
        ConvertType type = ConvertType::Omr;
        muse::String convertedFileName;
        DownloadStatus downloadStatus = DownloadStatus::NotStarted;
        muse::cloud::ConvertStatus lastHandledStatus = muse::cloud::ConvertStatus::Unknown;
    };

    void watch(int queueId, ConvertType type, const muse::String& convertedFileName);
    void poll();
    void giveUpPolling(const muse::Ret& ret);

    void loadWatchedItems();
    void saveWatchedItems();

    muse::String convertedFileNameFor(int queueId) const;

    void onStatusChanged(const muse::cloud::ConvertQueueItem& item);
    bool shouldHandle(int queueId, muse::cloud::ConvertStatus status);

    void downloadIfNotAlready(ConvertType type, int queueId);
    void fetchScoreUrlAndDownload(ConvertType type, int queueId);
    void downloadScoreAndFinish(ConvertType type, int queueId, const muse::cloud::SignedMsczUrl& urlInfo);
    void markDownloaded(int queueId);
    void clearDownloading(int queueId);
    void finishConvert(const muse::Ret& ret, const muse::io::path_t& path = muse::io::path_t());
    void failConvert(muse::Ret ret, ConvertType type, int queueId, const muse::String& convertedFileName);

    ConvertConfig m_config;

    QTimer m_timer;
    int m_pollIntervalMs = MIN_RETRY_INTERVAL_MS;
    int m_pollFailureCount = 0;
    std::unordered_map<int /*queueId*/, WatchedItem> m_watchedItems;
    bool m_pollInProgress = false;
    muse::async::Channel<muse::Ret, muse::io::path_t> m_convertFinished;
    muse::async::Channel<ConvertType, int> m_reviewRequested;
    muse::async::Notification m_fileNamesBeingConvertedChanged;
};
}
