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
#include <gmock/gmock.h>

#include <QDateTime>

#include "async/promise.h"
#include "global/dataformatter.h"
#include "global/io/path.h"
#include "global/types/datetime.h"

#include "project/qml/MuseScore/Project/internal/ScoresPage/cloudscoresmodel.h"

#include "project/tests/mocks/convertfiletoscorescenariomock.h"
#include "project/tests/mocks/deletecloudscorescenariomock.h"
#include "project/tests/mocks/projectconfigurationmock.h"
#include "cloud/tests/mocks/authorizationservicemock.h"
#include "cloud/tests/mocks/musescorecomservicemock.h"

using namespace ::testing;
using namespace mu::project;
using namespace muse;
using namespace muse::cloud;

namespace {
using ScoresListPromise = async::Promise<ScoresList>;

constexpr int BATCH_SIZE = 20;

constexpr int STATUS_PROCESSING = 0;
constexpr int STATUS_FAILED = 1;

std::vector<int> idRange(int first, int last)
{
    std::vector<int> ids;
    ids.reserve(last - first + 1);

    for (int id = first; id <= last; ++id) {
        ids.push_back(id);
    }

    return ids;
}

ScoresList makeScoresList(const std::vector<int>& ids, int totalScoresCount)
{
    ScoresList list;
    list.meta.totalScoresCount = totalScoresCount;
    list.items.reserve(ids.size());

    for (int id : ids) {
        ScoresList::Item item;
        item.id = id;
        item.title = QString("Score %1").arg(id);
        list.items.push_back(item);
    }

    return list;
}

WatchedScore makeWatchedScore(int convertId, ConvertStatus status = ConvertStatus::Processing,
                              std::optional<int> scoreId = std::nullopt)
{
    WatchedScore score;
    score.conversion.id = convertId;
    score.conversion.type = ConvertType::Omr;
    score.conversion.status = status;
    score.scoreId = scoreId;
    score.name = String::fromStdString("Conversion " + std::to_string(convertId));

    return score;
}

UploadingScore makeUploadingScore(int uploadId)
{
    UploadingScore score;
    score.uploadId = uploadId;
    score.name = String::fromStdString("Upload " + std::to_string(uploadId));

    return score;
}
}

namespace mu::project {
class Project_CloudScoresModelTest : public ::testing::Test
{
protected:
    struct Request {
        int batchSize = 0;
        int page = 0;
        ScoresListPromise::Resolve resolve;
        ScoresListPromise::Reject reject;
    };

    void SetUp() override
    {
        m_museScoreComService = std::make_shared<NiceMock<MuseScoreComServiceMock> >();
        m_authorization = std::make_shared<NiceMock<AuthorizationServiceMock> >();
        m_scenario = std::make_shared<NiceMock<ConvertFileToScoreScenarioMock> >();
        m_deleteScenario = std::make_shared<NiceMock<DeleteCloudScoreScenarioMock> >();
        m_configuration = std::make_shared<NiceMock<ProjectConfigurationMock> >();

        m_model.museScoreComService.set(m_museScoreComService);
        m_model.convertFileToScoreScenario.set(m_scenario);
        m_model.deleteCloudScoreScenario.set(m_deleteScenario);
        m_model.configuration.set(m_configuration);

        ON_CALL(*m_museScoreComService, authorization())
        .WillByDefault(Return(m_authorization));
        ON_CALL(*m_authorization, userAuthorized())
        .WillByDefault(ReturnPointee(&m_userAuthorized));

        ON_CALL(*m_scenario, watchedScores())
        .WillByDefault(ReturnPointee(&m_watchedScores));
        ON_CALL(*m_scenario, uploadingScores())
        .WillByDefault(ReturnPointee(&m_uploadingScores));
        ON_CALL(*m_scenario, pollingStatusChanged())
        .WillByDefault(Return(m_pollingStatusChanged));

        ON_CALL(*m_deleteScenario, scoreDeleted())
        .WillByDefault(Return(m_scoreDeleted));

        //! NOTE: the requests stay pending until a test resolves or rejects them
        ON_CALL(*m_museScoreComService, downloadScoresList(_, _))
        .WillByDefault(Invoke([this](int batchSize, int page) {
            return async::make_promise<ScoresList>([this, batchSize, page](ScoresListPromise::Resolve resolve,
                                                                           ScoresListPromise::Reject reject) {
                m_requests.push_back(Request { batchSize, page, resolve, reject });
                return ScoresListPromise::dummy_result();
            }, async::PromiseType::AsyncByBody);
        }));
    }

    void loadModel(bool signedIn = true)
    {
        m_userAuthorized.val = signedIn;
        m_model.load();
    }

    void loadScores(const ScoresList& scoresList, int desiredRowCount = BATCH_SIZE)
    {
        loadModel();
        m_model.setDesiredRowCount(desiredRowCount);
        resolveLastRequest(scoresList);
    }

    void resolveLastRequest(const ScoresList& scoresList)
    {
        ASSERT_FALSE(m_requests.empty());
        (void)m_requests.back().resolve(scoresList);
    }

