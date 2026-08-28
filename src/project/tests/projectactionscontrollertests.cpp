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

#include <QDir>

#include "async/processevents.h"

#include "project/internal/projectactionscontroller.h"
#include "project/projecterrors.h"

#include "cloud/clouderrors.h"
#include "engraving/engravingerrors.h"
#include "cloud/tests/mocks/authorizationservicemock.h"
#include "cloud/tests/mocks/musescorecomservicemock.h"
#include "context/tests/mocks/globalcontextmock.h"
#include "global/tests/mocks/filesystemmock.h"
#include "interactive/tests/mocks/interactivemock.h"
#include "multiwindows/tests/mocks/multiwindowsprovidermock.h"
#include "musesounds/tests/mocks/musesamplercheckupdatescenariomock.h"
#include "musesounds/tests/mocks/musesoundscheckupdatescenariomock.h"
#include "notation/tests/mocks/masternotationmock.h"
#include "notation/tests/mocks/notationmock.h"

#include "mocks/mscmetareadermock.h"
#include "mocks/notationprojectmock.h"
#include "mocks/notationreadersregistermock.h"
#include "mocks/opensaveprojectscenariomock.h"
#include "mocks/projectautosavermock.h"
#include "mocks/projectconfigurationmock.h"
#include "mocks/projectcreatormock.h"
#include "mocks/recentfilescontrollermock.h"
#include "mocks/saveprojectscenariomock.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

using namespace muse;
using namespace mu::project;

namespace mu::project {
class ProjectActionsControllerTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_controller = std::make_shared<ProjectActionsController>(modularity::globalCtx());

        m_configuration = std::make_shared<NiceMock<ProjectConfigurationMock> >();
        m_fileSystem = std::make_shared<NiceMock<io::FileSystemMock> >();
        m_multiwindows = std::make_shared<NiceMock<mi::MultiWindowsProviderMock> >();
        m_museScoreComService = std::make_shared<NiceMock<cloud::MuseScoreComServiceMock> >();
        m_authorization = std::make_shared<NiceMock<cloud::AuthorizationServiceMock> >();
        m_projectCreator = std::make_shared<NiceMock<ProjectCreatorMock> >();
        m_readers = std::make_shared<NiceMock<NotationReadersRegisterMock> >();
        m_autoSaver = std::make_shared<NiceMock<ProjectAutoSaverMock> >();
        m_recentFiles = std::make_shared<NiceMock<RecentFilesControllerMock> >();
        m_openSaveScenario = std::make_shared<NiceMock<OpenSaveProjectScenarioMock> >();
        m_interactive = std::make_shared<NiceMock<InteractiveMock> >();
        m_globalContext = std::make_shared<NiceMock<context::GlobalContextMock> >();
        m_museSounds = std::make_shared<NiceMock<musesounds::MuseSoundsCheckUpdateScenarioMock> >();
        m_museSampler = std::make_shared<NiceMock<musesounds::MuseSamplerCheckUpdateScenarioMock> >();
        m_saveScenario = std::make_shared<NiceMock<SaveProjectScenarioMock> >();
        m_mscMetaReader = std::make_shared<NiceMock<notation::MscMetaReaderMock> >();

        m_controller->configuration.set(m_configuration);
        m_controller->fileSystem.set(m_fileSystem);
        m_controller->multiwindowsProvider.set(m_multiwindows);
        m_controller->museScoreComService.set(m_museScoreComService);
        m_controller->projectCreator.set(m_projectCreator);
        m_controller->readers.set(m_readers);
        m_controller->projectAutoSaver.set(m_autoSaver);
        m_controller->recentFilesController.set(m_recentFiles);
        m_controller->openSaveProjectScenario.set(m_openSaveScenario);
        m_controller->interactive.set(m_interactive);
        m_controller->globalContext.set(m_globalContext);
        m_controller->museSoundsCheckUpdateScenario.set(m_museSounds);
        m_controller->museSamplerCheckUpdateScenario.set(m_museSampler);
        m_controller->saveProjectScenario.set(m_saveScenario);
        m_controller->mscMetaReader.set(m_mscMetaReader);

        m_project = std::make_shared<NiceMock<NotationProjectMock> >();
        m_masterNotation = std::make_shared<NiceMock<notation::MasterNotationMock> >();
        m_notation = std::make_shared<NiceMock<notation::NotationMock> >();

        ON_CALL(*m_project, masterNotation()).WillByDefault(Return(m_masterNotation));
        ON_CALL(*m_masterNotation, notation()).WillByDefault(Return(m_notation));
        ON_CALL(*m_museScoreComService, authorization()).WillByDefault(Return(m_authorization));

        // Nothing open, nothing in another window, and paths are taken at face value.
        ON_CALL(*m_globalContext, currentProject()).WillByDefault(Return(nullptr));
        ON_CALL(*m_multiwindows, isProjectAlreadyOpened(_)).WillByDefault(Return(false));
        ON_CALL(*m_fileSystem, absoluteFilePath(_))
        .WillByDefault([](const io::path_t& p) { return p; });

        // An ordinary local score that loads successfully.
        ON_CALL(*m_configuration, isCloudProject(_)).WillByDefault(Return(false));
        ON_CALL(*m_projectCreator, newProject(_)).WillByDefault(Return(m_project));
        ON_CALL(*m_project, load(_, _, _)).WillByDefault(Return(make_ok()));
        ON_CALL(*m_project, cloudInfo()).WillByDefault(ReturnRef(m_cloudInfo));

