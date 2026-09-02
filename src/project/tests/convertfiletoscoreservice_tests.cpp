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

#include "project/internal/convertfiletoscoreservice.h"
#include "project/projecterrors.h"

#include "cloud/clouderrors.h"

#include "global/async/processevents.h"
#include "global/modularity/ioc.h"
#include "global/progress.h"
#include "global/types/val.h"
#include "global/types/bytearray.h"
#include "global/serialization/json.h"

#include "mocks/projectconfigurationmock.h"
#include "global/tests/mocks/filesystemmock.h"
#include "cloud/tests/mocks/musescorecomservicemock.h"
#include "cloud/tests/mocks/musescorecomconvertservicemock.h"

using namespace ::testing;
using namespace mu::project;
using namespace muse;
using namespace muse::cloud;

namespace {
constexpr int TEST_QUEUE_ID = 42;

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

//! NOTE: the hardcoded values ConvertFileToScoreService::init() falls back to
void expectFallbackConfig(const ConvertConfig& config)
{
    EXPECT_EQ(config.omr.allowedExtensions, QStringList({ "pdf", "png", "jpg", "jpeg" }));
    EXPECT_EQ(config.omr.maxFileSizeBytes, 30LL * 1024 * 1024);
    EXPECT_EQ(config.omr.maxPages, 50);
    EXPECT_EQ(config.omr.maxImages, 15);
    EXPECT_EQ(config.audio2score.allowedExtensions, QStringList({ "mp3" }));
    EXPECT_EQ(config.audio2score.maxFileSizeBytes, 30LL * 1024 * 1024);
    EXPECT_EQ(config.audio2score.maxFiles, 1);
    EXPECT_EQ(config.audio2score.maxLinkLength, 2048);
    EXPECT_TRUE(config.audio2score.allowedLinkSources.testFlag(LinkSource::YouTube));
    EXPECT_TRUE(config.audio2score.allowedLinkSources.testFlag(LinkSource::AudioCom));
}

//! NOTE: a promise that never resolves, for tests that don't care about the outcome
template<typename T>
async::Promise<T> pendingPromise()
{
    return async::make_promise<T>([](auto resolve, auto reject) {
        (void)resolve;
        (void)reject;
        return async::Promise<T>::dummy_result();
    });
}
}

namespace mu::project {
class Project_ConvertFileToScoreServiceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_service = std::make_shared<ConvertFileToScoreService>(muse::modularity::globalCtx());

        m_museScoreComService = std::make_shared<NiceMock<MuseScoreComServiceMock> >();
        m_convertService = std::make_shared<NiceMock<MuseScoreComConvertServiceMock> >();
        m_fileSystem = std::make_shared<NiceMock<muse::io::FileSystemMock> >();
        m_configuration = std::make_shared<NiceMock<ProjectConfigurationMock> >();

        m_service->museScoreComService.set(m_museScoreComService);
        m_service->fileSystem.set(m_fileSystem);
        m_service->configuration.set(m_configuration);

        ON_CALL(*m_museScoreComService, convert())
        .WillByDefault(Return(m_convertService));

