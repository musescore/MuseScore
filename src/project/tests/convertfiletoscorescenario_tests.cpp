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

#include <QUrl>

#include "modularity/ioc.h"
#include "global/translation.h"
#include "global/dataformatter.h"
#include "global/io/ioretcodes.h"

#include "project/internal/convertfiletoscorescenario.h"
#include "project/projecterrors.h"
#include "project/projectcommands.h"

#include "utils/testutils.h"
#include "mocks/convertfiletoscoreservicemock.h"
#include "mocks/projectconfigurationmock.h"
#include "interactive/tests/mocks/interactivemock.h"
#include "rcommand/commandtypes.h"
#include "rcommand/tests/mocks/commanddispatchermock.h"
#include "cloud/tests/mocks/musescorecomservicemock.h"
#include "cloud/tests/mocks/authorizationservicemock.h"
#include "toast/tests/mocks/toastservicemock.h"
#include "multiwindows/tests/mocks/multiwindowsprovidermock.h"

namespace muse {
// Teach GoogleTest how to print UriQuery so failure diffs are readable
// instead of a raw byte dump
inline void PrintTo(const UriQuery& q, std::ostream* os)
{
    *os << q.toString();
}
}

using namespace ::testing;
using namespace mu::project;
using namespace muse;
using namespace muse::cloud;

namespace {
async::Promise<Val> resolvedValPromise(const Val& val = Val())
{
    return resolvedPromise<Val>(val);
}

//! NOTE: for dialogs whose resolution isn't relevant to the test -- resolving with an empty
//! Val would otherwise cascade into unrelated (and unmocked) downstream calls
async::Promise<Val> pendingValPromise()
{
    return pendingPromise<Val>();
}

async::Promise<Ret> resolvedRetPromise(const Ret& ret)
{
    return resolvedPromise<Ret>(ret);
}

async::Promise<IInteractive::Result> resolvedResultPromise(const IInteractive::Result& result = IInteractive::Result())
{
    return resolvedPromise<IInteractive::Result>(result);
}

async::Promise<toast::ToastResult> resolvedToastResultPromise(const toast::ToastResult& result = toast::ToastResult())
{
    return resolvedPromise<toast::ToastResult>(result);
}

RetVal<Progress> uploadStarted(const Progress& progress = Progress())
{
    return RetVal<Progress>::make_ok(progress);
}

RetVal<Progress> uploadNotStarted(const Ret& ret)
{
    return RetVal<Progress>::make_ret(ret);
}

Matcher<const IInteractive::Text&> TextIs(const std::string& text)
{
    return Field(&IInteractive::Text::text, text);
}

//! NOTE: ButtonData and ToastAction have no operator==, so match lists by one int field, in order
template<typename T>
Matcher<const std::vector<T>&> FieldsAre(int T::* field, const std::vector<int>& expected)
{
    std::vector<Matcher<const T&> > matchers;
    for (int value : expected) {
        matchers.push_back(Field(field, value));
    }
    return ElementsAreArray(matchers);
}

Matcher<const IInteractive::ButtonDatas&> ButtonIdsAre(const std::vector<int>& expectedIds)
{
    return FieldsAre(&IInteractive::ButtonData::btn, expectedIds);
}

Matcher<const std::vector<toast::ToastAction>&> ToastActionCodesAre(const std::vector<int>& expectedCodes)
{
    return FieldsAre(&toast::ToastAction::code, expectedCodes);
}

//! NOTE: shared by the ConvertFiles_* tests, which resolve the picker with a single OMR file
QVariantMap pickedOmrFileSelection()
{
    return {
        { "type", int(ConvertType::Omr) },
        { "paths", QStringList { "/some/file.xyz" } },
        { "convertedScoreName", QString("file") }
    };
}
}

namespace mu::project {
class Project_ConvertFileToScoreScenarioTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_scenario = std::make_shared<ConvertFileToScoreScenario>(muse::modularity::globalCtx());

        m_service = std::make_shared<NiceMock<ConvertFileToScoreServiceMock> >();
        m_interactive = std::make_shared<NiceMock<InteractiveMock> >();
        m_commandDispatcher = std::make_shared<NiceMock<muse::rcommand::CommandDispatcherMock> >();
        m_configuration = std::make_shared<NiceMock<ProjectConfigurationMock> >();
        m_museScoreComService = std::make_shared<NiceMock<MuseScoreComServiceMock> >();
        m_authorization = std::make_shared<NiceMock<AuthorizationServiceMock> >();
        m_toastService = std::make_shared<NiceMock<toast::ToastServiceMock> >();
        m_multiwindowsProvider = std::make_shared<NiceMock<muse::mi::MultiWindowsProviderMock> >();

        m_scenario->service.set(m_service);
        m_scenario->interactive.set(m_interactive);
        m_scenario->toastService.set(m_toastService);
        m_scenario->commandDispatcher.set(m_commandDispatcher);
        m_scenario->configuration.set(m_configuration);
        m_scenario->museScoreComService.set(m_museScoreComService);
        m_scenario->multiwindowsProvider.set(m_multiwindowsProvider);

        ON_CALL(*m_museScoreComService, authorization())
        .WillByDefault(Return(m_authorization));

        ON_CALL(*m_service, config())
        .WillByDefault(ReturnRef(m_config));
        ON_CALL(*m_service, convertFinished())
        .WillByDefault(Return(m_convertFinished));
        ON_CALL(*m_service, pollingStatusChanged())
        .WillByDefault(Return(m_pollingStatusChanged));

        ON_CALL(*m_interactive, buttonData(_))
        .WillByDefault(Invoke([](IInteractive::Button b) {
            return IInteractive::ButtonData(b, "");
        }));

        ON_CALL(*m_authorization, userAuthorized())
        .WillByDefault(Return(ValCh<bool> { true, {} }));

        ON_CALL(*m_authorization, checkCloudIsAvailable())
        .WillByDefault(Invoke([] {
            return resolvedRetPromise(make_ok());
        }));
    }

    //! NOTE: validateFiles' error dialogs all use a single Ok button, no default button and the WithIcon option
    void expectValidateFilesShowsError(Err err, const std::string& expectedTitle, const std::string& expectedText)
    {
        const io::paths_t paths { "/some/file.xyz" };

        // [GIVEN] The service reports a validation failure with the given error code
        ON_CALL(*m_service, validateFiles(paths))
        .WillByDefault(Return(RetVal<ConvertFilesValidation>::make_ret(make_ret(err))));

        // [THEN] The scenario shows the matching error dialog
        EXPECT_CALL(*m_interactive,
                    warning(expectedTitle, TextIs(expectedText), ButtonIdsAre({ int(IInteractive::Button::Ok) }),
                            int(IInteractive::Button::NoButton), IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
        .WillOnce(Invoke([](auto&&...) {
            return resolvedResultPromise();
        }));

        // [WHEN] Validating the files
        RetVal<ConvertFilesValidation> result = m_scenario->validateFiles(paths);

        // [THEN] Validation fails
        EXPECT_FALSE(result.ret);
    }

    void expectNoErrorDialog()
    {
        EXPECT_CALL(*m_interactive, warning(_, _, _, _, _, _)).Times(0);
        EXPECT_CALL(*m_interactive, error(_, _, _, _, _, _)).Times(0);
    }

    std::shared_ptr<ConvertFileToScoreScenario> m_scenario;
    std::shared_ptr<ConvertFileToScoreServiceMock> m_service;
    std::shared_ptr<InteractiveMock> m_interactive;
    std::shared_ptr<toast::ToastServiceMock> m_toastService;
    std::shared_ptr<muse::rcommand::CommandDispatcherMock> m_commandDispatcher;
    std::shared_ptr<ProjectConfigurationMock> m_configuration;
    std::shared_ptr<MuseScoreComServiceMock> m_museScoreComService;
    std::shared_ptr<AuthorizationServiceMock> m_authorization;
    std::shared_ptr<muse::mi::MultiWindowsProviderMock> m_multiwindowsProvider;

    ConvertConfig m_config;
    async::Channel<Ret, WatchedScore> m_convertFinished;
    async::Channel<PollingStatus> m_pollingStatusChanged;
};
}

