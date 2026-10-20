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

#pragma once

#include <optional>
#include <unordered_set>

#include <qqmlintegration.h>

#include "abstractscoresmodel.h"

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "iprojectconfiguration.h"
#include "iconvertfiletoscorescenario.h"
#include "cloud/musescorecom/imusescorecomservice.h"

namespace mu::project {
class CloudScoresModel : public AbstractScoresModel, public muse::async::Asyncable, public muse::Contextable
{
    Q_OBJECT

    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool hasMore READ hasMore NOTIFY hasMoreChanged)

    Q_PROPERTY(int desiredRowCount READ desiredRowCount WRITE setDesiredRowCount NOTIFY desiredRowCountChanged)

    QML_ELEMENT

    muse::GlobalInject<IProjectConfiguration> configuration;
    muse::GlobalInject<muse::cloud::IMuseScoreComService> museScoreComService;
    muse::ContextInject<IConvertFileToScoreScenario> convertFileToScoreScenario = { this };

public:
    CloudScoresModel(QObject* parent = nullptr);

    enum class State {
        Fine,
        Loading,
        NotSignedIn,
        Error
    };
    Q_ENUM(State)

    void load() override;
    Q_INVOKABLE void reload();

    Q_INVOKABLE void retryAllConversions();
    Q_INVOKABLE void cancelConversion(int convertType, int convertId);

    State state() const;
    bool hasMore() const;

    // Used by the view to request more items
    int desiredRowCount() const;
    void setDesiredRowCount(int count);

signals:
    void stateChanged();
    void hasMoreChanged();

    void desiredRowCountChanged();

private:
    void setState(State state);

    void loadItemsIfNecessary(std::optional<int> refreshPage = std::nullopt);
    bool needsLoading();
    size_t loadedCloudItemCount() const;
    bool containsCloudScore(int scoreId) const;

    std::vector<QVariantMap> buildWatchedItems(const std::unordered_set<int>& downloadedScoreIds = {}) const;
    void updateWatchedItems(const std::unordered_set<int>& downloadedScoreIds = {}, bool allowRefresh = true);

    State m_state = State::Fine;
    bool m_isRequestPending = false;
    std::optional<int> m_queuedRefreshPage;

    size_t m_totalItems = muse::nidx;
    size_t m_watchedItemCount = 0;

    int m_desiredRowCount = 0;

    bool m_pollingGaveUp = false;
};
}