        ON_CALL(*m_fileSystem, fileSize(_))
        .WillByDefault(Return(RetVal<uint64_t>::make_ok(1024)));
    }

    void setConfig(const ConvertConfig& config)
    {
        ON_CALL(*m_convertService, fetchConfig())
        .WillByDefault(Invoke([config] {
            return resolvedPromise<RetVal<ConvertConfig> >(RetVal<ConvertConfig>::make_ok(config));
        }));

        m_service->init();
        pumpEvents();
    }

    static ConvertConfig testConfig()
    {
        ConvertConfig config;
        config.omr.allowedExtensions = { "pdf", "png", "jpg", "jpeg" };
        config.audio2score.allowedExtensions = { "mp3" };
        return config;
    }

    //! NOTE: uploads the given file, resolves the upload with queueId, and lets the resulting
    //! poll (mocked to return queueList) run to completion
    void deliverQueueStatus(const ConvertQueueList& queueList, ConvertType type, int queueId, const QString& convertedFileName)
    {
        ON_CALL(*m_convertService, fetchQueue())
        .WillByDefault(Invoke([queueList] {
            return resolvedPromise<RetVal<ConvertQueueList> >(RetVal<ConvertQueueList>::make_ok(queueList));
        }));

        auto uploadProgress = std::make_shared<Progress>();
        ON_CALL(*m_convertService, upload(_))
        .WillByDefault(Return(uploadProgress));

        const io::paths_t paths { "/some/path/file.pdf" };
        const ConvertInput input = type == ConvertType::Omr
                                   ? ConvertInput(OmrConvertInput { paths })
                                   : ConvertInput(Audio2ScoreConvertInput { paths });

        m_service->startConvert(input, convertedFileName);

        uploadProgress->finish(ProgressResult::make_ok(Val(ValMap { { "id", Val(queueId) } })));
        pumpEvents();
    }

    std::shared_ptr<ConvertFileToScoreService> m_service;
    std::shared_ptr<MuseScoreComServiceMock> m_museScoreComService;
    std::shared_ptr<MuseScoreComConvertServiceMock> m_convertService;
    std::shared_ptr<muse::io::FileSystemMock> m_fileSystem;
    std::shared_ptr<ProjectConfigurationMock> m_configuration;
};
}

// ==================================================
// init()
// ==================================================

TEST_F(Project_ConvertFileToScoreServiceTest, Init_SetsFallbackConfig_BeforeFetchResolves)
{
    // [GIVEN] fetchConfig() that hasn't resolved yet
    ON_CALL(*m_convertService, fetchConfig())
    .WillByDefault(Invoke([] {
        return pendingPromise<RetVal<ConvertConfig> >();
    }));

    // [WHEN] Initializing the service
    m_service->init();

    // [THEN] The config is already usable, with the hardcoded fallback values
    expectFallbackConfig(m_service->config());
}

TEST_F(Project_ConvertFileToScoreServiceTest, Init_FetchConfigSucceeds_OverwritesFallbackConfig)
{
    // [GIVEN] fetchConfig() resolving with a server-provided config, distinct in every field from the fallback
    ConvertConfig serverConfig;
    serverConfig.omr.allowedExtensions = { "tif" };
    serverConfig.omr.maxFileSizeBytes = 111;
    serverConfig.omr.maxPages = 7;
    serverConfig.omr.maxImages = 99;
    serverConfig.audio2score.allowedExtensions = { "wav" };
    serverConfig.audio2score.maxFileSizeBytes = 222;
    serverConfig.audio2score.maxFiles = 3;
    serverConfig.audio2score.maxLinkLength = 333;
    serverConfig.audio2score.allowedLinkSources = LinkSource::AudioCom;
    ON_CALL(*m_convertService, fetchConfig())
    .WillByDefault(Invoke([serverConfig] {
        return resolvedPromise<RetVal<ConvertConfig> >(RetVal<ConvertConfig>::make_ok(serverConfig));
    }));

    // [WHEN] Initializing the service
    m_service->init();
    pumpEvents();

    // [THEN] The fetched config fully replaces the fallback
    const ConvertConfig& config = m_service->config();
    EXPECT_EQ(config.omr.allowedExtensions, QStringList({ "tif" }));
    EXPECT_EQ(config.omr.maxFileSizeBytes, 111);
    EXPECT_EQ(config.omr.maxPages, 7);
    EXPECT_EQ(config.omr.maxImages, 99);
    EXPECT_EQ(config.audio2score.allowedExtensions, QStringList({ "wav" }));
    EXPECT_EQ(config.audio2score.maxFileSizeBytes, 222);
    EXPECT_EQ(config.audio2score.maxFiles, 3);
    EXPECT_EQ(config.audio2score.maxLinkLength, 333);
    EXPECT_TRUE(config.audio2score.allowedLinkSources.testFlag(LinkSource::AudioCom));
    EXPECT_FALSE(config.audio2score.allowedLinkSources.testFlag(LinkSource::YouTube));
}

