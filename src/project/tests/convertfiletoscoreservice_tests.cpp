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

#include <algorithm>
#include <thread>
#include <vector>

#include <QUrl>

#include "project/internal/convertfiletoscoreservice.h"
#include "project/projecterrors.h"

#include "cloud/clouderrors.h"
#include "network/networkerrors.h"

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
#include "multiwindows/tests/mocks/multiwindowsprovidermock.h"

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

RetVal<ScoreInfo> okScoreInfo(int scoreId, const QString& title = "My Score")
{
    ScoreInfo info;
    info.id = scoreId;
    info.title = title;
    return RetVal<ScoreInfo>::make_ok(info);
}

//! NOTE: the hardcoded values ConvertFileToScoreService::init() falls back to
void expectFallbackConfig(const ConvertConfig& config)
{
    EXPECT_EQ(config.omr.pdf.maxFileSizeBytes, 78643200);
    EXPECT_EQ(config.omr.pdf.maxFiles, 1);
    EXPECT_EQ(config.omr.pdf.maxPages, 50);
    EXPECT_EQ(config.omr.images.allowedExtensions, QStringList({ "jpeg", "jpg", "png" }));
    EXPECT_EQ(config.omr.images.maxFileSizeBytes, 78643200);
    EXPECT_EQ(config.omr.images.maxFiles, 15);
    EXPECT_EQ(config.audio2score.file.allowedExtensions, QStringList({ "mp3" }));
    EXPECT_EQ(config.audio2score.file.maxFileSizeBytes, 52428800);
    EXPECT_EQ(config.audio2score.file.maxFiles, 1);
    EXPECT_EQ(config.audio2score.link.maxLength, 2048);
    EXPECT_TRUE(config.audio2score.link.allowedSources.testFlag(LinkSource::YouTube));
    EXPECT_TRUE(config.audio2score.link.allowedSources.testFlag(LinkSource::AudioCom));
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

bool uploadDataMatchesPaths(const ConvertUploadData& data, const io::paths_t& paths)
{
    if (data.files.size() != paths.size()) {
        return false;
    }

    for (size_t i = 0; i < paths.size(); ++i) {
        if (data.files[i].fileName != io::filename(paths[i])) {
            return false;
        }
    }

    return true;
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
        m_multiWindowsProvider = std::make_shared<NiceMock<muse::mi::MultiWindowsProviderMock> >();

        m_service->museScoreComService.set(m_museScoreComService);
        m_service->fileSystem.set(m_fileSystem);
        m_service->configuration.set(m_configuration);
        m_service->multiwindowsProvider.set(m_multiWindowsProvider);

        ON_CALL(*m_museScoreComService, convert())
        .WillByDefault(Return(m_convertService));

        ON_CALL(*m_fileSystem, fileSize(_))
        .WillByDefault(Return(RetVal<uint64_t>::make_ok(1024)));

        ON_CALL(*m_fileSystem, readFile(_))
        .WillByDefault(Return(RetVal<ByteArray>::make_ok(ByteArray())));
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
        config.omr.images.allowedExtensions = { "png", "jpg", "jpeg" };
        config.audio2score.file.allowedExtensions = { "mp3" };
        return config;
    }

    //! NOTE: uploads the given file, resolves the upload with queueId, and lets the resulting
    //! poll (mocked to return queueList) run to completion
    void deliverQueueStatus(const ConvertQueueList& queueList, ConvertType type, int queueId, const QString& convertedScoreName)
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

        m_service->startConvert(input, convertedScoreName);

        uploadProgress->finish(ProgressResult::make_ok(Val(ValMap { { "id", Val(queueId) } })));
        pumpEvents();
    }

    //! NOTE: uploads the given file and resolves with queueId, triggering a fresh poll
    //! (watch() always re-polls all watched items) without touching the fetchQueue mock,
    //! which the caller owns - lets a test drive N polls without waiting on the real QTimer
    void uploadAndResolve(int queueId, const QString& convertedScoreName, const io::paths_t& paths)
    {
        auto uploadProgress = std::make_shared<Progress>();
        EXPECT_CALL(*m_convertService, upload(Truly([paths](const ConvertUploadDataPtr& data) {
            return uploadDataMatchesPaths(*data, paths);
        })))
        .WillOnce(Return(uploadProgress));

        m_service->startConvert(OmrConvertInput { paths }, convertedScoreName);

        uploadProgress->finish(ProgressResult::make_ok(Val(ValMap { { "id", Val(queueId) } })));
        pumpEvents();
    }

    std::shared_ptr<ConvertFileToScoreService> m_service;
    std::shared_ptr<MuseScoreComServiceMock> m_museScoreComService;
    std::shared_ptr<MuseScoreComConvertServiceMock> m_convertService;
    std::shared_ptr<muse::io::FileSystemMock> m_fileSystem;
    std::shared_ptr<ProjectConfigurationMock> m_configuration;
    std::shared_ptr<muse::mi::MultiWindowsProviderMock> m_multiWindowsProvider;
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
    serverConfig.omr.pdf.maxFileSizeBytes = 111;
    serverConfig.omr.pdf.maxFiles = 4;
    serverConfig.omr.pdf.maxPages = 7;
    serverConfig.omr.images.allowedExtensions = { "tif" };
    serverConfig.omr.images.maxFileSizeBytes = 112;
    serverConfig.omr.images.maxFiles = 99;
    serverConfig.audio2score.file.allowedExtensions = { "wav" };
    serverConfig.audio2score.file.maxFileSizeBytes = 222;
    serverConfig.audio2score.file.maxFiles = 3;
    serverConfig.audio2score.link.maxLength = 333;
    serverConfig.audio2score.link.allowedSources = LinkSource::AudioCom;
    ON_CALL(*m_convertService, fetchConfig())
    .WillByDefault(Invoke([serverConfig] {
        return resolvedPromise<RetVal<ConvertConfig> >(RetVal<ConvertConfig>::make_ok(serverConfig));
    }));

    // [WHEN] Initializing the service
    m_service->init();
    pumpEvents();

    // [THEN] The fetched config fully replaces the fallback
    const ConvertConfig& config = m_service->config();
    EXPECT_EQ(config.omr.pdf.maxFileSizeBytes, 111);
    EXPECT_EQ(config.omr.pdf.maxFiles, 4);
    EXPECT_EQ(config.omr.pdf.maxPages, 7);
    EXPECT_EQ(config.omr.images.allowedExtensions, QStringList({ "tif" }));
    EXPECT_EQ(config.omr.images.maxFileSizeBytes, 112);
    EXPECT_EQ(config.omr.images.maxFiles, 99);
    EXPECT_EQ(config.audio2score.file.allowedExtensions, QStringList({ "wav" }));
    EXPECT_EQ(config.audio2score.file.maxFileSizeBytes, 222);
    EXPECT_EQ(config.audio2score.file.maxFiles, 3);
    EXPECT_EQ(config.audio2score.link.maxLength, 333);
    EXPECT_TRUE(config.audio2score.link.allowedSources.testFlag(LinkSource::AudioCom));
    EXPECT_FALSE(config.audio2score.link.allowedSources.testFlag(LinkSource::YouTube));
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
    // [GIVEN] A configured maximum PDF file count
    ConvertConfig config = testConfig();
    config.omr.pdf.maxFiles = 1;
    setConfig(config);

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
    config.audio2score.file.maxFileSizeBytes = 100;
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
    config.audio2score.file.maxFiles = 1;
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
    config.omr.images.maxFiles = 2;
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
    config.omr.images.maxFileSizeBytes = 100;
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
    config.omr.pdf.maxFileSizeBytes = 100;
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
    config.audio2score.link.allowedSources = LinkSource::YouTube;
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
    config.audio2score.link.allowedSources = LinkSource::AudioCom;
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
    m_service->convertFinished().onReceive(nullptr, [&](const Ret& ret, const ScoreInfo&) {
        received = true;
        receivedRet = ret;
    });

    // [WHEN] Starting the conversion
    const io::paths_t paths { "/some/path/file.pdf" };
    Ret ret = m_service->startConvert(OmrConvertInput { paths }, u"My Score");
    EXPECT_TRUE(ret);

    // [THEN] The failure is forwarded synchronously, carrying the intended file name
    uploadProgress->finish(make_ret(Ret::Code::UnknownError, std::string("network error")));
    ASSERT_TRUE(received);
    EXPECT_FALSE(receivedRet);
    EXPECT_EQ(receivedRet.data<String>(CONVERT_FAILED_FILE_NAME_KEY, String()), u"My Score");
}

