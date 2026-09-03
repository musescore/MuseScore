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

#include <QTimer>

#include "async/processevents.h"

#include "project/internal/saveprojectscenario.h"
#include "project/projecterrors.h"

#include "cloud/clouderrors.h"
#include "cloud/qml/Muse/Cloud/enums.h"

#include "cloud/tests/mocks/audiocomservicemock.h"
#include "cloud/tests/mocks/authorizationservicemock.h"
#include "cloud/tests/mocks/musescorecomservicemock.h"
#include "context/tests/mocks/globalcontextmock.h"
#include "global/tests/mocks/filesystemmock.h"
#include "interactive/tests/mocks/interactivemock.h"
#include "interactive/tests/mocks/platforminteractivemock.h"
#include "notation/tests/mocks/masternotationmock.h"
#include "notation/tests/mocks/notationconfigurationmock.h"
#include "notation/tests/mocks/notationinteractionmock.h"
#include "notation/tests/mocks/notationmock.h"

#include "mocks/exportprojectscenariomock.h"
#include "mocks/notationprojectmock.h"
#include "mocks/openprojectscenariomock.h"
#include "mocks/projectconfigurationmock.h"
#include "mocks/recentfilescontrollermock.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

using namespace muse;
using namespace mu::project;

namespace mu::project {
class SaveProjectScenarioTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_scenario = std::make_shared<SaveProjectScenario>(modularity::globalCtx());

        m_configuration = std::make_shared<NiceMock<ProjectConfigurationMock> >();
        m_fileSystem = std::make_shared<NiceMock<io::FileSystemMock> >();
        m_notationConfiguration = std::make_shared<NiceMock<notation::NotationConfigurationMock> >();
        m_museScoreComService = std::make_shared<NiceMock<cloud::MuseScoreComServiceMock> >();
        m_audioComService = std::make_shared<NiceMock<cloud::AudioComServiceMock> >();
        m_authorization = std::make_shared<NiceMock<cloud::AuthorizationServiceMock> >();
        m_platformInteractive = std::make_shared<NiceMock<PlatformInteractiveMock> >();
        m_recentFiles = std::make_shared<NiceMock<RecentFilesControllerMock> >();
        m_exportScenario = std::make_shared<NiceMock<ExportProjectScenarioMock> >();
        m_openScenario = std::make_shared<NiceMock<OpenProjectScenarioMock> >();
        m_interactive = std::make_shared<NiceMock<InteractiveMock> >();
        m_globalContext = std::make_shared<NiceMock<context::GlobalContextMock> >();

        m_scenario->configuration.set(m_configuration);
        m_scenario->fileSystem.set(m_fileSystem);
        m_scenario->notationConfiguration.set(m_notationConfiguration);
        m_scenario->museScoreComService.set(m_museScoreComService);
        m_scenario->audioComService.set(m_audioComService);
        m_scenario->platformInteractive.set(m_platformInteractive);
        m_scenario->recentFilesController.set(m_recentFiles);
        m_scenario->exportProjectScenario.set(m_exportScenario);
        m_scenario->openProjectScenario.set(m_openScenario);
        m_scenario->interactive.set(m_interactive);
        m_scenario->globalContext.set(m_globalContext);

        m_project = std::make_shared<NiceMock<NotationProjectMock> >();
        m_masterNotation = std::make_shared<NiceMock<notation::MasterNotationMock> >();
        m_notation = std::make_shared<NiceMock<notation::NotationMock> >();
        m_interaction = std::make_shared<NiceMock<notation::NotationInteractionMock> >();

        ON_CALL(*m_globalContext, currentProject()).WillByDefault(Return(m_project));
        ON_CALL(*m_project, masterNotation()).WillByDefault(Return(m_masterNotation));
        ON_CALL(*m_masterNotation, notation()).WillByDefault(Return(m_notation));
        ON_CALL(*m_notation, interaction()).WillByDefault(Return(m_interaction));
        ON_CALL(*m_museScoreComService, authorization()).WillByDefault(Return(m_authorization));
        ON_CALL(*m_audioComService, authorization()).WillByDefault(Return(m_authorization));

        // A healthy score that saves successfully, unless a test says otherwise.
        ON_CALL(*m_project, canSave()).WillByDefault(Return(make_ok()));
        ON_CALL(*m_project, save(_, _, _)).WillByDefault(Return(make_ok()));
        ON_CALL(*m_project, cloudInfo()).WillByDefault(ReturnRef(m_cloudInfo));

        // A publish also uploads the rendered audio; hand it a progress of its own.
        ON_CALL(*m_museScoreComService, uploadAudio(_, _, _))
        .WillByDefault([](DevicePtr, const QString&, const QUrl&) { return std::make_shared<Progress>(); });

        // Dialogs resolve immediately so that unstubbed paths do not abort the test.
        ON_CALL(*m_interactive, warning(_, _, _, _, _, _)).WillByDefault([] { return resolvedResult(); });
        ON_CALL(*m_interactive, error(_, _, _, _, _, _)).WillByDefault([] { return resolvedResult(); });
        ON_CALL(*m_interactive, buttonData(_)).WillByDefault([](IInteractive::Button btn) {
            return IInteractive::ButtonData(btn, "");
        });
        ON_CALL(*m_interactive, info(_, _, _, _, _, _)).WillByDefault([] { return resolvedResult(); });
        ON_CALL(*m_interactive, open(_)).WillByDefault([] {
            return async::make_promise<Val>([](auto resolve, auto) {
                return resolve(Val());
            });
        });
    }

    static async::Promise<IInteractive::Result> resolvedResult()
    {
        return async::make_promise<IInteractive::Result>([](auto resolve, auto) {
            return resolve(IInteractive::Result(int(IInteractive::Button::Ok)));
        });
    }

    Ret saveProject(SaveMode mode, SaveLocationType type = SaveLocationType::Undefined, bool force = false)
    {
        return m_scenario->saveProject(mode, type, force);
    }

    Ret saveProjectAt(const SaveLocation& location, SaveMode mode = SaveMode::Save, bool force = false)
    {
        return m_scenario->saveProjectAt(location, mode, force);
    }

    Ret saveProjectAt(const rcommand::Params& params)
    {
        return m_scenario->saveProjectAt(params);
    }

    bool saveProjectToCloud(const CloudProjectInfo& info, SaveMode mode = SaveMode::Save)
    {
        return m_scenario->saveProjectToCloud(info, mode);
    }

    RetVal<bool> needGenerateAudio(bool isPublic)
    {
        return m_scenario->needGenerateAudio(isPublic);
    }

    static ::testing::Matcher<const UriQuery&> IsDialog(const std::string& uri)
    {
        return ::testing::Truly([uri](const UriQuery& q) {
            return q.uri().toString() == uri;
        });
    }

    static ::testing::Matcher<const UriQuery&> IsLoginDialog()
    {
        return IsDialog("muse://cloud/requireauthorization");
    }

    static ::testing::Matcher<const UriQuery&> IsSaveToCloudDialog()
    {
        return IsDialog("musescore://project/savetocloud");
    }

    static ::testing::Matcher<const UriQuery&> IsAudioGenerationSettingsDialog()
    {
        return IsDialog("musescore://project/audiogenerationsettings");
    }

    //! The account is signed in, so nothing is asked.
    void givenSignedIn()
    {
        ValCh<bool> authorized;
        authorized.val = true;
        ON_CALL(*m_authorization, userAuthorized()).WillByDefault(Return(authorized));
    }

    //! The account is signed out, and the login dialog answers with `answer`.
    void givenLoginDialogAnswers(const RetVal<Val>& answer)
    {
        ValCh<bool> authorized;
        authorized.val = false;
        ON_CALL(*m_authorization, userAuthorized()).WillByDefault(Return(authorized));
        ON_CALL(*m_interactive, openSync(IsLoginDialog())).WillByDefault(Return(answer));
    }

    static void drainDeferredCalls()
    {
        async::processMessages();
    }

    //! The save location dialog is settled on "local file", and resolves to `path`.
    void givenUserPicksLocalFile(const io::path_t& path)
    {
        ON_CALL(*m_configuration, shouldAskSaveLocationType()).WillByDefault(Return(false));
        ON_CALL(*m_configuration, lastUsedSaveLocationType()).WillByDefault(Return(SaveLocationType::Local));
        ON_CALL(*m_interactive, selectSavingFileSync(_, _, _, _)).WillByDefault(Return(path));
    }

    //! The user dismisses the file dialog without choosing anything.
    void givenUserCancelsTheSaveDialog()
    {
        givenUserPicksLocalFile(io::path_t());
    }

    //! The cloud destination dialog is answered with "publish under this name".
    void givenUserConfirmsCloudDialog(const QString& name = "Score")
    {
        QVariantMap answer;
        answer["response"] = int(cloud::SaveToCloudResponse::SaveToCloudResponse::Ok);
        answer["name"] = name;
        answer["visibility"] = int(cloud::Visibility::Private);
        answer["replaceExisting"] = false;
        ON_CALL(*m_interactive, openSync(_))
        .WillByDefault(Return(RetVal<Val>::make_ok(Val::fromQVariant(answer))));
    }

    void givenReachableCloud()
    {
        ON_CALL(*m_authorization, checkCloudIsAvailable()).WillByDefault(Return(make_ok()));
        givenSignedIn();

        // Settled audio generation settings, so that the decision does not open a dialog and abort the save.
        ON_CALL(*m_configuration, hasAskedAudioGenerationSettings()).WillByDefault(Return(true));
        ON_CALL(*m_configuration, generateAudioTimePeriodType()).WillByDefault(Return(GenerateAudioTimePeriodType::Never));
    }

    //! NOTE `uploadProject` blocks on a nested QEventLoop until the progress reports it is finished,
    //! so the result has to be delivered from inside that loop rather than before it starts.
    void givenUploadFinishesWith(const Ret& ret, const ValMap& result, std::function<void()> alsoDo = nullptr)
    {
        ON_CALL(*m_museScoreComService, uploadScore(_, _, _, _, _))
        .WillByDefault([ret, result, alsoDo](DevicePtr, const QString&, cloud::Visibility, const QUrl&, int) {
            auto progress = std::make_shared<Progress>();
            QTimer::singleShot(0, [progress, ret, result, alsoDo]() {
                if (alsoDo) {
                    alsoDo();
                }

                ProgressResult res;
                res.ret = ret;
                res.val = Val(result);
                progress->finish(res);
            });
            return progress;
        });
    }

    //! NOTE Mirrors the button ids that projectactionscontroller.cpp assigns to its custom buttons.
    static constexpr int RETRY_SAVE_BTN_ID = int(IInteractive::Button::CustomButton);
    static constexpr int SAVE_AS_BTN_ID = RETRY_SAVE_BTN_ID + 1;
    static constexpr int SAVE_ANYWAY_BTN_ID = int(IInteractive::Button::CustomButton);
    static constexpr int REVERT_TO_LAST_SAVED_BTN_ID = SAVE_ANYWAY_BTN_ID + 1;

    std::shared_ptr<SaveProjectScenario> m_scenario;

    std::shared_ptr<ProjectConfigurationMock> m_configuration;
    std::shared_ptr<io::FileSystemMock> m_fileSystem;
    std::shared_ptr<notation::NotationConfigurationMock> m_notationConfiguration;
    std::shared_ptr<cloud::MuseScoreComServiceMock> m_museScoreComService;
    std::shared_ptr<cloud::AudioComServiceMock> m_audioComService;
    std::shared_ptr<cloud::AuthorizationServiceMock> m_authorization;
    std::shared_ptr<PlatformInteractiveMock> m_platformInteractive;
    std::shared_ptr<RecentFilesControllerMock> m_recentFiles;
    std::shared_ptr<ExportProjectScenarioMock> m_exportScenario;
    std::shared_ptr<OpenProjectScenarioMock> m_openScenario;
    std::shared_ptr<InteractiveMock> m_interactive;
    std::shared_ptr<context::GlobalContextMock> m_globalContext;

    std::shared_ptr<NotationProjectMock> m_project;
    std::shared_ptr<notation::MasterNotationMock> m_masterNotation;
    std::shared_ptr<notation::NotationMock> m_notation;
    std::shared_ptr<notation::NotationInteractionMock> m_interaction;

    CloudProjectInfo m_cloudInfo;
};

