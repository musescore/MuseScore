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
    muse::ContextInject<muse::cloud::IMuseScoreComService> museScoreComService = { this };
    muse::GlobalInject<muse::io::IFileSystem> fileSystem;
    muse::GlobalInject<IProjectConfiguration> configuration;

public:
    explicit ConvertFileToScoreService(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    void init();
    void resumeConvert();

    const ConvertConfig& config() const override;

    muse::Ret convert(const ConvertInput& input, const QString& convertedFileName) override;

    muse::async::Channel<muse::Ret, muse::io::path_t> convertFinished() const override;

    muse::async::Channel<int, ConvertType> reviewRequested() const override;
    void submitReview(int queueId, ReviewRating rating) override;

private:
    enum class DownloadStatus {
        NotStarted,
        Downloading
    };

    struct WatchedItem {
        ConvertType type = ConvertType::Omr;
        QString convertedFileName;
        DownloadStatus downloadStatus = DownloadStatus::NotStarted;
        muse::cloud::ConvertStatus lastHandledStatus = muse::cloud::ConvertStatus::Unknown;
    };

    void watch(int queueId, ConvertType type, const QString& convertedFileName);
    void poll();

    void loadWatchedItems();
    void saveWatchedItems();

    QString convertedFileNameFor(int queueId) const;

    void onStatusChanged(const muse::cloud::ConvertQueueItem& item);
    bool shouldHandle(int queueId, muse::cloud::ConvertStatus status);

    void downloadIfNotAlready(ConvertType type, int queueId);
    void fetchScoreUrlAndDownload(ConvertType type, int queueId);
    void downloadScoreAndFinish(const muse::cloud::SignedMsczUrl& urlInfo);
    void markDownloaded(int queueId);
    void clearDownloading(int queueId);
    void finishConvert(const muse::Ret& ret, const muse::io::path_t& path = muse::io::path_t());

    ConvertConfig m_config;

    QTimer m_timer;
    std::unordered_map<int /*queueId*/, WatchedItem> m_watchedItems;
    bool m_pollInProgress = false;
    int m_pollFailureCount = 0;
    muse::async::Channel<muse::Ret, muse::io::path_t> m_convertFinished;
    muse::async::Channel<int, ConvertType> m_reviewRequested;
};
}