TEST_F(Project_ConvertFileToScoreServiceTest, StartConvert_UploadSucceeds_PersistsWatchedItemAndPolls)
{
    // [GIVEN] The upload succeeds with queue id TEST_QUEUE_ID
    ON_CALL(*m_configuration, watchedConvertsJsonPath())
    .WillByDefault(Return(io::path_t("/watched.json")));

    auto uploadProgress = std::make_shared<Progress>();
    const io::paths_t paths { "/some/path/file.pdf" };

    EXPECT_CALL(*m_convertService, upload(Truly([&](const ConvertUploadDataPtr& data) {
        return data->type == ConvertType::Omr && uploadDataMatchesPaths(*data, paths);
    })))
    .WillOnce(Return(uploadProgress));

    // [THEN] Polling begins, and the watched item is persisted with its id, type and file name
    EXPECT_CALL(*m_convertService, fetchQueue())
    .Times(1)
    .WillOnce(Invoke([] {
        return pendingPromise<RetVal<ConvertQueueList> >();
    }));

    bool savedExpectedEntry = false;
    EXPECT_CALL(*m_fileSystem, writeFile(io::path_t("/watched.json"), _))
    .WillOnce(Invoke([&](const io::path_t&, const ByteArray& data) {
        std::string err;
        JsonDocument json = JsonDocument::fromJson(data, &err);
        if (json.isArray() && json.rootArray().size() == 1) {
            JsonObject obj = json.rootArray().at(0).toObject();
            savedExpectedEntry = obj.value("id").toInt() == TEST_QUEUE_ID
                                 && obj.value("type").toInt() == int(ConvertType::Omr)
                                 && obj.value("convertedScoreName").toStdString() == "My Score";
        }
        return make_ok();
    }));

    // [WHEN] Starting the conversion
    Ret ret = m_service->startConvert(OmrConvertInput { paths }, u"My Score");
    EXPECT_TRUE(ret);

    uploadProgress->finish(ProgressResult::make_ok(Val(ValMap { { "id", Val(TEST_QUEUE_ID) } })));

    EXPECT_TRUE(savedExpectedEntry);
}

TEST_F(Project_ConvertFileToScoreServiceTest, StartConvert_UploadSucceeds_PersistsAudio2ScoreType)
{
    // [GIVEN] The upload succeeds for an Audio2Score conversion
    ON_CALL(*m_configuration, watchedConvertsJsonPath())
    .WillByDefault(Return(io::path_t("/watched.json")));

    auto uploadProgress = std::make_shared<Progress>();
    ON_CALL(*m_convertService, upload(_))
    .WillByDefault(Return(uploadProgress));
    ON_CALL(*m_convertService, fetchQueue())
    .WillByDefault(Invoke([] {
        return pendingPromise<RetVal<ConvertQueueList> >();
    }));

    bool savedExpectedType = false;
    EXPECT_CALL(*m_fileSystem, writeFile(io::path_t("/watched.json"), _))
    .WillOnce(Invoke([&](const io::path_t&, const ByteArray& data) {
        std::string err;
        JsonDocument json = JsonDocument::fromJson(data, &err);
        if (json.isArray() && json.rootArray().size() == 1) {
            JsonObject obj = json.rootArray().at(0).toObject();
            savedExpectedType = obj.value("type").toInt() == int(ConvertType::Audio2Score);
        }
        return make_ok();
    }));

    // [WHEN] Starting an Audio2Score conversion
    const io::paths_t paths { "/some/path/file.mp3" };
    m_service->startConvert(Audio2ScoreConvertInput { paths }, u"My Song");
    uploadProgress->finish(ProgressResult::make_ok(Val(ValMap { { "id", Val(TEST_QUEUE_ID) } })));

    // [THEN] The persisted type is Audio2Score
    EXPECT_TRUE(savedExpectedType);
}

// ==================================================
// watchedScores()
// ==================================================

TEST_F(Project_ConvertFileToScoreServiceTest, WatchedScores_Initially_Empty)
{
    EXPECT_TRUE(m_service->watchedScores().val.empty());
}