// ─── Where does the score go: ask, or save silently ──────────────────────────

TEST_F(SaveProjectScenarioTests, SaveProject_ExistingLocalScore_SavesWithoutAsking)
{
    //! [GIVEN] An already saved local score...
    ON_CALL(*m_project, isNewlyCreated()).WillByDefault(Return(false));
    ON_CALL(*m_project, isCloudProject()).WillByDefault(Return(false));

    //! [THEN] The save location is not asked for, and the score is written back over itself:
    //! an empty path is what tells the project to keep the file it already has
    EXPECT_CALL(*m_interactive, selectSavingFileSync(_, _, _, _)).Times(0);
    EXPECT_CALL(*m_project, save(io::path_t(), SaveMode::Save, true)).Times(1);

    //! [WHEN] Saving it...
    Ret ret = saveProject(SaveMode::Save);

    EXPECT_TRUE(ret);
}

TEST_F(SaveProjectScenarioTests, SaveProject_ExistingCloudScore_SavesWithoutAsking)
{
    //! [GIVEN] An already saved cloud score...
    ON_CALL(*m_project, isNewlyCreated()).WillByDefault(Return(false));
    ON_CALL(*m_project, isCloudProject()).WillByDefault(Return(true));

    //! [GIVEN] ...while the cloud is unreachable, so only the local write happens
    ON_CALL(*m_authorization, checkCloudIsAvailable()).WillByDefault(Return(make_ret(Ret::Code::InternalError)));
    ON_CALL(*m_configuration, showCloudIsNotAvailableWarning()).WillByDefault(Return(false));

    m_cloudInfo.name = "Symphony";
    m_cloudInfo.sourceUrl = QUrl("score-42");

    const io::path_t path = "project.mscz";
    ON_CALL(*m_project, path()).WillByDefault(Return(path));

    //! [THEN] The save location is not asked for; the score keeps the cloud details it already had,
    //! and is still written to its own file so that the work survives until the connection returns
    EXPECT_CALL(*m_interactive, selectSavingFileSync(_, _, _, _)).Times(0);
    EXPECT_CALL(*m_project, setCloudInfo(::testing::AllOf(
                                             ::testing::Field(&CloudProjectInfo::name, QString("Symphony")),
                                             ::testing::Field(&CloudProjectInfo::sourceUrl, m_cloudInfo.sourceUrl)))).Times(1);
    EXPECT_CALL(*m_project, save(path, SaveMode::Save, true)).Times(1);

    //! [WHEN] Saving it...
    saveProject(SaveMode::Save);
}

TEST_F(SaveProjectScenarioTests, SaveProject_NewlyCreatedScore_AsksForSaveLocation)
{
    //! [GIVEN] A score that has never been saved...
    ON_CALL(*m_project, isNewlyCreated()).WillByDefault(Return(true));
    ON_CALL(*m_project, isCloudProject()).WillByDefault(Return(false));

    //! [THEN] The save location is asked for
    givenUserCancelsTheSaveDialog();
    EXPECT_CALL(*m_interactive, selectSavingFileSync(_, _, _, _)).Times(1);

    //! [WHEN] Saving it...
    Ret ret = saveProject(SaveMode::Save);

    //! [THEN] Cancelling the dialog cancels the save
    EXPECT_FALSE(ret);
}

TEST_F(SaveProjectScenarioTests, SaveProject_SaveAs_AlwaysAsksForSaveLocation)
{
    //! [GIVEN] An already saved local score...
    ON_CALL(*m_project, isNewlyCreated()).WillByDefault(Return(false));
    ON_CALL(*m_project, isCloudProject()).WillByDefault(Return(false));

    //! [THEN] "Save as" asks for a location even though the score already has one
    givenUserCancelsTheSaveDialog();
    EXPECT_CALL(*m_interactive, selectSavingFileSync(_, _, _, _)).Times(1);

    //! [WHEN] Saving it as a new file...
    saveProject(SaveMode::SaveAs);
}

TEST_F(SaveProjectScenarioTests, SaveProject_LocalScoreToCloud_AsksForSaveLocation)
{
    //! [GIVEN] An already saved score that does not live in the cloud yet...
    ON_CALL(*m_project, isNewlyCreated()).WillByDefault(Return(false));
    ON_CALL(*m_project, isCloudProject()).WillByDefault(Return(false));

    //! [THEN] The cloud is a destination it has never had, so it is asked for
    givenReachableCloud();
    ON_CALL(*m_interactive, openSync(_))
    .WillByDefault(Return(RetVal<Val>(make_ret(Ret::Code::Cancel))));
    EXPECT_CALL(*m_interactive, openSync(IsSaveToCloudDialog())).Times(1);

    //! [WHEN] Saving it to the cloud...
    saveProject(SaveMode::Save, SaveLocationType::Cloud);
}

