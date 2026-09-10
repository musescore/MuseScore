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

#include <optional>
#include <vector>

#include <QObject>
#include <QTimer>

#include "project/iconvertfiletoscoreservice.h"

#include "async/asyncable.h"
#include "modularity/ioc.h"

#include "cloud/musescorecom/imusescorecomservice.h"
#include "io/ifilesystem.h"
#include "project/iprojectconfiguration.h"

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

    muse::Ret startConvert(const ConvertInput& input, const muse::String& convertedScoreName) override;
    muse::async::Channel<muse::Ret, ScoreInfo> convertFinished() const override;

    muse::ValNt<WatchedScoreList> watchedScores() const override;

    muse::async::Channel<PollingFailure> pollingFailed() const override;
    void retryPolling() override;

    muse::async::Channel<int> reviewRequested() const override;
    void submitReview(int scoreId, ReviewRating rating, const QString& comment = QString()) override;
    void submitReviewComment(int scoreId, const QString& comment) override;

private:
    static constexpr int MIN_RETRY_INTERVAL_MS = 60000;
    static constexpr int MAX_RETRY_INTERVAL_MS = 10 * 60000;
    static constexpr int MAX_POLL_RETRY_ATTEMPTS = 5; // gives up after ~15 minutes

    void loadWatchedScores();
    void saveWatchedScores();

    void watch(ConvertType type, int itemId, const muse::String& convertedScoreName);
    void poll();
    void resetPollState();
    void handlePollFailure(const muse::Ret& ret);
    void giveUpPolling(const muse::Ret& ret);
    void updateWatchedScores(const muse::cloud::ConvertQueueList& queue);

    void handleItem(WatchedScore& watched, muse::cloud::ConvertStatus status, muse::cloud::ConvertErrorCode errorCode,
                    std::optional<int> scoreId);
    void reportReady(WatchedScore& watched, muse::cloud::ConvertStatus status, int scoreId);

    void finishConvert(const muse::Ret& ret, const ScoreInfo& scoreInfo = ScoreInfo());

    WatchedScore* findWatchedScoreByScoreId(int scoreId);

    ConvertConfig m_config;

    QTimer m_timer;
    int m_pollIntervalMs = MIN_RETRY_INTERVAL_MS;
    int m_pollFailureCount = 0;
    std::vector<WatchedScore> m_watchedScores;
    bool m_pollInProgress = false;

    muse::async::Channel<PollingFailure> m_pollingFailed;
    muse::async::Notification m_watchedScoresChanged;
    muse::async::Channel<muse::Ret, ScoreInfo> m_convertFinished;
    muse::async::Channel<int> m_reviewRequested;
};
}