        // The notation page is already up, so opening does not have to navigate.
        ON_CALL(*m_interactive, isOpened(::testing::An<const Uri&>()))
        .WillByDefault(Return(RetVal<bool>::make_ok(true)));

        // Dialogs answer immediately so that unstubbed paths do not abort the test.
        ON_CALL(*m_interactive, warning(_, _, _, _, _, _)).WillByDefault([] { return resolvedResult(); });
        ON_CALL(*m_interactive, error(_, _, _, _, _, _)).WillByDefault([] { return resolvedResult(); });
        ON_CALL(*m_interactive, info(_, _, _, _, _, _)).WillByDefault([] { return resolvedResult(); });
        ON_CALL(*m_interactive, warningSync(_, _, _, _, _, _))
        .WillByDefault(Return(IInteractive::Result(int(IInteractive::Button::Cancel))));
        ON_CALL(*m_interactive, errorSync(_, _, _, _, _, _))
        .WillByDefault(Return(IInteractive::Result(int(IInteractive::Button::Cancel))));
        ON_CALL(*m_interactive, buttonData(_)).WillByDefault([](IInteractive::Button btn) {
            return IInteractive::ButtonData(btn, "");
        });
        ON_CALL(*m_interactive, open(_)).WillByDefault([] {
            return async::make_promise<Val>([](auto resolve, auto) { return resolve(Val()); });
        });

        // A logged-in account, a server that answers, and downloads that hand back a progress
        // the test can finish.
        ON_CALL(*m_authorization, ensureAuthorization(_, _)).WillByDefault(Return(RetVal<Val>::make_ok(Val())));
        ON_CALL(*m_museScoreComService, downloadScoreInfo(::testing::An<int>()))
        .WillByDefault(Return(RetVal<cloud::ScoreInfo>::make_ok(cloud::ScoreInfo())));
        ON_CALL(*m_authorization, accountInfo()).WillByDefault(ReturnRef(m_accountInfo));
        ON_CALL(*m_museScoreComService, downloadScore(_, _, _, _))
        .WillByDefault([this](int, DevicePtr, const QString&, const QString&) {
            m_download = std::make_shared<Progress>();
            return m_download;
        });

        // Downloading writes a real file, so point it somewhere writable.
        m_downloadDir = io::path_t(std::string(project_test_DATA_ROOT)) + "/../cloud-tmp";
        QDir().mkpath(m_downloadDir.toQString());
        ON_CALL(*m_configuration, cloudProjectPath(_))
        .WillByDefault([this](int id) { return m_downloadDir + "/" + std::to_string(id) + ".mscz"; });
    }

    static async::Promise<IInteractive::Result> resolvedResult()
    {
        return async::make_promise<IInteractive::Result>([](auto resolve, auto) {
            return resolve(IInteractive::Result(int(IInteractive::Button::Ok)));
        });
    }

    //! NOTE Friendship is not inherited, and TEST_F bodies live in a subclass of this fixture,
    //! so the private entry points are reached through these forwarders.
    Ret openProject(const io::path_t& path, const QString& displayNameOverride = QString())
    {
        return m_controller->openProject(path, displayNameOverride);
    }

    Ret openProject(const rcommand::Params& params)
    {
        return m_controller->openProject(params);
    }

    void downloadAndOpenCloudProject(int scoreId, const QString& hash = QString(), const QString& secret = QString(),
                                     bool isOwner = true)
    {
        m_controller->downloadAndOpenCloudProject(scoreId, hash, secret, isOwner);
    }

    //! NOTE The download subscribes and returns; its result arrives afterwards.
    void finishDownloadWith(const Ret& ret)
    {
        ASSERT_TRUE(m_download);
        ProgressResult res;
        res.ret = ret;
        m_download->finish(res);
    }

    static void drainDeferredCalls()
    {
        async::processMessages();
    }

    std::shared_ptr<ProjectActionsController> m_controller;

    std::shared_ptr<ProjectConfigurationMock> m_configuration;
    std::shared_ptr<io::FileSystemMock> m_fileSystem;
    std::shared_ptr<mi::MultiWindowsProviderMock> m_multiwindows;
    std::shared_ptr<cloud::MuseScoreComServiceMock> m_museScoreComService;
    std::shared_ptr<cloud::AuthorizationServiceMock> m_authorization;
    std::shared_ptr<ProjectCreatorMock> m_projectCreator;
    std::shared_ptr<NotationReadersRegisterMock> m_readers;
    std::shared_ptr<ProjectAutoSaverMock> m_autoSaver;
    std::shared_ptr<RecentFilesControllerMock> m_recentFiles;
    std::shared_ptr<OpenSaveProjectScenarioMock> m_openSaveScenario;
    std::shared_ptr<InteractiveMock> m_interactive;
    std::shared_ptr<context::GlobalContextMock> m_globalContext;
    std::shared_ptr<musesounds::MuseSoundsCheckUpdateScenarioMock> m_museSounds;
    std::shared_ptr<musesounds::MuseSamplerCheckUpdateScenarioMock> m_museSampler;
    std::shared_ptr<SaveProjectScenarioMock> m_saveScenario;
    std::shared_ptr<notation::MscMetaReaderMock> m_mscMetaReader;

    std::shared_ptr<NotationProjectMock> m_project;
    std::shared_ptr<notation::MasterNotationMock> m_masterNotation;
    std::shared_ptr<notation::NotationMock> m_notation;

    CloudProjectInfo m_cloudInfo;
    cloud::AccountInfo m_accountInfo;
    io::path_t m_downloadDir;
    ProgressPtr m_download;
};