    void rejectLastRequest()
    {
        ASSERT_FALSE(m_requests.empty());
        (void)m_requests.back().reject(int(Ret::Code::UnknownError), "error");
    }

    void sendPollingGaveUp(bool gaveUp)
    {
        PollingFailure failure;
        failure.gaveUp = gaveUp;
        m_pollingStatusChanged.send(failure);
    }

    QVariant data(int row, const QByteArray& roleName) const
    {
        return m_model.data(m_model.index(row, 0), m_model.roleNames().key(roleName));
    }

    QVariantMap score(int row) const
    {
        return data(row, "score").toMap();
    }

    QStringList names() const
    {
        const int rowCount = m_model.rowCount();

        QStringList result;
        result.reserve(rowCount);

        for (int row = 0; row < rowCount; ++row) {
            result << data(row, "name").toString();
        }

        return result;
    }

    std::vector<Request> m_requests;

    ValCh<bool> m_userAuthorized;
    ValNt<WatchedScoreList> m_watchedScores;
    ValNt<UploadingScoreList> m_uploadingScores;
    async::Channel<PollingStatus> m_pollingStatusChanged;
    async::Channel<int> m_scoreDeleted;

    CloudScoresModel m_model;

    std::shared_ptr<MuseScoreComServiceMock> m_museScoreComService;
    std::shared_ptr<AuthorizationServiceMock> m_authorization;
    std::shared_ptr<ConvertFileToScoreScenarioMock> m_scenario;
    std::shared_ptr<DeleteCloudScoreScenarioMock> m_deleteScenario;
    std::shared_ptr<ProjectConfigurationMock> m_configuration;
};
}

// ==================================================
// Initial state
// ==================================================

TEST_F(Project_CloudScoresModelTest, InitialState_IsEmpty)
{
    // [GIVEN] A freshly created model
    // [THEN] It has no scores, and nothing is known about the server yet
    EXPECT_EQ(m_model.rowCount(), 0);
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Fine);
    EXPECT_TRUE(m_model.hasMore());
    EXPECT_EQ(m_model.desiredRowCount(), 0);
}

TEST_F(Project_CloudScoresModelTest, RoleNames)
{
    const QList<QByteArray> roles = m_model.roleNames().values();

    EXPECT_EQ(roles.size(), 4);
    EXPECT_TRUE(roles.contains("name"));
    EXPECT_TRUE(roles.contains("isNoResultsFound"));
    EXPECT_TRUE(roles.contains("processingStatus"));
    EXPECT_TRUE(roles.contains("score"));
}

// ==================================================
// load()
// ==================================================

TEST_F(Project_CloudScoresModelTest, Load_NotSignedIn_DoesNotRequestScores)
{
    // [WHEN] Loading the model while signed out
    loadModel(false /*signedIn*/);

    // [THEN] The model reports that the user is not signed in
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::NotSignedIn);

    // [WHEN] The view asks for some rows
    m_model.setDesiredRowCount(BATCH_SIZE);

    // [THEN] Nothing is requested
    EXPECT_TRUE(m_requests.empty());
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::NotSignedIn);
}

TEST_F(Project_CloudScoresModelTest, Load_SignedIn_NoRowsDesired_DoesNotRequestScores)
{
    // [WHEN] Loading the model while signed in, but the view hasn't asked for any rows yet
    loadModel();

    // [THEN] Nothing is requested
    EXPECT_TRUE(m_requests.empty());
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Fine);
}

// ==================================================
// setDesiredRowCount()
// ==================================================

TEST_F(Project_CloudScoresModelTest, SetDesiredRowCount_SameValue_DoesNothing)
{
    // [THEN] Count the desiredRowCountChanged notifications
    int desiredRowCountChangedCount = 0;
    QObject::connect(&m_model, &CloudScoresModel::desiredRowCountChanged, [&desiredRowCountChangedCount]() {
        ++desiredRowCountChangedCount;
    });

    // [WHEN] Setting the same value twice
    m_model.setDesiredRowCount(BATCH_SIZE);
    m_model.setDesiredRowCount(BATCH_SIZE);

    // [THEN] Only the first change is notified
    EXPECT_EQ(m_model.desiredRowCount(), BATCH_SIZE);
    EXPECT_EQ(desiredRowCountChangedCount, 1);
}

TEST_F(Project_CloudScoresModelTest, SetDesiredRowCount_RequestsFirstPage)
{
    // [GIVEN] A loaded model
    loadModel();

    // [WHEN] The view asks for some rows
    m_model.setDesiredRowCount(BATCH_SIZE);

    // [THEN] The first page is requested
    ASSERT_EQ(m_requests.size(), 1);
    EXPECT_EQ(m_requests.back().batchSize, BATCH_SIZE);
    EXPECT_EQ(m_requests.back().page, 1);
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Loading);
}

TEST_F(Project_CloudScoresModelTest, SetDesiredRowCount_WhileRequestPending_DoesNotRequestAgain)
{
    // [GIVEN] A pending request
    loadModel();
    m_model.setDesiredRowCount(BATCH_SIZE);

    // [WHEN] The view asks for more rows
    m_model.setDesiredRowCount(2 * BATCH_SIZE);

    // [THEN] There is still a single request
    EXPECT_EQ(m_requests.size(), 1);
}