TEST_F(SaveProjectScenarioTests, SaveProject_SaveCopy_AsksForSaveLocation)
{
    //! [GIVEN] An already saved local score...
    ON_CALL(*m_project, isNewlyCreated()).WillByDefault(Return(false));
    ON_CALL(*m_project, isCloudProject()).WillByDefault(Return(false));

    //! [THEN] A copy is a new file, so its location is asked for
    givenUserCancelsTheSaveDialog();
    EXPECT_CALL(*m_interactive, selectSavingFileSync(_, _, _, _)).Times(1);

    //! [WHEN] Saving a copy...
    saveProject(SaveMode::SaveCopy);
}

TEST_F(SaveProjectScenarioTests, SaveProject_ChosenLocalLocation_IsUsedAsGiven)
{
    //! [GIVEN] A score that has never been saved...
    ON_CALL(*m_project, isNewlyCreated()).WillByDefault(Return(true));
    ON_CALL(*m_project, isCloudProject()).WillByDefault(Return(false));

    //! [GIVEN] ...and a user who picks a local file for it
    const io::path_t chosen = "chosen.mscz";
    givenUserPicksLocalFile(chosen);
    ON_CALL(*m_project, path()).WillByDefault(Return(chosen));

    //! [THEN] The score lands exactly where the user pointed, and that file is the one remembered
    EXPECT_CALL(*m_project, save(chosen, SaveMode::Save, true)).Times(1);
    EXPECT_CALL(*m_recentFiles, prependRecentFile(RecentFile(chosen))).Times(1);

    //! [WHEN] Saving it...
    Ret ret = saveProject(SaveMode::Save);

    EXPECT_TRUE(ret);
}

TEST_F(SaveProjectScenarioTests, SaveProject_ChosenCloudLocation_IsAppliedToTheProject)
{
    //! [GIVEN] A score that has never been saved...
    ON_CALL(*m_project, isNewlyCreated()).WillByDefault(Return(true));
    ON_CALL(*m_project, isCloudProject()).WillByDefault(Return(false));

    //! [GIVEN] ...and a user who picks the cloud for it, naming the score in the dialog
    givenReachableCloud();
    ON_CALL(*m_configuration, shouldAskSaveLocationType()).WillByDefault(Return(false));
    ON_CALL(*m_configuration, lastUsedSaveLocationType()).WillByDefault(Return(SaveLocationType::Cloud));

    QVariantMap answer;
    answer["response"] = int(cloud::SaveToCloudResponse::SaveToCloudResponse::Ok);
    answer["name"] = "Chosen name";
    answer["visibility"] = int(cloud::Visibility::Private);
    answer["replaceExisting"] = false;
    ON_CALL(*m_interactive, openSync(_)).WillByDefault(Return(RetVal<Val>::make_ok(Val::fromQVariant(answer))));

    ON_CALL(*m_project, writeToDevice(_)).WillByDefault(Return(make_ok()));
    ON_CALL(*m_configuration, cloudProjectSavingPath(0)).WillByDefault(Return(io::path_t("cloud.mscz")));
    givenUploadFinishesWith(make_ok(), ValMap());

    std::vector<CloudProjectInfo> stored;
    ON_CALL(*m_project, setCloudInfo(_)).WillByDefault([&stored](const CloudProjectInfo& i) {
        stored.push_back(i);
    });

    //! [THEN] The score is written to the path reserved for cloud scores on the way to the cloud
    EXPECT_CALL(*m_project, save(io::path_t("cloud.mscz"), SaveMode::Save, true)).Times(1);

    //! [WHEN] Saving it...
    saveProject(SaveMode::Save);

    //! [THEN] The name the user gave in the dialog is what lands on the project
    ASSERT_FALSE(stored.empty());
    EXPECT_EQ(stored.front().name, QString("Chosen name"));
}

TEST_F(SaveProjectScenarioTests, SaveProject_Forced_SkipsTheCanSaveCheck)
{
    //! [GIVEN] A corrupted score the user has chosen to save anyway...
    ON_CALL(*m_project, isNewlyCreated()).WillByDefault(Return(false));
    ON_CALL(*m_project, isCloudProject()).WillByDefault(Return(false));
    ON_CALL(*m_project, canSave()).WillByDefault(Return(make_ret(Err::CorruptionError)));

    //! [THEN] The corruption check is not consulted at all, and the score is written back over itself
    EXPECT_CALL(*m_project, canSave()).Times(0);
    EXPECT_CALL(*m_project, save(io::path_t(), SaveMode::Save, true)).Times(1);

    //! [WHEN] Saving it with force...
    saveProject(SaveMode::Save, SaveLocationType::Undefined, true /*force*/);
}

TEST_F(SaveProjectScenarioTests, SaveProject_NoOpenScore_IsRefusedInsteadOfCrashing)
{
    //! [GIVEN] No score open at all...
    ON_CALL(*m_globalContext, currentProject()).WillByDefault(Return(nullptr));

    //! [THEN] Nothing is asked and nothing is written
    EXPECT_CALL(*m_interactive, selectSavingFileSync(_, _, _, _)).Times(0);

    //! [WHEN] A save is requested anyway, as the command path allows...
    Ret ret = saveProject(SaveMode::Save);

    //! [THEN] It is refused with a clear reason
    EXPECT_EQ(ret.code(), int(Err::NoProjectError));
}

TEST_F(SaveProjectScenarioTests, ShareAudio_NoOpenScore_IsRefusedInsteadOfCrashing)
{
    //! [GIVEN] A reachable cloud, so that the sharing gets as far as needing the score...
    givenReachableCloud();

    //! [GIVEN] ...and no score open at all
    ON_CALL(*m_globalContext, currentProject()).WillByDefault(Return(nullptr));

    //! [THEN] Nothing is asked and nothing is uploaded
    EXPECT_CALL(*m_interactive, openSync(IsSaveToCloudDialog())).Times(0);
    EXPECT_CALL(*m_audioComService, uploadAudio(_, _, _, _, _, _)).Times(0);

    //! [WHEN] Sharing the audio anyway, as the menu allows...
    Ret ret = m_scenario->shareAudio();

    //! [THEN] It is refused with a clear reason
    EXPECT_EQ(ret.code(), int(Err::NoProjectError));
}

// ─── One save at a time ──────────────────────────────────────────────────────

TEST_F(SaveProjectScenarioTests, SaveProject_AlreadySaving_RefusesToStartAgain)
{
    //! [GIVEN] A score that has never been saved...
    ON_CALL(*m_project, isNewlyCreated()).WillByDefault(Return(true));
    ON_CALL(*m_project, isCloudProject()).WillByDefault(Return(false));

    //! [GIVEN] ...and a location dialog that re-enters the save, the way a nested event loop does
    Ret nested;
    bool busyDuringDialog = false;

    givenUserCancelsTheSaveDialog();
    ON_CALL(*m_interactive, selectSavingFileSync(_, _, _, _))
    .WillByDefault([this, &nested, &busyDuringDialog](const std::string&, const io::path_t&,
                                                      const std::vector<std::string>&, bool) {
        busyDuringDialog = m_scenario->isBusy(BusyStatus::Saving);
        nested = saveProject(SaveMode::Save);
        return io::path_t();
    });

    //! [WHEN] Saving it...
    saveProject(SaveMode::Save);

    //! [THEN] The save was marked busy while the dialog was up, and the re-entrant call was refused
    EXPECT_TRUE(busyDuringDialog);
    EXPECT_EQ(nested.code(), int(Ret::Code::Busy));
}

TEST_F(SaveProjectScenarioTests, SaveProject_AfterSaving_IsNoLongerBusy)
{
    //! [GIVEN] An already saved local score...
    ON_CALL(*m_project, isNewlyCreated()).WillByDefault(Return(false));
    ON_CALL(*m_project, isCloudProject()).WillByDefault(Return(false));

    //! [WHEN] Saving it once...
    EXPECT_TRUE(saveProject(SaveMode::Save));

    //! [THEN] The busy flag is released, so the next save is not refused
    EXPECT_FALSE(m_scenario->isBusy(BusyStatus::Saving));
    EXPECT_TRUE(saveProject(SaveMode::Save));
}

// ─── The public entry point used by the close and quit flows ─────────────────

TEST_F(SaveProjectScenarioTests, SaveProjectToPath_AlreadySaving_RefusesToStartAgain)
{
    //! [GIVEN] A write that re-enters the save while it is still in progress...
    bool nested = true;

    ON_CALL(*m_project, save(_, _, _))
    .WillByDefault([this, &nested](const io::path_t&, SaveMode, bool) {
        nested = m_scenario->saveProject(io::path_t("other.mscz"));
        return make_ok();
    });

    //! [WHEN] Saving to an explicit path...
    bool ok = m_scenario->saveProject(io::path_t("explicit.mscz"));

    //! [THEN] The outer save succeeds and the re-entrant one is refused
    EXPECT_TRUE(ok);
    EXPECT_FALSE(nested);
}