TEST_F(Project_ConvertFileToScoreServiceTest, WatchedScores_AfterStartConvert_ContainsScoreAndFiresChanged)
{
    // [GIVEN] The upload succeeds, and polling is left pending (the item stays watched)
    ON_CALL(*m_configuration, watchedConvertsJsonPath())
    .WillByDefault(Return(io::path_t("/watched.json")));
    ON_CALL(*m_fileSystem, writeFile(_, _))
    .WillByDefault(Return(make_ok()));

    auto uploadProgress = std::make_shared<Progress>();
    const io::paths_t paths { "/some/path/file.pdf" };
    ON_CALL(*m_convertService, upload(_))
    .WillByDefault(Return(uploadProgress));
    ON_CALL(*m_convertService, fetchQueue())
    .WillByDefault(Invoke([] {
        return pendingPromise<RetVal<ConvertQueueList> >();
    }));

    bool changed = false;
    m_service->watchedScores().notification.onNotify(nullptr, [&] {
        changed = true;
    });

    // [WHEN] Starting the conversion and letting the upload resolve
    m_service->startConvert(OmrConvertInput { paths }, u"My Score");
    uploadProgress->finish(ProgressResult::make_ok(Val(ValMap { { "id", Val(TEST_QUEUE_ID) } })));

    // [THEN] The score being converted is reported, and the change is signaled
    EXPECT_TRUE(changed);
    const WatchedScoreList watchedScores = m_service->watchedScores().val;
    ASSERT_EQ(watchedScores.size(), 1u);
    EXPECT_EQ(watchedScores.front().name, u"My Score");
}

TEST_F(Project_ConvertFileToScoreServiceTest, WatchedScores_AfterDone_NoLongerContainsScore)
{
    // [GIVEN] The queue reports the conversion as done, with its scoreId
    ConvertQueueItem item;
    item.id = TEST_QUEUE_ID;
    item.type = ConvertType::Omr;
    item.status = ConvertStatus::Done;
    item.scoreId = 555;

    ON_CALL(*m_museScoreComService, downloadScoreInfo(555))
    .WillByDefault(Invoke([] { return okScoreInfo(555); }));

    bool changed = false;
    m_service->watchedScores().notification.onNotify(nullptr, [&] {
        changed = true;
    });

    // [WHEN] Uploading and polling the status
    deliverQueueStatus({ item }, ConvertType::Omr, TEST_QUEUE_ID, "My Score");

    // [THEN] The item is reported ready and immediately erased, so it's no longer being converted
    EXPECT_TRUE(changed);
    EXPECT_TRUE(m_service->watchedScores().val.empty());
}

TEST_F(Project_ConvertFileToScoreServiceTest, WatchedScores_ExternalProcessingItem_AddedToWatchedScores)
{
    // [GIVEN] The queue reports an item that was never started via startConvert() locally,
    // alongside the one that was
    const int externalId = TEST_QUEUE_ID + 1;

    ConvertQueueItem ownItem;
    ownItem.id = TEST_QUEUE_ID;
    ownItem.type = ConvertType::Omr;
    ownItem.status = ConvertStatus::Processing;

    ConvertQueueItem externalItem;
    externalItem.id = externalId;
    externalItem.type = ConvertType::Omr;
    externalItem.status = ConvertStatus::Processing;
    externalItem.filename = "Externally Started Score";

    // [WHEN] Uploading and polling the status
    deliverQueueStatus({ ownItem, externalItem }, ConvertType::Omr, TEST_QUEUE_ID, "My Score");

    // [THEN] Both items are watched - the one we started, and the one discovered via the queue
    const WatchedScoreList watchedScores = m_service->watchedScores().val;
    ASSERT_EQ(watchedScores.size(), 2u);

    const auto externalIt = std::find_if(watchedScores.begin(), watchedScores.end(), [externalId](const WatchedScore& watched) {
        return watched.conversion.id == externalId;
    });
    ASSERT_NE(externalIt, watchedScores.end());
    EXPECT_EQ(externalIt->name, u"Externally Started Score");
    EXPECT_FALSE(externalIt->scoreId.has_value());
}

// ==================================================
// resumeConvert()
// ==================================================

TEST_F(Project_ConvertFileToScoreServiceTest, ResumeConvert_LoadsPersistedWatchedItem)
{
    // [GIVEN] A previously persisted pending conversion
    JsonObject obj;
    obj["id"] = TEST_QUEUE_ID;
    obj["type"] = int(ConvertType::Audio2Score);
    obj["convertedScoreName"] = "My Score";

    JsonArray array;
    array << obj;
    JsonDocument json(array);

    ON_CALL(*m_configuration, watchedConvertsJsonPath())
    .WillByDefault(Return(io::path_t("/watched.json")));
    ON_CALL(*m_fileSystem, readFile(io::path_t("/watched.json")))
    .WillByDefault(Return(RetVal<ByteArray>::make_ok(json.toJson())));

    ON_CALL(*m_convertService, fetchQueue())
    .WillByDefault(Invoke([] {
        return pendingPromise<RetVal<ConvertQueueList> >();
    }));

    bool changed = false;
    m_service->watchedScores().notification.onNotify(nullptr, [&] {
        changed = true;
    });

    // [WHEN] Resuming
    m_service->resumeConvert();

    // [THEN] The persisted item is restored and reported as being converted, and polling resumes
    EXPECT_TRUE(changed);
    const WatchedScoreList watchedScores = m_service->watchedScores().val;
    ASSERT_EQ(watchedScores.size(), 1u);
    EXPECT_EQ(watchedScores.front().name, u"My Score");
}

TEST_F(Project_ConvertFileToScoreServiceTest, ResumeConvert_AwaitingReviewItemWithScoreId_SendsReviewRequested)
{
    // [GIVEN] A persisted item that was already reported ready and awaiting review before the app closed
    JsonObject obj;
    obj["id"] = TEST_QUEUE_ID;
    obj["type"] = int(ConvertType::Omr);
    obj["status"] = int(ConvertStatus::AwaitingReview);
    obj["convertedScoreName"] = "My Score";
    obj["scoreId"] = 555;

    JsonArray array;
    array << obj;
    JsonDocument json(array);

    ON_CALL(*m_configuration, watchedConvertsJsonPath())
    .WillByDefault(Return(io::path_t("/watched.json")));
    ON_CALL(*m_fileSystem, readFile(io::path_t("/watched.json")))
    .WillByDefault(Return(RetVal<ByteArray>::make_ok(json.toJson())));

    ON_CALL(*m_convertService, fetchQueue())
    .WillByDefault(Invoke([] {
        return pendingPromise<RetVal<ConvertQueueList> >();
    }));

    bool reviewRequested = false;
    int reviewScoreId = 0;
    m_service->reviewRequested().onReceive(nullptr, [&](int scoreId) {
        reviewRequested = true;
        reviewScoreId = scoreId;
    });

    // [WHEN] Resuming
    m_service->resumeConvert();

    // [THEN] The review is requested immediately, carrying the previously reported scoreId
    ASSERT_TRUE(reviewRequested);
    EXPECT_EQ(reviewScoreId, 555);
}