// ==================================================
// init()
// ==================================================

TEST_F(Project_ConvertFileToScoreScenarioTest, Init_Success_ShowsScoreReadyNotificationAndForwards)
{
    // [GIVEN] The scenario is wired up to the service's channels
    m_scenario->init();

    WatchedScore watched;
    watched.scoreId = 555;
    watched.name = u"My Score";

    constexpr int openScoreBtn = int(toast::ToastActionCode::Custom) + 1;

    const std::string title = muse::trc("project/convert", "Your score is ready!");
    const std::string text = muse::qtrc("project/convert", "‘%1’ has finished processing and is ready to open.")
                             .arg("My Score").toStdString();

    // [THEN] The "score ready" notification is shown
    EXPECT_CALL(*m_toastService,
                show(title, text, muse::ui::IconCode::Code::TICK_FILLED, true,
                     ToastActionCodesAre({ int(toast::ToastActionCode::Dismiss), openScoreBtn })))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedToastResultPromise();
    }));

    bool forwarded = false;
    Ret forwardedRet;
    WatchedScore forwardedWatched;
    m_scenario->convertFinished().onReceive(nullptr, [&](const Ret& ret, const WatchedScore& w) {
        forwarded = true;
        forwardedRet = ret;
        forwardedWatched = w;
    });

    // [WHEN] The service reports a successful conversion
    m_convertFinished.send(make_ok(), watched);

    // [THEN] The result, including the WatchedScore payload, is forwarded to the scenario's own convertFinished channel
    EXPECT_TRUE(forwarded);
    EXPECT_TRUE(forwardedRet);
    ASSERT_TRUE(forwardedWatched.scoreId.has_value());
    EXPECT_EQ(*forwardedWatched.scoreId, 555);
    EXPECT_EQ(forwardedWatched.name, u"My Score");
}

TEST_F(Project_ConvertFileToScoreScenarioTest, Init_Success_OpenScoreButton_DispatchesOpenScoreUrl)
{
    // [GIVEN] The scenario is wired up to the service's channels
    m_scenario->init();

    WatchedScore watched;
    watched.scoreId = 555;
    watched.name = u"My Score";

    // [GIVEN] The user clicks "Open score" on the ready notification
    constexpr int openScoreBtn = int(toast::ToastActionCode::Custom) + 1;
    ON_CALL(*m_toastService, show(_, _, _, _, _))
    .WillByDefault(Invoke([](auto&&...) {
        return resolvedToastResultPromise(toast::ToastResult(openScoreBtn));
    }));

    // [THEN] The score is opened via the cloud open-score URL, not a local path
    EXPECT_CALL(*m_commandDispatcher, dispatch(Truly([](const rcommand::Request& request) {
        return request.command == PROJECT_OPEN_COMMAND
               && QUrl(request.params.at("url").toQString()) == QUrl("musescore://open-score/555");
    })))
    .Times(1)
    .WillOnce([](const rcommand::Request& request) {
        return async::make_promise<rcommand::Response>([request](auto resolve) {
            return resolve(rcommand::make_response(request, make_ok()));
        });
    });

    // [WHEN] The service reports a successful conversion
    m_convertFinished.send(make_ok(), watched);
    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, Init_Success_ScoreAlreadyOpen_DoesNotShowNotification)
{
    // [GIVEN] The scenario is wired up to the service's channels
    m_scenario->init();

    WatchedScore watched;
    watched.scoreId = 555;
    watched.name = u"My Score";

    // [GIVEN] The converted score's project is already open, in this or another window
    const io::path_t path = "/some/path/My Score.mscz";
    ON_CALL(*m_configuration, cloudProjectPath(555))
    .WillByDefault(Return(path));
    ON_CALL(*m_multiwindowsProvider, isProjectAlreadyOpened(path))
    .WillByDefault(Return(true));

    // [THEN] The "score ready" notification is not shown
    EXPECT_CALL(*m_toastService, show(_, _, _, _, _)).Times(0);

    bool forwarded = false;
    m_scenario->convertFinished().onReceive(nullptr, [&](const Ret&, const WatchedScore&) {
        forwarded = true;
    });

    // [WHEN] The service reports a successful conversion
    m_convertFinished.send(make_ok(), watched);

    // [THEN] The result is still forwarded to the scenario's own convertFinished channel
    EXPECT_TRUE(forwarded);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, Init_Failure_ShowsConvertFailedNotificationAndForwards)
{
    // [GIVEN] The scenario is wired up to the service's channels
    m_scenario->init();

    Ret ret = make_ret(Err::ConvertProcessingFailed);

    WatchedScore watched;
    watched.name = u"My Score";

    const std::string title = muse::trc("project/convert", "Error processing score");
    const std::string text = muse::qtrc("project/convert", "We weren’t able to convert ‘%1’. Please try again with a better quality file.")
                             .arg("My Score").toStdString();

    // [THEN] The "convert failed" notification is shown
    EXPECT_CALL(*m_toastService,
                show(title, text, muse::ui::IconCode::Code::ERROR_FILLED, true,
                     ToastActionCodesAre({ int(toast::ToastActionCode::TryAgain), int(toast::ToastActionCode::Dismiss) })))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedToastResultPromise();
    }));

    bool forwarded = false;
    Ret forwardedRet;
    m_scenario->convertFinished().onReceive(nullptr, [&](const Ret& r, const WatchedScore&) {
        forwarded = true;
        forwardedRet = r;
    });

    // [WHEN] The service reports a failed conversion
    m_convertFinished.send(ret, watched);

    // [THEN] The failure is still forwarded to the scenario's own convertFinished channel
    EXPECT_TRUE(forwarded);
    EXPECT_FALSE(forwardedRet);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, Init_Failure_TryAgain_RestartsConvert)
{
    // [GIVEN] The scenario is wired up to the service's channels
    m_scenario->init();

    // [GIVEN] The user clicks "Try another file" on the failure toast
    ON_CALL(*m_toastService, show(_, _, _, _, _))
    .WillByDefault(Invoke([](auto&&...) {
        return resolvedToastResultPromise(toast::ToastResult(int(toast::ToastActionCode::TryAgain)));
    }));

    // [GIVEN] The user re-selects a file to convert
    ON_CALL(*m_interactive, open(UriQuery("musescore://project/convert/selectfiles")))
    .WillByDefault(Invoke([](auto&&...) {
        return resolvedValPromise(Val::fromQVariant(pickedOmrFileSelection()));
    }));

    // [THEN] Conversion is restarted with the newly selected file
    EXPECT_CALL(*m_service, uploadToConvert(Truly([](const ConvertInput& input) {
        return convertTypeOf(input) == ConvertType::Omr && convertPathsOf(input) == io::paths_t { "/some/file.xyz" };
    }), muse::String(u"file")))
    .WillOnce(Return(uploadStarted()));

    // [WHEN] The service reports a failed conversion
    Ret ret = make_ret(Err::ConvertProcessingFailed);
    WatchedScore watched;
    watched.name = u"My Score";
    m_convertFinished.send(ret, watched);

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, Init_Failure_Ok_DoesNotRestartConvert)
{
    // [GIVEN] The scenario is wired up to the service's channels
    m_scenario->init();

    // [GIVEN] The user clicks "OK" on the failure toast
    ON_CALL(*m_toastService, show(_, _, _, _, _))
    .WillByDefault(Invoke([](auto&&...) {
        return resolvedToastResultPromise(toast::ToastResult(int(toast::ToastActionCode::Dismiss)));
    }));

    // [THEN] No new conversion is started
    EXPECT_CALL(*m_interactive, open(_)).Times(0);
    EXPECT_CALL(*m_service, uploadToConvert(_, _)).Times(0);

    // [WHEN] The service reports a failed conversion
    m_convertFinished.send(make_ret(Err::ConvertProcessingFailed), WatchedScore());

    pumpEvents();
}