// ==================================================
// Scores list loaded
// ==================================================

TEST_F(Project_CloudScoresModelTest, ScoresLoaded_FillsModel)
{
    // [GIVEN] A pending request
    loadModel();
    m_model.setDesiredRowCount(BATCH_SIZE);

    // [THEN] Count the hasMoreChanged notifications
    int hasMoreChangedCount = 0;
    QObject::connect(&m_model, &CloudScoresModel::hasMoreChanged, [&hasMoreChangedCount]() { ++hasMoreChangedCount; });

    // [WHEN] The server returns all scores of the user
    resolveLastRequest(makeScoresList({ 1, 2, 3 }, 3));

    // [THEN] They are shown, in order
    const QStringList expectedNames = { "Score 1", "Score 2", "Score 3" };
    EXPECT_EQ(m_model.rowCount(), expectedNames.size());
    EXPECT_EQ(names(), expectedNames);
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Fine);

    // [THEN] There is nothing more to load
    EXPECT_FALSE(m_model.hasMore());
    EXPECT_EQ(hasMoreChangedCount, 1);
    EXPECT_EQ(m_requests.size(), 1);
}

TEST_F(Project_CloudScoresModelTest, ScoresLoaded_ItemFields)
{
    // [GIVEN] A score on the server
    ScoresList::Item item;
    item.id = 7;
    item.title = "My score";
    item.lastModified = QDateTime(QDate(2026, 1, 15), QTime(12, 0));
    item.fileSize = 2048;
    item.thumbnailUrl = "https://link.xyz/thumbnail.png";
    item.visibility = cloud::Visibility::Public;
    item.viewCount = 15;

    ScoresList list;
    list.items.push_back(item);
    list.meta.totalScoresCount = 1;

    ON_CALL(*m_configuration, cloudProjectPath(7))
    .WillByDefault(Return(io::path_t("/cloud/7.mscz")));

    // [WHEN] It is loaded
    loadScores(list);

    // [THEN] The model exposes all its properties
    ASSERT_EQ(m_model.rowCount(), 1);
    const QVariantMap map = score(0);

    EXPECT_EQ(map.value("name").toString(), "My score");
    EXPECT_EQ(map.value("path").toString(), "/cloud/7.mscz");
    EXPECT_EQ(map.value("suffix").toString(), "");
    EXPECT_EQ(map.value("fileSize").toString(), DataFormatter::formatFileSize(2048).toQString());
    EXPECT_EQ(map.value("timeSinceModified").toString(),
              DataFormatter::formatTimeSince(Date::fromQDate(item.lastModified.date())).toQString());
    EXPECT_EQ(map.value("thumbnailUrl").toString(), "https://link.xyz/thumbnail.png");
    EXPECT_TRUE(map.value("isCloud").toBool());
    EXPECT_EQ(map.value("scoreId").toInt(), 7);
    EXPECT_EQ(map.value("cloudVisibility").toInt(), int(cloud::Visibility::Public));
    EXPECT_EQ(map.value("cloudViewCount").toInt(), 15);
    EXPECT_FALSE(map.value("isCreateNew").toBool());
    EXPECT_FALSE(map.value("isNoResultsFound").toBool());
}

TEST_F(Project_CloudScoresModelTest, ScoresLoaded_UnknownFileSize_IsEmpty)
{
    // [WHEN] Loading a score whose file size is unknown
    loadScores(makeScoresList({ 1 }, 1));

    // [THEN] No file size is shown
    EXPECT_TRUE(score(0).value("fileSize").toString().isEmpty());
}

TEST_F(Project_CloudScoresModelTest, ScoresLoaded_EnoughRows_WaitsUntilMoreAreDesired)
{
    // [WHEN] The first page satisfies the view, but there are more scores on the server
    loadScores(makeScoresList(idRange(1, 20), 45));

    // [THEN] Nothing more is requested
    EXPECT_EQ(m_model.rowCount(), 20);
    EXPECT_TRUE(m_model.hasMore());
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Fine);
    EXPECT_EQ(m_requests.size(), 1);

    // [WHEN] The view asks for more rows
    m_model.setDesiredRowCount(2 * BATCH_SIZE);

    // [THEN] The second page is requested
    ASSERT_EQ(m_requests.size(), 2);
    EXPECT_EQ(m_requests.back().page, 2);
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Loading);
}

TEST_F(Project_CloudScoresModelTest, ScoresLoaded_MoreRowsDesired_RequestsNextPage)
{
    // [GIVEN] The view asks for two pages at once
    loadModel();
    m_model.setDesiredRowCount(2 * BATCH_SIZE);

    // [WHEN] The first page arrives
    resolveLastRequest(makeScoresList(idRange(1, 20), 45));

    // [THEN] The second page is requested right away
    ASSERT_EQ(m_requests.size(), 2);
    EXPECT_EQ(m_requests.back().page, 2);
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Loading);

    // [WHEN] The second page arrives
    resolveLastRequest(makeScoresList(idRange(21, 40), 45));

    // [THEN] The scores of both pages are shown, and loading stops
    EXPECT_EQ(m_model.rowCount(), 40);
    EXPECT_EQ(names().first(), "Score 1");
    EXPECT_EQ(names().last(), "Score 40");
    EXPECT_TRUE(m_model.hasMore());
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Fine);
    EXPECT_EQ(m_requests.size(), 2);
}

