/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "cloudscoresmodel.h"

#include "containers.h"
#include "dataformatter.h"
#include "types/datetime.h"

#include "log.h"

using namespace muse;
using namespace mu::project;

static const int BATCH_SIZE = 20;

static constexpr int PROCESSING_STATUS_PROCESSING = 0;
static constexpr int PROCESSING_STATUS_FAILED = 1;

CloudScoresModel::CloudScoresModel(QObject* parent)
    : AbstractScoresModel(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void CloudScoresModel::load()
{
    auto onUserAuthorizedChanged = [this](bool authorized) {
        if (authorized) {
            setState(State::Loading);
            loadItemsIfNecessary();
        } else {
            setState(State::NotSignedIn);
        }
    };

    ValCh<bool> authorized = museScoreComService()->authorization()->userAuthorized();

    onUserAuthorizedChanged(authorized.val);

    authorized.ch.onReceive(this, onUserAuthorizedChanged);

    connect(this, &CloudScoresModel::desiredRowCountChanged, this, [this]() {
        loadItemsIfNecessary();
    });

    convertFileToScoreScenario()->watchedScores().notification.onNotify(this, [this]() {
        updateWatchedItems();
    });

    convertFileToScoreScenario()->pollingFailed().onReceive(this, [this](const PollingFailure& failure) {
        if (failure.gaveUp) {
            m_pollingGaveUp = true;
            updateWatchedItems();
        }
    });
}

void CloudScoresModel::reload()
{
    beginResetModel();

    m_items = buildWatchedItems();
    m_watchedItemCount = m_items.size();
    m_totalItems = muse::nidx;
    m_desiredRowCount = 0;

    endResetModel();

    emit hasMoreChanged();
    emit desiredRowCountChanged();

    setState(State::Loading);
}

void CloudScoresModel::retryAllConversions()
{
    m_pollingGaveUp = false;
    convertFileToScoreScenario()->retryPolling();
    updateWatchedItems();
}

void CloudScoresModel::cancelConversion(int convertType, int convertId)
{
    convertFileToScoreScenario()->cancelConversion(static_cast<ConvertType>(convertType), convertId);
}

CloudScoresModel::State CloudScoresModel::state() const
{
    return m_state;
}

void CloudScoresModel::setState(State state)
{
    if (m_state == state) {
        return;
    }

    m_state = state;
    emit stateChanged();
}

bool CloudScoresModel::hasMore() const
{
    return m_totalItems == muse::nidx || loadedCloudItemCount() < m_totalItems;
}

int CloudScoresModel::desiredRowCount() const
{
    return m_desiredRowCount;
}

void CloudScoresModel::setDesiredRowCount(int count)
{
    if (m_desiredRowCount == count) {
        return;
    }

    m_desiredRowCount = count;
    emit desiredRowCountChanged();
}

void CloudScoresModel::loadItemsIfNecessary(std::optional<int> refreshPage)
{
    if (m_isRequestPending) {
        if (refreshPage.has_value()) {
            m_queuedRefreshPage = refreshPage;
        }
        return;
    }

    if (m_state == State::Error || m_state == State::NotSignedIn) {
        return;
    }

    const bool isRefresh = refreshPage.has_value();

    if (!isRefresh && !needsLoading()) {
        setState(State::Fine);
        return;
    }

    int page = refreshPage.value_or(static_cast<int>(loadedCloudItemCount()) / BATCH_SIZE + 1);

    if (!isRefresh) {
        setState(State::Loading);
    }

    m_isRequestPending = true;

    museScoreComService()->downloadScoresList(BATCH_SIZE, page)
    .onResolve(this, [this, isRefresh](const cloud::ScoresList& scoresList) {
        std::unordered_set<int> downloadedScoreIds;
        downloadedScoreIds.reserve(scoresList.items.size());
        for (const cloud::ScoresList::Item& item : scoresList.items) {
            downloadedScoreIds.insert(item.id);
        }

        updateWatchedItems(downloadedScoreIds, /*allowRefresh*/ false);
        int insertAt = isRefresh ? static_cast<int>(m_watchedItemCount) : static_cast<int>(m_items.size());

        for (const cloud::ScoresList::Item& item : scoresList.items) {
            if (containsCloudScore(item.id)) {
                continue;
            }

            QVariantMap obj;

            obj[NAME_KEY] = item.title;
            obj[PATH_KEY] = configuration()->cloudProjectPath(item.id).toQString();
            obj[SUFFIX_KEY] = "";
            obj[FILE_SIZE_KEY] = (item.fileSize > 0) ? DataFormatter::formatFileSize(item.fileSize).toQString() : QString();
            obj[IS_CLOUD_KEY] = true;
            obj[CLOUD_SCORE_ID_KEY] = item.id;
            obj[TIME_SINCE_MODIFIED_KEY] = DataFormatter::formatTimeSince(Date::fromQDate(item.lastModified.date())).toQString();
            obj[THUMBNAIL_URL_KEY] = item.thumbnailUrl;
            obj[IS_CREATE_NEW_KEY] = false;
            obj[IS_NO_RESULTS_FOUND_KEY] = false;
            obj[CLOUD_VISIBILITY_KEY] = static_cast<int>(item.visibility);
            obj[CLOUD_VIEW_COUNT_KEY] = item.viewCount;

            beginInsertRows(QModelIndex(), insertAt, insertAt);
            m_items.insert(m_items.begin() + insertAt, obj);
            endInsertRows();
            ++insertAt;
        }

        m_totalItems = scoresList.meta.totalScoresCount;
        emit hasMoreChanged();

        m_isRequestPending = false;

        if (m_queuedRefreshPage) {
            const int queuedPage = *m_queuedRefreshPage;
            m_queuedRefreshPage.reset();
            loadItemsIfNecessary(queuedPage);
        } else if (!isRefresh) {
            loadItemsIfNecessary();
        }
    })
    .onReject(this, [this, isRefresh](int code, const std::string& err) {
        LOGE() << "Loading scores list failed: [" << code << "] " << err;
        m_isRequestPending = false;

        if (m_queuedRefreshPage) {
            const int queuedPage = *m_queuedRefreshPage;
            m_queuedRefreshPage.reset();
            loadItemsIfNecessary(queuedPage);
        } else if (!isRefresh) {
            setState(State::Error);
        }
    });
}

bool CloudScoresModel::needsLoading()
{
    return hasMore() && static_cast<int>(m_items.size()) < m_desiredRowCount;
}

size_t CloudScoresModel::loadedCloudItemCount() const
{
    return m_items.size() - m_watchedItemCount;
}

bool CloudScoresModel::containsCloudScore(int scoreId) const
{
    for (size_t i = m_watchedItemCount; i < m_items.size(); ++i) {
        if (m_items.at(i).value(CLOUD_SCORE_ID_KEY).toInt() == scoreId) {
            return true;
        }
    }

    return false;
}

std::vector<QVariantMap> CloudScoresModel::buildWatchedItems(const std::unordered_set<int>& downloadedScoreIds) const
{
    const WatchedScoreList watchedScores = convertFileToScoreScenario()->watchedScores().val;
    std::vector<QVariantMap> items;

    for (const WatchedScore& watchedScore : watchedScores) {
        if (watchedScore.conversion.status != ConvertStatus::Processing) {
            continue;
        }

        if (watchedScore.scoreId
            && (containsCloudScore(*watchedScore.scoreId) || muse::contains(downloadedScoreIds, *watchedScore.scoreId))) {
            //! NOTE: score already exists, we just haven't updated its status locally yet
            continue;
        }

        QVariantMap obj;
        obj[NAME_KEY] = watchedScore.name.toQString();
        obj[IS_CREATE_NEW_KEY] = false;
        obj[IS_NO_RESULTS_FOUND_KEY] = false;
        obj[PROCESSING_STATUS_KEY] = m_pollingGaveUp ? PROCESSING_STATUS_FAILED : PROCESSING_STATUS_PROCESSING;
        obj[CONVERT_ID_KEY] = watchedScore.conversion.id;
        obj[CONVERT_TYPE_KEY] = static_cast<int>(watchedScore.conversion.type);
        obj[IS_CLOUD_KEY] = false;
        obj[CLOUD_VISIBILITY_KEY] = static_cast<int>(cloud::Visibility::Private);
        items.push_back(obj);
    }

    return items;
}

void CloudScoresModel::updateWatchedItems(const std::unordered_set<int>& downloadedScoreIds, bool allowRefresh)
{
    const std::vector<QVariantMap> watchedItems = buildWatchedItems(downloadedScoreIds);
    const bool watchedItemFinished = watchedItems.size() < m_watchedItemCount;

    if (m_watchedItemCount > 0) {
        beginRemoveRows(QModelIndex(), 0, static_cast<int>(m_watchedItemCount) - 1);
        m_items.erase(m_items.begin(), m_items.begin() + m_watchedItemCount);
        endRemoveRows();
        m_watchedItemCount = 0;
    }

    if (!watchedItems.empty()) {
        beginInsertRows(QModelIndex(), 0, static_cast<int>(watchedItems.size()) - 1);
        m_items.insert(m_items.begin(), watchedItems.begin(), watchedItems.end());
        endInsertRows();

        m_watchedItemCount = watchedItems.size();
    }

    if (watchedItemFinished && allowRefresh) {
        //! NOTE: a conversion finished - fetch the newest page to pick it up
        loadItemsIfNecessary(1);
    }
}