TEST_F(Project_ConvertFileToScoreServiceTest, ResumeConvert_AwaitingReviewItemWithoutScoreId_DoesNotSendReviewRequested)
{
    // [GIVEN] A persisted item that was awaiting review, but never got a chance to report a scoreId before the app closed
    JsonObject obj;
    obj["id"] = TEST_QUEUE_ID;
    obj["type"] = int(ConvertType::Omr);
    obj["status"] = int(ConvertStatus::AwaitingReview);
    obj["convertedScoreName"] = "My Score";

    JsonArray array;
    array << obj;
    JsonDocument json(array);

    ON_CALL(*m_configuration, watchedConvertsJsonPath())
    .WillByDefault(Return(io::path_t("/watched.json")));
    ON_CALL(*m_fileSystem, readFile(io::path_t("/watched.json")))
    .WillByDefault(Return(RetVal<ByteArray>::make_ok(json.toJson())));

    ON_CALL(*m_convertService, fetchQueue())
    .WillByDefault(Invoke([] {
        return pendingPromise<RetVal<ConvertQueueList> >();
    }));

    bool reviewRequested = false;
    m_service->reviewRequested().onReceive(nullptr, [&](int) {
        reviewRequested = true;
    });

    // [WHEN] Resuming
    m_service->resumeConvert();

    // [THEN] No review is requested yet - there's no scoreId to identify the score by
    EXPECT_FALSE(reviewRequested);
}

// ==================================================
// polling / score info fetch pipeline
// ==================================================

TEST_F(Project_ConvertFileToScoreServiceTest, Poll_DoneStatus_FetchesScoreInfoAndFinishes)
{
    // [GIVEN] The queue reports the conversion as done, with its scoreId
    ConvertQueueItem item;
    item.id = TEST_QUEUE_ID;
    item.type = ConvertType::Omr;
    item.status = ConvertStatus::Done;
    item.scoreId = 555;

    ON_CALL(*m_museScoreComService, downloadScoreInfo(555))
    .WillByDefault(Invoke([] { return okScoreInfo(555, "My Score"); }));

    bool received = false;
    Ret receivedRet;
    ScoreInfo receivedInfo;
    m_service->convertFinished().onReceive(nullptr, [&](const Ret& ret, const ScoreInfo& info) {
        received = true;
        receivedRet = ret;
        receivedInfo = info;
    });

    // [WHEN] Uploading and polling the status
    deliverQueueStatus({ item }, ConvertType::Omr, TEST_QUEUE_ID, "My Score");

    // [THEN] The conversion finishes successfully, carrying the score's info
    ASSERT_TRUE(received);
    EXPECT_TRUE(receivedRet);
    EXPECT_EQ(receivedInfo.id, 555);
    EXPECT_EQ(receivedInfo.title, "My Score");
}

TEST_F(Project_ConvertFileToScoreServiceTest, Poll_AwaitingReviewWithoutScoreId_DoesNotReportYet)
{
    // [GIVEN] The queue reports the conversion as awaiting review, but hasn't assigned a scoreId yet
    ConvertQueueItem item;
    item.id = TEST_QUEUE_ID;
    item.type = ConvertType::Omr;
    item.status = ConvertStatus::AwaitingReview;

    // [THEN] There's nothing to identify the score by yet, so nothing is fetched or reported
    EXPECT_CALL(*m_museScoreComService, downloadScoreInfo(An<int>())).Times(0);

    bool reviewRequested = false;
    bool convertFinished = false;
    m_service->reviewRequested().onReceive(nullptr, [&](int) {
        reviewRequested = true;
    });
    m_service->convertFinished().onReceive(nullptr, [&](const Ret&, const ScoreInfo&) {
        convertFinished = true;
    });

    // [WHEN] Uploading and polling the status
    deliverQueueStatus({ item }, ConvertType::Omr, TEST_QUEUE_ID, "My Score");

    // [THEN] Neither signal fires yet
    EXPECT_FALSE(reviewRequested);
    EXPECT_FALSE(convertFinished);
}

TEST_F(Project_ConvertFileToScoreServiceTest, Poll_AwaitingReviewWithScoreId_EmitsReviewRequestedAndConvertFinished)
{
    // [GIVEN] The queue reports the conversion as awaiting review, with its scoreId
    ConvertQueueItem item;
    item.id = TEST_QUEUE_ID;
    item.type = ConvertType::Omr;
    item.status = ConvertStatus::AwaitingReview;
    item.scoreId = 555;

    ON_CALL(*m_museScoreComService, downloadScoreInfo(555))
    .WillByDefault(Invoke([] { return okScoreInfo(555); }));

    bool reviewRequested = false;
    int reviewScoreId = 0;
    m_service->reviewRequested().onReceive(nullptr, [&](int scoreId) {
        reviewRequested = true;
        reviewScoreId = scoreId;
    });

    bool convertFinished = false;
    Ret convertFinishedRet;
    m_service->convertFinished().onReceive(nullptr, [&](const Ret& ret, const ScoreInfo&) {
        convertFinished = true;
        convertFinishedRet = ret;
    });

    // [WHEN] Uploading and polling the status
    deliverQueueStatus({ item }, ConvertType::Omr, TEST_QUEUE_ID, "My Score");

    // [THEN] The score is already usable, so both signals fire immediately
    ASSERT_TRUE(convertFinished);
    EXPECT_TRUE(convertFinishedRet);

    ASSERT_TRUE(reviewRequested);
    EXPECT_EQ(reviewScoreId, 555);
}

TEST_F(Project_ConvertFileToScoreServiceTest, Poll_ItemNeverInQueueWithoutScoreId_TreatedAsFailed)
{
    // [GIVEN] The item never appears in the queue at all, and never reported a scoreId - there's
    // no way to identify a resulting score, so it can't be recovered as a success

    bool received = false;
    Ret receivedRet;
    m_service->convertFinished().onReceive(nullptr, [&](const Ret& ret, const ScoreInfo&) {
        received = true;
        receivedRet = ret;
    });

    // [WHEN] Uploading, then polling an empty queue
    deliverQueueStatus({}, ConvertType::Omr, TEST_QUEUE_ID, "My Score");

    // [THEN] The conversion is reported as failed
    ASSERT_TRUE(received);
    EXPECT_FALSE(receivedRet);
}