TEST_F(Project_CloudScoresModelTest, ScoresLoaded_NoMoreScores_StopsLoading)
{
    // [GIVEN] The view asks for more rows than there are scores on the server
    loadModel();
    m_model.setDesiredRowCount(2 * BATCH_SIZE);

    // [WHEN] The server returns all scores of the user
    resolveLastRequest(makeScoresList({ 1, 2, 3 }, 3));

    // [THEN] Nothing more is requested
    EXPECT_EQ(m_requests.size(), 1);
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Fine);
}

TEST_F(Project_CloudScoresModelTest, LoadFailed_SetsErrorState)
{
    // [GIVEN] A pending request
    loadModel();
    m_model.setDesiredRowCount(BATCH_SIZE);

    // [WHEN] It fails
    rejectLastRequest();

    // [THEN] The model reports an error
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Error);
    EXPECT_EQ(m_model.rowCount(), 0);

    // [WHEN] The view asks for more rows
    m_model.setDesiredRowCount(2 * BATCH_SIZE);

    // [THEN] Nothing is requested until the model is reloaded
    EXPECT_EQ(m_requests.size(), 1);
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Error);
}

// ==================================================
// reload()
// ==================================================

TEST_F(Project_CloudScoresModelTest, Reload_ResetsModel)
{
    // [GIVEN] Some loaded scores
    loadScores(makeScoresList({ 1, 2, 3 }, 3));
    ASSERT_FALSE(m_model.hasMore());

    // [THEN] Count the modelReset notifications
    int modelResetCount = 0;
    QObject::connect(&m_model, &CloudScoresModel::modelReset, [&modelResetCount]() { ++modelResetCount; });

    // [WHEN] Reloading
    m_model.reload();

    // [THEN] The model is empty again, and waits for the view to ask for rows
    EXPECT_EQ(modelResetCount, 1);
    EXPECT_EQ(m_model.rowCount(), 0);
    EXPECT_TRUE(m_model.hasMore());
    EXPECT_EQ(m_model.desiredRowCount(), 0);
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Loading);
    EXPECT_EQ(m_requests.size(), 1);

    // [WHEN] The view asks for some rows
    m_model.setDesiredRowCount(BATCH_SIZE);

    // [THEN] The first page is requested again
    ASSERT_EQ(m_requests.size(), 2);
    EXPECT_EQ(m_requests.back().page, 1);
}

TEST_F(Project_CloudScoresModelTest, Reload_AfterError_AllowsLoadingAgain)
{
    // [GIVEN] A failed request
    loadModel();
    m_model.setDesiredRowCount(BATCH_SIZE);
    rejectLastRequest();
    ASSERT_EQ(m_model.state(), CloudScoresModel::State::Error);

    // [WHEN] Reloading, and the view asks for some rows
    m_model.reload();
    m_model.setDesiredRowCount(BATCH_SIZE);

    // [THEN] The first page is requested again
    ASSERT_EQ(m_requests.size(), 2);
    EXPECT_EQ(m_requests.back().page, 1);
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Loading);

    // [WHEN] It succeeds
    resolveLastRequest(makeScoresList({ 1 }, 1));

    // [THEN] The error is gone
    const QStringList expectedNames = { "Score 1" };
    EXPECT_EQ(m_model.rowCount(), expectedNames.size());
    EXPECT_EQ(names(), expectedNames);
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Fine);
}

TEST_F(Project_CloudScoresModelTest, Reload_KeepsScoresBeingProcessed)
{
    // [GIVEN] A score that is being processed, and some loaded scores
    loadModel();
    m_watchedScores.set({ makeWatchedScore(10) });
    m_model.setDesiredRowCount(BATCH_SIZE);
    resolveLastRequest(makeScoresList({ 1, 2 }, 2));
    QStringList expectedNames = { "Conversion 10", "Score 1", "Score 2" };
    ASSERT_EQ(m_model.rowCount(), expectedNames.size());
    ASSERT_EQ(names(), expectedNames);

    // [WHEN] Reloading
    m_model.reload();

    // [THEN] Only the score that is being processed remains
    expectedNames = { "Conversion 10" };
    EXPECT_EQ(m_model.rowCount(), expectedNames.size());
    EXPECT_EQ(names(), expectedNames);
}