// ─── Which kind of thing are we being asked to open ──────────────────────────

TEST_F(ProjectActionsControllerTests, OpenProject_LocalFileUrl_OpensThatFile)
{
    //! [GIVEN] A local file...
    ProjectFile file(QUrl::fromLocalFile("/scores/symphony.mscz"));

    //! [THEN] It is loaded and becomes the current score
    EXPECT_CALL(*m_project, load(io::path_t("/scores/symphony.mscz"), _, _)).Times(1);
    EXPECT_CALL(*m_globalContext, setCurrentProject(_)).Times(1);

    //! [WHEN] Opening it...
    Ret ret = m_controller->openProject(file);

    EXPECT_TRUE(ret);
}

TEST_F(ProjectActionsControllerTests, OpenProject_MuseScoreUrlThatIsNotAScore_IsRejected)
{
    //! [GIVEN] A musescore:// url that does not point at a score...
    ProjectFile file(QUrl("musescore://something-else/42"));

    //! [THEN] Nothing is loaded
    EXPECT_CALL(*m_project, load(_, _, _)).Times(0);

    //! [WHEN] Opening it...
    Ret ret = m_controller->openProject(file);

    //! [THEN] It is refused as an unsupported address
    EXPECT_EQ(ret.code(), int(Err::UnsupportedUrl));
}

TEST_F(ProjectActionsControllerTests, OpenProject_ForeignUrl_IsRejected)
{
    //! [GIVEN] A url of a scheme the app knows nothing about...
    ProjectFile file(QUrl("https://example.com/score.mscz"));

    //! [THEN] Nothing is loaded
    EXPECT_CALL(*m_project, load(_, _, _)).Times(0);

    //! [WHEN] Opening it...
    Ret ret = m_controller->openProject(file);

    EXPECT_EQ(ret.code(), int(Err::UnsupportedUrl));
}

TEST_F(ProjectActionsControllerTests, OpenProject_NothingGiven_AsksTheUserForAFile)
{
    //! [GIVEN] No file named, and a user who picks one...
    const io::path_t picked = "/scores/picked.mscz";
    ON_CALL(*m_interactive, selectOpeningFile(_, _, _))
    .WillByDefault([picked](const std::string&, const io::path_t&, const std::vector<std::string>&) {
        return async::make_promise<io::path_t>([picked](auto resolve, auto) { return resolve(picked); });
    });

    //! [THEN] The chosen file is opened, and its folder is remembered for next time
    EXPECT_CALL(*m_configuration, setLastOpenedProjectsPath(io::path_t("/scores"))).Times(1);
    EXPECT_CALL(*m_project, load(picked, _, _)).Times(1);

    //! [WHEN] Opening without naming a file...
    m_controller->openProject(ProjectFile());
    drainDeferredCalls();
}

TEST_F(ProjectActionsControllerTests, OpenProject_FileDialogCancelled_OpensNothing)
{
    //! [GIVEN] No file named, and a user who dismisses the dialog...
    ON_CALL(*m_interactive, selectOpeningFile(_, _, _))
    .WillByDefault([](const std::string&, const io::path_t&, const std::vector<std::string>&) {
        return async::make_promise<io::path_t>([](auto resolve, auto) { return resolve(io::path_t()); });
    });

    //! [THEN] Nothing is loaded and nothing is remembered
    EXPECT_CALL(*m_configuration, setLastOpenedProjectsPath(_)).Times(0);
    EXPECT_CALL(*m_project, load(_, _, _)).Times(0);

    //! [WHEN] Opening without naming a file...
    m_controller->openProject(ProjectFile());
    drainDeferredCalls();
}

// ─── Deciding where the score should be opened ───────────────────────────────

TEST_F(ProjectActionsControllerTests, OpenProject_AlreadyOpenHere_JustShowsIt)
{
    //! [GIVEN] The very score that is already open in this window...
    ON_CALL(*m_globalContext, currentProject()).WillByDefault(Return(m_project));
    ON_CALL(*m_project, path()).WillByDefault(Return(io::path_t("/scores/symphony.mscz")));

    //! [THEN] It is not loaded again and no second window is opened
    EXPECT_CALL(*m_project, load(_, _, _)).Times(0);
    EXPECT_CALL(*m_multiwindows, openNewWindow(_)).Times(0);

    //! [WHEN] Opening it again...
    Ret ret = openProject("/scores/symphony.mscz");

    EXPECT_TRUE(ret);
}

TEST_F(ProjectActionsControllerTests, OpenProject_AlreadyOpenElsewhere_RaisesThatWindow)
{
    //! [GIVEN] A score already open in another window...
    ON_CALL(*m_multiwindows, isProjectAlreadyOpened(_)).WillByDefault(Return(true));

    //! [THEN] That window is brought forward instead of loading a second copy
    EXPECT_CALL(*m_multiwindows, activateWindowWithProject(io::path_t("/scores/symphony.mscz"))).Times(1);
    EXPECT_CALL(*m_project, load(_, _, _)).Times(0);

    //! [WHEN] Opening it...
    Ret ret = openProject("/scores/symphony.mscz");

    EXPECT_TRUE(ret);
}