TEST_F(SaveProjectScenarioTests, SaveProjectToPath_PathGiven_WritesThereWithoutAsking)
{
    //! [GIVEN] An explicit destination...
    const io::path_t path = "explicit.mscz";

    //! [THEN] It is used as is, and nothing is asked
    EXPECT_CALL(*m_interactive, selectSavingFileSync(_, _, _, _)).Times(0);
    EXPECT_CALL(*m_project, save(path, SaveMode::Save, true)).Times(1);

    //! [WHEN] Saving to that path...
    bool ok = m_scenario->saveProject(path);

    EXPECT_TRUE(ok);
}

TEST_F(SaveProjectScenarioTests, SaveProjectToPath_NoPath_SavesTheScoreWhereItAlreadyLives)
{
    //! [GIVEN] An already saved local score...
    ON_CALL(*m_project, isNewlyCreated()).WillByDefault(Return(false));
    ON_CALL(*m_project, isCloudProject()).WillByDefault(Return(false));

    //! [THEN] Omitting the path falls back to a plain save over the existing file
    EXPECT_CALL(*m_interactive, selectSavingFileSync(_, _, _, _)).Times(0);
    EXPECT_CALL(*m_project, save(io::path_t(), SaveMode::Save, true)).Times(1);

    //! [WHEN] Saving without naming a path...
    bool ok = m_scenario->saveProject();

    EXPECT_TRUE(ok);
}

TEST_F(SaveProjectScenarioTests, SaveProjectToPath_WriteFails_ReportsFailure)
{
    //! [GIVEN] A score whose write will fail...
    ON_CALL(*m_project, save(_, _, _)).WillByDefault(Return(make_ret(Ret::Code::InternalError)));

    //! [WHEN] Saving it to an explicit path...
    bool ok = m_scenario->saveProject(io::path_t("explicit.mscz"));

    //! [THEN] The failure is reported, so that the close and quit flows can stop
    EXPECT_FALSE(ok);
}

// ─── Writing to disk ─────────────────────────────────────────────────────────

TEST_F(SaveProjectScenarioTests, SaveProjectAt_LocalLocation_WritesToTheGivenPath)
{
    //! [GIVEN] A local destination...
    const io::path_t path = "score.mscz";
    ON_CALL(*m_project, path()).WillByDefault(Return(path));

    //! [THEN] The score is written exactly there, and that file is remembered
    EXPECT_CALL(*m_project, save(path, SaveMode::Save, true)).Times(1);
    EXPECT_CALL(*m_recentFiles, prependRecentFile(RecentFile(path))).Times(1);

    //! [WHEN] Saving to it...
    Ret ret = saveProjectAt(SaveLocation(path));

    EXPECT_TRUE(ret);
}

TEST_F(SaveProjectScenarioTests, SaveProjectAt_WriteFails_DoesNotRememberTheFile)
{
    //! [GIVEN] A score whose write will fail...
    ON_CALL(*m_project, save(_, _, _)).WillByDefault(Return(make_ret(Ret::Code::InternalError)));

    //! [THEN] A file that was never written must not enter the recent files list
    EXPECT_CALL(*m_recentFiles, prependRecentFile(_)).Times(0);

    //! [WHEN] Saving it...
    Ret ret = saveProjectAt(SaveLocation(io::path_t("score.mscz")));

    EXPECT_FALSE(ret);
}

TEST_F(SaveProjectScenarioTests, SaveProjectAt_ScoreCannotBeSaved_DoesNotWrite)
{
    //! [GIVEN] A score that reports it cannot be saved...
    ON_CALL(*m_project, canSave()).WillByDefault(Return(make_ret(Err::NoPartsError)));

    //! [THEN] Nothing is written
    EXPECT_CALL(*m_project, save(_, _, _)).Times(0);

    //! [WHEN] Saving it...
    Ret ret = saveProjectAt(SaveLocation(io::path_t("score.mscz")));

    EXPECT_FALSE(ret);
}

TEST_F(SaveProjectScenarioTests, SaveProjectAt_Forced_WritesWithoutCheckingCanSave)
{
    //! [GIVEN] A score that reports it cannot be saved...
    ON_CALL(*m_project, canSave()).WillByDefault(Return(make_ret(Err::CorruptionError)));

    //! [THEN] Forcing the save bypasses the check and writes anyway
    EXPECT_CALL(*m_project, save(io::path_t("score.mscz"), SaveMode::Save, true)).Times(1);

    //! [WHEN] Saving it with force...
    saveProjectAt(SaveLocation(io::path_t("score.mscz")), SaveMode::Save, true /*force*/);
}

TEST_F(SaveProjectScenarioTests, SaveProjectAt_UndefinedLocation_Fails)
{
    //! [GIVEN] A location that is neither local nor cloud...
    //! [THEN] Nothing is written
    EXPECT_CALL(*m_project, save(_, _, _)).Times(0);

    //! [WHEN] Saving to it...
    Ret ret = saveProjectAt(SaveLocation());

    EXPECT_FALSE(ret);
}

TEST_F(SaveProjectScenarioTests, SaveProjectAt_FromCommandParams_UsesTheGivenPath)
{
    //! [GIVEN] A "save at" command carrying a path...
    rcommand::Params params;
    params["path"] = Val("from-command.mscz");

    //! [THEN] The score is written there
    EXPECT_CALL(*m_project, save(io::path_t("from-command.mscz"), SaveMode::Save, true)).Times(1);

    //! [WHEN] Handling the command...
    Ret ret = saveProjectAt(params);

    EXPECT_TRUE(ret);
}

TEST_F(SaveProjectScenarioTests, SaveProjectAt_FromCommandParams_EmptyPathIsRejected)
{
    //! [GIVEN] A "save at" command with no path...
    rcommand::Params params;
    params["path"] = Val("");

    //! [THEN] Nothing is written
    EXPECT_CALL(*m_project, save(_, _, _)).Times(0);

    //! [WHEN] Handling the command...
    Ret ret = saveProjectAt(params);

    //! [THEN] The command is rejected as malformed
    EXPECT_EQ(ret.code(), int(Ret::Code::BadArgs));
}

// ─── Saving a score that reports problems ────────────────────────────────────

TEST_F(SaveProjectScenarioTests, SaveProjectAt_CorruptedScoreAndUserAgrees_WritesAnyway)
{
    //! [GIVEN] An existing score that reports corruption...
    ON_CALL(*m_project, isNewlyCreated()).WillByDefault(Return(false));
    ON_CALL(*m_project, canSave()).WillByDefault(Return(make_ret(Err::CorruptionError)));

    //! [GIVEN] ...and a user who picks "Save anyway"
    ON_CALL(*m_interactive, errorSync(_, _, _, _, _, _))
    .WillByDefault(Return(IInteractive::Result(SAVE_ANYWAY_BTN_ID)));

    //! [THEN] The score is written despite the corruption
    EXPECT_CALL(*m_project, save(io::path_t("corrupted.mscz"), SaveMode::Save, true)).Times(1);

    //! [WHEN] Saving it locally...
    saveProjectAt(SaveLocation(io::path_t("corrupted.mscz")));
}

TEST_F(SaveProjectScenarioTests, SaveProjectAt_CorruptedOnOpeningAndCloudTarget_DoesNotWrite)
{
    //! [GIVEN] A score that arrived corrupted and is headed for the cloud...
    ON_CALL(*m_project, canSave()).WillByDefault(Return(make_ret(Err::CorruptionUponOpenningError)));

    //! [THEN] A corrupted score is never uploaded, and nothing is written
    EXPECT_CALL(*m_project, save(_, _, _)).Times(0);

    //! [WHEN] Saving it to the cloud...
    Ret ret = saveProjectAt(SaveLocation(CloudProjectInfo()));

    EXPECT_FALSE(ret);
}

// ─── The score became corrupted while being written ──────────────────────────