// ==================================================
// init() -- pollingStatusChanged()
// ==================================================

TEST_F(Project_ConvertFileToScoreScenarioTest, Init_PollingStatusChanged_ShowsToastOnceAfterThreshold)
{
    // [GIVEN] The scenario is wired up to the service's channels
    m_scenario->init();

    const std::string title = muse::trc("project/convert", "We’re having trouble connecting to MuseScore.com.");
    const std::string text = muse::trc("project/convert", "We’ll keep trying intermittently.");

    // [THEN] The connectivity toast is shown exactly once
    EXPECT_CALL(*m_toastService, showWarning(title, text)).Times(1);

    // [WHEN] Polling fails below the elapsed threshold (5 minutes), then reaches and passes it
    m_pollingStatusChanged.send(PollingFailure { Ret(), secs_t(0.0), secs_t(0.0), false });
    m_pollingStatusChanged.send(PollingFailure { Ret(), secs_t(60.0), secs_t(0.0), false });
    m_pollingStatusChanged.send(PollingFailure { Ret(), secs_t(180.0), secs_t(0.0), false });
    m_pollingStatusChanged.send(PollingFailure { Ret(), secs_t(300.0), secs_t(0.0), false });
    m_pollingStatusChanged.send(PollingFailure { Ret(), secs_t(420.0), secs_t(0.0), false });
}

TEST_F(Project_ConvertFileToScoreScenarioTest, Init_PollingStatusChanged_GaveUp_ShowsConnectionLostToast)
{
    // [GIVEN] The scenario is wired up to the service's channels
    m_scenario->init();

    const std::string title = muse::trc("project/convert", "Unable to connect to MuseScore.com");
    const std::string text = muse::trc("project/convert",
                                       "An internet connection is required for file conversion. Please check your internet connection or try again later.");

    // [THEN] The "connection lost" toast is shown, with a Dismiss and an accented Retry action
    EXPECT_CALL(*m_toastService,
                show(title, text, muse::ui::IconCode::Code::ERROR_FILLED, true,
                     ToastActionCodesAre({ int(toast::ToastActionCode::Dismiss), int(toast::ToastActionCode::TryAgain) })))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedToastResultPromise();
    }));

    // [WHEN] Polling gives up
    m_pollingStatusChanged.send(PollingFailure { Ret(), secs_t(600.0), secs_t(0.0), true });
}

TEST_F(Project_ConvertFileToScoreScenarioTest, Init_PollingStatusChanged_GaveUp_Retry_RetriesPolling)
{
    // [GIVEN] The scenario is wired up to the service's channels
    m_scenario->init();

    // [GIVEN] The user clicks "Retry" on the connection lost toast
    ON_CALL(*m_toastService, show(_, _, _, _, _))
    .WillByDefault(Invoke([](auto&&...) {
        return resolvedToastResultPromise(toast::ToastResult(int(toast::ToastActionCode::TryAgain)));
    }));

    // [THEN] Polling is retried
    EXPECT_CALL(*m_service, retryPolling()).Times(1);

    // [WHEN] Polling gives up
    m_pollingStatusChanged.send(PollingFailure { Ret(), secs_t(600.0), secs_t(0.0), true });

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, Init_PollingStatusChanged_GaveUp_Dismiss_DoesNotRetryPolling)
{
    // [GIVEN] The scenario is wired up to the service's channels
    m_scenario->init();

    // [GIVEN] The user dismisses the connection lost toast
    ON_CALL(*m_toastService, show(_, _, _, _, _))
    .WillByDefault(Invoke([](auto&&...) {
        return resolvedToastResultPromise(toast::ToastResult(int(toast::ToastActionCode::Dismiss)));
    }));

    // [THEN] Polling is not retried
    EXPECT_CALL(*m_service, retryPolling()).Times(0);

    // [WHEN] Polling gives up
    m_pollingStatusChanged.send(PollingFailure { Ret(), secs_t(600.0), secs_t(0.0), true });

    pumpEvents();
}

// ==================================================
// config()
// ==================================================

TEST_F(Project_ConvertFileToScoreScenarioTest, Config_DelegatesToService)
{
    // [GIVEN] A service-provided config
    m_config.omr.images.maxFiles = 42;

    // [WHEN] Reading the scenario's config
    // [THEN] It is the same object the service returns
    EXPECT_EQ(&m_scenario->config(), &m_config);
    EXPECT_EQ(m_scenario->config().omr.images.maxFiles, 42);
}

// ==================================================
// isFileSupported()
// ==================================================

TEST_F(Project_ConvertFileToScoreScenarioTest, IsFileSupported_DelegatesToService)
{
    // [GIVEN] A file the service considers supported
    const io::path_t path = "/some/file.xyz";
    EXPECT_CALL(*m_service, isFileSupported(path))
    .WillOnce(Return(true));

    // [WHEN] Asking the scenario
    // [THEN] It delegates to the service
    EXPECT_TRUE(m_scenario->isFileSupported(path));
}