TEST_F(ProjectActionsControllerTests, OpenProject_ThisWindowIsTaken_OpensANewWindow)
{
    //! [GIVEN] A different score already open in this window...
    ON_CALL(*m_globalContext, currentProject()).WillByDefault(Return(m_project));
    ON_CALL(*m_project, path()).WillByDefault(Return(io::path_t("/scores/other.mscz")));

    //! [THEN] The new score gets a window of its own, and this one is left alone
    EXPECT_CALL(*m_multiwindows, openNewWindow(QStringList { "/scores/symphony.mscz" })).Times(1);
    EXPECT_CALL(*m_globalContext, setCurrentProject(_)).Times(0);

    //! [WHEN] Opening it...
    Ret ret = openProject("/scores/symphony.mscz");

    EXPECT_TRUE(ret);
}

TEST_F(ProjectActionsControllerTests, OpenProject_NewWindowWithDisplayName_PassesTheNameAlong)
{
    //! [GIVEN] A different score already open in this window...
    ON_CALL(*m_globalContext, currentProject()).WillByDefault(Return(m_project));
    ON_CALL(*m_project, path()).WillByDefault(Return(io::path_t("/scores/other.mscz")));

    //! [THEN] The display name travels to the new window, so the title is not lost
    const QStringList expected { "/scores/symphony.mscz", "--score-display-name-override", "Cloud title" };
    EXPECT_CALL(*m_multiwindows, openNewWindow(expected)).Times(1);

    //! [WHEN] Opening it with a display name...
    openProject("/scores/symphony.mscz", "Cloud title");
}

TEST_F(ProjectActionsControllerTests, OpenProject_EmptyPath_IsRefused)
{
    //! [GIVEN] A path that resolves to nothing...
    ON_CALL(*m_fileSystem, absoluteFilePath(_)).WillByDefault(Return(io::path_t()));

    //! [THEN] Nothing is loaded
    EXPECT_CALL(*m_project, load(_, _, _)).Times(0);

    //! [WHEN] Opening it...
    Ret ret = openProject("/scores/symphony.mscz");

    EXPECT_FALSE(ret);
}

// ─── Cloud scores ────────────────────────────────────────────────────────────

TEST_F(ProjectActionsControllerTests, OpenProject_CloudScoreAndCloudReachable_DownloadsTheLatestVersion)
{
    //! [GIVEN] A cloud score and a reachable cloud...
    ON_CALL(*m_configuration, isCloudProject(_)).WillByDefault(Return(true));
    ON_CALL(*m_configuration, isLegacyCloudProject(_)).WillByDefault(Return(false));
    ON_CALL(*m_configuration, cloudScoreIdFromPath(_)).WillByDefault(Return(42));
    ON_CALL(*m_authorization, checkCloudIsAvailable()).WillByDefault(Return(make_ok()));

    //! [THEN] The freshest version is fetched rather than the local copy being loaded
    EXPECT_CALL(*m_authorization, ensureAuthorization(_, _)).Times(1);
    EXPECT_CALL(*m_project, load(_, _, _)).Times(0);

    //! [WHEN] Opening it...
    openProject("/cloud/42.mscz");
}

TEST_F(ProjectActionsControllerTests, OpenProject_CloudScoreOffline_OpensTheLocalCopy)
{
    //! [GIVEN] A cloud score, an unreachable cloud, but the file is on disk...
    ON_CALL(*m_configuration, isCloudProject(_)).WillByDefault(Return(true));
    ON_CALL(*m_configuration, isLegacyCloudProject(_)).WillByDefault(Return(false));
    ON_CALL(*m_authorization, checkCloudIsAvailable()).WillByDefault(Return(make_ret(Ret::Code::InternalError)));
    ON_CALL(*m_fileSystem, exists(_)).WillByDefault(Return(true));

    //! [THEN] The local copy is opened so the user can keep working offline
    EXPECT_CALL(*m_project, load(io::path_t("/cloud/42.mscz"), _, _)).Times(1);
    EXPECT_CALL(*m_openSaveScenario, showCloudOpenError(_)).Times(0);

    //! [WHEN] Opening it...
    openProject("/cloud/42.mscz");
}

TEST_F(ProjectActionsControllerTests, OpenProject_CloudScoreOfflineAndNotOnDisk_ReportsTheNetwork)
{
    //! [GIVEN] A cloud score, an unreachable cloud, and no local copy...
    ON_CALL(*m_configuration, isCloudProject(_)).WillByDefault(Return(true));
    ON_CALL(*m_configuration, isLegacyCloudProject(_)).WillByDefault(Return(false));
    ON_CALL(*m_authorization, checkCloudIsAvailable()).WillByDefault(Return(make_ret(Ret::Code::InternalError)));
    ON_CALL(*m_fileSystem, exists(_)).WillByDefault(Return(false));

    //! [THEN] There is nothing to open, and the user is told why
    EXPECT_CALL(*m_openSaveScenario, showCloudOpenError(_)).Times(1);
    EXPECT_CALL(*m_project, load(_, _, _)).Times(0);

    //! [WHEN] Opening it...
    Ret ret = openProject("/cloud/42.mscz");

    EXPECT_EQ(ret.code(), int(cloud::Err::NetworkError));
}