TEST_F(SaveProjectScenarioTests, SaveProjectLocally_CorruptedOnSaveAndRetry_RewritesWithoutABackup)
{
    //! [GIVEN] A write that corrupts the file, and a user who chooses "Try again"...
    const io::path_t path = "score.mscz";
    ON_CALL(*m_interactive, errorSync(_, _, _, _, _, _))
    .WillByDefault(Return(IInteractive::Result(RETRY_SAVE_BTN_ID)));

    //! [THEN] The retry deliberately skips the backup: the target is already corrupted, and backing it
    //! up again would overwrite the healthy backup made on the first attempt
    EXPECT_CALL(*m_project, save(path, SaveMode::Save, true))
    .WillOnce(Return(make_ret(Err::CorruptionUponSavingError)));
    EXPECT_CALL(*m_project, save(path, SaveMode::Save, false))
    .WillOnce(Return(make_ok()));

    //! [WHEN] Saving it, then letting the deferred retry run...
    saveProjectAt(SaveLocation(path));
    drainDeferredCalls();
}

TEST_F(SaveProjectScenarioTests, SaveProjectLocally_CorruptedOnSaveAndSaveAs_AsksForANewLocation)
{
    //! [GIVEN] An existing score whose write corrupts the file...
    ON_CALL(*m_project, isNewlyCreated()).WillByDefault(Return(false));
    ON_CALL(*m_project, isCloudProject()).WillByDefault(Return(false));
    ON_CALL(*m_project, save(_, _, _)).WillByDefault(Return(make_ret(Err::CorruptionUponSavingError)));

    //! [GIVEN] ...and a user who chooses "Save as"
    ON_CALL(*m_interactive, errorSync(_, _, _, _, _, _))
    .WillByDefault(Return(IInteractive::Result(SAVE_AS_BTN_ID)));

    //! [THEN] A fresh destination is asked for, so the healthy version can go somewhere else
    givenUserCancelsTheSaveDialog();
    EXPECT_CALL(*m_interactive, selectSavingFileSync(_, _, _, _)).Times(1);

    //! [WHEN] Saving it, then letting the deferred call run...
    saveProjectAt(SaveLocation(io::path_t("score.mscz")));
    drainDeferredCalls();
}

TEST_F(SaveProjectScenarioTests, SaveProjectLocally_CorruptedOnSaveAndCancel_DoesNotRetry)
{
    //! [GIVEN] A write that corrupts the file, and a user who cancels...
    const io::path_t path = "score.mscz";
    ON_CALL(*m_interactive, errorSync(_, _, _, _, _, _))
    .WillByDefault(Return(IInteractive::Result(int(IInteractive::Button::Cancel))));

    //! [THEN] The write is attempted exactly once, and nothing is retried
    EXPECT_CALL(*m_project, save(path, SaveMode::Save, true))
    .Times(1)
    .WillOnce(Return(make_ret(Err::CorruptionUponSavingError)));
    EXPECT_CALL(*m_interactive, selectSavingFileSync(_, _, _, _)).Times(0);

    //! [WHEN] Saving it, then letting any deferred call run...
    Ret ret = saveProjectAt(SaveLocation(path));
    drainDeferredCalls();

    EXPECT_FALSE(ret);
}

TEST_F(SaveProjectScenarioTests, SaveProjectLocally_OrdinaryFailure_WarnsTheUser)
{
    //! [GIVEN] A write that fails for a reason other than corruption...
    ON_CALL(*m_project, save(_, _, _)).WillByDefault(Return(make_ret(Ret::Code::InternalError)));

    //! [THEN] The user is warned, and the corruption dialog is not used
    EXPECT_CALL(*m_interactive, warning(_, _, _, _, _, _)).Times(1);
    EXPECT_CALL(*m_interactive, errorSync(_, _, _, _, _, _)).Times(0);

    //! [WHEN] Saving it...
    saveProjectAt(SaveLocation(io::path_t("score.mscz")));
}

// ─── Saving to the cloud ─────────────────────────────────────────────────────

TEST_F(SaveProjectScenarioTests, SaveProjectToCloud_CloudUnreachable_SavesLocallyAndReportsSuccess)
{
    //! [GIVEN] An unreachable cloud...
    ON_CALL(*m_authorization, checkCloudIsAvailable()).WillByDefault(Return(make_ret(Ret::Code::InternalError)));
    ON_CALL(*m_configuration, showCloudIsNotAvailableWarning()).WillByDefault(Return(false));
    ON_CALL(*m_configuration, cloudProjectSavingPath(0)).WillByDefault(Return(io::path_t("cloud.mscz")));

    //! [THEN] The score is written to disk and nothing is uploaded
    EXPECT_CALL(*m_project, save(io::path_t("cloud.mscz"), SaveMode::Save, true)).Times(1);
    EXPECT_CALL(*m_museScoreComService, uploadScore(_, _, _, _, _)).Times(0);

    //! [WHEN] Saving it to the cloud...
    bool ok = saveProjectToCloud(CloudProjectInfo());

    //! [THEN] This counts as success: the work is safe on disk and will sync once the connection returns
    EXPECT_TRUE(ok);
}

TEST_F(SaveProjectScenarioTests, SaveProjectToCloud_AlreadyACloudProject_WritesToItsOwnFile)
{
    //! [GIVEN] A score that already lives in the cloud and has a local file of its own...
    ON_CALL(*m_project, isCloudProject()).WillByDefault(Return(true));
    ON_CALL(*m_project, path()).WillByDefault(Return(io::path_t("project.mscz")));
    ON_CALL(*m_authorization, checkCloudIsAvailable()).WillByDefault(Return(make_ret(Ret::Code::InternalError)));
    ON_CALL(*m_configuration, showCloudIsNotAvailableWarning()).WillByDefault(Return(false));

    //! [THEN] It is written back over that same file
    EXPECT_CALL(*m_project, save(io::path_t("project.mscz"), SaveMode::Save, true)).Times(1);

    //! [WHEN] Saving it to the cloud...
    saveProjectToCloud(CloudProjectInfo(), SaveMode::Save);
}

TEST_F(SaveProjectScenarioTests, SaveProjectToCloud_NotACloudProjectYet_WritesToTheCloudSavingPath)
{
    //! [GIVEN] A score that is going to the cloud for the first time...
    ON_CALL(*m_project, isCloudProject()).WillByDefault(Return(false));
    ON_CALL(*m_configuration, cloudProjectSavingPath(0)).WillByDefault(Return(io::path_t("cloud.mscz")));
    ON_CALL(*m_authorization, checkCloudIsAvailable()).WillByDefault(Return(make_ret(Ret::Code::InternalError)));
    ON_CALL(*m_configuration, showCloudIsNotAvailableWarning()).WillByDefault(Return(false));

    //! [THEN] It is written to the path reserved for cloud scores, not to wherever it was before
    EXPECT_CALL(*m_project, save(io::path_t("cloud.mscz"), SaveMode::Save, true)).Times(1);

    //! [WHEN] Saving it to the cloud...
    saveProjectToCloud(CloudProjectInfo(), SaveMode::Save);
}

TEST_F(SaveProjectScenarioTests, SaveProjectToCloud_UserChoosesToSaveLocallyInstead_WritesThereAndStops)
{
    //! [GIVEN] A reachable cloud whose login dialog is answered with "Save to computer"...
    ON_CALL(*m_authorization, checkCloudIsAvailable()).WillByDefault(Return(make_ok()));

    using Response = cloud::SaveToCloudResponse::SaveToCloudResponse;
    givenLoginDialogAnswers(RetVal<Val>::make_ok(Val(int(Response::SaveLocallyInstead))));

    givenUserPicksLocalFile(io::path_t("instead.mscz"));

    //! [THEN] The score goes to the chosen local file, the preference is remembered, and nothing is uploaded
    EXPECT_CALL(*m_project, save(io::path_t("instead.mscz"), SaveMode::Save, true)).Times(1);
    EXPECT_CALL(*m_configuration, setLastUsedSaveLocationType(SaveLocationType::Local)).Times(1);
    EXPECT_CALL(*m_museScoreComService, uploadScore(_, _, _, _, _)).Times(0);

    //! [WHEN] Saving it to the cloud...
    bool ok = saveProjectToCloud(CloudProjectInfo());

    //! [THEN] The cloud save itself did not happen
    EXPECT_FALSE(ok);
}

TEST_F(SaveProjectScenarioTests, SaveProjectToCloud_LocalPathCancelled_WritesNothing)
{
    //! [GIVEN] A user who picks "Save to computer" and then cancels the file dialog...
    ON_CALL(*m_authorization, checkCloudIsAvailable()).WillByDefault(Return(make_ok()));

    using Response = cloud::SaveToCloudResponse::SaveToCloudResponse;
    givenLoginDialogAnswers(RetVal<Val>::make_ok(Val(int(Response::SaveLocallyInstead))));

    givenUserCancelsTheSaveDialog();

    //! [THEN] Nothing is written anywhere
    EXPECT_CALL(*m_project, save(_, _, _)).Times(0);

    //! [WHEN] Saving it to the cloud...
    bool ok = saveProjectToCloud(CloudProjectInfo());

    EXPECT_FALSE(ok);
}