TEST_F(Project_CloudScoresModelTest, Reload_WhileRequestPending_DiscardsItsResult)
{
    // [GIVEN] A pending request
    loadModel();
    m_model.setDesiredRowCount(BATCH_SIZE);

    // [WHEN] Reloading, and the view asks for some rows
    m_model.reload();
    m_model.setDesiredRowCount(BATCH_SIZE);

    // [THEN] No new request is started while the old one is pending
    EXPECT_EQ(m_requests.size(), 1);

    // [WHEN] The old request succeeds
    resolveLastRequest(makeScoresList({ 1, 2, 3 }, 3));

    // [THEN] Its result is discarded, and the first page is requested again
    EXPECT_EQ(m_model.rowCount(), 0);
    ASSERT_EQ(m_requests.size(), 2);
    EXPECT_EQ(m_requests.back().page, 1);

    // [WHEN] The new request succeeds
    resolveLastRequest(makeScoresList({ 4, 5 }, 2));

    // [THEN] Its result is shown
    const QStringList expectedNames = { "Score 4", "Score 5" };
    EXPECT_EQ(m_model.rowCount(), expectedNames.size());
    EXPECT_EQ(names(), expectedNames);
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Fine);
}

TEST_F(Project_CloudScoresModelTest, Reload_WhileRequestPending_IgnoresItsFailure)
{
    // [GIVEN] A pending request
    loadModel();
    m_model.setDesiredRowCount(BATCH_SIZE);

    // [WHEN] Reloading, and the view asks for some rows
    m_model.reload();
    m_model.setDesiredRowCount(BATCH_SIZE);

    // [WHEN] The old request fails
    rejectLastRequest();

    // [THEN] The failure is ignored, and the first page is requested again
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Loading);
    ASSERT_EQ(m_requests.size(), 2);
    EXPECT_EQ(m_requests.back().page, 1);
}

// ==================================================
// Authorization
// ==================================================

TEST_F(Project_CloudScoresModelTest, SignIn_StartsLoading)
{
    // [GIVEN] A signed out user, and a view that asks for some rows
    loadModel(false /*signedIn*/);
    m_model.setDesiredRowCount(BATCH_SIZE);
    ASSERT_TRUE(m_requests.empty());

    // [WHEN] The user signs in
    m_userAuthorized.set(true);

    // [THEN] The first page is requested
    ASSERT_EQ(m_requests.size(), 1);
    EXPECT_EQ(m_requests.back().page, 1);
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Loading);
}

TEST_F(Project_CloudScoresModelTest, SignOut_SetsNotSignedInState)
{
    // [GIVEN] Some loaded scores
    loadScores(makeScoresList(idRange(1, 20), 45));

    // [WHEN] The user signs out
    m_userAuthorized.set(false);

    // [THEN] The model reports that the user is not signed in
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::NotSignedIn);

    // [WHEN] The view asks for more rows
    m_model.setDesiredRowCount(2 * BATCH_SIZE);

    // [THEN] Nothing is requested
    EXPECT_EQ(m_requests.size(), 1);
}

// ==================================================
// Scores being processed
// ==================================================

TEST_F(Project_CloudScoresModelTest, WatchedScores_ProcessingOnesAreShownFirst)
{
    // [GIVEN] Some loaded scores
    loadScores(makeScoresList({ 1, 2 }, 2));

    // [WHEN] Some conversions are being watched
    m_watchedScores.set({
        makeWatchedScore(10, ConvertStatus::Processing),
        makeWatchedScore(11, ConvertStatus::AwaitingReview),
        makeWatchedScore(12, ConvertStatus::Done),
        makeWatchedScore(13, ConvertStatus::Failed),
        makeWatchedScore(14, ConvertStatus::Processing)
    });

    // [THEN] Only the ones that are being processed are shown, above the loaded scores
    const QStringList expectedNames = { "Conversion 10", "Conversion 14", "Score 1", "Score 2" };
    EXPECT_EQ(m_model.rowCount(), expectedNames.size());
    EXPECT_EQ(names(), expectedNames);
    EXPECT_EQ(m_requests.size(), 1);
}

TEST_F(Project_CloudScoresModelTest, WatchedScores_ItemFields)
{
    // [GIVEN] An audio conversion that is being processed
    WatchedScore watchedScore = makeWatchedScore(10);
    watchedScore.conversion.type = ConvertType::Audio2Score;

    // [WHEN] It is being watched
    loadModel();
    m_watchedScores.set({ watchedScore });

    // [THEN] The model exposes all its properties
    ASSERT_EQ(m_model.rowCount(), 1);
    const QVariantMap map = score(0);

    EXPECT_EQ(map.value("name").toString(), "Conversion 10");
    EXPECT_EQ(map.value("processingStatus").toInt(), STATUS_PROCESSING);
    EXPECT_EQ(map.value("convertId").toInt(), 10);
    EXPECT_EQ(map.value("convertType").toInt(), int(ConvertType::Audio2Score));
    EXPECT_FALSE(map.value("isCloud").toBool());
    EXPECT_EQ(map.value("cloudVisibility").toInt(), int(cloud::Visibility::Private));
    EXPECT_FALSE(map.value("isCreateNew").toBool());
    EXPECT_FALSE(map.value("isNoResultsFound").toBool());

    EXPECT_EQ(data(0, "processingStatus").toInt(), STATUS_PROCESSING);
}