TEST_F(Project_ConvertFileToScoreServiceTest, Poll_PreviouslyReportedItemDropsFromQueue_SilentlyErasedWithoutDuplicateReport)
{
    // [GIVEN] The item was already reported ready (AwaitingReview, with its scoreId) on the first poll,
    // then disappears from the queue entirely on the second poll
    const int otherQueueId = TEST_QUEUE_ID + 1;

    ConvertQueueItem awaitingItem;
    awaitingItem.id = TEST_QUEUE_ID;
    awaitingItem.type = ConvertType::Omr;
    awaitingItem.status = ConvertStatus::AwaitingReview;
    awaitingItem.scoreId = 555;

    ConvertQueueItem otherItem;
    otherItem.id = otherQueueId;
    otherItem.type = ConvertType::Omr;
    otherItem.status = ConvertStatus::Processing;

    EXPECT_CALL(*m_convertService, fetchQueue())
    .Times(2)
    .WillOnce(Invoke([awaitingItem] {
        return resolvedPromise<RetVal<ConvertQueueList> >(RetVal<ConvertQueueList>::make_ok(ConvertQueueList { awaitingItem }));
    }))
    .WillOnce(Invoke([otherItem] {
        //! NOTE: awaitingItem has now dropped out of the queue entirely
        return resolvedPromise<RetVal<ConvertQueueList> >(RetVal<ConvertQueueList>::make_ok(ConvertQueueList { otherItem }));
    }));

    EXPECT_CALL(*m_museScoreComService, downloadScoreInfo(555))
    .Times(1)
    .WillOnce(Invoke([] { return okScoreInfo(555); }));

    int convertFinishedCount = 0;
    m_service->convertFinished().onReceive(nullptr, [&](const Ret&, const ScoreInfo&) {
        ++convertFinishedCount;
    });

    // [WHEN] Starting the conversion - the first poll reports it as awaiting review, already reporting it once
    uploadAndResolve(TEST_QUEUE_ID, "My Score", { "/some/path/a.pdf" });
    EXPECT_EQ(convertFinishedCount, 1);

    // [WHEN] Starting an unrelated conversion triggers a second poll; the original item has now dropped
    uploadAndResolve(otherQueueId, "Other Score", { "/some/path/b.pdf" });

    // [THEN] No duplicate report, and downloadScoreInfo() was only ever called once
    EXPECT_EQ(convertFinishedCount, 1);
}

TEST_F(Project_ConvertFileToScoreServiceTest, Poll_SameIdDifferentType_DoesNotCrossMatch)
{
    // [GIVEN] An Omr item and an Audio2Score item happen to share the same numeric id
    // (each type has its own id sequence server-side, so collisions are possible)
    JsonObject omrObj;
    omrObj["id"] = TEST_QUEUE_ID;
    omrObj["type"] = int(ConvertType::Omr);
    omrObj["startedLocally"] = true;
    omrObj["convertedScoreName"] = "Omr Score";

    JsonObject audioObj;
    audioObj["id"] = TEST_QUEUE_ID;
    audioObj["type"] = int(ConvertType::Audio2Score);
    audioObj["startedLocally"] = true;
    audioObj["convertedScoreName"] = "Audio Score";

    JsonArray array;
    array << omrObj << audioObj;
    JsonDocument json(array);

    ON_CALL(*m_configuration, watchedConvertsJsonPath())
    .WillByDefault(Return(io::path_t("/watched.json")));
    ON_CALL(*m_fileSystem, readFile(io::path_t("/watched.json")))
    .WillByDefault(Return(RetVal<ByteArray>::make_ok(json.toJson())));
    ON_CALL(*m_fileSystem, writeFile(_, _))
    .WillByDefault(Return(make_ok()));

    // [GIVEN] The queue reports the Omr item as failed, and the Audio2Score item as done; they
    // must not be mistaken for each other just because they share the same numeric id
    ConvertQueueItem failedOmrItem;
    failedOmrItem.id = TEST_QUEUE_ID;
    failedOmrItem.type = ConvertType::Omr;
    failedOmrItem.status = ConvertStatus::Failed;
    failedOmrItem.errorCode = ConvertErrorCode::FileTooLarge;
    failedOmrItem.filename = "Omr Score";

    ConvertQueueItem doneAudioItem;
    doneAudioItem.id = TEST_QUEUE_ID;
    doneAudioItem.type = ConvertType::Audio2Score;
    doneAudioItem.status = ConvertStatus::Done;
    doneAudioItem.scoreId = 999;
    doneAudioItem.filename = "Audio Score";

    ON_CALL(*m_convertService, fetchQueue())
    .WillByDefault(Invoke([failedOmrItem, doneAudioItem] {
        return resolvedPromise<RetVal<ConvertQueueList> >(RetVal<ConvertQueueList>::make_ok(ConvertQueueList { failedOmrItem,
                                                                                                               doneAudioItem }));
    }));

    ON_CALL(*m_museScoreComService, downloadScoreInfo(999))
    .WillByDefault(Invoke([] { return okScoreInfo(999, "Audio Score"); }));

    std::vector<Ret> receivedRets;
    m_service->convertFinished().onReceive(nullptr, [&](const Ret& ret, const ScoreInfo&) {
        receivedRets.push_back(ret);
    });

    // [WHEN] Resuming loads both items and triggers a poll
    m_service->resumeConvert();
    pumpEvents();

    // [THEN] Exactly one failure (the Omr one) and one success (the Audio2Score one) are reported -
    // if type were ignored during matching, the two items could be mixed up with each other
    ASSERT_EQ(receivedRets.size(), 2u);

    const auto failureIt = std::find_if(receivedRets.begin(), receivedRets.end(), [](const Ret& ret) { return !ret; });
    ASSERT_NE(failureIt, receivedRets.end());
    EXPECT_EQ(failureIt->data<String>(CONVERT_FAILED_FILE_NAME_KEY, String()), u"Omr Score");

    const auto successIt = std::find_if(receivedRets.begin(), receivedRets.end(), [](const Ret& ret) { return bool(ret); });
    ASSERT_NE(successIt, receivedRets.end());
}