TEST_F(SaveProjectScenarioTests, SaveProjectToCloud_NotLoggedIn_WritesNothing)
{
    //! [GIVEN] A reachable cloud the user refuses to log in to...
    ON_CALL(*m_authorization, checkCloudIsAvailable()).WillByDefault(Return(make_ok()));
    givenLoginDialogAnswers(RetVal<Val>(make_ret(Ret::Code::Cancel)));

    //! [THEN] Nothing is written and nothing is uploaded
    EXPECT_CALL(*m_project, save(_, _, _)).Times(0);
    EXPECT_CALL(*m_museScoreComService, uploadScore(_, _, _, _, _)).Times(0);

    //! [WHEN] Saving it to the cloud...
    bool ok = saveProjectToCloud(CloudProjectInfo());

    EXPECT_FALSE(ok);
}

TEST_F(SaveProjectScenarioTests, SaveProjectAt_TextBeingEdited_CommitsTheTextFirst)
{
    //! [GIVEN] A score with an unfinished text edit...
    ON_CALL(*m_interaction, isTextEditingStarted()).WillByDefault(Return(true));

    //! [THEN] The text is committed before anything is written, so the edit is not lost
    ::testing::Sequence seq;
    EXPECT_CALL(*m_interaction, endEditText()).InSequence(seq);
    EXPECT_CALL(*m_project, save(io::path_t("score.mscz"), SaveMode::Save, true)).InSequence(seq);

    //! [WHEN] Saving it...
    saveProjectAt(SaveLocation(io::path_t("score.mscz")));
}

TEST_F(SaveProjectScenarioTests, SaveProjectAt_NoTextBeingEdited_WritesStraightAway)
{
    //! [GIVEN] A score with no text edit in progress...
    ON_CALL(*m_interaction, isTextEditingStarted()).WillByDefault(Return(false));

    //! [THEN] Nothing is committed
    EXPECT_CALL(*m_interaction, endEditText()).Times(0);
    EXPECT_CALL(*m_project, save(io::path_t("score.mscz"), SaveMode::Save, true)).Times(1);

    //! [WHEN] Saving it...
    saveProjectAt(SaveLocation(io::path_t("score.mscz")));
}

// ─── Up-to-date cloud details before uploading ───────────────────────────────

TEST_F(SaveProjectScenarioTests, SaveProjectToCloud_ScoreWentPublicOnTheWeb_AsksBeforeOverwriting)
{
    //! [GIVEN] A cloud score that has meanwhile been made public on the site...
    givenReachableCloud();

    const QUrl sourceUrl("score-42");

    cloud::ScoreInfo remote;
    remote.title = "Renamed on the web";
    remote.visibility = cloud::Visibility::Public;
    ON_CALL(*m_museScoreComService, downloadScoreInfo(sourceUrl))
    .WillByDefault(Return(RetVal<cloud::ScoreInfo>::make_ok(remote)));

    //! [GIVEN] ...and a user who declines the warning
    ON_CALL(*m_interactive, warningSync(_, _, _, _, _, _))
    .WillByDefault(Return(IInteractive::Result(int(IInteractive::Button::Cancel))));

    //! [THEN] Nothing is written and nothing is uploaded
    EXPECT_CALL(*m_project, save(_, _, _)).Times(0);
    EXPECT_CALL(*m_museScoreComService, uploadScore(_, _, _, _, _)).Times(0);

    //! [WHEN] Saving it to the cloud...
    CloudProjectInfo info;
    info.sourceUrl = sourceUrl;
    EXPECT_FALSE(saveProjectToCloud(info, SaveMode::Save));
}

TEST_F(SaveProjectScenarioTests, SaveProjectToCloud_RemoteInfoUnavailable_KeepsTheLastKnownDetails)
{
    //! [GIVEN] A cloud score whose remote details cannot be fetched...
    const QUrl sourceUrl("score-42");

    givenReachableCloud();
    ON_CALL(*m_museScoreComService, downloadScoreInfo(sourceUrl))
    .WillByDefault(Return(RetVal<cloud::ScoreInfo>(make_ret(Ret::Code::InternalError))));

    CloudProjectInfo info;
    info.name = "Locally known name";
    info.visibility = cloud::Visibility::Private;
    info.sourceUrl = sourceUrl;

    //! [THEN] The save carries on with the name and visibility we already had
    EXPECT_CALL(*m_project, setCloudInfo(::testing::AllOf(
                                             ::testing::Field(&CloudProjectInfo::name, QString("Locally known name")),
                                             ::testing::Field(&CloudProjectInfo::visibility, cloud::Visibility::Private)))).Times(1);

    //! [WHEN] Saving it to the cloud...
    saveProjectToCloud(info, SaveMode::Save);
}

// ─── Uploading ───────────────────────────────────────────────────────────────

TEST_F(SaveProjectScenarioTests, SaveProjectToCloud_UploadSucceeds_CountsTheSaveAndStoresTheNewSource)
{
    //! [GIVEN] A reachable cloud that accepts the upload...
    givenReachableCloud();
    ON_CALL(*m_project, writeToDevice(_)).WillByDefault(Return(make_ok()));

    ValMap result;
    result["sourceUrl"] = Val("score-99");
    result["editUrl"] = Val("score-99-edit");
    result["revisionId"] = Val(7);

    givenUploadFinishesWith(make_ok(), result);

    std::vector<CloudProjectInfo> stored;
    ON_CALL(*m_project, setCloudInfo(_)).WillByDefault([&stored](const CloudProjectInfo& i) {
        stored.push_back(i);
    });

    //! [THEN] The score is uploaded once, under the name and visibility it carries
    EXPECT_CALL(*m_museScoreComService,
                uploadScore(_, QString("Symphony"), cloud::Visibility::Private, QUrl("score-99"), 0)).Times(1);

    //! [WHEN] Saving it to the cloud...
    CloudProjectInfo info;
    info.name = "Symphony";
    info.sourceUrl = QUrl("score-99");
    saveProjectToCloud(info, SaveMode::SaveAs);

    //! [THEN] The revision the server came back with is stored on the project
    ASSERT_FALSE(stored.empty());
    EXPECT_EQ(stored.back().revisionId, 7);

    //! [THEN] The save counter advanced, which is what drives "generate audio every Nth save"
    ON_CALL(*m_configuration, hasAskedAudioGenerationSettings()).WillByDefault(Return(true));
    ON_CALL(*m_configuration, generateAudioTimePeriodType())
    .WillByDefault(Return(GenerateAudioTimePeriodType::AfterCertainNumberOfSaves));
    ON_CALL(*m_configuration, numberOfSavesToGenerateAudio()).WillByDefault(Return(5));
    EXPECT_FALSE(needGenerateAudio(false).val);
}

TEST_F(SaveProjectScenarioTests, SaveProjectToCloud_UploadFails_ReportsTheError)
{
    //! [GIVEN] A reachable cloud whose upload fails...
    givenReachableCloud();
    ON_CALL(*m_project, writeToDevice(_)).WillByDefault(Return(make_ok()));
    givenUploadFinishesWith(make_ret(Ret::Code::InternalError), ValMap());

    //! [THEN] The failure is shown through the cloud save error dialog
    EXPECT_CALL(*m_interactive, warningSync(_, _, _, _, _, _))
    .WillOnce(Return(IInteractive::Result(int(IInteractive::Button::Cancel))));

    //! [WHEN] Saving it to the cloud...
    CloudProjectInfo info;
    info.sourceUrl = QUrl("score-99");
    EXPECT_FALSE(saveProjectToCloud(info, SaveMode::SaveAs));
}

TEST_F(SaveProjectScenarioTests, SaveProjectToCloud_ProjectCannotBeSerialised_DoesNotUpload)
{
    //! [GIVEN] A score that cannot be written into the upload buffer...
    givenReachableCloud();
    ON_CALL(*m_project, writeToDevice(_)).WillByDefault(Return(make_ret(Ret::Code::InternalError)));

    //! [THEN] Nothing is uploaded
    EXPECT_CALL(*m_museScoreComService, uploadScore(_, _, _, _, _)).Times(0);

    //! [WHEN] Saving it to the cloud...
    CloudProjectInfo info;
    info.sourceUrl = QUrl("score-99");
    EXPECT_FALSE(saveProjectToCloud(info, SaveMode::SaveAs));
}