TEST_F(ProjectActionsControllerTests, OpenProject_LegacyCloudScore_IsOpenedAsAnOrdinaryFile)
{
    //! [GIVEN] A score from the old cloud layout...
    ON_CALL(*m_configuration, isCloudProject(_)).WillByDefault(Return(true));
    ON_CALL(*m_configuration, isLegacyCloudProject(_)).WillByDefault(Return(true));

    //! [THEN] It is loaded straight from disk, without consulting the cloud at all
    EXPECT_CALL(*m_authorization, checkCloudIsAvailable()).Times(0);
    EXPECT_CALL(*m_project, load(io::path_t("/scores/legacy.mscz"), _, _)).Times(1);

    //! [WHEN] Opening it...
    openProject("/scores/legacy.mscz");
}

// ─── One open at a time ──────────────────────────────────────────────────────

TEST_F(ProjectActionsControllerTests, OpenProject_AlreadyOpening_RefusesToStartAgain)
{
    //! [GIVEN] An open whose path resolution re-enters the open, the way a nested event loop does
    Ret nested;
    ON_CALL(*m_fileSystem, absoluteFilePath(_))
    .WillByDefault([this, &nested](const io::path_t& p) {
        if (nested.valid()) {
            return p;
        }
        nested = openProject("/scores/other.mscz");
        return p;
    });

    //! [WHEN] Opening a score...
    openProject("/scores/symphony.mscz");

    //! [THEN] The re-entrant call is refused
    EXPECT_EQ(nested.code(), int(Ret::Code::Busy));
}

// ─── Opening from a command ──────────────────────────────────────────────────

TEST_F(ProjectActionsControllerTests, OpenProject_FromCommandParams_UsesUrlAndDisplayName)
{
    //! [GIVEN] An "open" command carrying a file url and a display name...
    ON_CALL(*m_globalContext, currentProject()).WillByDefault(Return(m_project));
    ON_CALL(*m_project, path()).WillByDefault(Return(io::path_t("/scores/other.mscz")));

    rcommand::Params params;
    params["url"] = Val(QUrl::fromLocalFile("/scores/symphony.mscz").toString().toStdString());
    params["display_name"] = Val("Cloud title");

    //! [THEN] Both reach the new window
    const QStringList expected { "/scores/symphony.mscz", "--score-display-name-override", "Cloud title" };
    EXPECT_CALL(*m_multiwindows, openNewWindow(expected)).Times(1);

    //! [WHEN] Handling the command...
    openProject(params);
}
// ─── Downloading a cloud score ───────────────────────────────────────────────

TEST_F(ProjectActionsControllerTests, DownloadCloudScore_NoScoreId_ReportsInsteadOfDownloading)
{
    //! [GIVEN] A cloud score whose id never made it to the server...
    //! [THEN] The user is told, and nothing is fetched
    EXPECT_CALL(*m_openSaveScenario, showCloudOpenError(_)).Times(1);
    EXPECT_CALL(*m_museScoreComService, downloadScore(_, _, _, _)).Times(0);

    //! [WHEN] Opening it...
    downloadAndOpenCloudProject(0);
}

TEST_F(ProjectActionsControllerTests, DownloadCloudScore_NotLoggedIn_FetchesNothing)
{
    //! [GIVEN] A user who declines to log in...
    ON_CALL(*m_authorization, ensureAuthorization(_, _))
    .WillByDefault(Return(RetVal<Val>(make_ret(Ret::Code::Cancel))));

    //! [THEN] Nothing is fetched
    EXPECT_CALL(*m_museScoreComService, downloadScore(_, _, _, _)).Times(0);

    //! [WHEN] Opening it...
    downloadAndOpenCloudProject(42);
}

TEST_F(ProjectActionsControllerTests, DownloadCloudScore_ScoreInfoUnavailable_ReportsIt)
{
    //! [GIVEN] A server that will not say anything about the score...
    ON_CALL(*m_museScoreComService, downloadScoreInfo(::testing::An<int>()))
    .WillByDefault(Return(RetVal<cloud::ScoreInfo>(make_ret(Ret::Code::InternalError))));

    //! [THEN] The user is told, and nothing is fetched
    EXPECT_CALL(*m_openSaveScenario, showCloudOpenError(_)).Times(1);
    EXPECT_CALL(*m_museScoreComService, downloadScore(_, _, _, _)).Times(0);

    //! [WHEN] Opening it...
    downloadAndOpenCloudProject(42);
}

TEST_F(ProjectActionsControllerTests, DownloadCloudScore_LocalCopyIsUpToDate_SkipsTheDownload)
{
    //! [GIVEN] A local copy at the same revision as the server's...
    cloud::ScoreInfo remote;
    remote.revisionId = 7;
    remote.title = "Symphony";
    ON_CALL(*m_museScoreComService, downloadScoreInfo(::testing::An<int>()))
    .WillByDefault(Return(RetVal<cloud::ScoreInfo>::make_ok(remote)));

    CloudProjectInfo local;
    local.revisionId = 7;
    ON_CALL(*m_mscMetaReader, readCloudProjectInfo(_))
    .WillByDefault(Return(RetVal<CloudProjectInfo>::make_ok(local)));

    //! [THEN] The copy on disk is opened as is, without going over the network again
    EXPECT_CALL(*m_museScoreComService, downloadScore(_, _, _, _)).Times(0);
    EXPECT_CALL(*m_globalContext, setCurrentProject(_)).Times(1);

    //! [WHEN] Opening it...
    downloadAndOpenCloudProject(42);
}