TEST_F(Project_ConvertFileToScoreServiceTest, Poll_FailedStatus_ForwardsProcessingFailure)
{
    // [GIVEN] The queue reports the conversion as failed
    ConvertQueueItem item;
    item.id = TEST_QUEUE_ID;
    item.type = ConvertType::Omr;
    item.status = ConvertStatus::Failed;
    item.errorCode = ConvertErrorCode::FileTooLarge;
    item.filename = "My Score";

    bool received = false;
    Ret receivedRet;
    m_service->convertFinished().onReceive(nullptr, [&](const Ret& ret, const ScoreInfo&) {
        received = true;
        receivedRet = ret;
    });

    // [WHEN] Uploading and polling the status
    deliverQueueStatus({ item }, ConvertType::Omr, TEST_QUEUE_ID, "My Score");

    // [THEN] The failure is forwarded, carrying the intended file name
    ASSERT_TRUE(received);
    EXPECT_FALSE(receivedRet);
    EXPECT_EQ(receivedRet.code(), int(mu::project::Err::ConvertProcessingFailed));
    EXPECT_EQ(receivedRet.data<String>(CONVERT_FAILED_FILE_NAME_KEY, String()), u"My Score");
}

TEST_F(Project_ConvertFileToScoreServiceTest, Poll_ScoreInfoFetchFails_RetriesOnNextPoll)
{
    // [GIVEN] The queue reports the conversion as done, with its scoreId, on both polls
    const int otherQueueId = TEST_QUEUE_ID + 1;

    ConvertQueueItem doneItem;
    doneItem.id = TEST_QUEUE_ID;
    doneItem.type = ConvertType::Omr;
    doneItem.status = ConvertStatus::Done;
    doneItem.scoreId = 555;

    ConvertQueueItem otherItem;
    otherItem.id = otherQueueId;
    otherItem.type = ConvertType::Omr;
    otherItem.status = ConvertStatus::Processing;

    ON_CALL(*m_convertService, fetchQueue())
    .WillByDefault(Invoke([doneItem, otherItem] {
        return resolvedPromise<RetVal<ConvertQueueList> >(RetVal<ConvertQueueList>::make_ok(ConvertQueueList { doneItem, otherItem }));
    }));

    // [GIVEN] Fetching the score's info fails transiently the first time, succeeds the second
    EXPECT_CALL(*m_museScoreComService, downloadScoreInfo(555))
    .Times(2)
    .WillOnce(Return(RetVal<ScoreInfo>::make_ret(make_ret(muse::cloud::Err::NetworkError))))
    .WillOnce(Invoke([] { return okScoreInfo(555); }));

    bool received = false;
    Ret receivedRet;
    m_service->convertFinished().onReceive(nullptr, [&](const Ret& ret, const ScoreInfo&) {
        received = true;
        receivedRet = ret;
    });

    // [WHEN] Starting the conversion - the first poll's score info fetch fails transiently
    uploadAndResolve(TEST_QUEUE_ID, "My Score", { "/some/path/a.pdf" });
    EXPECT_FALSE(received);

    // [WHEN] Starting an unrelated conversion triggers a second poll, retrying the fetch
    uploadAndResolve(otherQueueId, "Other Score", { "/some/path/b.pdf" });

    // [THEN] The retried fetch succeeds and the conversion finishes
    ASSERT_TRUE(received);
    EXPECT_TRUE(receivedRet);
}

// ==================================================
// retry logic (poll / download failures)
// ==================================================

TEST_F(Project_ConvertFileToScoreServiceTest, Poll_NonRetryableFetchFailure_FinishesImmediatelyWithError)
{
    // [GIVEN] Checking the queue status fails with a permanent (non-retryable) error
    ON_CALL(*m_convertService, fetchQueue())
    .WillByDefault(Invoke([] {
        return resolvedPromise<RetVal<ConvertQueueList> >(
            RetVal<ConvertQueueList>::make_ret(make_ret(muse::cloud::Err::Status401_AuthorizationRequired)));
    }));

    bool gaveUp = false;
    Ret gaveUpRet;
    m_service->pollingFailed().onReceive(nullptr, [&](const PollingFailure& failure) {
        gaveUp = failure.gaveUp;
        gaveUpRet = failure.ret;
    });

    // [WHEN] Starting the conversion, triggering the first poll
    uploadAndResolve(TEST_QUEUE_ID, "My Score", { "/some/path/file.pdf" });

    // [THEN] Polling gives up on the first attempt, forwarding the permanent error
    ASSERT_TRUE(gaveUp);
    EXPECT_EQ(gaveUpRet.code(), int(muse::cloud::Err::Status401_AuthorizationRequired));
}

TEST_F(Project_ConvertFileToScoreServiceTest, Poll_RetryableFetchFailure_KeepsWatchingItemForNextPoll)
{
    // [GIVEN] The first status check fails with a transient network error;
    // the second succeeds, reporting the originally watched item as done
    const int otherQueueId = TEST_QUEUE_ID + 1;

    ConvertQueueItem watchedItem;
    watchedItem.id = TEST_QUEUE_ID;
    watchedItem.type = ConvertType::Omr;
    watchedItem.status = ConvertStatus::Done;
    watchedItem.scoreId = 555;

    ConvertQueueItem otherItem;
    otherItem.id = otherQueueId;
    otherItem.type = ConvertType::Omr;
    otherItem.status = ConvertStatus::Processing;

    EXPECT_CALL(*m_convertService, fetchQueue())
    .Times(2)
    .WillOnce(Invoke([] {
        return resolvedPromise<RetVal<ConvertQueueList> >(
            RetVal<ConvertQueueList>::make_ret(make_ret(muse::cloud::Err::NetworkError)));
    }))
    .WillOnce(Invoke([watchedItem, otherItem] {
        return resolvedPromise<RetVal<ConvertQueueList> >(RetVal<ConvertQueueList>::make_ok(ConvertQueueList { watchedItem,
                                                                                                               otherItem }));
    }));

    ON_CALL(*m_museScoreComService, downloadScoreInfo(555))
    .WillByDefault(Invoke([] { return okScoreInfo(555); }));

    bool received = false;
    m_service->convertFinished().onReceive(nullptr, [&](const Ret&, const ScoreInfo&) {
        received = true;
    });

    // [WHEN] Starting the first conversion - its poll fails with a transient network error
    uploadAndResolve(TEST_QUEUE_ID, "My Score", { "/some/path/a.pdf" });

    // [THEN] Nothing is finished yet, the item was not dropped from the watch list
    EXPECT_FALSE(received);

    // [AND WHEN] Starting an unrelated conversion triggers a second poll, checking the original item again too
    uploadAndResolve(otherQueueId, "Other Score", { "/some/path/b.pdf" });

    // [THEN] The item survived the transient failure and was processed once the queue succeeded
    EXPECT_TRUE(received);
}