TEST_F(SaveProjectScenarioTests, SaveProjectToCloud_AudioCannotBeRendered_DoesNotUpload)
{
    //! [GIVEN] A public score, which always needs an mp3, whose export fails...
    givenReachableCloud();
    givenUserConfirmsCloudDialog();
    ON_CALL(*m_exportScenario, exportScores(_, _, _, _)).WillByDefault(Return(false));

    //! [THEN] Nothing is uploaded, because a public score without audio has no web playback
    EXPECT_CALL(*m_museScoreComService, uploadScore(_, _, _, _, _)).Times(0);

    //! [WHEN] Saving it to the cloud...
    CloudProjectInfo info;
    info.visibility = cloud::Visibility::Public;
    info.sourceUrl = QUrl("score-99");
    EXPECT_FALSE(saveProjectToCloud(info, SaveMode::SaveAs));
}

TEST_F(SaveProjectScenarioTests, SaveProjectToCloud_AlreadyUploading_DoesNotStartASecondUpload)
{
    //! [GIVEN] An upload already in flight, re-entered from within its own event loop...
    givenReachableCloud();
    ON_CALL(*m_project, writeToDevice(_)).WillByDefault(Return(make_ok()));

    bool reentered = false;
    bool nested = false;
    givenUploadFinishesWith(make_ok(), ValMap(), [this, &reentered, &nested]() {
        reentered = true;
        nested = saveProjectToCloud(CloudProjectInfo(), SaveMode::SaveAs);
    });

    //! [THEN] Only one upload is started
    EXPECT_CALL(*m_museScoreComService, uploadScore(_, _, _, _, _)).Times(1);

    //! [WHEN] Saving it to the cloud...
    CloudProjectInfo info;
    info.sourceUrl = QUrl("score-99");
    saveProjectToCloud(info, SaveMode::SaveAs);

    //! [THEN] The re-entrant call really happened...
    ASSERT_TRUE(reentered);

    //! [THEN] ...and reported success without doing anything, so the caller does not retry
    EXPECT_TRUE(nested);
}

TEST_F(SaveProjectScenarioTests, SaveProjectToCloud_ConflictRetry_LeavesTheFirstUploadDisconnected)
{
    //! [GIVEN] A reachable cloud where the first upload hits a conflict and the retry goes through...
    givenReachableCloud();
    ON_CALL(*m_project, writeToDevice(_)).WillByDefault(Return(make_ok()));

    const ProgressPtr firstUpload = std::make_shared<Progress>();
    const ProgressPtr retriedUpload = std::make_shared<Progress>();
    const auto started = std::make_shared<int>(0);

    ON_CALL(*m_museScoreComService, uploadScore(_, _, _, _, _))
    .WillByDefault([firstUpload, retriedUpload, started](DevicePtr, const QString&, cloud::Visibility, const QUrl&, int) {
        const bool isFirst = (*started)++ == 0;
        const ProgressPtr progress = isFirst ? firstUpload : retriedUpload;
        const Ret ret = isFirst ? make_ret(cloud::Err::Status409_Conflict) : make_ok();

        QTimer::singleShot(0, [progress, ret]() {
            ProgressResult res;
            res.ret = ret;
            res.val = Val(ValMap());
            progress->finish(res);
        });

        return progress;
    });

    //! [GIVEN] ...because the user answers the conflict dialog with "Publish as new score"
    static constexpr int PUBLISH_AS_NEW_SCORE_BTN_ID = int(IInteractive::Button::CustomButton) + 4;
    ON_CALL(*m_interactive, warningSync(_, _, _, _, _, _))
    .WillByDefault(Return(IInteractive::Result(PUBLISH_AS_NEW_SCORE_BTN_ID)));

    //! [THEN] Both uploads are started, and the conflict is reported exactly once
    EXPECT_CALL(*m_museScoreComService, uploadScore(_, _, _, _, _)).Times(2);
    EXPECT_CALL(*m_interactive, warningSync(_, _, _, _, _, _)).Times(1);

    //! [WHEN] Saving it to the cloud...
    CloudProjectInfo info;
    info.sourceUrl = QUrl("score-99");
    saveProjectToCloud(info, SaveMode::SaveAs);

    //! [WHEN] ...and the upload that was retried reports again afterwards
    ProgressResult stale;
    stale.ret = make_ret(cloud::Err::Status409_Conflict);
    firstUpload->finish(stale);

    //! [THEN] Nothing listens to it any more: the retry replaced it, and its handler wrote to a
    //! stack frame that is now gone
}

// ─── Whether an mp3 is generated for the cloud ───────────────────────────────

TEST_F(SaveProjectScenarioTests, NeedGenerateAudio_PublicUpload_AlwaysGenerates)
{
    //! [GIVEN] Audio generation switched off entirely...
    ON_CALL(*m_configuration, hasAskedAudioGenerationSettings()).WillByDefault(Return(true));
    ON_CALL(*m_configuration, generateAudioTimePeriodType()).WillByDefault(Return(GenerateAudioTimePeriodType::Never));

    //! [WHEN] Uploading publicly...
    RetVal<bool> need = needGenerateAudio(true);

    //! [THEN] Audio is generated regardless, because public scores need web playback
    EXPECT_TRUE(need.ret);
    EXPECT_TRUE(need.val);
}

TEST_F(SaveProjectScenarioTests, NeedGenerateAudio_PrivateUploadAndNever_DoesNotGenerate)
{
    //! [GIVEN] Audio generation switched off...
    ON_CALL(*m_configuration, hasAskedAudioGenerationSettings()).WillByDefault(Return(true));
    ON_CALL(*m_configuration, generateAudioTimePeriodType()).WillByDefault(Return(GenerateAudioTimePeriodType::Never));

    //! [WHEN] Uploading privately...
    RetVal<bool> need = needGenerateAudio(false);

    //! [THEN] No audio is generated
    EXPECT_TRUE(need.ret);
    EXPECT_FALSE(need.val);
}

TEST_F(SaveProjectScenarioTests, NeedGenerateAudio_PrivateUploadAndAlways_Generates)
{
    //! [GIVEN] Audio generation switched on for every save...
    ON_CALL(*m_configuration, hasAskedAudioGenerationSettings()).WillByDefault(Return(true));
    ON_CALL(*m_configuration, generateAudioTimePeriodType()).WillByDefault(Return(GenerateAudioTimePeriodType::Always));

    //! [WHEN] Uploading privately...
    RetVal<bool> need = needGenerateAudio(false);

    //! [THEN] Audio is generated
    EXPECT_TRUE(need.ret);
    EXPECT_TRUE(need.val);
}

TEST_F(SaveProjectScenarioTests, NeedGenerateAudio_EveryNthSave_GeneratesOnTheFirstSave)
{
    //! [GIVEN] Audio generation set to happen every fifth save...
    ON_CALL(*m_configuration, hasAskedAudioGenerationSettings()).WillByDefault(Return(true));
    ON_CALL(*m_configuration, generateAudioTimePeriodType())
    .WillByDefault(Return(GenerateAudioTimePeriodType::AfterCertainNumberOfSaves));
    ON_CALL(*m_configuration, numberOfSavesToGenerateAudio()).WillByDefault(Return(5));

    //! [WHEN] Uploading privately with no saves counted yet...
    RetVal<bool> need = needGenerateAudio(false);

    //! [THEN] Audio is generated, because the counter starts at zero and zero is a multiple of five
    EXPECT_TRUE(need.ret);
    EXPECT_TRUE(need.val);
}

TEST_F(SaveProjectScenarioTests, NeedGenerateAudio_SettingsNeverShown_AsksBeforeDeciding)
{
    //! [GIVEN] The audio generation settings have never been shown...
    ON_CALL(*m_configuration, hasAskedAudioGenerationSettings()).WillByDefault(Return(false));
    ON_CALL(*m_configuration, generateAudioTimePeriodType()).WillByDefault(Return(GenerateAudioTimePeriodType::Never));

    //! [THEN] The settings dialog is opened before the decision is made
    EXPECT_CALL(*m_interactive, openSync(IsAudioGenerationSettingsDialog())).Times(1);

    //! [WHEN] Deciding whether to generate audio for a private upload...
    needGenerateAudio(false);
}

// ─── Giving up on a corrupted score ──────────────────────────────────────────

