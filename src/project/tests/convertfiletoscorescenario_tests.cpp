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

#include <thread>

#include <QUrl>

#include "async/processevents.h"
#include "modularity/ioc.h"
#include "global/dataformatter.h"

#include "project/internal/convertfiletoscorescenario.h"
#include "project/projecterrors.h"

#include "mocks/convertfiletoscoreservicemock.h"
#include "mocks/projectconfigurationmock.h"
#include "global/tests/mocks/interactivemock.h"
#include "actions/tests/mocks/actionsdispatchermock.h"
#include "cloud/tests/mocks/musescorecomservicemock.h"
#include "cloud/tests/mocks/authorizationservicemock.h"

using namespace ::testing;
using namespace mu::project;
using namespace muse;
using namespace muse::cloud;

namespace {
void pumpEvents(int iterations = 10)
{
    const std::thread::id thisThId = std::this_thread::get_id();
    for (int i = 0; i < iterations; ++i) {
        muse::async::processMessages(thisThId);
    }
}

//! NOTE: keep the body queued, don't run it eagerly
//! otherwise resolve() can fire before onResolve/onReject are attached
template<typename T>
async::Promise<T> resolvedPromise(const T& val)
{
    return async::make_promise<T>([val](auto resolve, auto reject) {
        (void)reject;
        return resolve(val);
    });
}

async::Promise<Val> resolvedValPromise(const Val& val = Val())
{
    return resolvedPromise<Val>(val);
}

async::Promise<IInteractive::Result> resolvedResultPromise(const IInteractive::Result& result = IInteractive::Result())
{
    return resolvedPromise<IInteractive::Result>(result);
}

Matcher<const IInteractive::Text&> TextIs(const std::string& text)
{
    return Field(&IInteractive::Text::text, text);
}

//! NOTE: ButtonData has no operator==, so match button lists by their btn ids, in order
Matcher<const IInteractive::ButtonDatas&> ButtonIdsAre(const std::vector<int>& expectedIds)
{
    return Truly([expectedIds](const IInteractive::ButtonDatas& buttons) {
        if (buttons.size() != expectedIds.size()) {
            return false;
        }
        for (size_t i = 0; i < buttons.size(); ++i) {
            if (buttons[i].btn != expectedIds[i]) {
                return false;
            }
        }
        return true;
    });
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
        m_dispatcher = std::make_shared<NiceMock<muse::actions::ActionsDispatcherMock> >();
        m_configuration = std::make_shared<NiceMock<ProjectConfigurationMock> >();
        m_museScoreComService = std::make_shared<NiceMock<MuseScoreComServiceMock> >();
        m_authorization = std::make_shared<NiceMock<AuthorizationServiceMock> >();

        m_scenario->service.set(m_service);
        m_scenario->interactive.set(m_interactive);
        m_scenario->dispatcher.set(m_dispatcher);
        m_scenario->configuration.set(m_configuration);
        m_scenario->museScoreComService.set(m_museScoreComService);

        ON_CALL(*m_museScoreComService, authorization())
        .WillByDefault(Return(m_authorization));

        ON_CALL(*m_service, config())
        .WillByDefault(ReturnRef(m_config));

        ON_CALL(*m_interactive, buttonData(_))
        .WillByDefault(Invoke([](IInteractive::Button b) {
            return IInteractive::ButtonData(b, "");
        }));

        ON_CALL(*m_authorization, userAuthorized())
        .WillByDefault(Return(ValCh<bool> { true, {} }));

        ON_CALL(*m_authorization, checkCloudIsAvailableAsync())
        .WillByDefault(Invoke([] {
            return resolvedPromise<Ret>(make_ok());
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
                    error(expectedTitle, TextIs(expectedText), ButtonIdsAre({ int(IInteractive::Button::Ok) }),
                          int(IInteractive::Button::NoButton), IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
        .Times(1)
        .WillOnce(Invoke([](auto&&...) {
            return resolvedResultPromise();
        }));

        // [WHEN] Validating the files
        RetVal<ConvertFilesValidation> result = m_scenario->validateFiles(paths);

        // [THEN] Validation fails
        EXPECT_FALSE(result.ret);
    }

    std::shared_ptr<ConvertFileToScoreScenario> m_scenario;
    std::shared_ptr<ConvertFileToScoreServiceMock> m_service;
    std::shared_ptr<InteractiveMock> m_interactive;
    std::shared_ptr<muse::actions::ActionsDispatcherMock> m_dispatcher;
    std::shared_ptr<ProjectConfigurationMock> m_configuration;
    std::shared_ptr<MuseScoreComServiceMock> m_museScoreComService;
    std::shared_ptr<AuthorizationServiceMock> m_authorization;

    ConvertConfig m_config;
};
}

// ==================================================
// init()
// ==================================================

TEST_F(Project_ConvertFileToScoreScenarioTest, Init_Success_ShowsScoreReadyNotificationAndForwards)
{
    // [GIVEN] The service's channels, wired up via init()
    async::Channel<Ret, io::path_t> convertFinished;
    async::Channel<ConvertType, int> reviewRequested;
    ON_CALL(*m_service, convertFinished()).WillByDefault(Return(convertFinished));
    ON_CALL(*m_service, reviewRequested()).WillByDefault(Return(reviewRequested));
    m_scenario->init();

    const io::path_t path = "/some/path/My Score.xyz";
    constexpr int openScoreBtn = int(IInteractive::Button::CustomButton) + 1;
    constexpr int dismissBtn = int(IInteractive::Button::CustomButton) + 2;

    const std::string title = muse::trc("project/convert", "Your score is ready!");
    const std::string text = muse::qtrc("project/convert", "‘%1’ has finished processing and is ready to open.")
                             .arg("My Score").toStdString();

    // [THEN] The "score ready" notification is shown
    EXPECT_CALL(*m_interactive,
                info(title, TextIs(text), ButtonIdsAre({ dismissBtn, openScoreBtn }), dismissBtn,
                     IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
    .Times(1)
    .WillOnce(Invoke([](auto&&...) {
        return resolvedResultPromise();
    }));

    bool forwarded = false;
    Ret forwardedRet;
    m_scenario->convertFinished().onReceive(nullptr, [&](const Ret& ret, const io::path_t&) {
        forwarded = true;
        forwardedRet = ret;
    });

    // [WHEN] The service reports a successful conversion
    convertFinished.send(make_ok(), path);

    // [THEN] The result is forwarded to the scenario's own convertFinished channel
    EXPECT_TRUE(forwarded);
    EXPECT_TRUE(forwardedRet);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, Init_Failure_NoNotificationButStillForwards)
{
    // [GIVEN] The service's channels, wired up via init()
    async::Channel<Ret, io::path_t> convertFinished;
    async::Channel<ConvertType, int> reviewRequested;
    ON_CALL(*m_service, convertFinished()).WillByDefault(Return(convertFinished));
    ON_CALL(*m_service, reviewRequested()).WillByDefault(Return(reviewRequested));
    m_scenario->init();

    // [THEN] No notification is shown for a failed conversion
    EXPECT_CALL(*m_interactive, info(_, _, _, _, _, _)).Times(0);

    bool forwarded = false;
    Ret forwardedRet;
    m_scenario->convertFinished().onReceive(nullptr, [&](const Ret& ret, const io::path_t&) {
        forwarded = true;
        forwardedRet = ret;
    });

    // [WHEN] The service reports a failed conversion
    convertFinished.send(make_ret(Err::ConvertProcessingFailed), io::path_t());

    // [THEN] The failure is still forwarded to the scenario's own convertFinished channel
    EXPECT_TRUE(forwarded);
    EXPECT_FALSE(forwardedRet);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, Init_ReviewRequested_Good_SubmitsGoodRating)
{
    // [GIVEN] The service's channels, wired up via init()
    async::Channel<Ret, io::path_t> convertFinished;
    async::Channel<ConvertType, int> reviewRequested;
    ON_CALL(*m_service, convertFinished()).WillByDefault(Return(convertFinished));
    ON_CALL(*m_service, reviewRequested()).WillByDefault(Return(reviewRequested));
    m_scenario->init();

    constexpr int goodBtn = int(IInteractive::Button::CustomButton) + 1;
    constexpr int badBtn = int(IInteractive::Button::CustomButton) + 2;

    const std::string title = muse::trc("project/convert", "How does your score look?");
    const std::string text = muse::trc("project/convert",
                                       "We’re always improving our score conversion accuracy. Let us know how we did with this one.");

    // [THEN] The review rating dialog is shown, and the user's pick of "Good" is submitted
    EXPECT_CALL(*m_interactive,
                question(title, TextIs(text), ButtonIdsAre({ goodBtn, badBtn }), goodBtn, IInteractive::Options(), std::string()))
    .WillOnce(Invoke([goodBtn](auto&&...) {
        return resolvedResultPromise(IInteractive::Result(goodBtn));
    }));

    EXPECT_CALL(*m_service, submitReview(ConvertType::Omr, 42, ReviewRating::Good, QString()))
    .Times(1);

    // [WHEN] The service requests a review for a finished conversion
    reviewRequested.send(ConvertType::Omr, 42);

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, Init_ReviewRequested_Bad_SubmitsBadRating)
{
    // [GIVEN] The service's channels, wired up via init()
    async::Channel<Ret, io::path_t> convertFinished;
    async::Channel<ConvertType, int> reviewRequested;
    ON_CALL(*m_service, convertFinished()).WillByDefault(Return(convertFinished));
    ON_CALL(*m_service, reviewRequested()).WillByDefault(Return(reviewRequested));
    m_scenario->init();

    constexpr int goodBtn = int(IInteractive::Button::CustomButton) + 1;
    constexpr int badBtn = int(IInteractive::Button::CustomButton) + 2;

    const std::string title = muse::trc("project/convert", "How does your score look?");
    const std::string text = muse::trc("project/convert",
                                       "We’re always improving our score conversion accuracy. Let us know how we did with this one.");

    // [THEN] The review rating dialog is shown, and the user's pick of "Bad" is submitted
    EXPECT_CALL(*m_interactive,
                question(title, TextIs(text), ButtonIdsAre({ goodBtn, badBtn }), goodBtn, IInteractive::Options(), std::string()))
    .WillOnce(Invoke([badBtn](auto&&...) {
        return resolvedResultPromise(IInteractive::Result(badBtn));
    }));

    EXPECT_CALL(*m_service, submitReview(ConvertType::Audio2Score, 7, ReviewRating::Bad, QString()))
    .Times(1);

    // [WHEN] The service requests a review for a finished conversion
    reviewRequested.send(ConvertType::Audio2Score, 7);

    pumpEvents();
}

// ==================================================
// config()
// ==================================================

TEST_F(Project_ConvertFileToScoreScenarioTest, Config_DelegatesToService)
{
    // [GIVEN] A service-provided config
    m_config.omr.maxImages = 42;

    // [WHEN] Reading the scenario's config
    // [THEN] It is the same object the service returns
    EXPECT_EQ(&m_scenario->config(), &m_config);
    EXPECT_EQ(m_scenario->config().omr.maxImages, 42);
}

// ==================================================
// checkConvertIsAllowed()
// ==================================================

TEST_F(Project_ConvertFileToScoreScenarioTest, CheckConvertIsAllowed_ShowsError_WhenCloudUnavailable)
{
    // [GIVEN] The cloud is unavailable
    ON_CALL(*m_authorization, checkCloudIsAvailableAsync())
    .WillByDefault(Invoke([] {
        return resolvedPromise<Ret>(Ret(false));
    }));

    const std::string title = muse::trc("project/convert", "Unable to connect to MuseScore.com");
    const std::string text = muse::trc("project/convert",
                                       "An internet connection is required to convert a file. Please check your internet connection or try again later.");

    // [THEN] The "cloud unavailable" error is shown
    EXPECT_CALL(*m_interactive,
                error(title, TextIs(text), ButtonIdsAre({ int(IInteractive::Button::Ok) }), int(IInteractive::Button::NoButton),
                      IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
    .Times(1)
    .WillOnce(Invoke([](auto&&...) {
        return resolvedResultPromise();
    }));

    bool resolved = false;
    Ret capturedRet;

    // [WHEN] Checking whether converting is allowed
    m_scenario->checkConvertIsAllowed().onResolve(nullptr, [&](const Ret& ret) {
        resolved = true;
        capturedRet = ret;
    });

    pumpEvents();

    // [THEN] It resolves with a failure
    EXPECT_TRUE(resolved);
    EXPECT_FALSE(capturedRet);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, CheckConvertIsAllowed_Resolves_WhenAlreadyAuthorized)
{
    // [GIVEN] The cloud is available and the user is already authorized (default SetUp state)

    // [THEN] No authorization dialog is shown
    EXPECT_CALL(*m_interactive, open(_)).Times(0);

    bool resolved = false;
    Ret capturedRet;

    // [WHEN] Checking whether converting is allowed
    m_scenario->checkConvertIsAllowed().onResolve(nullptr, [&](const Ret& ret) {
        resolved = true;
        capturedRet = ret;
    });

    pumpEvents();

    // [THEN] It resolves successfully
    EXPECT_TRUE(resolved);
    EXPECT_TRUE(capturedRet);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, CheckConvertIsAllowed_PromptsAuthorization_WhenNotAuthorized)
{
    // [GIVEN] The cloud is available but the user isn't authorized
    ON_CALL(*m_authorization, userAuthorized())
    .WillByDefault(Return(ValCh<bool> { false, {} }));

    const std::string dialogText = muse::trc("project/convert", "Log in or create a free account on MuseScore.com to convert a file.");

    UriQuery expectedQuery("muse://cloud/requireauthorization");
    expectedQuery.addParam("text", Val(dialogText));
    expectedQuery.addParam("cloudCode", Val(QString()));
    expectedQuery.addParam("publishingScore", Val(false));

    // [THEN] The authorization dialog is opened, and the user completes it
    EXPECT_CALL(*m_interactive, open(expectedQuery))
    .Times(1)
    .WillOnce(Invoke([](auto&&...) {
        return resolvedValPromise();
    }));

    bool resolved = false;
    Ret capturedRet;

    // [WHEN] Checking whether converting is allowed
    m_scenario->checkConvertIsAllowed().onResolve(nullptr, [&](const Ret& ret) {
        resolved = true;
        capturedRet = ret;
    });

    pumpEvents();

    // [THEN] It resolves successfully
    EXPECT_TRUE(resolved);
    EXPECT_TRUE(capturedRet);
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
    EXPECT_CALL(*m_interactive, error(_, _, _, _, _, _)).Times(0);

    // [WHEN] Validating the files
    RetVal<ConvertFilesValidation> result = m_scenario->validateFiles(paths);

    // [THEN] The validation result is returned as-is
    EXPECT_TRUE(result.ret);
    EXPECT_EQ(result.val.type, ConvertType::Omr);
    EXPECT_EQ(result.val.category, FileCategory::Pdf);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateFiles_ValidationFailedCode_NoDialog)
{
    // [GIVEN] The service reports a generic validation failure (e.g. no files selected)
    const io::paths_t paths {};
    ON_CALL(*m_service, validateFiles(paths))
    .WillByDefault(Return(RetVal<ConvertFilesValidation>::make_ret(make_ret(Err::ConvertValidationFailed))));

    // [THEN] No dialog is shown for this code, since it's not routed to any specific one
    EXPECT_CALL(*m_interactive, error(_, _, _, _, _, _)).Times(0);

    // [WHEN] Validating the files
    RetVal<ConvertFilesValidation> result = m_scenario->validateFiles(paths);

    // [THEN] Validation fails
    EXPECT_FALSE(result.ret);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateFiles_UnsupportedFormat_ShowsDialog)
{
    expectValidateFilesShowsError(Err::ConvertUnsupportedFormat,
                                  muse::trc("project/convert", "This file type is not compatible"),
                                  muse::trc("project/convert", "Make sure you’re importing a suitable PDF, image or MP3 file."));
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateFiles_MixedFileTypes_ShowsDialog)
{
    expectValidateFilesShowsError(Err::ConvertMixedFileTypes,
                                  muse::trc("project/convert", "Please select files of the same type"),
                                  muse::trc("project/convert",
                                            "Per conversion, you may select either one MP3 file, one PDF file, or multiple image files."));
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
    m_config.audio2score.maxFileSizeBytes = 30LL * 1024 * 1024;
    const QString size = DataFormatter::formatFileSize(size_t(m_config.audio2score.maxFileSizeBytes));
    const std::string text = muse::qtrc("project/convert", "The maximum file size is %1. Reduce the size of your file and try again.")
                             .arg(size).toStdString();

    expectValidateFilesShowsError(Err::ConvertAudioFileTooLarge, muse::trc("project/convert", "This file is too large"), text);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateFiles_FileTooLarge_ShowsDialog)
{
    // [GIVEN] A configured maximum OMR file size
    m_config.omr.maxFileSizeBytes = 30LL * 1024 * 1024;
    const QString size = DataFormatter::formatFileSize(size_t(m_config.omr.maxFileSizeBytes));
    const std::string text = muse::qtrc("project/convert", "The maximum file size is %1. Reduce the size of your file and try again.")
                             .arg(size).toStdString();

    expectValidateFilesShowsError(Err::ConvertFileTooLarge, muse::trc("project/convert", "This file is too large"), text);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateFiles_CombinedImageTooLarge_ShowsDialog)
{
    // [GIVEN] A configured maximum combined image size
    m_config.omr.maxFileSizeBytes = 30LL * 1024 * 1024;
    const QString size = DataFormatter::formatFileSize(size_t(m_config.omr.maxFileSizeBytes));
    const std::string text = muse::qtrc("project/convert",
                                        "The maximum combined file size for all images is %1. Choose a smaller file or remove some images to continue.")
                             .arg(size).toStdString();

    expectValidateFilesShowsError(Err::ConvertCombinedImageTooLarge, muse::trc("project/convert", "Maximum file size exceeded"), text);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateFiles_TooManyAudioFiles_ShowsDialog)
{
    // [GIVEN] A configured maximum audio file count
    m_config.audio2score.maxFiles = 3;
    const std::string text = muse::qtrc("project/convert",
                                        "You can convert up to %1 audio files at a time. Remove some files and try again.")
                             .arg(m_config.audio2score.maxFiles).toStdString();

    expectValidateFilesShowsError(Err::ConvertTooManyAudioFiles, muse::trc("project/convert", "Too many files selected"), text);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateFiles_TooManyImages_ShowsDialog)
{
    // [GIVEN] A configured maximum image count
    m_config.omr.maxImages = 15;
    const std::string text = muse::qtrc("project/convert", "You can convert up to %1 images at a time. Remove some images and try again.")
                             .arg(m_config.omr.maxImages).toStdString();

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
    EXPECT_CALL(*m_interactive, error(_, _, _, _, _, _)).Times(0);

    // [WHEN] Validating the link
    Ret ret = m_scenario->validateLink(link);

    // [THEN] It succeeds
    EXPECT_TRUE(ret);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateLink_Unsupported_ShowsDialog_BothSources)
{
    // [GIVEN] Both link sources are allowed, but the service rejects the link
    m_config.audio2score.allowedLinkSources = LinkSource::YouTube | LinkSource::AudioCom;
    const QUrl link("https://link.xyz");
    ON_CALL(*m_service, validateLink(link))
    .WillByDefault(Return(make_ret(Err::ConvertUnsupportedLink)));

    const std::string title = muse::trc("project/convert", "Please use a compatible URL");
    const std::string text = muse::trc("project/convert", "Make sure you’re using a valid link from YouTube or Audio.com.");

    // [THEN] The error dialog mentions both sources
    EXPECT_CALL(*m_interactive,
                error(title, TextIs(text), ButtonIdsAre({ int(IInteractive::Button::Ok) }), int(IInteractive::Button::NoButton),
                      IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
    .Times(1)
    .WillOnce(Invoke([](auto&&...) {
        return resolvedResultPromise();
    }));

    // [WHEN] Validating the link
    Ret ret = m_scenario->validateLink(link);

    // [THEN] It fails
    EXPECT_FALSE(ret);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ValidateLink_Unsupported_ShowsDialog_UnsetSourcesFallsBackToBoth)
{
    // [GIVEN] No link sources are configured, and the service rejects the link
    m_config.audio2score.allowedLinkSources = LinkSources();
    const QUrl link("https://link.xyz");
    ON_CALL(*m_service, validateLink(link))
    .WillByDefault(Return(make_ret(Err::ConvertUnsupportedLink)));

    const std::string title = muse::trc("project/convert", "Please use a compatible URL");
    const std::string text = muse::trc("project/convert", "Make sure you’re using a valid link from YouTube or Audio.com.");

    // [THEN] The error dialog falls back to mentioning both sources
    EXPECT_CALL(*m_interactive,
                error(title, TextIs(text), ButtonIdsAre({ int(IInteractive::Button::Ok) }), int(IInteractive::Button::NoButton),
                      IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
    .Times(1)
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
    m_config.audio2score.allowedLinkSources = LinkSource::YouTube;
    const QUrl link("https://link.xyz");
    ON_CALL(*m_service, validateLink(link))
    .WillByDefault(Return(make_ret(Err::ConvertUnsupportedLink)));

    const std::string title = muse::trc("project/convert", "Please use a compatible URL");
    const std::string text = muse::trc("project/convert", "Make sure you’re using a valid link from YouTube.");

    // [THEN] The error dialog mentions YouTube only
    EXPECT_CALL(*m_interactive,
                error(title, TextIs(text), ButtonIdsAre({ int(IInteractive::Button::Ok) }), int(IInteractive::Button::NoButton),
                      IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
    .Times(1)
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
    m_config.audio2score.allowedLinkSources = LinkSource::AudioCom;
    const QUrl link("https://link.xyz");
    ON_CALL(*m_service, validateLink(link))
    .WillByDefault(Return(make_ret(Err::ConvertUnsupportedLink)));

    const std::string title = muse::trc("project/convert", "Please use a compatible URL");
    const std::string text = muse::trc("project/convert", "Make sure you’re using a valid link from Audio.com.");

    // [THEN] The error dialog mentions Audio.com only
    EXPECT_CALL(*m_interactive,
                error(title, TextIs(text), ButtonIdsAre({ int(IInteractive::Button::Ok) }), int(IInteractive::Button::NoButton),
                      IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
    .Times(1)
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

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_StopsWhenNotAllowed)
{
    // [GIVEN] The cloud is unavailable
    ON_CALL(*m_authorization, checkCloudIsAvailableAsync())
    .WillByDefault(Invoke([] {
        return resolvedPromise<Ret>(Ret(false));
    }));

    const std::string title = muse::trc("project/convert", "Unable to connect to MuseScore.com");
    const std::string text = muse::trc("project/convert",
                                       "An internet connection is required to convert a file. Please check your internet connection or try again later.");

    // [THEN] The "cloud unavailable" error is shown, but neither validation, confirmation, nor conversion is attempted
    EXPECT_CALL(*m_interactive,
                error(title, TextIs(text), ButtonIdsAre({ int(IInteractive::Button::Ok) }), int(IInteractive::Button::NoButton),
                      IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
    .Times(1)
    .WillOnce(Invoke([](auto&&...) {
        return resolvedResultPromise();
    }));
    EXPECT_CALL(*m_service, validateFiles(_)).Times(0);
    EXPECT_CALL(*m_interactive, question(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(*m_service, startConvert(_, _)).Times(0);

    // [WHEN] Converting files
    m_scenario->convertFiles(io::paths_t { "/some/file.xyz" });

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_StopsWhenValidationFails)
{
    // [GIVEN] The service rejects the files
    const io::paths_t paths { "/some/file.xyz" };
    ON_CALL(*m_service, validateFiles(paths))
    .WillByDefault(Return(RetVal<ConvertFilesValidation>::make_ret(make_ret(Err::ConvertUnsupportedFormat))));

    const std::string title = muse::trc("project/convert", "This file type is not compatible");
    const std::string text = muse::trc("project/convert", "Make sure you’re importing a suitable PDF, image or MP3 file.");

    // [THEN] The validation error is shown, but neither confirmation nor conversion is attempted
    EXPECT_CALL(*m_interactive,
                error(title, TextIs(text), ButtonIdsAre({ int(IInteractive::Button::Ok) }), int(IInteractive::Button::NoButton),
                      IInteractive::Options(IInteractive::Option::WithIcon), std::string()))
    .Times(1)
    .WillOnce(Invoke([](auto&&...) {
        return resolvedResultPromise();
    }));
    EXPECT_CALL(*m_interactive, question(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(*m_service, startConvert(_, _)).Times(0);

    // [WHEN] Converting files
    m_scenario->convertFiles(paths);

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_Proceeds_StartsOmrConvert)
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

    // [THEN] The conversion is started as an OMR conversion of the given paths
    EXPECT_CALL(*m_service, startConvert(Truly([&](const ConvertInput& input) {
        return convertTypeOf(input) == ConvertType::Omr && convertPathsOf(input) == paths;
    }), QString("file")))
    .WillOnce(Return(make_ok()));

    // [WHEN] Converting files
    m_scenario->convertFiles(paths);

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_Proceeds_StartsAudio2ScoreConvert)
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

    // [THEN] The conversion is started as an Audio2Score conversion of the given paths
    EXPECT_CALL(*m_service, startConvert(Truly([&](const ConvertInput& input) {
        return convertTypeOf(input) == ConvertType::Audio2Score && convertPathsOf(input) == paths;
    }), QString("song")))
    .WillOnce(Return(make_ok()));

    // [WHEN] Converting files
    m_scenario->convertFiles(paths);

    pumpEvents();
}

TEST_F(Project_ConvertFileToScoreScenarioTest, ConvertFiles_UserCancelsConfirm_DoesNotStartConvert)
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

    // [THEN] The conversion is not started
    EXPECT_CALL(*m_service, startConvert(_, _)).Times(0);

    // [WHEN] Converting files
    m_scenario->convertFiles(paths);

    pumpEvents();
}

// ==================================================
// startConvert()
// ==================================================

TEST_F(Project_ConvertFileToScoreScenarioTest, StartConvert_ShowsProcessingDialog_WhenEnabled)
{
    // [GIVEN] The processing dialog is enabled in the config
    ON_CALL(*m_configuration, showConvertFileProcessingDialog())
    .WillByDefault(Return(true));
    ON_CALL(*m_service, startConvert(_, _))
    .WillByDefault(Return(make_ok()));

    constexpr int uploadMoreBtn = int(IInteractive::Button::CustomButton) + 1;
    constexpr int goToScoresBtn = int(IInteractive::Button::CustomButton) + 2;

    const std::string title = muse::trc("project/convert", "Your score is being processed");
    const std::string text = muse::trc("project/convert",
                                       "We’ll notify you once the score is ready to open. "
                                       "You can check the status of the score in Home > Scores.");

    // [THEN] The processing dialog is shown
    EXPECT_CALL(*m_interactive,
                info(title, TextIs(text), ButtonIdsAre({ uploadMoreBtn, goToScoresBtn, int(IInteractive::Button::Ok) }),
                     int(IInteractive::Button::Ok), IInteractive::Options(IInteractive::Option::WithDontShowAgainCheckBox),
                     std::string()))
    .Times(1)
    .WillOnce(Invoke([](auto&&...) {
        return resolvedResultPromise();
    }));

    // [WHEN] Starting the conversion
    const OmrConvertInput input { io::paths_t { "/some/file.xyz" } };
    Ret ret = m_scenario->startConvert(input, "file");

    // [THEN] It delegates to the service and succeeds
    EXPECT_TRUE(ret);
}

TEST_F(Project_ConvertFileToScoreScenarioTest, StartConvert_NoProcessingDialog_WhenDisabled)
{
    // [GIVEN] The processing dialog is disabled in the config
    ON_CALL(*m_configuration, showConvertFileProcessingDialog())
    .WillByDefault(Return(false));
    ON_CALL(*m_service, startConvert(_, _))
    .WillByDefault(Return(make_ok()));

    // [THEN] No processing dialog is shown
    EXPECT_CALL(*m_interactive, info(_, _, _, _, _, _)).Times(0);

    // [WHEN] Starting the conversion
    const OmrConvertInput input { io::paths_t { "/some/file.xyz" } };
    m_scenario->startConvert(input, "file");
}