TEST_F(Project_ConvertFileToScoreServiceTest, Poll_ConsecutiveRetryableFetchFailures_GivesUpAfterMaxAttempts)
{
    // [GIVEN] Every status check fails with a transient network error. MAX_POLL_RETRY_ATTEMPTS
    // (see convertfiletoscoreservice.h) is 5, so the 5th attempt should be the last one
    const int maxAttempts = 5;

    EXPECT_CALL(*m_convertService, fetchQueue())
    .Times(maxAttempts)
    .WillRepeatedly(Invoke([] {
        return resolvedPromise<RetVal<ConvertQueueList> >(
            RetVal<ConvertQueueList>::make_ret(make_ret(muse::cloud::Err::NetworkError)));
    }));

    bool gaveUp = false;
    Ret gaveUpRet;
    m_service->pollingFailed().onReceive(nullptr, [&](const PollingFailure& failure) {
        gaveUp = failure.gaveUp;
        gaveUpRet = failure.ret;
    });

    // [WHEN] Starting a new conversion re-triggers polling, each attempt failing
    for (int i = 0; i < maxAttempts; ++i) {
        uploadAndResolve(TEST_QUEUE_ID + i, QString("Score %1").arg(i),
                         { io::path_t(std::string("/some/path/") + std::to_string(i) + ".pdf") });

        if (i < maxAttempts - 1) {
            EXPECT_FALSE(gaveUp);
        }
    }

    // [THEN] Polling gives up after the max number of consecutive failures, forwarding the last error
    ASSERT_TRUE(gaveUp);
    EXPECT_EQ(gaveUpRet.code(), int(muse::cloud::Err::NetworkError));
}

TEST_F(Project_ConvertFileToScoreServiceTest, Poll_RetryableFetchFailure_ReportsPollingFailedNotGivingUp)
{
    // [GIVEN] The status check fails with a transient network error
    ON_CALL(*m_convertService, fetchQueue())
    .WillByDefault(Invoke([] {
        return resolvedPromise<RetVal<ConvertQueueList> >(
            RetVal<ConvertQueueList>::make_ret(make_ret(muse::cloud::Err::NetworkError)));
    }));

    bool received = false;
    PollingFailure failure;
    m_service->pollingFailed().onReceive(nullptr, [&](const PollingFailure& f) {
        received = true;
        failure = f;
    });

    // [WHEN] Starting the conversion, triggering the first poll
    uploadAndResolve(TEST_QUEUE_ID, "My Score", { "/some/path/file.pdf" });

    // [THEN] The first failure is reported as still retrying, at the normal (non-backed-off) interval
    ASSERT_TRUE(received);
    EXPECT_FALSE(failure.gaveUp);
    EXPECT_EQ(failure.attempt, 1);
    EXPECT_EQ(failure.maxAttempts, 5);
    EXPECT_DOUBLE_EQ(failure.nextInterval.raw(), 60.0);
    EXPECT_EQ(failure.ret.code(), int(muse::cloud::Err::NetworkError));
}

TEST_F(Project_ConvertFileToScoreServiceTest, RetryPolling_AfterGivingUp_ResumesPolling)
{
    // [GIVEN] Polling has given up after MAX_POLL_RETRY_ATTEMPTS consecutive failures
    const int maxAttempts = 5;

    EXPECT_CALL(*m_convertService, fetchQueue())
    .Times(maxAttempts)
    .WillRepeatedly(Invoke([] {
        return resolvedPromise<RetVal<ConvertQueueList> >(
            RetVal<ConvertQueueList>::make_ret(make_ret(muse::cloud::Err::NetworkError)));
    }));

    bool gaveUp = false;
    m_service->pollingFailed().onReceive(nullptr, [&](const PollingFailure& failure) {
        gaveUp = failure.gaveUp;
    });

    for (int i = 0; i < maxAttempts; ++i) {
        uploadAndResolve(TEST_QUEUE_ID + i, QString("Score %1").arg(i),
                         { io::path_t(std::string("/some/path/") + std::to_string(i) + ".pdf") });
    }
    ASSERT_TRUE(gaveUp);

    // [THEN] Calling retryPolling() checks the status again
    bool polledAgain = false;
    EXPECT_CALL(*m_convertService, fetchQueue())
    .Times(1)
    .WillOnce(Invoke([&] {
        polledAgain = true;
        return pendingPromise<RetVal<ConvertQueueList> >();
    }));

    // [WHEN] Retrying polling
    m_service->retryPolling();

    EXPECT_TRUE(polledAgain);
}

TEST_F(Project_ConvertFileToScoreServiceTest, RetryPolling_NoPendingItems_DoesNotPoll)
{
    // [THEN] The status is never checked
    EXPECT_CALL(*m_convertService, fetchQueue()).Times(0);

    // [WHEN] Retrying polling with nothing being converted
    m_service->retryPolling();
}

TEST_F(Project_ConvertFileToScoreServiceTest, Poll_SuccessBetweenFetchFailures_ResetsConsecutiveFailureCount)
{
    // [GIVEN] A pattern of failures with a success in between: 3 failures, then a success, then
    // 4 more failures - never 5 CONSECUTIVE failures, so polling should never give up
    ConvertQueueItem processingItem;
    processingItem.id = TEST_QUEUE_ID;
    processingItem.type = ConvertType::Omr;
    processingItem.status = ConvertStatus::Processing;

    auto failure = [] {
        return resolvedPromise<RetVal<ConvertQueueList> >(
            RetVal<ConvertQueueList>::make_ret(make_ret(muse::cloud::Err::NetworkError)));
    };
    auto success = [processingItem] {
        return resolvedPromise<RetVal<ConvertQueueList> >(RetVal<ConvertQueueList>::make_ok(ConvertQueueList { processingItem }));
    };

    EXPECT_CALL(*m_convertService, fetchQueue())
    .Times(8)
    .WillOnce(Invoke(failure))
    .WillOnce(Invoke(failure))
    .WillOnce(Invoke(failure))
    .WillOnce(Invoke(success))
    .WillOnce(Invoke(failure))
    .WillOnce(Invoke(failure))
    .WillOnce(Invoke(failure))
    .WillOnce(Invoke(failure));

    bool gaveUp = false;
    m_service->pollingFailed().onReceive(nullptr, [&](const PollingFailure& failure) {
        gaveUp = failure.gaveUp;
    });

    // [WHEN] Triggering 8 polls in a row
    for (int i = 0; i < 8; ++i) {
        uploadAndResolve(TEST_QUEUE_ID, QString("Score %1").arg(i),
                         { io::path_t(std::string("/some/path/") + std::to_string(i) + ".pdf") });
    }

    // [THEN] Polling never gave up, since no 5 failures happened consecutively
    EXPECT_FALSE(gaveUp);
}