TEST_F(Project_ConvertFileToScoreServiceTest, Init_FetchConfigFails_KeepsFallbackConfig)
{
    // [GIVEN] fetchConfig() resolving with a failure
    ON_CALL(*m_convertService, fetchConfig())
    .WillByDefault(Invoke([] {
        return resolvedPromise<RetVal<ConvertConfig> >(RetVal<ConvertConfig>::make_ret(make_ret(muse::cloud::Err::UnknownError)));
    }));

    // [WHEN] Initializing the service
    m_service->init();
    pumpEvents();

    // [THEN] The fallback config is kept
    expectFallbackConfig(m_service->config());
}

// ==================================================
// isFileSupported()
// ==================================================

TEST_F(Project_ConvertFileToScoreServiceTest, IsFileSupported_True_ForPdf)
{
    // [GIVEN] A populated config, supporting pdf
    setConfig(testConfig());

    // [WHEN] Asking about a PDF file
    // [THEN] It is supported
    EXPECT_TRUE(m_service->isFileSupported("/some/file.pdf"));
}

TEST_F(Project_ConvertFileToScoreServiceTest, IsFileSupported_False_ForUnknownExtension)
{
    // [GIVEN] A populated config
    setConfig(testConfig());

    // [WHEN] Asking about a file with an unrecognized extension
    // [THEN] It is not supported
    EXPECT_FALSE(m_service->isFileSupported("/some/file.xyz"));
}

// ==================================================
// validateFiles()
// ==================================================

TEST_F(Project_ConvertFileToScoreServiceTest, ValidateFiles_Empty_ValidationFailed)
{
    // [GIVEN] A populated config
    setConfig(testConfig());

    // [WHEN] Validating an empty file list
    RetVal<ConvertFilesValidation> result = m_service->validateFiles(io::paths_t {});

    // [THEN] It fails with a generic validation error
    EXPECT_FALSE(result.ret);
    EXPECT_EQ(result.ret.code(), int(mu::project::Err::ConvertValidationFailed));
}

TEST_F(Project_ConvertFileToScoreServiceTest, ValidateFiles_UnknownExtension_UnsupportedFormat)
{
    // [GIVEN] A populated config
    setConfig(testConfig());

    // [WHEN] Validating a file with an unrecognized extension
    RetVal<ConvertFilesValidation> result = m_service->validateFiles(io::paths_t { "/some/file.xyz" });

    // [THEN] It fails as an unsupported format
    EXPECT_FALSE(result.ret);
    EXPECT_EQ(result.ret.code(), int(mu::project::Err::ConvertUnsupportedFormat));
}

TEST_F(Project_ConvertFileToScoreServiceTest, ValidateFiles_MixedCategories_MixedFileTypes)
{
    // [GIVEN] A populated config, supporting png and mp3
    setConfig(testConfig());

    // [WHEN] Validating an image together with an audio file
    RetVal<ConvertFilesValidation> result = m_service->validateFiles(io::paths_t { "/some/file.png", "/some/file.mp3" });

    // [THEN] It fails as mixed file types
    EXPECT_FALSE(result.ret);
    EXPECT_EQ(result.ret.code(), int(mu::project::Err::ConvertMixedFileTypes));
}

TEST_F(Project_ConvertFileToScoreServiceTest, ValidateFiles_MultiplePdfFiles_MultiplePdfFiles)
{
    // [GIVEN] A populated config, supporting pdf
    setConfig(testConfig());

    // [WHEN] Validating two PDF files
    RetVal<ConvertFilesValidation> result = m_service->validateFiles(io::paths_t { "/some/a.pdf", "/some/b.pdf" });

    // [THEN] It fails as multiple PDF files
    EXPECT_FALSE(result.ret);
    EXPECT_EQ(result.ret.code(), int(mu::project::Err::ConvertMultiplePdfFiles));
}

TEST_F(Project_ConvertFileToScoreServiceTest, ValidateFiles_Omr_Success)
{
    // [GIVEN] A populated config, supporting pdf
    setConfig(testConfig());

    // [WHEN] Validating a single PDF file
    RetVal<ConvertFilesValidation> result = m_service->validateFiles(io::paths_t { "/some/file.pdf" });

    // [THEN] It succeeds as an OMR conversion of a PDF
    EXPECT_TRUE(result.ret);
    EXPECT_EQ(result.val.type, ConvertType::Omr);
    EXPECT_EQ(result.val.category, FileCategory::Pdf);
}