TEST_F(SaveProjectScenarioTests, SaveProjectAt_CorruptedScoreAndUserReverts_ReopensTheLastSavedVersion)
{
    //! [GIVEN] An already saved score that reports corruption, so reverting is possible...
    ON_CALL(*m_project, isNewlyCreated()).WillByDefault(Return(false));
    ON_CALL(*m_project, canSave()).WillByDefault(Return(make_ret(Err::CorruptionError)));

    //! [GIVEN] ...a user who picks "Revert to last saved" and confirms losing the changes
    ON_CALL(*m_interactive, errorSync(_, _, _, _, _, _))
    .WillByDefault(Return(IInteractive::Result(REVERT_TO_LAST_SAVED_BTN_ID)));
    ON_CALL(*m_interactive, warning(_, _, _, _, _, _))
    .WillByDefault([] {
        return async::make_promise<IInteractive::Result>([](auto resolve, auto) {
            return resolve(IInteractive::Result(int(IInteractive::Button::Yes)));
        });
    });

    //! [THEN] The file is read again from disk, and the corrupted state is not written anywhere
    EXPECT_CALL(*m_openScenario, revertToLastSaved()).Times(1);
    EXPECT_CALL(*m_project, save(_, _, _)).Times(0);

    //! [WHEN] Saving it locally, then letting the confirmation resolve...
    saveProjectAt(SaveLocation(io::path_t("corrupted.mscz")));
    drainDeferredCalls();
}

TEST_F(SaveProjectScenarioTests, SaveProjectAt_RevertIsConfirmedWithNo_ChangesAreKept)
{
    //! [GIVEN] The same score, but a user who backs out of the confirmation...
    ON_CALL(*m_project, isNewlyCreated()).WillByDefault(Return(false));
    ON_CALL(*m_project, canSave()).WillByDefault(Return(make_ret(Err::CorruptionError)));
    ON_CALL(*m_interactive, errorSync(_, _, _, _, _, _))
    .WillByDefault(Return(IInteractive::Result(REVERT_TO_LAST_SAVED_BTN_ID)));
    ON_CALL(*m_interactive, warning(_, _, _, _, _, _))
    .WillByDefault([] {
        return async::make_promise<IInteractive::Result>([](auto resolve, auto) {
            return resolve(IInteractive::Result(int(IInteractive::Button::No)));
        });
    });

    //! [THEN] Nothing is reopened, so the unsaved work survives
    EXPECT_CALL(*m_openScenario, revertToLastSaved()).Times(0);

    //! [WHEN] Saving it locally, then letting the confirmation resolve...
    saveProjectAt(SaveLocation(io::path_t("corrupted.mscz")));
    drainDeferredCalls();
}
TEST_F(SaveProjectScenarioTests, SaveProjectToCloud_AlreadySignedIn_DoesNotAskToLogIn)
{
    //! [GIVEN] A reachable cloud and a signed-in account...
    givenReachableCloud();
    ON_CALL(*m_project, writeToDevice(_)).WillByDefault(Return(make_ok()));
    givenUploadFinishesWith(make_ok(), ValMap());

    //! [THEN] The login dialog is not shown, and the upload goes ahead
    EXPECT_CALL(*m_interactive, openSync(IsLoginDialog())).Times(0);
    EXPECT_CALL(*m_museScoreComService,
                uploadScore(_, QString(), cloud::Visibility::Private, QUrl("score-99"), 0)).Times(1);

    //! [WHEN] Saving to the cloud...
    CloudProjectInfo info;
    info.sourceUrl = QUrl("score-99");
    saveProjectToCloud(info, SaveMode::SaveAs);
}
// ─── Publishing ──────────────────────────────────────────────────────────────

TEST_F(SaveProjectScenarioTests, Publish_AudioCannotBeRendered_ReportsFailure)
{
    //! [GIVEN] A publish whose mp3 export fails...
    givenReachableCloud();
    givenUserConfirmsCloudDialog();

    //! [THEN] The export of the current score is attempted and fails, and nothing is uploaded
    EXPECT_CALL(*m_exportScenario, exportScores(notation::INotationPtrList { m_notation }, _,
                                                INotationWriter::UnitType::PER_PART, false)).WillOnce(Return(false));
    EXPECT_CALL(*m_museScoreComService, uploadScore(_, _, _, _, _)).Times(0);

    //! [WHEN] Publishing...
    Ret ret = m_scenario->publish();

    //! [THEN] A score without audio has no web playback, so this is reported as bad data
    EXPECT_EQ(ret.code(), int(Ret::Code::BadData));
}

TEST_F(SaveProjectScenarioTests, Publish_UploadFails_ReportsFailure)
{
    //! [GIVEN] A publish whose upload fails...
    givenReachableCloud();
    givenUserConfirmsCloudDialog();
    ON_CALL(*m_exportScenario, exportScores(_, _, _, _)).WillByDefault(Return(true));
    ON_CALL(*m_project, writeToDevice(_)).WillByDefault(Return(make_ok()));
    givenUploadFinishesWith(make_ret(Ret::Code::InternalError), ValMap());

    //! [WHEN] Publishing...
    Ret ret = m_scenario->publish();

    //! [THEN] The failure reaches the caller instead of being swallowed
    EXPECT_FALSE(ret);
}

TEST_F(SaveProjectScenarioTests, Publish_EverythingSucceeds_ReportsSuccess)
{
    //! [GIVEN] A publish that goes through...
    givenReachableCloud();
    givenUserConfirmsCloudDialog("Published score");
    ON_CALL(*m_exportScenario, exportScores(_, _, _, _)).WillByDefault(Return(true));
    ON_CALL(*m_project, writeToDevice(_)).WillByDefault(Return(make_ok()));
    givenUploadFinishesWith(make_ok(), ValMap());

    //! [THEN] The score is uploaded as a new one, under the name given in the dialog
    EXPECT_CALL(*m_museScoreComService,
                uploadScore(_, QString("Published score"), cloud::Visibility::Private, QUrl(), 0)).Times(1);

    //! [WHEN] Publishing...
    Ret ret = m_scenario->publish();

    EXPECT_TRUE(ret);
}

TEST_F(SaveProjectScenarioTests, Publish_UserChoosesToSaveLocallyInstead_WritesThereAndStops)
{
    //! [GIVEN] A reachable cloud whose login dialog is answered with "Save to computer"...
    ON_CALL(*m_authorization, checkCloudIsAvailable()).WillByDefault(Return(make_ok()));

    using Response = cloud::SaveToCloudResponse::SaveToCloudResponse;
    givenLoginDialogAnswers(RetVal<Val>::make_ok(Val(int(Response::SaveLocallyInstead))));

    givenUserPicksLocalFile(io::path_t("instead.mscz"));

    //! [THEN] The score goes to the chosen local file, the preference is remembered, and nothing is published
    EXPECT_CALL(*m_project, save(io::path_t("instead.mscz"), SaveMode::Save, true)).Times(1);
    EXPECT_CALL(*m_configuration, setLastUsedSaveLocationType(SaveLocationType::Local)).Times(1);
    EXPECT_CALL(*m_museScoreComService, uploadScore(_, _, _, _, _)).Times(0);

    //! [WHEN] Publishing...
    Ret ret = m_scenario->publish();

    EXPECT_TRUE(ret);
}

TEST_F(SaveProjectScenarioTests, Publish_LocalPathCancelled_WritesNothing)
{
    //! [GIVEN] A user who picks "Save to computer" and then cancels the file dialog...
    ON_CALL(*m_authorization, checkCloudIsAvailable()).WillByDefault(Return(make_ok()));

    using Response = cloud::SaveToCloudResponse::SaveToCloudResponse;
    givenLoginDialogAnswers(RetVal<Val>::make_ok(Val(int(Response::SaveLocallyInstead))));

    givenUserCancelsTheSaveDialog();

    //! [THEN] Nothing is written anywhere and nothing is published
    EXPECT_CALL(*m_project, save(_, _, _)).Times(0);
    EXPECT_CALL(*m_museScoreComService, uploadScore(_, _, _, _, _)).Times(0);

    //! [WHEN] Publishing...
    Ret ret = m_scenario->publish();

    EXPECT_FALSE(ret);
}

TEST_F(SaveProjectScenarioTests, NeedGenerateAudio_BrokenSaveInterval_DoesNotDivideByZero)
{
    //! [GIVEN] A settings file whose "every N saves" interval is zero...
    ON_CALL(*m_configuration, hasAskedAudioGenerationSettings()).WillByDefault(Return(true));
    ON_CALL(*m_configuration, generateAudioTimePeriodType())
    .WillByDefault(Return(GenerateAudioTimePeriodType::AfterCertainNumberOfSaves));
    ON_CALL(*m_configuration, numberOfSavesToGenerateAudio()).WillByDefault(Return(0));

    //! [WHEN] Deciding whether to generate audio...
    RetVal<bool> need = needGenerateAudio(false);

    //! [THEN] The decision is made without dividing by zero, and the save is not blocked
    EXPECT_TRUE(need.ret);
    EXPECT_TRUE(need.val);
}
}