// ==================================================
// validateFiles()
// ==================================================

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateFiles_Success_NoDialog)
{
    // [GIVEN] The service reports a successful validation
    const io::paths_t paths { "/some/file.xyz" };
    const ConvertFilesValidation validation { ConvertType::Omr, FileCategory::Pdf };
    ON_CALL(*m_service, validateFiles(paths))
    .WillByDefault(Return(RetVal<ConvertFilesValidation>::make_ok(validation)));

    // [THEN] No error dialog is shown
    expectNoErrorDialog();

    // [WHEN] Validating the files
    RetVal<ConvertFilesValidation> result = m_scenario->validateFiles(paths);

    // [THEN] The validation result is returned as-is
    EXPECT_TRUE(result.ret);
    EXPECT_EQ(result.val.type, ConvertType::Omr);
    EXPECT_EQ(result.val.category, FileCategory::Pdf);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateFiles_ValidationFailedCode_ShowsUnknownErrorDialog)
{
    // [GIVEN] The service reports a generic validation failure , which isn't
    // routed to any specific dialog, and so falls back to the generic "unknown error" one
    expectValidateFilesShowsError(Err::ConvertValidationFailed,
                                  muse::trc("project/convert", "Something went wrong"),
                                  muse::trc("project/convert", "Check your internet connection and try again."));
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateFiles_UnsupportedFormat_ShowsDialog)
{
    expectValidateFilesShowsError(Err::ConvertUnsupportedFormat,
                                  muse::trc("project/convert", "This file type is not compatible"),
                                  muse::trc("project/convert", "Make sure you’re importing a suitable PDF, image or audio file."));
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateFiles_MixedFileTypes_ShowsDialog)
{
    expectValidateFilesShowsError(Err::ConvertMixedFileTypes,
                                  muse::trc("project/convert", "Please select files of the same type"),
                                  muse::trc("project/convert",
                                            "Per conversion, you may select either one audio file, one PDF file, or multiple image files."));
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateFiles_MultiplePdfFiles_ShowsDialog)
{
    expectValidateFilesShowsError(Err::ConvertMultiplePdfFiles,
                                  muse::trc("project/convert", "Please select a single PDF file"),
                                  muse::trc("project/convert", "Only one PDF file can be converted at a time."));
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateFiles_AudioFileTooLarge_ShowsDialog)
{
    // [GIVEN] A configured maximum audio file size
    m_config.audio2score.file.maxFileSizeBytes = 30LL * 1024 * 1024;
    const QString size = DataFormatter::formatFileSize(size_t(m_config.audio2score.file.maxFileSizeBytes));
    const std::string text = muse::qtrc("project/convert", "The maximum file size is %1. Reduce the size of your file and try again.")
                             .arg(size).toStdString();

    expectValidateFilesShowsError(Err::ConvertAudioFileTooLarge, muse::trc("project/convert", "This file is too large"), text);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateFiles_FileTooLarge_ShowsDialog)
{
    // [GIVEN] A configured maximum OMR file size
    m_config.omr.pdf.maxFileSizeBytes = 30LL * 1024 * 1024;
    const QString size = DataFormatter::formatFileSize(size_t(m_config.omr.pdf.maxFileSizeBytes));
    const std::string text = muse::qtrc("project/convert", "The maximum file size is %1. Reduce the size of your file and try again.")
                             .arg(size).toStdString();

    expectValidateFilesShowsError(Err::ConvertFileTooLarge, muse::trc("project/convert", "This file is too large"), text);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateFiles_CombinedImageTooLarge_ShowsDialog)
{
    // [GIVEN] A configured maximum combined image size
    m_config.omr.images.maxFileSizeBytes = 30LL * 1024 * 1024;
    const QString size = DataFormatter::formatFileSize(size_t(m_config.omr.images.maxFileSizeBytes));
    const std::string text = muse::qtrc("project/convert",
                                        "The maximum combined file size for all images is %1. Choose a smaller file or remove some images to continue.")
                             .arg(size).toStdString();

    expectValidateFilesShowsError(Err::ConvertCombinedImageTooLarge, muse::trc("project/convert", "Maximum file size exceeded"), text);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateFiles_TooManyAudioFiles_ShowsDialog)
{
    // [GIVEN] A configured maximum audio file count
    m_config.audio2score.file.maxFiles = 3;
    const std::string text = muse::qtrc("project/convert",
                                        "You can convert up to %n audio file(s) at a time. Remove some files and try again.",
                                        nullptr, m_config.audio2score.file.maxFiles).toStdString();

    expectValidateFilesShowsError(Err::ConvertTooManyAudioFiles, muse::trc("project/convert", "Too many files selected"), text);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateFiles_TooManyImages_ShowsDialog)
{
    // [GIVEN] A configured maximum image count
    m_config.omr.images.maxFiles = 15;
    const std::string text = muse::qtrc("project/convert", "You can convert up to %n image(s) at a time. Remove some images and try again.",
                                        nullptr, m_config.omr.images.maxFiles).toStdString();

    expectValidateFilesShowsError(Err::ConvertTooManyImages, muse::trc("project/convert", "Too many images selected"), text);
}

// ==================================================
// validateLink()
// ==================================================

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateLink_Success_NoDialog)
{
    // [GIVEN] The service accepts the link
    const QUrl link("https://link.xyz");
    ON_CALL(*m_service, validateLink(link))
    .WillByDefault(Return(make_ok()));

    // [THEN] No error dialog is shown
    expectNoErrorDialog();

    // [WHEN] Validating the link
    Ret ret = m_scenario->validateLink(link);

    // [THEN] It succeeds
    EXPECT_TRUE(ret);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateLink_Unsupported_ShowsDialog_BothSources)
{
    // [GIVEN] Both link sources are allowed, but the service rejects the link
    m_config.audio2score.link.allowedSources = LinkSource::YouTube | LinkSource::AudioCom;
    const QUrl link("https://link.xyz");
    ON_CALL(*m_service, validateLink(link))
    .WillByDefault(Return(make_ret(Err::ConvertUnsupportedLink)));

    const std::string title = muse::trc("project/convert", "Please use a compatible URL");
    const std::string text = muse::trc("project/convert", "Make sure you’re using a valid link from YouTube or Audio.com.");

    // [THEN] The error dialog mentions both sources
    EXPECT_CALL(*m_interactive,
                warning(title, TextIs(text), ButtonIdsAre({ int(IInteractive::Button::Ok) }), int(IInteractive::Button::NoButton),
                        IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedResultPromise();
    }));

    // [WHEN] Validating the link
    Ret ret = m_scenario->validateLink(link);

    // [THEN] It fails
    EXPECT_FALSE(ret);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateLink_Unsupported_ShowsDialog_UnsetSourcesEmptyText)
{
    // [GIVEN] No link sources are configured, and the service rejects the link
    m_config.audio2score.link.allowedSources = LinkSources();
    const QUrl link("https://link.xyz");
    ON_CALL(*m_service, validateLink(link))
    .WillByDefault(Return(make_ret(Err::ConvertUnsupportedLink)));

    const std::string title = muse::trc("project/convert", "Please use a compatible URL");

    // [THEN] The error dialog has no source-specific text
    EXPECT_CALL(*m_interactive,
                warning(title, TextIs(std::string()), ButtonIdsAre({ int(IInteractive::Button::Ok) }), int(IInteractive::Button::NoButton),
                        IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedResultPromise();
    }));

    // [WHEN] Validating the link
    Ret ret = m_scenario->validateLink(link);

    // [THEN] It fails
    EXPECT_FALSE(ret);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateLink_Unsupported_ShowsDialog_YouTubeOnly)
{
    // [GIVEN] Only YouTube is allowed, and the service rejects the link
    m_config.audio2score.link.allowedSources = LinkSource::YouTube;
    const QUrl link("https://link.xyz");
    ON_CALL(*m_service, validateLink(link))
    .WillByDefault(Return(make_ret(Err::ConvertUnsupportedLink)));

    const std::string title = muse::trc("project/convert", "Please use a compatible URL");
    const std::string text = muse::trc("project/convert", "Make sure you’re using a valid link from YouTube.");

    // [THEN] The error dialog mentions YouTube only
    EXPECT_CALL(*m_interactive,
                warning(title, TextIs(text), ButtonIdsAre({ int(IInteractive::Button::Ok) }), int(IInteractive::Button::NoButton),
                        IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedResultPromise();
    }));

    // [WHEN] Validating the link
    Ret ret = m_scenario->validateLink(link);

    // [THEN] It fails
    EXPECT_FALSE(ret);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateLink_Unsupported_ShowsDialog_AudioComOnly)
{
    // [GIVEN] Only Audio.com is allowed, and the service rejects the link
    m_config.audio2score.link.allowedSources = LinkSource::AudioCom;
    const QUrl link("https://link.xyz");
    ON_CALL(*m_service, validateLink(link))
    .WillByDefault(Return(make_ret(Err::ConvertUnsupportedLink)));

    const std::string title = muse::trc("project/convert", "Please use a compatible URL");
    const std::string text = muse::trc("project/convert", "Make sure you’re using a valid link from Audio.com.");

    // [THEN] The error dialog mentions Audio.com only
    EXPECT_CALL(*m_interactive,
                warning(title, TextIs(text), ButtonIdsAre({ int(IInteractive::Button::Ok) }), int(IInteractive::Button::NoButton),
                        IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedResultPromise();
    }));

    // [WHEN] Validating the link
    Ret ret = m_scenario->validateLink(link);

    // [THEN] It fails
    EXPECT_FALSE(ret);
}