// ==================================================
// submitReview() / submitReviewComment()
// ==================================================

TEST_F(Project_ConvertFileToScoreServiceTest, SubmitReview_Good_DelegatesToConvertService)
{
    // [GIVEN] A watched item already reported ready and awaiting review, identified by its scoreId
    ConvertQueueItem item;
    item.id = TEST_QUEUE_ID;
    item.type = ConvertType::Omr;
    item.status = ConvertStatus::AwaitingReview;
    item.scoreId = 555;

    ON_CALL(*m_museScoreComService, downloadScoreInfo(555))
    .WillByDefault(Invoke([] { return okScoreInfo(555); }));

    deliverQueueStatus({ item }, ConvertType::Omr, TEST_QUEUE_ID, "My Score");

    // [THEN] The rating is delegated to the convert service, resolving the scoreId back to its conversion
    EXPECT_CALL(*m_convertService, submitReview(ConvertType::Omr, TEST_QUEUE_ID, ReviewRating::Good, QString()))
    .WillOnce(Invoke([](auto, auto, auto, auto) {
        return resolvedPromise<RetVal<ConvertResult> >(RetVal<ConvertResult>::make_ok(ConvertResult {}));
    }));

    // [WHEN] Submitting a "Good" review with no comment
    m_service->submitReview(555, ReviewRating::Good);
}

TEST_F(Project_ConvertFileToScoreServiceTest, SubmitReview_BadWithComment_DelegatesToConvertService)
{
    // [GIVEN] A watched item already reported ready and awaiting review, identified by its scoreId
    ConvertQueueItem item;
    item.id = 7;
    item.type = ConvertType::Audio2Score;
    item.status = ConvertStatus::AwaitingReview;
    item.scoreId = 555;

    ON_CALL(*m_museScoreComService, downloadScoreInfo(555))
    .WillByDefault(Invoke([] { return okScoreInfo(555); }));

    deliverQueueStatus({ item }, ConvertType::Audio2Score, 7, "My Score");

    // [THEN] The rating and comment are delegated to the convert service, resolving the scoreId back to its conversion
    EXPECT_CALL(*m_convertService, submitReview(ConvertType::Audio2Score, 7, ReviewRating::Bad, QString("Too many wrong notes")))
    .WillOnce(Invoke([](auto, auto, auto, auto) {
        return resolvedPromise<RetVal<ConvertResult> >(RetVal<ConvertResult>::make_ok(ConvertResult {}));
    }));

    // [WHEN] Submitting a "Bad" review with a comment
    m_service->submitReview(555, ReviewRating::Bad, "Too many wrong notes");
}

TEST_F(Project_ConvertFileToScoreServiceTest, SubmitReviewComment_DelegatesToConvertService)
{
    // [GIVEN] A watched item already reported ready and awaiting review, identified by its scoreId
    ConvertQueueItem item;
    item.id = 7;
    item.type = ConvertType::Audio2Score;
    item.status = ConvertStatus::AwaitingReview;
    item.scoreId = 555;

    ON_CALL(*m_museScoreComService, downloadScoreInfo(555))
    .WillByDefault(Invoke([] { return okScoreInfo(555); }));

    deliverQueueStatus({ item }, ConvertType::Audio2Score, 7, "My Score");

    // [THEN] The comment is delegated to the convert service, resolving the scoreId back to its conversion
    EXPECT_CALL(*m_convertService, submitReviewComment(ConvertType::Audio2Score, 7, QString("Great job")))
    .WillOnce(Invoke([](auto, auto, auto) {
        return resolvedPromise<RetVal<ConvertResult> >(RetVal<ConvertResult>::make_ok(ConvertResult {}));
    }));

    // [WHEN] Submitting a follow-up comment
    m_service->submitReviewComment(555, "Great job");
}

// ==================================================
// deleteConversion()
// ==================================================

TEST_F(Project_ConvertFileToScoreServiceTest, DeleteConversion_Success_RemovesFromWatchedScores)
{
    // [GIVEN] A watched, still-processing conversion
    ConvertQueueItem item;
    item.id = TEST_QUEUE_ID;
    item.type = ConvertType::Omr;
    item.status = ConvertStatus::Processing;

    deliverQueueStatus({ item }, ConvertType::Omr, TEST_QUEUE_ID, "My Score");
    ASSERT_EQ(m_service->watchedScores().val.size(), 1u);

    // [THEN] The deletion is delegated to the convert service
    EXPECT_CALL(*m_convertService, deleteConversion(ConvertType::Omr, TEST_QUEUE_ID))
    .WillOnce(Invoke([](auto, auto) {
        return resolvedPromise<Ret>(make_ok());
    }));

    // [WHEN] Deleting the conversion
    m_service->deleteConversion(ConvertType::Omr, TEST_QUEUE_ID);
    pumpEvents();

    // [THEN] It is no longer watched
    EXPECT_TRUE(m_service->watchedScores().val.empty());
}

TEST_F(Project_ConvertFileToScoreServiceTest, DeleteConversion_Fails_KeepsWatching)
{
    // [GIVEN] A watched, still-processing conversion
    ConvertQueueItem item;
    item.id = TEST_QUEUE_ID;
    item.type = ConvertType::Omr;
    item.status = ConvertStatus::Processing;

    deliverQueueStatus({ item }, ConvertType::Omr, TEST_QUEUE_ID, "My Score");
    ASSERT_EQ(m_service->watchedScores().val.size(), 1u);

    // [THEN] The deletion is delegated to the convert service, but fails
    EXPECT_CALL(*m_convertService, deleteConversion(ConvertType::Omr, TEST_QUEUE_ID))
    .WillOnce(Invoke([](auto, auto) {
        return resolvedPromise<Ret>(make_ret(muse::cloud::Err::UnknownError));
    }));

    // [WHEN] Deleting the conversion
    m_service->deleteConversion(ConvertType::Omr, TEST_QUEUE_ID);
    pumpEvents();

    // [THEN] It is still watched
    EXPECT_EQ(m_service->watchedScores().val.size(), 1u);
}