TEST_F(ProjectActionsControllerTests, DownloadCloudScore_LocalCopyIsStale_FetchesTheNewVersion)
{
    //! [GIVEN] A local copy older than the server's...
    cloud::ScoreInfo remote;
    remote.revisionId = 9;
    ON_CALL(*m_museScoreComService, downloadScoreInfo(::testing::An<int>()))
    .WillByDefault(Return(RetVal<cloud::ScoreInfo>::make_ok(remote)));

    CloudProjectInfo local;
    local.revisionId = 7;
    ON_CALL(*m_mscMetaReader, readCloudProjectInfo(_))
    .WillByDefault(Return(RetVal<CloudProjectInfo>::make_ok(local)));

    //! [THEN] The newer version is fetched
    EXPECT_CALL(*m_museScoreComService, downloadScore(42, _, _, _)).Times(1);

    //! [WHEN] Opening it...
    downloadAndOpenCloudProject(42);
}

TEST_F(ProjectActionsControllerTests, DownloadCloudScore_DownloadFails_ReportsAndOpensNothing)
{
    //! [GIVEN] A download that will fail...
    ON_CALL(*m_mscMetaReader, readCloudProjectInfo(_))
    .WillByDefault(Return(RetVal<CloudProjectInfo>(make_ret(Ret::Code::InternalError))));

    //! [THEN] The user is told, and no score becomes current
    EXPECT_CALL(*m_openSaveScenario, showCloudOpenError(_)).Times(1);
    EXPECT_CALL(*m_globalContext, setCurrentProject(_)).Times(0);

    //! [WHEN] Opening it, then letting the download report back...
    downloadAndOpenCloudProject(42);
    finishDownloadWith(make_ret(Ret::Code::InternalError));
}

TEST_F(ProjectActionsControllerTests, DownloadCloudScore_DownloadSucceeds_OpensItWithItsCloudDetails)
{
    //! [GIVEN] A fresh download of a score the user owns...
    cloud::ScoreInfo remote;
    remote.revisionId = 9;
    remote.title = "Symphony";
    ON_CALL(*m_museScoreComService, downloadScoreInfo(::testing::An<int>()))
    .WillByDefault(Return(RetVal<cloud::ScoreInfo>::make_ok(remote)));
    ON_CALL(*m_mscMetaReader, readCloudProjectInfo(_))
    .WillByDefault(Return(RetVal<CloudProjectInfo>(make_ret(Ret::Code::InternalError))));

    //! [THEN] The score opens carrying the details the server gave, and is remembered
    EXPECT_CALL(*m_project, setCloudInfo(::testing::Field(&CloudProjectInfo::revisionId, 9))).Times(1);
    EXPECT_CALL(*m_recentFiles, prependRecentFile(_)).Times(1);
    EXPECT_CALL(*m_globalContext, setCurrentProject(_)).Times(1);

    //! [WHEN] Opening it, then letting the download report back...
    downloadAndOpenCloudProject(42);
    finishDownloadWith(make_ok());
}

TEST_F(ProjectActionsControllerTests, DownloadCloudScore_SomeoneElsesScore_OpensAsAFreshUnsavedCopy)
{
    //! [GIVEN] A score belonging to another account...
    ON_CALL(*m_mscMetaReader, readCloudProjectInfo(_))
    .WillByDefault(Return(RetVal<CloudProjectInfo>(make_ret(Ret::Code::InternalError))));

    //! [THEN] It becomes an unsaved score of the user's own, and is not put in recent files
    EXPECT_CALL(*m_project, markAsNewlyCreated()).Times(1);
    EXPECT_CALL(*m_recentFiles, prependRecentFile(_)).Times(0);

    //! [WHEN] Opening it, then letting the download report back...
    downloadAndOpenCloudProject(42, QString(), QString(), false /*isOwner*/);
    finishDownloadWith(make_ok());
}

// ─── Opening a musescore.com link ────────────────────────────────────────────

TEST_F(ProjectActionsControllerTests, OpenScoreUrl_NotAScoreId_IsRejected)
{
    //! [GIVEN] An open-score link whose last segment is not a number...
    //! [WHEN] Following it...
    Ret ret = m_controller->openProject(ProjectFile(QUrl("musescore://open-score/not-a-number")));

    //! [THEN] It is refused as malformed
    EXPECT_EQ(ret.code(), int(Err::MalformedOpenScoreUrl));
}