// ==================================================
// convertFiles()
// ==================================================

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_ShowsError_WhenCloudUnavailable)
{
    // [GIVEN] The cloud is unavailable
    ON_CALL(*m_authorization, checkCloudIsAvailable())
    .WillByDefault(Invoke([] {
        return resolvedRetPromise(Ret(false));
    }));

    const std::string title = muse::trc("project/convert", "Unable to connect to MuseScore.com");
    const std::string text = muse::trc("project/convert",
                                       "An internet connection is required for file conversion. Please check your internet connection or try again later.");

    // [THEN] The "cloud unavailable" error is shown
    EXPECT_CALL(*m_interactive,
                warning(title, TextIs(text), ButtonIdsAre({ int(IInteractive::Button::Ok) }), int(IInteractive::Button::NoButton),
                        IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedResultPromise();
    }));

    // [THEN] The picker is never opened
    EXPECT_CALL(*m_interactive, open(_)).Times(0);

    // [WHEN] Converting with no pre-selected files
    m_scenario->convertFiles();

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_OpensPicker_WhenAlreadyAuthorized)
{
    // [GIVEN] The cloud is available and the user is already authorized (default SetUp state)

    // [THEN] No authorization dialog is shown, and the flow proceeds to open the picker
    EXPECT_CALL(*m_interactive, open(UriQuery("muse://cloud/requireauthorization"))).Times(0);
    EXPECT_CALL(*m_interactive, open(UriQuery("musescore://project/convert/selectfiles")))
    .WillOnce(Invoke([](auto&&...) {
        return pendingValPromise();
    }));

    // [WHEN] Converting with no pre-selected files
    m_scenario->convertFiles();

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_PromptsAuthorization_WhenNotAuthorized)
{
    // [GIVEN] The cloud is available but the user isn't authorized
    ON_CALL(*m_authorization, userAuthorized())
    .WillByDefault(Return(ValCh<bool> { false, {} }));

    const std::string dialogText = muse::trc("project/convert", "Log in or create a free account on MuseScore.com to convert a file.");

    UriQuery expectedAuthQuery("muse://cloud/requireauthorization");
    expectedAuthQuery.addParam("text", Val(dialogText));
    expectedAuthQuery.addParam("cloudCode", Val(QString()));
    expectedAuthQuery.addParam("publishingScore", Val(false));

    // [THEN] The authorization dialog is opened, and the user completes it
    EXPECT_CALL(*m_interactive, open(expectedAuthQuery))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedValPromise();
    }));

    // [THEN] Once authorized, the flow proceeds to open the picker
    EXPECT_CALL(*m_interactive, open(UriQuery("musescore://project/convert/selectfiles")))
    .WillOnce(Invoke([](auto&&...) {
        return pendingValPromise();
    }));

    // [WHEN] Converting with no pre-selected files
    m_scenario->convertFiles();

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_WithPaths_StopsWhenNotAllowed)
{
    // [GIVEN] The cloud is unavailable
    ON_CALL(*m_authorization, checkCloudIsAvailable())
    .WillByDefault(Invoke([] {
        return resolvedRetPromise(Ret(false));
    }));

    const std::string title = muse::trc("project/convert", "Unable to connect to MuseScore.com");
    const std::string text = muse::trc("project/convert",
                                       "An internet connection is required for file conversion. Please check your internet connection or try again later.");

    // [THEN] The "cloud unavailable" error is shown, but neither validation, confirmation, nor conversion is attempted
    EXPECT_CALL(*m_interactive,
                warning(title, TextIs(text), ButtonIdsAre({ int(IInteractive::Button::Ok) }), int(IInteractive::Button::NoButton),
                        IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedResultPromise();
    }));
    EXPECT_CALL(*m_service, validateFiles(_)).Times(0);
    EXPECT_CALL(*m_interactive, question(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(*m_service, uploadToConvert(_, _)).Times(0);

    // [WHEN] Converting files
    m_scenario->convertFiles(io::paths_t { "/some/file.xyz" });

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_WithPaths_StopsWhenValidationFails)
{
    // [GIVEN] The service rejects the files (the exact error dialog per code is covered by the validateFiles() tests)
    const io::paths_t paths { "/some/file.xyz" };
    ON_CALL(*m_service, validateFiles(paths))
    .WillByDefault(Return(RetVal<ConvertFilesValidation>::make_ret(make_ret(Err::ConvertUnsupportedFormat))));

    // [THEN] The validation error dialog is shown
    EXPECT_CALL(*m_interactive, warning(_, _, _, _, _, _))
    .WillOnce(Invoke([](auto&&...) { return resolvedResultPromise(); }));

    // [THEN] Neither confirmation nor conversion is attempted
    EXPECT_CALL(*m_interactive, question(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(*m_service, uploadToConvert(_, _)).Times(0);

    // [WHEN] Converting files
    m_scenario->convertFiles(paths);

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_ShowsProcessingDialog_WhenEnabled)
{
    // [GIVEN] The processing dialog is enabled in the config
    ON_CALL(*m_configuration, showConvertFileProcessingDialog())
    .WillByDefault(Return(true));

    // [GIVEN] The upload starts, but doesn't finish
    ON_CALL(*m_service, uploadToConvert(_, _))
    .WillByDefault(Return(uploadStarted()));

    // [GIVEN] The user picks a file via the picker dialog
    EXPECT_CALL(*m_interactive, open(UriQuery("musescore://project/convert/selectfiles")))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedValPromise(Val::fromQVariant(pickedOmrFileSelection()));
    }));

    // [THEN] The processing dialog is shown while the upload is still in progress
    EXPECT_CALL(*m_interactive, open(UriQuery("musescore://project/convert/processing")))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedValPromise();
    }));

    // [WHEN] Converting with no pre-selected files
    m_scenario->convertFiles();

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_GoToScores_OpensHomeScoresPage)
{
    // [GIVEN] The processing dialog is enabled in the config
    ON_CALL(*m_configuration, showConvertFileProcessingDialog())
    .WillByDefault(Return(true));
    ON_CALL(*m_service, uploadToConvert(_, _))
    .WillByDefault(Return(uploadStarted()));

    // [GIVEN] The user picks a file via the picker dialog
    EXPECT_CALL(*m_interactive, open(UriQuery("musescore://project/convert/selectfiles")))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedValPromise(Val::fromQVariant(pickedOmrFileSelection()));
    }));

    // [GIVEN] The user clicks "Go to scores" in the processing dialog
    EXPECT_CALL(*m_interactive, open(UriQuery("musescore://project/convert/processing")))
    .WillOnce(Invoke([](auto&&...) {
        QVariantMap result { { "action", QString("goToScores") }, { "showAgain", true } };
        return resolvedValPromise(Val::fromQVariant(result));
    }));

    // [THEN] The app navigates to Home > Scores page, on the "My online scores" tab
    EXPECT_CALL(*m_interactive, open(UriQuery("musescore://home?section=scores&subSection=myOnlineScores")))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedValPromise();
    }));

    // [WHEN] Converting with no pre-selected files
    m_scenario->convertFiles();

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_DontShowAgain_UpdatesConfig)
{
    // [GIVEN] The processing dialog is enabled in the config
    ON_CALL(*m_configuration, showConvertFileProcessingDialog())
    .WillByDefault(Return(true));
    ON_CALL(*m_service, uploadToConvert(_, _))
    .WillByDefault(Return(uploadStarted()));

    // [GIVEN] The user picks a file via the picker dialog
    ON_CALL(*m_interactive, open(UriQuery("musescore://project/convert/selectfiles")))
    .WillByDefault(Invoke([](auto&&...) {
        return resolvedValPromise(Val::fromQVariant(pickedOmrFileSelection()));
    }));

    // [GIVEN] The user checks "Don't show again" and dismisses the processing dialog
    ON_CALL(*m_interactive, open(UriQuery("musescore://project/convert/processing")))
    .WillByDefault(Invoke([](auto&&...) {
        QVariantMap result { { "action", QString() }, { "showAgain", false } };
        return resolvedValPromise(Val::fromQVariant(result));
    }));

    // [THEN] The preference is persisted
    EXPECT_CALL(*m_configuration, setShowConvertFileProcessingDialog(false))
    .Times(1);

    // [WHEN] Converting with no pre-selected files
    m_scenario->convertFiles();

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_ProcessingDialogDismissed_DoesNotChangeConfig)
{
    // [GIVEN] The processing dialog is enabled in the config
    ON_CALL(*m_configuration, showConvertFileProcessingDialog())
    .WillByDefault(Return(true));
    ON_CALL(*m_service, uploadToConvert(_, _))
    .WillByDefault(Return(uploadStarted()));

    // [GIVEN] The user picks a file via the picker dialog
    ON_CALL(*m_interactive, open(UriQuery("musescore://project/convert/selectfiles")))
    .WillByDefault(Invoke([](auto&&...) {
        return resolvedValPromise(Val::fromQVariant(pickedOmrFileSelection()));
    }));

    // [GIVEN] The user closes the processing dialog without clicking a button (e.g. Escape),
    //! so the dialog's promise rejects instead of resolving
    ON_CALL(*m_interactive, open(UriQuery("musescore://project/convert/processing")))
    .WillByDefault(Invoke([](auto&&...) {
        return async::make_promise<Val>([](auto resolve, auto reject) {
            (void)resolve;
            return reject(int(Ret::Code::Cancel), std::string());
        });
    }));

    // [THEN] The "show again" preference is left untouched
    EXPECT_CALL(*m_configuration, setShowConvertFileProcessingDialog(_))
    .Times(0);

    // [WHEN] Converting with no pre-selected files
    m_scenario->convertFiles();

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_NoProcessingDialog_WhenDisabled)
{
    // [GIVEN] The processing dialog is disabled in the config
    ON_CALL(*m_configuration, showConvertFileProcessingDialog())
    .WillByDefault(Return(false));
    ON_CALL(*m_service, uploadToConvert(_, _))
    .WillByDefault(Return(uploadStarted()));

    // [GIVEN] The user picks a file via the picker dialog
    EXPECT_CALL(*m_interactive, open(UriQuery("musescore://project/convert/selectfiles")))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedValPromise(Val::fromQVariant(pickedOmrFileSelection()));
    }));

    // [THEN] No processing dialog is shown
    EXPECT_CALL(*m_interactive, open(UriQuery("musescore://project/convert/processing"))).Times(0);

    // [WHEN] Converting with no pre-selected files
    m_scenario->convertFiles();

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_SchemelessLink_StartsAudio2ScoreConvertWithScheme)
{
    // [GIVEN] The user enters a link without a scheme in the picker dialog
    const QVariantMap selectionMap {
        { "type", int(ConvertType::Audio2Score) },
        { "link", QString("link.xyz/song") },
        { "convertedScoreName", QString("song") }
    };
    EXPECT_CALL(*m_interactive, open(UriQuery("musescore://project/convert/selectfiles")))
    .WillOnce(Invoke([selectionMap](auto&&...) {
        return resolvedValPromise(Val::fromQVariant(selectionMap));
    }));

    // [THEN] The conversion is started with the link completed with a scheme
    EXPECT_CALL(*m_service, uploadToConvert(Truly([](const ConvertInput& input) {
        return convertTypeOf(input) == ConvertType::Audio2Score && convertLinkOf(input) == QUrl("http://link.xyz/song");
    }), muse::String(u"song")))
    .WillOnce(Return(uploadStarted()));

    // [WHEN] Converting with no pre-selected files
    m_scenario->convertFiles();

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_FileReadFailure_ShowsFileReadError)
{
    // [GIVEN] The service fails to read the picked file
    ON_CALL(*m_service, uploadToConvert(_, _))
    .WillByDefault(Return(uploadNotStarted(io::make_ret(io::Err::FSReadError))));

    // [GIVEN] The user picks a file via the picker dialog
    ON_CALL(*m_interactive, open(UriQuery("musescore://project/convert/selectfiles")))
    .WillByDefault(Invoke([](auto&&...) {
        return resolvedValPromise(Val::fromQVariant(pickedOmrFileSelection()));
    }));

    const std::string title = muse::trc("project/convert", "This file could not be read");
    const std::string text = muse::trc("project/convert", "Make sure the file still exists and that you have permission to open it.");

    // [THEN] The "file could not be read" dialog is shown, and no processing dialog is shown
    EXPECT_CALL(*m_interactive,
                warning(title, TextIs(text), ButtonIdsAre({ int(IInteractive::Button::Ok) }), int(IInteractive::Button::NoButton),
                        IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedResultPromise();
    }));
    EXPECT_CALL(*m_toastService, show(_, _, _, _, _)).Times(0);

    // [WHEN] Converting with no pre-selected files
    m_scenario->convertFiles();

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_MultipleFilesReadFailure_ShowsPluralFileReadError)
{
    // [GIVEN] The service fails to read one of the picked files
    ON_CALL(*m_service, uploadToConvert(_, _))
    .WillByDefault(Return(uploadNotStarted(io::make_ret(io::Err::FSNotExist))));

    // [GIVEN] The user picks multiple images via the picker dialog
    ON_CALL(*m_interactive, open(UriQuery("musescore://project/convert/selectfiles")))
    .WillByDefault(Invoke([](auto&&...) {
        QVariantMap selection {
            { "type", int(ConvertType::Omr) },
            { "paths", QStringList { "/some/a.png", "/some/b.png" } },
            { "convertedScoreName", QString("file") }
        };
        return resolvedValPromise(Val::fromQVariant(selection));
    }));

    const std::string title = muse::trc("project/convert", "One or more files could not be read");
    const std::string text = muse::trc("project/convert", "Make sure the files still exist and that you have permission to open them.");

    // [THEN] The plural "files could not be read" dialog is shown, and no processing dialog is shown
    EXPECT_CALL(*m_interactive,
                warning(title, TextIs(text), ButtonIdsAre({ int(IInteractive::Button::Ok) }), int(IInteractive::Button::NoButton),
                        IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedResultPromise();
    }));
    EXPECT_CALL(*m_toastService, show(_, _, _, _, _)).Times(0);

    // [WHEN] Converting with no pre-selected files
    m_scenario->convertFiles();

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_Failure_ShowsUnknownError)
{
    // [GIVEN] The service fails to start the conversion
    ON_CALL(*m_service, uploadToConvert(_, _))
    .WillByDefault(Return(uploadNotStarted(make_ret(Err::ConvertProcessingFailed))));

    // [GIVEN] The user picks a file via the picker dialog
    ON_CALL(*m_interactive, open(UriQuery("musescore://project/convert/selectfiles")))
    .WillByDefault(Invoke([](auto&&...) {
        return resolvedValPromise(Val::fromQVariant(pickedOmrFileSelection()));
    }));

    const std::string title = muse::trc("project/convert", "Something went wrong");
    const std::string text = muse::trc("project/convert", "Check your internet connection and try again.");

    // [THEN] The generic "unknown error" dialog is shown, and no processing dialog is shown
    EXPECT_CALL(*m_interactive,
                warning(title, TextIs(text), ButtonIdsAre({ int(IInteractive::Button::Ok) }), int(IInteractive::Button::NoButton),
                        IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedResultPromise();
    }));
    EXPECT_CALL(*m_toastService, show(_, _, _, _, _)).Times(0);

    // [WHEN] Converting with no pre-selected files
    m_scenario->convertFiles();

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_UploadFailure_ShowsUploadFailedNotification)
{
    // [GIVEN] The upload starts
    Progress uploadProgress;
    ON_CALL(*m_service, uploadToConvert(_, _))
    .WillByDefault(Return(uploadStarted(uploadProgress)));

    // [GIVEN] The user picks a file via the picker dialog
    EXPECT_CALL(*m_interactive, open(UriQuery("musescore://project/convert/selectfiles")))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedValPromise(Val::fromQVariant(pickedOmrFileSelection()));
    }));

    const std::string title = muse::trc("project/convert", "Error uploading score");
    const std::string text = muse::qtrc("project/convert", "We weren’t able to upload ‘%1’. Check your internet connection and try again.")
                             .arg("file").toStdString();

    // [THEN] The "upload failed" notification is shown, with a Dismiss and an accented Retry action
    EXPECT_CALL(*m_toastService,
                show(title, text, muse::ui::IconCode::Code::ERROR_FILLED, true,
                     ToastActionCodesAre({ int(toast::ToastActionCode::Dismiss), int(toast::ToastActionCode::TryAgain) })))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedToastResultPromise();
    }));

    // [THEN] No error dialog is shown
    expectNoErrorDialog();

    // [WHEN] Converting with no pre-selected files, and the upload then fails
    m_scenario->convertFiles();
    pumpEvents();

    uploadProgress.finish(make_ret(Err::ConvertUploadFailed));
    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_UploadCanceled_ShowsNoError)
{
    // [GIVEN] The upload starts
    Progress uploadProgress;
    ON_CALL(*m_service, uploadToConvert(_, _))
    .WillByDefault(Return(uploadStarted(uploadProgress)));

    // [GIVEN] The user picks a file via the picker dialog
    EXPECT_CALL(*m_interactive, open(UriQuery("musescore://project/convert/selectfiles")))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedValPromise(Val::fromQVariant(pickedOmrFileSelection()));
    }));

    // [THEN] Neither a notification nor an error dialog is shown
    EXPECT_CALL(*m_toastService, show(_, _, _, _, _)).Times(0);
    expectNoErrorDialog();

    // [WHEN] Converting with no pre-selected files, and the upload is then canceled
    m_scenario->convertFiles();
    pumpEvents();

    uploadProgress.finish(make_ret(Ret::Code::Cancel));
    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_UploadFailure_Retry_ReuploadsSameInput)
{
    // [GIVEN] The user picks a file via the picker dialog, only once
    EXPECT_CALL(*m_interactive, open(UriQuery("musescore://project/convert/selectfiles")))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedValPromise(Val::fromQVariant(pickedOmrFileSelection()));
    }));

    // [GIVEN] The user clicks "Retry" on the upload failed toast
    ON_CALL(*m_toastService, show(_, _, _, _, _))
    .WillByDefault(Invoke([](auto&&...) {
        return resolvedToastResultPromise(toast::ToastResult(int(toast::ToastActionCode::TryAgain)));
    }));

    // [THEN] The same input is uploaded again: the first upload fails, the retry succeeds
    Progress firstUploadProgress;
    EXPECT_CALL(*m_service, uploadToConvert(Truly([](const ConvertInput& input) {
        return convertTypeOf(input) == ConvertType::Omr && convertPathsOf(input) == io::paths_t { "/some/file.xyz" };
    }), muse::String(u"file")))
    .WillOnce(Return(uploadStarted(firstUploadProgress)))
    .WillOnce(Return(uploadStarted()));

    // [WHEN] Converting with no pre-selected files, and the first upload then fails
    m_scenario->convertFiles();
    pumpEvents();

    firstUploadProgress.finish(make_ret(Err::ConvertUploadFailed));
    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_WithPaths_Proceeds_StartsOmrConvert)
{
    // [GIVEN] The service accepts the file as an OMR candidate
    const io::paths_t paths { "/some/path/file.xyz" };
    ON_CALL(*m_service, validateFiles(paths))
    .WillByDefault(Return(RetVal<ConvertFilesValidation>::make_ok(ConvertFilesValidation { ConvertType::Omr, FileCategory::Pdf })));

    constexpr int proceedBtn = int(IInteractive::Button::CustomButton) + 1;

    const std::string title = muse::trc("project/convert", "Would you like to convert this file to a score?");
    const std::string text = muse::trc("project/convert",
                                       "This file needs to be converted online before it can be edited. Would you like to proceed?");

    // [GIVEN] The user proceeds with the confirmation dialog
    EXPECT_CALL(*m_interactive,
                question(title, TextIs(text), ButtonIdsAre({ int(IInteractive::Button::Cancel), proceedBtn }), proceedBtn,
                         IInteractive::Options(), std::string()))
    .WillOnce(Invoke([proceedBtn](auto&&...) {
        return resolvedResultPromise(IInteractive::Result(proceedBtn));
    }));

    // [GIVEN] The file-specific step opens with the dropped file pre-selected, and the user converts it
    UriQuery expectedQuery("musescore://project/convert/selectfiles");
    expectedQuery.addParam("initialPaths", Val(ValList { Val(paths.front()) }));
    expectedQuery.addParam("initialConvertType", Val(ConvertType::Omr));

    const QVariantMap selectionMap {
        { "type", int(ConvertType::Omr) },
        { "paths", QStringList { "/some/path/file.xyz" } },
        { "convertedScoreName", QString("file") }
    };
    EXPECT_CALL(*m_interactive, open(expectedQuery))
    .WillOnce(Invoke([selectionMap](auto&&...) {
        return resolvedValPromise(Val::fromQVariant(selectionMap));
    }));

    // [THEN] The conversion is started as an OMR conversion of the given paths
    EXPECT_CALL(*m_service, uploadToConvert(Truly([&](const ConvertInput& input) {
        return convertTypeOf(input) == ConvertType::Omr && convertPathsOf(input) == paths;
    }), muse::String(u"file")))
    .WillOnce(Return(uploadStarted()));

    // [WHEN] Converting files
    m_scenario->convertFiles(paths);

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_WithPaths_Proceeds_StartsAudio2ScoreConvert)
{
    // [GIVEN] The service accepts the file as an Audio2Score candidate
    const io::paths_t paths { "/some/path/song.xyz" };
    ON_CALL(*m_service, validateFiles(paths))
    .WillByDefault(Return(RetVal<ConvertFilesValidation>::make_ok(ConvertFilesValidation { ConvertType::Audio2Score,
                                                                                           FileCategory::Audio })));

    constexpr int proceedBtn = int(IInteractive::Button::CustomButton) + 1;

    const std::string title = muse::trc("project/convert", "Would you like to convert this file to a score?");
    const std::string text = muse::trc("project/convert",
                                       "This file needs to be converted online before it can be edited. Would you like to proceed?");

    // [GIVEN] The user proceeds with the confirmation dialog
    EXPECT_CALL(*m_interactive,
                question(title, TextIs(text), ButtonIdsAre({ int(IInteractive::Button::Cancel), proceedBtn }), proceedBtn,
                         IInteractive::Options(), std::string()))
    .WillOnce(Invoke([proceedBtn](auto&&...) {
        return resolvedResultPromise(IInteractive::Result(proceedBtn));
    }));

    // [GIVEN] The file-specific step opens with the dropped file pre-selected, and the user converts it
    UriQuery expectedQuery("musescore://project/convert/selectfiles");
    expectedQuery.addParam("initialPaths", Val(ValList { Val(paths.front()) }));
    expectedQuery.addParam("initialConvertType", Val(ConvertType::Audio2Score));

    const QVariantMap selectionMap {
        { "type", int(ConvertType::Audio2Score) },
        { "paths", QStringList { "/some/path/song.xyz" } },
        { "convertedScoreName", QString("song") }
    };
    EXPECT_CALL(*m_interactive, open(expectedQuery))
    .WillOnce(Invoke([selectionMap](auto&&...) {
        return resolvedValPromise(Val::fromQVariant(selectionMap));
    }));

    // [THEN] The conversion is started as an Audio2Score conversion of the given paths
    EXPECT_CALL(*m_service, uploadToConvert(Truly([&](const ConvertInput& input) {
        return convertTypeOf(input) == ConvertType::Audio2Score && convertPathsOf(input) == paths;
    }), muse::String(u"song")))
    .WillOnce(Return(uploadStarted()));

    // [WHEN] Converting files
    m_scenario->convertFiles(paths);

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_WithPaths_UserCancelsConfirm_DoesNotStartConvert)
{
    // [GIVEN] The service accepts the file
    const io::paths_t paths { "/some/path/file.xyz" };
    ON_CALL(*m_service, validateFiles(paths))
    .WillByDefault(Return(RetVal<ConvertFilesValidation>::make_ok(ConvertFilesValidation { ConvertType::Omr, FileCategory::Pdf })));

    constexpr int proceedBtn = int(IInteractive::Button::CustomButton) + 1;

    const std::string title = muse::trc("project/convert", "Would you like to convert this file to a score?");
    const std::string text = muse::trc("project/convert",
                                       "This file needs to be converted online before it can be edited. Would you like to proceed?");

    // [GIVEN] The user cancels the confirmation dialog
    EXPECT_CALL(*m_interactive,
                question(title, TextIs(text), ButtonIdsAre({ int(IInteractive::Button::Cancel), proceedBtn }), proceedBtn,
                         IInteractive::Options(), std::string()))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedResultPromise(IInteractive::Result(int(IInteractive::Button::Cancel)));
    }));

    // [THEN] The file-specific step is never opened, and the conversion is not started
    EXPECT_CALL(*m_interactive, open(_)).Times(0);
    EXPECT_CALL(*m_service, uploadToConvert(_, _)).Times(0);

    // [WHEN] Converting files
    m_scenario->convertFiles(paths);

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_RepeatedCallsWhilePending_OnlyOpensPickerOnce_ThenAllowsNewConvert)
{
    // [GIVEN] The user will eventually pick a file and complete a conversion
    ON_CALL(*m_service, uploadToConvert(_, _))
    .WillByDefault(Invoke([](auto&&...) { return uploadStarted(); }));
    ON_CALL(*m_interactive, open(UriQuery("musescore://project/convert/selectfiles")))
    .WillByDefault(Invoke([](auto&&...) {
        return resolvedValPromise(Val::fromQVariant(pickedOmrFileSelection()));
    }));

    // [THEN] The picker is opened exactly once, even though convertFiles() was called twice in a
    // row before checkConvertIsAllowed() resolved (e.g. a double-click on "Convert...")
    EXPECT_CALL(*m_interactive, open(UriQuery("musescore://project/convert/selectfiles")))
    .Times(1);

    // [WHEN] Converting is triggered twice in a row
    m_scenario->convertFiles();
    m_scenario->convertFiles();

    pumpEvents();

    // [THEN] Once that flow has completed, a later convert is not blocked by the earlier one
    EXPECT_CALL(*m_interactive, open(UriQuery("musescore://project/convert/selectfiles")))
    .Times(1);

    // [WHEN] Converting again
    m_scenario->convertFiles();

    pumpEvents();
}