TEST_F(Project_CloudScoresModelTest, WatchedScores_AlreadyLoadedScore_IsNotShown)
{
    // [GIVEN] Some loaded scores
    loadScores(makeScoresList({ 1, 2 }, 2));

    // [WHEN] A watched conversion refers to one of them
    m_watchedScores.set({ makeWatchedScore(10, ConvertStatus::Processing, 2) });

    // [THEN] It is not shown twice
    const QStringList expectedNames = { "Score 1", "Score 2" };
    EXPECT_EQ(m_model.rowCount(), expectedNames.size());
    EXPECT_EQ(names(), expectedNames);
}

TEST_F(Project_CloudScoresModelTest, WatchedScores_DownloadedScore_ReplacesPlaceholder)
{
    // [GIVEN] A watched conversion whose score isn't loaded yet
    loadModel();
    m_watchedScores.set({ makeWatchedScore(10, ConvertStatus::Processing, 2) });
    QStringList expectedNames = { "Conversion 10" };
    ASSERT_EQ(m_model.rowCount(), expectedNames.size());
    ASSERT_EQ(names(), expectedNames);

    // [WHEN] The score arrives with the scores list
    m_model.setDesiredRowCount(BATCH_SIZE);
    resolveLastRequest(makeScoresList({ 1, 2 }, 2));

    // [THEN] The placeholder is gone, and no refresh is needed
    expectedNames = { "Score 1", "Score 2" };
    EXPECT_EQ(m_model.rowCount(), expectedNames.size());
    EXPECT_EQ(names(), expectedNames);
    EXPECT_EQ(m_requests.size(), 1);
}

TEST_F(Project_CloudScoresModelTest, WatchedScoreFinished_RefreshesFirstPage)
{
    // [GIVEN] Some loaded scores, and two conversions that are being processed
    loadScores(makeScoresList({ 1, 2, 3 }, 3));
    m_watchedScores.set({ makeWatchedScore(10), makeWatchedScore(11) });
    ASSERT_EQ(m_requests.size(), 1);

    // [THEN] Count the stateChanged notifications
    int stateChangedCount = 0;
    QObject::connect(&m_model, &CloudScoresModel::stateChanged, [&stateChangedCount]() { ++stateChangedCount; });

    // [WHEN] One of the conversions finishes
    m_watchedScores.set({ makeWatchedScore(10, ConvertStatus::Done, 4), makeWatchedScore(11) });

    // [THEN] Its placeholder is removed, and the first page is requested in the background
    QStringList expectedNames = { "Conversion 11", "Score 1", "Score 2", "Score 3" };
    EXPECT_EQ(m_model.rowCount(), expectedNames.size());
    EXPECT_EQ(names(), expectedNames);
    ASSERT_EQ(m_requests.size(), 2);
    EXPECT_EQ(m_requests.back().page, 1);

    // [WHEN] The first page arrives
    resolveLastRequest(makeScoresList({ 4, 1, 2, 3 }, 4));

    // [THEN] The new score is inserted above the loaded ones
    expectedNames = { "Conversion 11", "Score 4", "Score 1", "Score 2", "Score 3" };
    EXPECT_EQ(m_model.rowCount(), expectedNames.size());
    EXPECT_EQ(names(), expectedNames);
    EXPECT_FALSE(m_model.hasMore());
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Fine);
    EXPECT_EQ(stateChangedCount, 0);
    EXPECT_EQ(m_requests.size(), 2);
}

TEST_F(Project_CloudScoresModelTest, WatchedScoreFinished_RefreshFailed_KeepsLoadedScores)
{
    // [GIVEN] Some loaded scores, and a conversion that has just finished
    loadScores(makeScoresList({ 1, 2, 3 }, 3));
    m_watchedScores.set({ makeWatchedScore(10) });
    m_watchedScores.set({});
    ASSERT_EQ(m_requests.size(), 2);

    // [WHEN] The refresh fails
    rejectLastRequest();

    // [THEN] The loaded scores remain, and no error is reported
    const QStringList expectedNames = { "Score 1", "Score 2", "Score 3" };
    EXPECT_EQ(m_model.rowCount(), expectedNames.size());
    EXPECT_EQ(names(), expectedNames);
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Fine);
}

TEST_F(Project_CloudScoresModelTest, WatchedScoreFinished_WhileRequestPending_RefreshesAfterwards)
{
    // [GIVEN] A conversion that is being processed, and a pending request
    loadModel();
    m_watchedScores.set({ makeWatchedScore(10) });
    m_model.setDesiredRowCount(BATCH_SIZE);
    ASSERT_EQ(m_requests.size(), 1);

    // [WHEN] The conversion finishes
    m_watchedScores.set({});

    // [THEN] The refresh waits for the pending request
    EXPECT_EQ(m_requests.size(), 1);

    // [WHEN] The pending request succeeds
    resolveLastRequest(makeScoresList({ 1, 2 }, 2));

    // [THEN] The first page is requested again
    QStringList expectedNames = { "Score 1", "Score 2" };
    EXPECT_EQ(m_model.rowCount(), expectedNames.size());
    EXPECT_EQ(names(), expectedNames);
    ASSERT_EQ(m_requests.size(), 2);
    EXPECT_EQ(m_requests.back().page, 1);

    // [WHEN] The refresh succeeds
    resolveLastRequest(makeScoresList({ 3, 1, 2 }, 3));

    // [THEN] The new score is inserted above the loaded ones
    expectedNames = { "Score 3", "Score 1", "Score 2" };
    EXPECT_EQ(m_model.rowCount(), expectedNames.size());
    EXPECT_EQ(names(), expectedNames);
    EXPECT_EQ(m_requests.size(), 2);
}

