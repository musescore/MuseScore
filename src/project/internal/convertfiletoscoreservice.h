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

#include <functional>
#include <memory>
#include <vector>

#include <QObject>
#include <QTimer>

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "io/ifilesystem.h"

#include "cloud/musescorecom/imusescorecomservice.h"

#include "project/iconvertfiletoscoreservice.h"
#include "project/iprojectconfiguration.h"

class QBuffer;

namespace mu::project {
class ConvertFileToScoreService : public QObject, public IConvertFileToScoreService, public muse::async::Asyncable, public muse::Contextable
{
    Q_OBJECT

public:
    muse::ContextInject<muse::cloud::IMuseScoreComService> museScoreComService = { this };
    muse::GlobalInject<muse::io::IFileSystem> fileSystem;
    muse::GlobalInject<IProjectConfiguration> configuration;

    explicit ConvertFileToScoreService(const muse::modularity::ContextPtr& iocCtx, QObject* parent = nullptr)
        : QObject(parent), muse::Contextable(iocCtx) {}

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

    muse::async::Channel<PollingFailure> pollingFailed() const override;
    void retryPolling() override;

    muse::async::Channel<ConvertType, int> reviewRequested() const override;
    void submitReview(ConvertType type, int itemId, ReviewRating rating, const QString& comment = QString()) override;
    void submitReviewComment(ConvertType type, int itemId, const QString& comment) override;

private:
    static constexpr int MIN_RETRY_INTERVAL_MS = 60000;
    static constexpr int MAX_RETRY_INTERVAL_MS = 10 * 60000;
    static constexpr int MAX_POLL_RETRY_ATTEMPTS = 5; // gives up after ~15 minutes

    static constexpr int MAX_FS_RETRY_ATTEMPTS = 5;
    static constexpr int FS_RETRY_INTERVAL_MS = 100;

    enum class DownloadStatus {
        NotStarted,
        Downloading
    };

    struct WatchedItem {
        int id = 0;
        ConvertType type = ConvertType::Omr;
        muse::String convertedFileName;
        muse::cloud::ConvertStatus convertStatus = muse::cloud::ConvertStatus::Unknown;
        DownloadStatus downloadStatus = DownloadStatus::NotStarted;

        bool operator==(const WatchedItem& other) const
        {
            return id == other.id
                   && type == other.type
                   && convertedFileName == other.convertedFileName
                   && convertStatus == other.convertStatus
                   && downloadStatus == other.downloadStatus;
        }
    };

    void watch(ConvertType type, int itemId, const muse::String& convertedFileName);
    void poll();
    void resetPollState();
    void handlePollFailure(const muse::Ret& ret);
    void giveUpPolling(const muse::Ret& ret);
    void updateWatchedItems(const muse::cloud::ConvertQueueList& queue);

    void loadWatchedItems();
    void saveWatchedItems();

    std::vector<WatchedItem>::iterator findWatchedItem(ConvertType type, int itemId);
    void eraseWatchedItem(ConvertType type, int itemId);

    void handleItem(WatchedItem& item, muse::cloud::ConvertStatus status, muse::cloud::ConvertErrorCode errorCode);

    void downloadIfNotAlready(WatchedItem& item);
    void fetchScoreUrlAndDownload(ConvertType type, int itemId, const muse::String& convertedFileName);
    void downloadScoreAndFinish(ConvertType type, int itemId, const muse::String& convertedFileName,
                                const muse::cloud::SignedMsczUrl& urlInfo);
    void writeConvertedScore(const muse::String& convertedFileName, const std::shared_ptr<QBuffer>& scoreData,
                             std::function<void(const muse::RetVal<muse::io::path_t>&)> onFinished);
    void makePathWithRetry(const muse::io::path_t& dir, int attempt, std::function<void(const muse::Ret&)> onFinished);
    void writeFileWithRetry(const muse::io::path_t& path, const std::shared_ptr<QBuffer>& scoreData, int attempt,
                            std::function<void(const muse::Ret&)> onFinished);
    void markDownloaded(ConvertType type, int itemId);
    void clearDownloading(ConvertType type, int itemId);
    void finishConvert(const muse::Ret& ret, const muse::io::path_t& path = muse::io::path_t());
    void failConvert(muse::Ret ret, ConvertType type, int itemId, const muse::String& convertedFileName);

    ConvertConfig m_config;

    QTimer m_timer;
    int m_pollIntervalMs = MIN_RETRY_INTERVAL_MS;
    int m_pollFailureCount = 0;
    std::vector<WatchedItem> m_watchedItems;
    bool m_pollInProgress = false;
    muse::async::Channel<muse::Ret, muse::io::path_t> m_convertFinished;
    muse::async::Channel<ConvertType, int> m_reviewRequested;
    muse::async::Notification m_fileNamesBeingConvertedChanged;
    muse::async::Channel<PollingFailure> m_pollingFailed;
};
}