// ==================================================
// cancelUpload()
// ==================================================

TEST_F(Project_ConvertFileToScoreScenarioTest, CancelUpload_UserConfirms_CancelsUpload)
{
    constexpr int uploadId = 7;

    const std::string title = muse::trc("project/convert", "Are you sure you want to cancel this file conversion?");
    const std::string text = muse::trc("project/convert",
                                       "Processing will be canceled and this score will be removed from your scores.");

    // [GIVEN] The user confirms with "Yes, cancel"
    EXPECT_CALL(*m_interactive,
                question(title, TextIs(text),
                         ButtonIdsAre({ int(IInteractive::Button::No), int(IInteractive::Button::Yes) }),
                         int(IInteractive::Button::No), IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedResultPromise(IInteractive::Result(int(IInteractive::Button::Yes)));
    }));

    // [THEN] The upload is canceled
    EXPECT_CALL(*m_service, cancelUpload(uploadId));

    // [WHEN] Cancelling the upload
    m_scenario->cancelUpload(uploadId);

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, CancelUpload_UserKeepsConverting_DoesNotCancelUpload)
{
    // [GIVEN] The user dismisses with "Continue converting"
    ON_CALL(*m_interactive, question(_, _, _, _, _, _))
    .WillByDefault(Invoke([](auto&&...) {
        return resolvedResultPromise(IInteractive::Result(int(IInteractive::Button::No)));
    }));

    // [THEN] The upload is not canceled
    EXPECT_CALL(*m_service, cancelUpload(_)).Times(0);

    // [WHEN] Cancelling the upload
    m_scenario->cancelUpload(7);

    pumpEvents();
}