TEST_F(Project_ConvertFileToScoreServiceTest, ValidateFiles_MultipleImages_Success)
{
    // [GIVEN] A populated config, supporting png and jpg
    setConfig(testConfig());

    // [WHEN] Validating multiple image files
    RetVal<ConvertFilesValidation> result = m_service->validateFiles(io::paths_t { "/some/a.png", "/some/b.jpg" });

    // [THEN] It succeeds as an OMR conversion of images
    EXPECT_TRUE(result.ret);
    EXPECT_EQ(result.val.type, ConvertType::Omr);
    EXPECT_EQ(result.val.category, FileCategory::Image);
}

TEST_F(Project_ConvertFileToScoreServiceTest, ValidateFiles_Audio2Score_Success)
{
    // [GIVEN] A populated config, supporting mp3
    setConfig(testConfig());

    // [WHEN] Validating a single audio file
    RetVal<ConvertFilesValidation> result = m_service->validateFiles(io::paths_t { "/some/file.mp3" });

    // [THEN] It succeeds as an Audio2Score conversion
    EXPECT_TRUE(result.ret);
    EXPECT_EQ(result.val.type, ConvertType::Audio2Score);
    EXPECT_EQ(result.val.category, FileCategory::Audio);
}

TEST_F(Project_ConvertFileToScoreServiceTest, ValidateFiles_AudioFileTooLarge_ShowsExpectedError)
{
    // [GIVEN] A configured maximum audio file size, and a file exceeding it
    ConvertConfig config = testConfig();
    config.audio2score.maxFileSizeBytes = 100;
    setConfig(config);

    ON_CALL(*m_fileSystem, fileSize(io::path_t("/some/file.mp3")))
    .WillByDefault(Return(RetVal<uint64_t>::make_ok(200)));

    // [WHEN] Validating the file
    RetVal<ConvertFilesValidation> result = m_service->validateFiles(io::paths_t { "/some/file.mp3" });

    // [THEN] It fails as too large
    EXPECT_FALSE(result.ret);
    EXPECT_EQ(result.ret.code(), int(mu::project::Err::ConvertAudioFileTooLarge));
}

TEST_F(Project_ConvertFileToScoreServiceTest, ValidateFiles_TooManyAudioFiles_TooManyAudioFiles)
{
    // [GIVEN] A configured maximum audio file count
    ConvertConfig config = testConfig();
    config.audio2score.maxFiles = 1;
    setConfig(config);

    // [WHEN] Validating more files than allowed
    RetVal<ConvertFilesValidation> result = m_service->validateFiles(io::paths_t { "/a.mp3", "/b.mp3" });

    // [THEN] It fails as too many audio files
    EXPECT_FALSE(result.ret);
    EXPECT_EQ(result.ret.code(), int(mu::project::Err::ConvertTooManyAudioFiles));
}

TEST_F(Project_ConvertFileToScoreServiceTest, ValidateFiles_TooManyImages_TooManyImages)
{
    // [GIVEN] A configured maximum image count
    ConvertConfig config = testConfig();
    config.omr.maxImages = 2;
    setConfig(config);

    // [WHEN] Validating more images than allowed
    RetVal<ConvertFilesValidation> result = m_service->validateFiles(io::paths_t { "/a.png", "/b.png", "/c.png" });

    // [THEN] It fails as too many images
    EXPECT_FALSE(result.ret);
    EXPECT_EQ(result.ret.code(), int(mu::project::Err::ConvertTooManyImages));
}

TEST_F(Project_ConvertFileToScoreServiceTest, ValidateFiles_CombinedImageTooLarge_CombinedImageTooLarge)
{
    // [GIVEN] A configured maximum combined image size, exceeded by the combined file sizes
    ConvertConfig config = testConfig();
    config.omr.maxFileSizeBytes = 100;
    setConfig(config);

    ON_CALL(*m_fileSystem, fileSize(_))
    .WillByDefault(Return(RetVal<uint64_t>::make_ok(60)));

    // [WHEN] Validating two images
    RetVal<ConvertFilesValidation> result = m_service->validateFiles(io::paths_t { "/a.png", "/b.png" });

    // [THEN] It fails as too large, combined
    EXPECT_FALSE(result.ret);
    EXPECT_EQ(result.ret.code(), int(mu::project::Err::ConvertCombinedImageTooLarge));
}

