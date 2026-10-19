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

#include <QString>
#include <QVariantMap>

#include "async/processevents.h"
#include "modularity/ioc.h"
#include "translation.h"

#include "project/internal/deletecloudscorescenario.h"

#include "utils/testutils.h"
#include "mocks/projectconfigurationmock.h"
#include "mocks/recentfilescontrollermock.h"
#include "global/tests/mocks/filesystemmock.h"
#include "global/tests/mocks/interactivemock.h"
#include "cloud/tests/mocks/musescorecomservicemock.h"

using namespace ::testing;
using namespace mu::project;
using namespace muse;
using namespace muse::cloud;

namespace {
constexpr int SCORE_ID = 555;

const io::path_t LOCAL_PATH = "/cloud/555.mscz";

UriQuery deleteDialogQuery(const QString& scoreName)
{
    UriQuery query("musescore://project/deletecloudscore");
    query.addParam("scoreName", Val(scoreName));

    return query;
}

Val deleteDialogResult(bool agreesToDelete)
{
    return Val::fromQVariant(QVariantMap { { "agreesToDelete", agreesToDelete } });
}
}

namespace mu::project {
class Project_DeleteCloudScoreScenarioTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_scenario = std::make_shared<DeleteCloudScoreScenario>(muse::modularity::globalCtx());

        m_configuration = std::make_shared<NiceMock<ProjectConfigurationMock> >();
        m_fileSystem = std::make_shared<NiceMock<io::FileSystemMock> >();
        m_museScoreComService = std::make_shared<NiceMock<MuseScoreComServiceMock> >();
        m_interactive = std::make_shared<NiceMock<InteractiveMock> >();
        m_recentFilesController = std::make_shared<NiceMock<RecentFilesControllerMock> >();

        m_scenario->configuration.set(m_configuration);
        m_scenario->fileSystem.set(m_fileSystem);
        m_scenario->museScoreComService.set(m_museScoreComService);
        m_scenario->interactive.set(m_interactive);
        m_scenario->recentFilesController.set(m_recentFilesController);
    }

    std::shared_ptr<DeleteCloudScoreScenario> m_scenario;
    std::shared_ptr<ProjectConfigurationMock> m_configuration;
    std::shared_ptr<io::FileSystemMock> m_fileSystem;
    std::shared_ptr<MuseScoreComServiceMock> m_museScoreComService;
    std::shared_ptr<InteractiveMock> m_interactive;
    std::shared_ptr<RecentFilesControllerMock> m_recentFilesController;
};
}

// ==================================================
// Confirmation
// ==================================================

TEST_F(Project_DeleteCloudScoreScenarioTest, DeleteScore_InvalidScoreId_DoesNothing)
{
    // [THEN] Neither the confirmation dialog is shown, nor the score is deleted
    EXPECT_CALL(*m_interactive, open(An<const UriQuery&>())).Times(0);
    EXPECT_CALL(*m_museScoreComService, deleteScore(_)).Times(0);

    // [WHEN] Deleting a score with an invalid id
    m_scenario->deleteScore(0, "My Score");
    m_scenario->deleteScore(-1, "My Score");
}

TEST_F(Project_DeleteCloudScoreScenarioTest, DeleteScore_UserDeclines_DoesNotDeleteScore)
{
    // [GIVEN] The user doesn't agree to delete the score
    EXPECT_CALL(*m_interactive, open(deleteDialogQuery("My Score")))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedPromise<Val>(deleteDialogResult(false));
    }));

    // [THEN] Nothing is deleted
    EXPECT_CALL(*m_museScoreComService, deleteScore(_)).Times(0);
    EXPECT_CALL(*m_recentFilesController, removeRecentFile(_)).Times(0);
    EXPECT_CALL(*m_fileSystem, remove(_, _)).Times(0);

    bool deleted = false;
    m_scenario->scoreDeleted().onReceive(nullptr, [&deleted](int) { deleted = true; });

    // [WHEN] Deleting a score
    m_scenario->deleteScore(SCORE_ID, "My Score");
    async::processMessages();

    EXPECT_FALSE(deleted);
}

TEST_F(Project_DeleteCloudScoreScenarioTest, DeleteScore_DialogClosedWithoutResult_DoesNotDeleteScore)
{
    // [GIVEN] The confirmation dialog is closed without a result
    EXPECT_CALL(*m_interactive, open(deleteDialogQuery("My Score")))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedPromise<Val>(Val());
    }));

    // [THEN] Nothing is deleted
    EXPECT_CALL(*m_museScoreComService, deleteScore(_)).Times(0);
    EXPECT_CALL(*m_recentFilesController, removeRecentFile(_)).Times(0);
    EXPECT_CALL(*m_fileSystem, remove(_, _)).Times(0);

    bool deleted = false;
    m_scenario->scoreDeleted().onReceive(nullptr, [&deleted](int) { deleted = true; });

    // [WHEN] Deleting a score
    m_scenario->deleteScore(SCORE_ID, "My Score");
    async::processMessages();

    EXPECT_FALSE(deleted);
}

// ==================================================
// Deletion
// ==================================================