// ==================================================
// cancelConversion()
// ==================================================

TEST_F(Project_ConvertFileToScoreScenarioTest, CancelConversion_UserConfirms_CancelsConversion)
{
    constexpr int convertId = 42;

    const std::string title = muse::trc("project/convert", "Are you sure you want to cancel this file conversion?");
    const std::string text = muse::trc("project/convert",
                                       "Processing will be canceled and this score will be removed from your scores.");

    // [GIVEN] The user confirms with "Yes, cancel"
    EXPECT_CALL(*m_interactive,
                question(title, TextIs(text),
                         ButtonIdsAre({ int(IInteractive::Button::No), int(IInteractive::Button::Yes) }),
                         int(IInteractive::Button::No), IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedResultPromise(IInteractive::Result(int(IInteractive::Button::Yes)));
    }));

    // [THEN] The conversion is canceled
    EXPECT_CALL(*m_service, cancelConversion(ConvertType::Omr, convertId))
    .Times(1);

    // [WHEN] Cancelling the conversion
    m_scenario->cancelConversion(ConvertType::Omr, convertId);

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, CancelConversion_UserKeepsConverting_DoesNotCancelConversion)
{
    // [GIVEN] The user dismisses with "Continue converting"
    ON_CALL(*m_interactive, question(_, _, _, _, _, _))
    .WillByDefault(Invoke([](auto&&...) {
        return resolvedResultPromise(IInteractive::Result(int(IInteractive::Button::No)));
    }));

    // [THEN] The conversion is not canceled
    EXPECT_CALL(*m_service, cancelConversion(_, _)).Times(0);

    // [WHEN] Cancelling the conversion
    m_scenario->cancelConversion(ConvertType::Omr, 42);

    pumpEvents();
}