TEST_F(Project_ConvertFileToScoreServiceTest, ValidateFiles_PdfTooLarge_FileTooLarge)
{
    // [GIVEN] A configured maximum OMR file size, and a PDF exceeding it
    ConvertConfig config = testConfig();
    config.omr.maxFileSizeBytes = 100;
    setConfig(config);

    ON_CALL(*m_fileSystem, fileSize(_))
    .WillByDefault(Return(RetVal<uint64_t>::make_ok(200)));

    // [WHEN] Validating the file
    RetVal<ConvertFilesValidation> result = m_service->validateFiles(io::paths_t { "/a.pdf" });

    // [THEN] It fails as too large
    EXPECT_FALSE(result.ret);
    EXPECT_EQ(result.ret.code(), int(mu::project::Err::ConvertFileTooLarge));
}

// ==================================================
// validateLink()
// ==================================================

TEST_F(Project_ConvertFileToScoreServiceTest, ValidateLink_YouTube_DefaultConfig_Success)
{
    // [WHEN] Validating a YouTube link, with no config fetched yet
    Ret ret = m_service->validateLink(QUrl("https://youtube.com/x"));

    // [THEN] It succeeds, since both sources are allowed by default
    EXPECT_TRUE(ret);
}

TEST_F(Project_ConvertFileToScoreServiceTest, ValidateLink_AudioCom_DefaultConfig_Success)
{
    // [WHEN] Validating an Audio.com link, with no config fetched yet
    Ret ret = m_service->validateLink(QUrl("https://audio.com/x"));

    // [THEN] It succeeds, since both sources are allowed by default
    EXPECT_TRUE(ret);
}

TEST_F(Project_ConvertFileToScoreServiceTest, ValidateLink_UnsupportedHost_Fails)
{
    // [WHEN] Validating a link from an unsupported host
    Ret ret = m_service->validateLink(QUrl("https://example.com/x"));

    // [THEN] It fails
    EXPECT_FALSE(ret);
    EXPECT_EQ(ret.code(), int(mu::project::Err::ConvertUnsupportedLink));
}

TEST_F(Project_ConvertFileToScoreServiceTest, ValidateLink_YouTubeOnlyConfig_RejectsAudioCom)
{
    // [GIVEN] A config only allowing YouTube links
    ConvertConfig config;
    config.audio2score.allowedLinkSources = LinkSource::YouTube;
    setConfig(config);

    // [WHEN] Validating an Audio.com link
    Ret ret = m_service->validateLink(QUrl("https://audio.com/x"));

    // [THEN] It fails
    EXPECT_FALSE(ret);
}

TEST_F(Project_ConvertFileToScoreServiceTest, ValidateLink_AudioComOnlyConfig_RejectsYouTube)
{
    // [GIVEN] A config only allowing Audio.com links
    ConvertConfig config;
    config.audio2score.allowedLinkSources = LinkSource::AudioCom;
    setConfig(config);

    // [WHEN] Validating a YouTube link
    Ret ret = m_service->validateLink(QUrl("https://youtube.com/x"));

    // [THEN] It fails
    EXPECT_FALSE(ret);
}

// ==================================================
// startConvert()
// ==================================================