// ==================================================
// Scores being uploaded
// ==================================================

TEST_F(Project_CloudScoresModelTest, UploadingScores_AreShownAboveProcessingOnes)
{
    // [GIVEN] Some loaded scores, and a conversion that is being processed
    loadScores(makeScoresList({ 1, 2 }, 2));
    m_watchedScores.set({ makeWatchedScore(10) });

    // [WHEN] Some scores are being uploaded
    m_uploadingScores.set({ makeUploadingScore(1), makeUploadingScore(2) });

    // [THEN] They are shown above the ones being processed
    const QStringList expectedNames = { "Upload 1", "Upload 2", "Conversion 10", "Score 1", "Score 2" };
    EXPECT_EQ(m_model.rowCount(), expectedNames.size());
    EXPECT_EQ(names(), expectedNames);
    EXPECT_EQ(m_requests.size(), 1);
}

TEST_F(Project_CloudScoresModelTest, UploadingScores_ItemFields)
{
    // [WHEN] A score is being uploaded
    loadModel();
    m_uploadingScores.set({ makeUploadingScore(7) });

    // [THEN] It is shown as being processed, identified by its upload id
    ASSERT_EQ(m_model.rowCount(), 1);
    const QVariantMap map = score(0);

    EXPECT_EQ(map.value("name").toString(), "Upload 7");
    EXPECT_EQ(map.value("processingStatus").toInt(), STATUS_PROCESSING);
    EXPECT_EQ(map.value("uploadId").toInt(), 7);
    EXPECT_FALSE(map.contains("convertId"));
    EXPECT_FALSE(map.value("isCloud").toBool());
    EXPECT_EQ(map.value("cloudVisibility").toInt(), int(cloud::Visibility::Private));
    EXPECT_FALSE(map.value("isCreateNew").toBool());
    EXPECT_FALSE(map.value("isNoResultsFound").toBool());
}

TEST_F(Project_CloudScoresModelTest, UploadingScoreRemoved_DoesNotRefresh)
{
    // [GIVEN] Some loaded scores, and a score that is being uploaded
    loadScores(makeScoresList({ 1, 2 }, 2));
    m_uploadingScores.set({ makeUploadingScore(1) });
    ASSERT_EQ(m_requests.size(), 1);

    // [WHEN] The upload fails or is canceled
    m_uploadingScores.set({});

    // [THEN] Its row disappears, and no refresh is requested
    const QStringList expectedNames = { "Score 1", "Score 2" };
    EXPECT_EQ(m_model.rowCount(), expectedNames.size());
    EXPECT_EQ(names(), expectedNames);
    EXPECT_EQ(m_requests.size(), 1);
}

TEST_F(Project_CloudScoresModelTest, UploadSucceeded_ShownAsProcessing_DoesNotRefresh)
{
    // [GIVEN] Some loaded scores, and a score that is being uploaded
    loadScores(makeScoresList({ 1, 2 }, 2));
    m_uploadingScores.set({ makeUploadingScore(1) });
    ASSERT_EQ(m_requests.size(), 1);

    // [WHEN] The upload succeeds, and its conversion is watched
    m_uploadingScores.set({});
    m_watchedScores.set({ makeWatchedScore(10) });

    // [THEN] The conversion takes its place, and no refresh is requested
    const QStringList expectedNames = { "Conversion 10", "Score 1", "Score 2" };
    EXPECT_EQ(m_model.rowCount(), expectedNames.size());
    EXPECT_EQ(names(), expectedNames);
    EXPECT_EQ(m_requests.size(), 1);
}

// ==================================================
// Polling status
// ==================================================

TEST_F(Project_CloudScoresModelTest, PollingGaveUp_MarksProcessingScoresAsFailed)
{
    // [GIVEN] A conversion that is being processed
    loadModel();
    m_watchedScores.set({ makeWatchedScore(10) });
    ASSERT_EQ(score(0).value("processingStatus").toInt(), STATUS_PROCESSING);

    // [WHEN] Polling gives up
    sendPollingGaveUp(true);

    // [THEN] The conversion is shown as failed
    ASSERT_EQ(m_model.rowCount(), 1);
    EXPECT_EQ(score(0).value("processingStatus").toInt(), STATUS_FAILED);

    // [WHEN] Polling succeeds again
    m_pollingStatusChanged.send(PollingSuccess());

    // [THEN] The conversion is shown as being processed
    ASSERT_EQ(m_model.rowCount(), 1);
    EXPECT_EQ(score(0).value("processingStatus").toInt(), STATUS_PROCESSING);
    EXPECT_TRUE(m_requests.empty());
}