TEST_F(ProjectActionsControllerTests, OpenScoreUrl_OwnScoreAlreadyOpenElsewhere_RaisesThatWindow)
{
    //! [GIVEN] A link to the user's own score, already open in another window...
    cloud::ScoreInfo remote;
    remote.owner.id = 5;
    ON_CALL(*m_museScoreComService, downloadScoreInfo(::testing::An<int>()))
    .WillByDefault(Return(RetVal<cloud::ScoreInfo>::make_ok(remote)));
    m_accountInfo.id = "5";
    ON_CALL(*m_multiwindows, isProjectAlreadyOpened(_)).WillByDefault(Return(true));

    //! [THEN] That window is raised instead of downloading a second copy
    EXPECT_CALL(*m_multiwindows, activateWindowWithProject(_)).Times(1);
    EXPECT_CALL(*m_museScoreComService, downloadScore(_, _, _, _)).Times(0);

    //! [WHEN] Following the link...
    m_controller->openProject(ProjectFile(QUrl("musescore://open-score/42")));
}

TEST_F(ProjectActionsControllerTests, OpenScoreUrl_WindowIsTaken_OpensANewWindowWithTheTitle)
{
    //! [GIVEN] A link to a score while this window already holds another...
    cloud::ScoreInfo remote;
    remote.title = "Cloud title";
    ON_CALL(*m_museScoreComService, downloadScoreInfo(::testing::An<int>()))
    .WillByDefault(Return(RetVal<cloud::ScoreInfo>::make_ok(remote)));
    ON_CALL(*m_globalContext, currentProject()).WillByDefault(Return(m_project));
    ON_CALL(*m_project, path()).WillByDefault(Return(io::path_t("/scores/other.mscz")));

    //! [THEN] The link and its title go to a new window
    const QStringList expected { "musescore://open-score/42", "--score-display-name-override", "Cloud title" };
    EXPECT_CALL(*m_multiwindows, openNewWindow(expected)).Times(1);

    //! [WHEN] Following the link...
    m_controller->openProject(ProjectFile(QUrl("musescore://open-score/42")));
}

TEST_F(ProjectActionsControllerTests, OpenScoreUrl_SharedLink_PassesHashAndSecretToTheDownload)
{
    //! [GIVEN] A shared link carrying its access parameters...
    ON_CALL(*m_mscMetaReader, readCloudProjectInfo(_))
    .WillByDefault(Return(RetVal<CloudProjectInfo>(make_ret(Ret::Code::InternalError))));

    //! [THEN] Both reach the download, otherwise a private score would be refused
    EXPECT_CALL(*m_museScoreComService, downloadScore(42, _, QString("abc"), QString("xyz"))).Times(1);

    //! [WHEN] Following the link...
    m_controller->openProject(ProjectFile(QUrl("musescore://open-score/42?h=abc&secret=xyz")));
}
// ─── A file that will not load on the first try ──────────────────────────────

TEST_F(ProjectActionsControllerTests, OpenProject_FileFromAnOlderVersionAndUserAgrees_LoadsItForcibly)
{
    //! [GIVEN] A score saved by an older version, and a user who wants it opened anyway...
    ON_CALL(*m_project, load(_, _, _))
    .WillByDefault(Return(make_ret(engraving::Err::FileTooOld)));
    ON_CALL(*m_interactive, warningSync(_, _, _, _, _, _))
    .WillByDefault(Return(IInteractive::Result(int(IInteractive::Button::CustomButton))));

    //! [THEN] It is read a second time, this time forcing the old format through
    EXPECT_CALL(*m_project, load(_, ::testing::Field(&OpenParams::forceMode, false), _)).Times(1);
    EXPECT_CALL(*m_project, load(_, ::testing::Field(&OpenParams::forceMode, true), _)).Times(1);

    //! [WHEN] Opening it...
    openProject("/scores/ancient.mscz");
}

TEST_F(ProjectActionsControllerTests, OpenProject_FileFromAnOlderVersionAndUserDeclines_GivesUp)
{
    //! [GIVEN] A score saved by an older version, and a user who cancels...
    ON_CALL(*m_project, load(_, _, _))
    .WillByDefault(Return(make_ret(engraving::Err::FileTooOld)));
    ON_CALL(*m_interactive, warningSync(_, _, _, _, _, _))
    .WillByDefault(Return(IInteractive::Result(int(IInteractive::Button::Cancel))));

    //! [THEN] There is no second attempt, and no score becomes current
    EXPECT_CALL(*m_project, load(_, _, _)).Times(1);
    EXPECT_CALL(*m_globalContext, setCurrentProject(_)).Times(0);

    //! [WHEN] Opening it...
    Ret ret = openProject("/scores/ancient.mscz");

    EXPECT_FALSE(ret);
}

TEST_F(ProjectActionsControllerTests, OpenProject_FileFromANewerVersion_RetriesOnlyIfCheckingIsOff)
{
    //! [GIVEN] A score saved by a newer version, with version checking switched off...
    ON_CALL(*m_project, load(_, _, _))
    .WillByDefault(Return(make_ret(engraving::Err::FileTooNew)));
    ON_CALL(*m_configuration, disableVersionChecking()).WillByDefault(Return(true));

    //! [THEN] The user is told, and the file is read again anyway
    EXPECT_CALL(*m_interactive, error(_, _, _, _, _, _)).Times(1);
    EXPECT_CALL(*m_project, load(_, ::testing::Field(&OpenParams::forceMode, false), _)).Times(1);
    EXPECT_CALL(*m_project, load(_, ::testing::Field(&OpenParams::forceMode, true), _)).Times(1);

    //! [WHEN] Opening it...
    openProject("/scores/future.mscz");
}