TEST_F(Project_ConvertFileToScoreServiceTest, StartConvert_UploadFails_ForwardsFailureWithFileName)
{
    // [GIVEN] The upload fails
    auto uploadProgress = std::make_shared<Progress>();
    ON_CALL(*m_convertService, upload(_))
    .WillByDefault(Return(uploadProgress));

    bool received = false;
    Ret receivedRet;
    m_service->convertFinished().onReceive(nullptr, [&](const Ret& ret, const io::path_t&) {
        received = true;
        receivedRet = ret;
    });

    // [WHEN] Starting the conversion
    const io::paths_t paths { "/some/path/file.pdf" };
    Ret ret = m_service->startConvert(OmrConvertInput { paths }, "My Score");
    EXPECT_TRUE(ret);

    // [THEN] The failure is forwarded synchronously, carrying the intended file name
    uploadProgress->finish(make_ret(Ret::Code::UnknownError, std::string("network error")));
    ASSERT_TRUE(received);
    EXPECT_FALSE(receivedRet);
    EXPECT_EQ(receivedRet.data<QString>(CONVERT_FAILED_FILE_NAME_KEY, QString()), QString("My Score"));
}

TEST_F(Project_ConvertFileToScoreServiceTest, StartConvert_UploadSucceeds_PersistsWatchedItemAndPolls)
{
    // [GIVEN] The upload succeeds with queue id TEST_QUEUE_ID
    ON_CALL(*m_configuration, pendingConvertsJsonPath())
    .WillByDefault(Return(io::path_t("/pending.json")));

    auto uploadProgress = std::make_shared<Progress>();
    const io::paths_t paths { "/some/path/file.pdf" };

    EXPECT_CALL(*m_convertService, upload(Truly([&](const ConvertInput& input) {
        return convertTypeOf(input) == ConvertType::Omr && convertPathsOf(input) == paths;
    })))
    .WillOnce(Return(uploadProgress));

    // [THEN] Polling begins, and the watched item is persisted with its id, type and file name
    EXPECT_CALL(*m_convertService, fetchQueue())
    .Times(1)
    .WillOnce(Invoke([] {
        return pendingPromise<RetVal<ConvertQueueList> >();
    }));

    bool savedExpectedEntry = false;
    EXPECT_CALL(*m_fileSystem, writeFile(io::path_t("/pending.json"), _))
    .WillOnce(Invoke([&](const io::path_t&, const ByteArray& data) {
        std::string err;
        JsonDocument json = JsonDocument::fromJson(data, &err);
        if (json.isArray() && json.rootArray().size() == 1) {
            JsonObject obj = json.rootArray().at(0).toObject();
            savedExpectedEntry = obj.value("id").toInt() == TEST_QUEUE_ID
                                 && obj.value("type").toInt() == int(ConvertType::Omr)
                                 && obj.value("convertedFileName").toStdString() == "My Score";
        }
        return make_ok();
    }));

    // [WHEN] Starting the conversion
    Ret ret = m_service->startConvert(OmrConvertInput { paths }, "My Score");
    EXPECT_TRUE(ret);

    uploadProgress->finish(ProgressResult::make_ok(Val(ValMap { { "id", Val(TEST_QUEUE_ID) } })));

    EXPECT_TRUE(savedExpectedEntry);
}

// ==================================================
// polling / download pipeline (via startConvert())
// ==================================================

TEST_F(Project_ConvertFileToScoreServiceTest, Poll_DoneStatus_DownloadsAndFinishesWithPath)
{
    // [GIVEN] The queue reports the conversion as done
    ConvertQueueItem item;
    item.id = TEST_QUEUE_ID;
    item.type = ConvertType::Omr;
    item.status = ConvertStatus::Done;

    ON_CALL(*m_convertService, fetchMsczUrl(ConvertType::Omr, TEST_QUEUE_ID))
    .WillByDefault(Invoke([] {
        SignedMsczUrl url;
        url.id = TEST_QUEUE_ID;
        url.type = ConvertType::Omr;
        url.url = QUrl("https://link.xyz/score.mscz");
        url.expiresInSeconds = 60;
        return resolvedPromise<RetVal<SignedMsczUrl> >(RetVal<SignedMsczUrl>::make_ok(url));
    }));

    auto downloadProgress = std::make_shared<Progress>();
    ON_CALL(*m_convertService, downloadConvertedScore(_, _))
    .WillByDefault(Return(downloadProgress));

    ON_CALL(*m_fileSystem, makePath(_))
    .WillByDefault(Return(make_ok()));
    ON_CALL(*m_fileSystem, writeFile(_, _))
    .WillByDefault(Return(make_ok()));
    ON_CALL(*m_configuration, convertedScoresPath())
    .WillByDefault(Return(io::path_t("/scores")));
    ON_CALL(*m_configuration, uniqueFileNameAddition(_, _, _))
    .WillByDefault(Return(std::string()));

    bool received = false;
    Ret receivedRet;
    io::path_t receivedPath;
    m_service->convertFinished().onReceive(nullptr, [&](const Ret& ret, const io::path_t& path) {
        received = true;
        receivedRet = ret;
        receivedPath = path;
    });

    // [WHEN] Uploading and polling the status
    deliverQueueStatus({ item }, ConvertType::Omr, TEST_QUEUE_ID, "My Score");

    // [AND WHEN] The download completes
    downloadProgress->finish(ProgressResult::make_ok(Val()));

    // [THEN] The conversion finishes successfully, with the downloaded score's path
    ASSERT_TRUE(received);
    EXPECT_TRUE(receivedRet);
    EXPECT_TRUE(receivedPath.hasSuffix("mscz"));
    EXPECT_NE(receivedPath.toStdString().find("scores"), std::string::npos);
}