TEST_F(Project_DeleteCloudScoreScenarioTest, DeleteScore_UserAgrees_DeletesScoreAndItsLocalCopy)
{
    // [GIVEN] The user agrees to delete the score
    EXPECT_CALL(*m_interactive, open(deleteDialogQuery("My Score")))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedPromise<Val>(deleteDialogResult(true));
    }));

    EXPECT_CALL(*m_configuration, cloudProjectPath(SCORE_ID))
    .WillOnce(Return(LOCAL_PATH));

    // [THEN] The score is deleted from the server, then its local copy is cleaned up
    {
        InSequence seq;

        EXPECT_CALL(*m_museScoreComService, deleteScore(SCORE_ID))
        .WillOnce(Invoke([](auto&&...) {
            return resolvedPromise<Ret>(make_ok());
        }));
        EXPECT_CALL(*m_recentFilesController, removeRecentFile(LOCAL_PATH));
        EXPECT_CALL(*m_fileSystem, remove(LOCAL_PATH, false))
        .WillOnce(Return(make_ok()));
    }

    EXPECT_CALL(*m_interactive, error(_, _, _, _, _, _)).Times(0);

    std::vector<int> deletedScoreIds;
    m_scenario->scoreDeleted().onReceive(nullptr, [&deletedScoreIds](int scoreId) { deletedScoreIds.push_back(scoreId); });

    // [WHEN] Deleting a score
    m_scenario->deleteScore(SCORE_ID, "My Score");
    async::processMessages(); // confirmation dialog
    async::processMessages(); // delete request

    // [THEN] The deletion is reported
    EXPECT_EQ(deletedScoreIds, std::vector<int>({ SCORE_ID }));
}

TEST_F(Project_DeleteCloudScoreScenarioTest, DeleteScore_LocalCopyRemovalFails_StillReportsDeletion)
{
    // [GIVEN] The user agrees to delete the score
    EXPECT_CALL(*m_interactive, open(deleteDialogQuery("My Score")))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedPromise<Val>(deleteDialogResult(true));
    }));

    EXPECT_CALL(*m_museScoreComService, deleteScore(SCORE_ID))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedPromise<Ret>(make_ok());
    }));

    // [GIVEN] The local copy can't be removed
    EXPECT_CALL(*m_configuration, cloudProjectPath(SCORE_ID))
    .WillOnce(Return(LOCAL_PATH));
    EXPECT_CALL(*m_recentFilesController, removeRecentFile(LOCAL_PATH));
    EXPECT_CALL(*m_fileSystem, remove(LOCAL_PATH, false))
    .WillOnce(Return(make_ret(Ret::Code::UnknownError)));

    // [THEN] No error is shown, as the score is gone from the server
    EXPECT_CALL(*m_interactive, error(_, _, _, _, _, _)).Times(0);

    std::vector<int> deletedScoreIds;
    m_scenario->scoreDeleted().onReceive(nullptr, [&deletedScoreIds](int scoreId) { deletedScoreIds.push_back(scoreId); });

    // [WHEN] Deleting a score
    m_scenario->deleteScore(SCORE_ID, "My Score");
    async::processMessages(); // confirmation dialog
    async::processMessages(); // delete request

    // [THEN] The deletion is reported
    EXPECT_EQ(deletedScoreIds, std::vector<int>({ SCORE_ID }));
}

TEST_F(Project_DeleteCloudScoreScenarioTest, DeleteScore_ServerFails_ShowsErrorAndKeepsLocalCopy)
{
    // [GIVEN] The user agrees to delete the score
    EXPECT_CALL(*m_interactive, open(deleteDialogQuery("My Score")))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedPromise<Val>(deleteDialogResult(true));
    }));

    // [GIVEN] The server request fails
    EXPECT_CALL(*m_museScoreComService, deleteScore(SCORE_ID))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedPromise<Ret>(make_ret(Ret::Code::UnknownError));
    }));

    // [THEN] The error is shown, with a single Ok button
    const std::string title = muse::trc("project/cloud", "Unable to delete the score");
    const std::string text = muse::trc("global", "Please check your internet connection or try again later.");
    const IInteractive::ButtonData okButton(IInteractive::Button::Ok, "OK");

    EXPECT_CALL(*m_interactive, buttonData(IInteractive::Button::Ok))
    .WillOnce(Return(okButton));
    EXPECT_CALL(*m_interactive, error(title, Field(&IInteractive::Text::text, text),
                                      ElementsAre(Field(&IInteractive::ButtonData::btn, okButton.btn)), _, _, _))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedPromise<IInteractive::Result>(IInteractive::Result());
    }));

    // [THEN] The local copy is kept
    EXPECT_CALL(*m_recentFilesController, removeRecentFile(_)).Times(0);
    EXPECT_CALL(*m_fileSystem, remove(_, _)).Times(0);

    bool deleted = false;
    m_scenario->scoreDeleted().onReceive(nullptr, [&deleted](int) { deleted = true; });

    // [WHEN] Deleting a score
    m_scenario->deleteScore(SCORE_ID, "My Score");
    async::processMessages(); // confirmation dialog
    async::processMessages(); // delete request

    // [THEN] The deletion isn't reported
    EXPECT_FALSE(deleted);
}