TEST_F(Project_CloudScoresModelTest, PollingFailed_NotGivenUp_DoesNothing)
{
    // [GIVEN] A conversion that is being processed
    loadModel();
    m_watchedScores.set({ makeWatchedScore(10) });

    // [THEN] Count the rowsRemoved notifications
    int rowsRemovedCount = 0;
    QObject::connect(&m_model, &CloudScoresModel::rowsRemoved, [&rowsRemovedCount]() { ++rowsRemovedCount; });

    // [WHEN] Polling fails, but will be retried
    sendPollingGaveUp(false);

    // [THEN] Nothing changes
    EXPECT_EQ(rowsRemovedCount, 0);
    EXPECT_EQ(score(0).value("processingStatus").toInt(), STATUS_PROCESSING);
}

// ==================================================
// retryAllConversions() / cancelUpload() / cancelConversion()
// ==================================================

TEST_F(Project_CloudScoresModelTest, RetryAllConversions_RestartsPolling)
{
    // [GIVEN] A conversion that is shown as failed, because polling gave up
    loadModel();
    m_watchedScores.set({ makeWatchedScore(10) });
    sendPollingGaveUp(true);
    ASSERT_EQ(score(0).value("processingStatus").toInt(), STATUS_FAILED);

    // [THEN] Polling is restarted
    EXPECT_CALL(*m_scenario, retryPolling());

    // [WHEN] Retrying
    m_model.retryAllConversions();

    // [THEN] The conversion is shown as being processed
    ASSERT_EQ(m_model.rowCount(), 1);
    EXPECT_EQ(score(0).value("processingStatus").toInt(), STATUS_PROCESSING);
}

TEST_F(Project_CloudScoresModelTest, CancelUpload_DelegatesToScenario)
{
    // [THEN] The scenario is asked to cancel the upload
    EXPECT_CALL(*m_scenario, cancelUpload(7));

    // [WHEN] Cancelling
    m_model.cancelUpload(7);
}

TEST_F(Project_CloudScoresModelTest, CancelConversion_DelegatesToScenario)
{
    // [THEN] The scenario is asked to cancel the conversion
    EXPECT_CALL(*m_scenario, cancelConversion(ConvertType::Audio2Score, 10));

    // [WHEN] Cancelling
    m_model.cancelConversion(int(ConvertType::Audio2Score), 10);
}

// ==================================================
// Score deleted
// ==================================================

TEST_F(Project_CloudScoresModelTest, ScoreDeleted_RemovesScore)
{
    // [GIVEN] A conversion that is being processed, and all scores of the user
    loadScores(makeScoresList({ 1, 2, 3 }, 3));
    m_watchedScores.set({ makeWatchedScore(10) });

    // [WHEN] One of the scores is deleted
    m_scoreDeleted.send(2);

    // [THEN] It is removed
    const QStringList expectedNames = { "Conversion 10", "Score 1", "Score 3" };
    EXPECT_EQ(m_model.rowCount(), expectedNames.size());
    EXPECT_EQ(names(), expectedNames);
    EXPECT_FALSE(m_model.hasMore());
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Fine);
    EXPECT_EQ(m_requests.size(), 1);
}

TEST_F(Project_CloudScoresModelTest, ScoreDeleted_UnknownScore_DoesNothing)
{
    // [GIVEN] Some loaded scores
    loadScores(makeScoresList({ 1, 2, 3 }, 3));

    // [THEN] Count the hasMoreChanged notifications
    int hasMoreChangedCount = 0;
    QObject::connect(&m_model, &CloudScoresModel::hasMoreChanged, [&hasMoreChangedCount]() { ++hasMoreChangedCount; });

    // [WHEN] A score that isn't loaded is deleted
    m_scoreDeleted.send(42);

    // [THEN] Nothing changes
    EXPECT_EQ(m_model.rowCount(), 3);
    EXPECT_EQ(hasMoreChangedCount, 0);
    EXPECT_EQ(m_requests.size(), 1);
}

TEST_F(Project_CloudScoresModelTest, ScoreDeleted_MoreScoresOnServer_RequestsReplacement)
{
    // [GIVEN] A full page of scores, with more on the server
    loadScores(makeScoresList(idRange(1, 20), 45));

    // [WHEN] One of the scores is deleted
    m_scoreDeleted.send(5);

    // [THEN] The page is requested again, to fill the gap
    EXPECT_EQ(m_model.rowCount(), 19);
    ASSERT_EQ(m_requests.size(), 2);
    EXPECT_EQ(m_requests.back().page, 1);
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Loading);

    // [WHEN] The page arrives, shifted by one score
    std::vector<int> ids = idRange(1, 4);
    ids.reserve(BATCH_SIZE);

    for (int id : idRange(6, 21)) {
        ids.push_back(id);
    }
    resolveLastRequest(makeScoresList(ids, 44));

    // [THEN] Only the score that wasn't loaded yet is added
    EXPECT_EQ(m_model.rowCount(), 20);
    EXPECT_EQ(names().last(), "Score 21");
    EXPECT_TRUE(m_model.hasMore());
    EXPECT_EQ(m_model.state(), CloudScoresModel::State::Fine);
    EXPECT_EQ(m_requests.size(), 2);
}