TEST_F(Project_ConvertFileToScoreServiceTest, Poll_AwaitingReview_EmitsReviewRequestedAndDownloads)
{
    // [GIVEN] The queue reports the conversion as awaiting review
    ConvertQueueItem item;
    item.id = TEST_QUEUE_ID;
    item.type = ConvertType::Omr;
    item.status = ConvertStatus::AwaitingReview;

    // [THEN] The score is downloaded even though the rating hasn't been submitted yet
    EXPECT_CALL(*m_convertService, fetchMsczUrl(ConvertType::Omr, TEST_QUEUE_ID))
    .Times(1)
    .WillOnce(Invoke([] {
        return pendingPromise<RetVal<SignedMsczUrl> >();
    }));

    bool reviewRequested = false;
    ConvertType reviewType = ConvertType::Audio2Score;
    int reviewQueueId = 0;
    m_service->reviewRequested().onReceive(nullptr, [&](ConvertType type, int queueId) {
        reviewRequested = true;
        reviewType = type;
        reviewQueueId = queueId;
    });

    // [WHEN] Uploading and polling the status
    deliverQueueStatus({ item }, ConvertType::Omr, TEST_QUEUE_ID, "My Score");

    // [THEN] A review is requested for the finished conversion
    ASSERT_TRUE(reviewRequested);
    EXPECT_EQ(reviewType, ConvertType::Omr);
    EXPECT_EQ(reviewQueueId, TEST_QUEUE_ID);
}

TEST_F(Project_ConvertFileToScoreServiceTest, Poll_ItemDroppedFromQueue_TreatedAsDoneAndDownloads)
{
    // [THEN] A watched item that disappears from the queue is treated the same as "Done"
    EXPECT_CALL(*m_convertService, fetchMsczUrl(ConvertType::Omr, TEST_QUEUE_ID))
    .Times(1)
    .WillOnce(Invoke([] {
        return pendingPromise<RetVal<SignedMsczUrl> >();
    }));

    // [WHEN] Uploading, then polling an empty queue
    deliverQueueStatus({}, ConvertType::Omr, TEST_QUEUE_ID, "My Score");
}

TEST_F(Project_ConvertFileToScoreServiceTest, Poll_FailedStatus_ForwardsProcessingFailure)
{
    // [GIVEN] The queue reports the conversion as failed
    ConvertQueueItem item;
    item.id = TEST_QUEUE_ID;
    item.type = ConvertType::Omr;
    item.status = ConvertStatus::Failed;
    item.errorCode = ConvertErrorCode::FileTooLarge;

    bool received = false;
    Ret receivedRet;
    m_service->convertFinished().onReceive(nullptr, [&](const Ret& ret, const io::path_t&) {
        received = true;
        receivedRet = ret;
    });

    // [WHEN] Uploading and polling the status
    deliverQueueStatus({ item }, ConvertType::Omr, TEST_QUEUE_ID, "My Score");

    // [THEN] The failure is forwarded, carrying the intended file name
    ASSERT_TRUE(received);
    EXPECT_FALSE(receivedRet);
    EXPECT_EQ(receivedRet.code(), int(mu::project::Err::ConvertProcessingFailed));
    EXPECT_EQ(receivedRet.data<QString>(CONVERT_FAILED_FILE_NAME_KEY, QString()), QString("My Score"));
}