TEST_F(ProjectActionsControllerTests, OpenProject_FileFromANewerVersionAndCheckingIsOn_GivesUp)
{
    //! [GIVEN] A score saved by a newer version, with version checking left on...
    ON_CALL(*m_project, load(_, _, _))
    .WillByDefault(Return(make_ret(engraving::Err::FileTooNew)));
    ON_CALL(*m_configuration, disableVersionChecking()).WillByDefault(Return(false));

    //! [THEN] It is read once and abandoned
    EXPECT_CALL(*m_project, load(_, _, _)).Times(1);

    //! [WHEN] Opening it...
    openProject("/scores/future.mscz");
}

TEST_F(ProjectActionsControllerTests, OpenProject_CorruptedFileAndUserAgrees_LoadsItForcibly)
{
    //! [GIVEN] A corrupted score the user wants opened regardless...
    ON_CALL(*m_project, load(_, _, _))
    .WillByDefault(Return(make_ret(engraving::Err::FileCorrupted)));
    ON_CALL(*m_interactive, warningSync(_, _, _, _, _, _))
    .WillByDefault(Return(IInteractive::Result(int(IInteractive::Button::CustomButton))));

    //! [THEN] It is read a second time, forcing past the damage
    EXPECT_CALL(*m_project, load(_, ::testing::Field(&OpenParams::forceMode, false), _)).Times(1);
    EXPECT_CALL(*m_project, load(_, ::testing::Field(&OpenParams::forceMode, true), _)).Times(1);

    //! [WHEN] Opening it...
    openProject("/scores/broken.mscz");
}

TEST_F(ProjectActionsControllerTests, OpenProject_CriticallyCorruptedFile_IsNeverRetried)
{
    //! [GIVEN] A score damaged beyond use...
    ON_CALL(*m_project, load(_, _, _))
    .WillByDefault(Return(make_ret(engraving::Err::FileCriticallyCorrupted)));

    //! [THEN] There is no second attempt, whatever the user answers
    EXPECT_CALL(*m_project, load(_, _, _)).Times(1);
    EXPECT_CALL(*m_globalContext, setCurrentProject(_)).Times(0);

    //! [WHEN] Opening it...
    Ret ret = openProject("/scores/ruined.mscz");

    EXPECT_FALSE(ret);
}

TEST_F(ProjectActionsControllerTests, OpenProject_UnreadableFile_IsReportedAndNotRetried)
{
    //! [GIVEN] A file that cannot be read at all...
    ON_CALL(*m_project, load(_, _, _))
    .WillByDefault(Return(make_ret(engraving::Err::FileNotFound)));

    //! [THEN] The user is told once, and there is no second attempt
    EXPECT_CALL(*m_interactive, error(_, _, _, _, _, _)).Times(1);
    EXPECT_CALL(*m_project, load(_, _, _)).Times(1);

    //! [WHEN] Opening it...
    Ret ret = openProject("/scores/missing.mscz");

    EXPECT_FALSE(ret);
}

TEST_F(ProjectActionsControllerTests, OpenProject_UserCancelledTheLoad_AsksNothingFurther)
{
    //! [GIVEN] A load the user cancelled from inside, for instance at a format prompt...
    ON_CALL(*m_project, load(_, _, _))
    .WillByDefault(Return(make_ret(Ret::Code::Cancel)));

    //! [THEN] Cancelling is not an error to report, and nothing is retried
    EXPECT_CALL(*m_interactive, error(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(*m_interactive, warningSync(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(*m_project, load(_, _, _)).Times(1);

    //! [WHEN] Opening it...
    openProject("/scores/symphony.mscz");
}

// ─── Unsaved work from a previous session ────────────────────────────────────

TEST_F(ProjectActionsControllerTests, OpenProject_HasAnAutosave_LoadsItAndKeepsTheOriginalPath)
{
    //! [GIVEN] A score with unsaved changes left over from a crash...
    ON_CALL(*m_autoSaver, projectHasUnsavedChanges(_)).WillByDefault(Return(true));
    ON_CALL(*m_autoSaver, projectAutoSavePath(_))
    .WillByDefault(Return(io::path_t("/scores/symphony.mscz,")));

    //! [THEN] The autosave is what gets read, but the score keeps pointing at the real file
    //! and is marked unsaved, so the user is not fooled into thinking it is on disk
    EXPECT_CALL(*m_project, load(io::path_t("/scores/symphony.mscz,"), _, _)).Times(1);
    EXPECT_CALL(*m_project, setPath(io::path_t("/scores/symphony.mscz"))).Times(1);
    EXPECT_CALL(*m_project, markAsUnsaved()).Times(1);

    //! [WHEN] Opening it...
    openProject("/scores/symphony.mscz");
}

TEST_F(ProjectActionsControllerTests, OpenProject_AutosaveOfANeverSavedScore_OpensAsNewlyCreated)
{
    //! [GIVEN] An autosave of a score that was never saved anywhere...
    ON_CALL(*m_autoSaver, isAutosaveOfNewlyCreatedProject(_)).WillByDefault(Return(true));

    //! [THEN] It stays a new score, and does not enter the recent files list
    EXPECT_CALL(*m_project, markAsNewlyCreated()).Times(1);
    EXPECT_CALL(*m_recentFiles, prependRecentFile(_)).Times(0);

    //! [WHEN] Opening it...
    openProject("/scores/untitled.mscz");
}
}