TEST_F(Project_ConvertFileToScoreServiceTest, Poll_ExpiredDownloadLink_ForwardsFailure)
{
    // [GIVEN] The score is done, but its download link has already expired
    ConvertQueueItem item;
    item.id = TEST_QUEUE_ID;
    item.type = ConvertType::Omr;
    item.status = ConvertStatus::Done;

    ON_CALL(*m_convertService, fetchMsczUrl(ConvertType::Omr, TEST_QUEUE_ID))
    .WillByDefault(Invoke([] {
        SignedMsczUrl url;
        url.id = TEST_QUEUE_ID;
        url.type = ConvertType::Omr;
        url.url = QUrl("https://link.xyz/score.mscz");
        url.expiresInSeconds = 0;
        return resolvedPromise<RetVal<SignedMsczUrl> >(RetVal<SignedMsczUrl>::make_ok(url));
    }));

    // [THEN] No download is attempted
    EXPECT_CALL(*m_convertService, downloadConvertedScore(_, _)).Times(0);

    bool received = false;
    Ret receivedRet;
    m_service->convertFinished().onReceive(nullptr, [&](const Ret& ret, const io::path_t&) {
        received = true;
        receivedRet = ret;
    });

    // [WHEN] Uploading and polling the status
    deliverQueueStatus({ item }, ConvertType::Omr, TEST_QUEUE_ID, "My Score");

    // [THEN] The failure is forwarded as an expired download link
    ASSERT_TRUE(received);
    EXPECT_FALSE(receivedRet);
    EXPECT_EQ(receivedRet.code(), int(mu::project::Err::DownloadLinkExpired));
}

// ==================================================
// submitReview() / submitReviewComment()
// ==================================================

TEST_F(Project_ConvertFileToScoreServiceTest, SubmitReview_Good_DelegatesToConvertService)
{
    // [THEN] The rating is delegated to the convert service
    EXPECT_CALL(*m_convertService, submitReview(ConvertType::Omr, TEST_QUEUE_ID, ReviewRating::Good, QString()))
    .WillOnce(Invoke([](auto, auto, auto, auto) {
        return resolvedPromise<RetVal<ConvertResult> >(RetVal<ConvertResult>::make_ok(ConvertResult {}));
    }));

    // [WHEN] Submitting a "Good" review with no comment
    m_service->submitReview(ConvertType::Omr, TEST_QUEUE_ID, ReviewRating::Good);
}

TEST_F(Project_ConvertFileToScoreServiceTest, SubmitReview_BadWithComment_DelegatesToConvertService)
{
    // [THEN] The rating and comment are delegated to the convert service
    EXPECT_CALL(*m_convertService, submitReview(ConvertType::Audio2Score, 7, ReviewRating::Bad, QString("Too many wrong notes")))
    .WillOnce(Invoke([](auto, auto, auto, auto) {
        return resolvedPromise<RetVal<ConvertResult> >(RetVal<ConvertResult>::make_ok(ConvertResult {}));
    }));

    // [WHEN] Submitting a "Bad" review with a comment
    m_service->submitReview(ConvertType::Audio2Score, 7, ReviewRating::Bad, "Too many wrong notes");
}

TEST_F(Project_ConvertFileToScoreServiceTest, SubmitReviewComment_DelegatesToConvertService)
{
    // [THEN] The comment is delegated to the convert service
    EXPECT_CALL(*m_convertService, submitReviewComment(ConvertType::Audio2Score, 7, QString("Great job")))
    .WillOnce(Invoke([](auto, auto, auto) {
        return resolvedPromise<RetVal<ConvertResult> >(RetVal<ConvertResult>::make_ok(ConvertResult {}));
    }));

    // [WHEN] Submitting a follow-up comment
    m_service->submitReviewComment(ConvertType::Audio2Score, 7, "Great job");
}
