/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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
#include <map>
#include <optional>

#include <QBuffer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QUrlQuery>

#include "async/asyncable.h"
#include "async/promise.h"
#include "async/processevents.h"
#include "global/io/ioretcodes.h"
#include "global/types/version.h"
#include "network/networkerrors.h"

#include "cloud/clouderrors.h"
#include "cloud/musescorecom/musescorecomservice.h"

#include "mocks/cloudconfigurationmock.h"
#include "global/tests/mocks/applicationmock.h"
#include "global/tests/mocks/filesystemmock.h"
#include "multiwindows/tests/mocks/multiwindowsprovidermock.h"
#include "network/tests/mocks/networkmanagercreatormock.h"
#include "network/tests/mocks/networkmanagermock.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;

using namespace muse;
using namespace muse::cloud;
using namespace muse::network;

namespace {
const std::string GET("GET");
const std::string POST("POST");
const std::string PUT("PUT");
const std::string DELETE_("DELETE");

const QString HOST("desktop.musescore.com");
const QString ACCOUNT_INFO_PATH("/editor/v1/me");
const QString REFRESH_TOKENS_PATH("/editor/v1/oauth/refresh");
const QString SCORE_INFO_PATH("/editor/v1/score/info");
const QString SCORES_LIST_PATH("/editor/v2/collection/scores");
const QString SCORE_DOWNLOAD_PATH("/editor/v1/score/download");
const QString SCORE_DOWNLOAD_SHARED_PATH("/editor/v1/score/download-shared");
const QString SCORE_UPLOAD_PATH("/editor/v1/score/upload");
const QString AUDIO_UPLOAD_PATH("/editor/v1/score/audio");
const QString SCORE_DELETE_PATH("/editor/v1/score/score");
const QString CONVERT_CONFIG_PATH("/static/musescore/studio/upload-config.json");
const QString CONVERT_PATH("/editor/v1/score/convert/convert");
const QString CONVERT_QUEUE_PATH("/editor/v1/score/convert/queue");
const QString CONVERT_REVIEW_PATH("/editor/v1/score/convert/review");
const QString CONVERT_COMMENT_PATH("/editor/v1/score/convert/comment");

const std::string CLIENT_ID("client-id");
const QString ACCESS_TOKEN("access-token");
const QString NEW_ACCESS_TOKEN("new-access-token");
const QString NEW_REFRESH_TOKEN("new-refresh-token");

constexpr int ACCOUNT_ID = 7;
constexpr int SCORE_ID = 100;

const QByteArray TOKENS_JSON(R"({ "access_token": "access-token", "refresh_token": "refresh-token" })");
const QByteArray NEW_TOKENS_JSON(R"({ "access_token": "new-access-token", "refresh_token": "new-refresh-token" })");

const QByteArray ACCOUNT_INFO_JSON(
    R"({
    "id": 7,
    "name": "User",
    "profile_url": "test://users/7",
    "avatar_url": "test://avatars/7.png"
})");

const QByteArray SCORE_INFO_JSON(
    R"({
    "id": 100,
    "revision_id": 3,
    "title": "Title",
    "description": "Description",
    "license": "all-rights-reserved",
    "tags": "piano,solo",
    "privacy": 1,
    "custom_url": "test://scores/100",
    "user": {
        "uid": 7,
        "username": "User",
        "custom_url": "test://users/7"
    }
})");

const QByteArray OTHER_USER_SCORE_INFO_JSON(R"({ "id": 100, "title": "Title", "user": { "uid": 8 } })");

const QByteArray SCORES_LIST_JSON(
    R"({
    "_meta": { "totalCount": 12, "pageCount": 2, "currentPage": 1, "perPage": 10 },
    "items": [
        {
            "id": 1,
            "title": "Converted",
            "date_updated": 1700000000,
            "current_revision": { "file_size": 2048 },
            "thumbnails": { "small": "test://thumbnails/1.png" },
            "privacy": 0,
            "view_count": 5,
            "conversion": { "id": 9, "type": "audio2score", "status": "awaiting_review" }
        },
        {
            "id": 2,
            "title": "Regular",
            "privacy": 2
        }
    ]
})");

const QByteArray LAST_PAGE_SCORES_LIST_JSON(
    R"({
    "_meta": { "totalCount": 12, "pageCount": 2, "currentPage": 2, "perPage": 10 },
    "items": [ { "id": 11, "title": "Score 11" }, { "id": 12, "title": "Score 12" } ]
})");

const QByteArray SCORE_UPLOAD_JSON(
    R"({
    "permalink": "test://scores/100",
    "edit_url": "test://scores/100/edit",
    "revision_id": 4
})");

const QByteArray CONVERT_CONFIG_JSON(
    R"({
    "version": 3,
    "omr": {
        "pdf": { "max_file_size_bytes": 52428800, "max_files": 1, "max_pages": 50 },
        "images": {
            "mime_types": [ "image/jpeg", "image/png", "image/x-nonexistent-format" ],
            "max_file_size_bytes": 10485760,
            "max_files": 20
        }
    },
    "audio2score": {
        "file": { "max_file_size_bytes": 104857600, "max_files": 2, "extensions": [ "mp3", "wav" ] },
        "link": { "max_length": 2048, "sources": [ "YouTube", "audio_com", "unknown_source" ] }
    }
})");

const QByteArray CONVERT_RESULT_JSON(R"({ "id": 11, "type": "omr", "status": "processing" })");

const QByteArray CONVERT_QUEUE_JSON(
    R"({
    "items": [
        {
            "id": 1,
            "type": "omr",
            "status": "done",
            "filename": "score.pdf",
            "score_id": 55,
            "date_created": 1700000000,
            "date_updated": 1700000100
        },
        {
            "id": 2,
            "type": "audio2score",
            "status": "failed",
            "link": "test://audio/abc",
            "score_id": null,
            "error_code": "mu_status_too_complex"
        },
        {
            "id": 3,
            "type": "unknown_type",
            "status": "unknown_status"
        }
    ]
})");

QByteArray errorCodeJson(const QByteArray& errorCode)
{
    return "{ \"error_code\": \"" + errorCode + "\" }";
}

Ret statusRet(int status)
{
    Ret ret = muse::network::make_ret(muse::network::Err::NetworkError);
    ret.setData("status", status);
    return ret;
}

const Ret TOKEN_EXPIRED_RET = statusRet(401);

int code(cloud::Err err)
{
    return static_cast<int>(err);
}

ConvertErrorCode convertErrorCode(const Ret& ret)
{
    return ret.data<ConvertErrorCode>(CONVERT_ERROR_CODE_KEY, ConvertErrorCode::Unknown);
}

QString queryValue(const QUrl& url, const QString& key)
{
    return QUrlQuery(url).queryItemValue(key);
}

DevicePtr makeDevice(const QByteArray& data = QByteArray())
{
    std::shared_ptr<QBuffer> buffer = std::make_shared<QBuffer>();
    buffer->setData(data);
    buffer->open(QIODevice::ReadWrite);
    return buffer;
}

ConvertUploadDataPtr makeConvertUploadData()
{
    std::shared_ptr<ConvertUploadData> data = std::make_shared<ConvertUploadData>();
    data->type = ConvertType::Omr;
    data->files.push_back(ConvertFileData { ByteArray("pdf"), "score.pdf" });
    data->filename = "Score";
    return data;
}
}

namespace muse::cloud {
class Cloud_MuseScoreComServiceTest : public ::testing::Test, public async::Asyncable
{
protected:
    struct Request {
        std::string verb;
        QUrl url;
        RequestHeaders headers;
        Progress progress;
    };

    void SetUp() override
    {
        m_configuration = std::make_shared<NiceMock<CloudConfigurationMock> >();
        m_application = std::make_shared<NiceMock<ApplicationMock> >();
        m_fileSystem = std::make_shared<NiceMock<io::FileSystemMock> >();
        m_multiwindowsProvider = std::make_shared<NiceMock<mi::MultiWindowsProviderMock> >();
        m_networkManagerCreator = std::make_shared<NiceMock<NetworkManagerCreatorMock> >();
        m_networkManager = std::make_shared<NiceMock<NetworkManagerMock> >();

        m_service = std::make_shared<MuseScoreComService>(muse::modularity::globalCtx());
        m_service->configuration.set(m_configuration);
        m_service->application.set(m_application);
        m_service->fileSystem.set(m_fileSystem);
        m_service->multiwindowsProvider.set(m_multiwindowsProvider);
        m_service->networkManagerCreator.set(m_networkManagerCreator);

        ON_CALL(*m_configuration, clientId())
        .WillByDefault(Return(CLIENT_ID));
        ON_CALL(*m_application, version())
        .WillByDefault(Return(Version(4, 7, 0)));
        ON_CALL(*m_multiwindowsProvider, resourceChanged())
        .WillByDefault(Return(m_resourceChanged));
        ON_CALL(*m_networkManagerCreator, makeNetworkManager())
        .WillByDefault(Return(m_networkManager));

        ON_CALL(*m_fileSystem, readFile(_))
        .WillByDefault(Return(RetVal<ByteArray>::make_ok(ByteArray::fromQByteArray(TOKENS_JSON))));
        ON_CALL(*m_fileSystem, writeFile(_, _))
        .WillByDefault(Return(muse::make_ok()));

        //! NOTE: the requests stay pending until a test finishes them
        ON_CALL(*m_networkManager, get(_, _, _))
        .WillByDefault(Invoke([this](const QUrl& url, IncomingDevicePtr device, const RequestHeaders& headers) {
            return onRequest(GET, url, device, headers);
        }));
        ON_CALL(*m_networkManager, post(_, _, _, _))
        .WillByDefault(Invoke([this](const QUrl& url, OutgoingDeviceVar, IncomingDevicePtr device, const RequestHeaders& headers) {
            return onRequest(POST, url, device, headers);
        }));
        ON_CALL(*m_networkManager, put(_, _, _, _))
        .WillByDefault(Invoke([this](const QUrl& url, OutgoingDeviceVar, IncomingDevicePtr device, const RequestHeaders& headers) {
            return onRequest(PUT, url, device, headers);
        }));
        ON_CALL(*m_networkManager, del(_, _, _))
        .WillByDefault(Invoke([this](const QUrl& url, IncomingDevicePtr device, const RequestHeaders& headers) {
            return onRequest(DELETE_, url, device, headers);
        }));

        willReply(ACCOUNT_INFO_PATH, ACCOUNT_INFO_JSON);
    }

    void initService()
    {
        m_service->init();
        processMessages();

        finishRequest(ACCOUNT_INFO_PATH);
    }

    void initServiceWithoutTokens()
    {
        ON_CALL(*m_fileSystem, readFile(_))
        .WillByDefault(Return(RetVal<ByteArray>::make_ret(muse::io::make_ret(muse::io::Err::FSNotExist))));

        m_service->init();
        processMessages();
    }

    void willReply(const QString& path, const QByteArray& reply)
    {
        m_replies[path] = reply;
    }

    RetVal<Progress> onRequest(const std::string& verb, const QUrl& url, IncomingDevicePtr device, const RequestHeaders& headers)
    {
        if (m_requestError) {
            return RetVal<Progress>::make_ret(*m_requestError);
        }

        auto it = m_replies.find(url.path());
        if (device && it != m_replies.cend()) {
            const bool wasOpen = device->isOpen();
            if (!wasOpen) {
                device->open(QIODevice::WriteOnly);
            }

            device->write(it->second);

            if (!wasOpen) {
                device->close();
            }
        }

        Request request;
        request.verb = verb;
        request.url = url;
        request.headers = headers;
        m_pendingRequests.push_back(request);

        return RetVal<Progress>::make_ok(request.progress);
    }

    std::vector<Request>::iterator findPendingRequest(const QString& path)
    {
        return std::find_if(m_pendingRequests.begin(), m_pendingRequests.end(), [&path](const Request& request) {
            return request.url.path() == path;
        });
    }

    const Request& pendingRequest(const QString& path)
    {
        auto it = findPendingRequest(path);
        if (it == m_pendingRequests.end()) {
            ADD_FAILURE() << "no pending request: " << path.toStdString();
            return m_noRequest;
        }

        return *it;
    }

    void finishRequest(const QString& path, const Ret& ret = muse::make_ok())
    {
        auto it = findPendingRequest(path);
        ASSERT_TRUE(it != m_pendingRequests.end()) << "no pending request: " << path.toStdString();

        //! NOTE: finishing a request may issue new ones
        Progress progress = it->progress;
        m_pendingRequests.erase(it);
        progress.finish(ret);

        processMessages();
    }

    void processMessages()
    {
        //! NOTE: a processed message may queue a new one
        constexpr int MAX_CHAINED_MESSAGES = 3;

        for (int i = 0; i < MAX_CHAINED_MESSAGES; ++i) {
            async::processMessages();
        }
    }

    void watch(const ProgressPtr& progress)
    {
        progress->finished().onReceive(this, [this](const ProgressResult& res) {
            m_progressResults.push_back(res);
        });
    }

    template<typename T>
    void watch(async::Promise<T> promise, std::optional<T>& result)
    {
        promise.onResolve(this, [&result](const T& res) {
            result = res;
        });
    }

    ProgressResult progressResult() const
    {
        if (m_progressResults.size() != 1) {
            ADD_FAILURE() << "the progress finished " << m_progressResults.size() << " times";
            return ProgressResult();
        }

        return m_progressResults.front();
    }

    std::shared_ptr<MuseScoreComService> m_service;

    std::shared_ptr<NiceMock<CloudConfigurationMock> > m_configuration;
    std::shared_ptr<NiceMock<ApplicationMock> > m_application;
    std::shared_ptr<NiceMock<io::FileSystemMock> > m_fileSystem;
    std::shared_ptr<NiceMock<mi::MultiWindowsProviderMock> > m_multiwindowsProvider;
    std::shared_ptr<NiceMock<NetworkManagerCreatorMock> > m_networkManagerCreator;
    std::shared_ptr<NiceMock<NetworkManagerMock> > m_networkManager;

    async::Channel<std::string> m_resourceChanged;

    std::map<QString /*path*/, QByteArray> m_replies;
    std::optional<Ret> m_requestError;

    std::vector<Request> m_pendingRequests;
    Request m_noRequest;
    std::vector<ProgressResult> m_progressResults;
};
}

// =============================================
// Init
// =============================================

TEST_F(Cloud_MuseScoreComServiceTest, Init_DownloadsAccountInfo)
{
    // [WHEN] The service is initialized
    m_service->init();
    processMessages();

    // [THEN] The account info is requested with the stored access token
    const Request& request = pendingRequest(ACCOUNT_INFO_PATH);
    EXPECT_EQ(request.verb, GET);
    EXPECT_EQ(request.url.host(), HOST);
    EXPECT_EQ(queryValue(request.url, "access_token"), ACCESS_TOKEN);

    // [THEN] The request has the service headers
    EXPECT_EQ(request.headers.rawHeaders.value("Accept"), QByteArray("application/json"));
    EXPECT_EQ(request.headers.rawHeaders.value("X-MS-CLIENT-ID"), QByteArray::fromStdString(CLIENT_ID));

    // [WHEN] The account info is received
    finishRequest(ACCOUNT_INFO_PATH);

    // [THEN] The user is authorized
    EXPECT_TRUE(m_service->userAuthorized().val);

    const AccountInfo& info = m_service->accountInfo();
    EXPECT_EQ(info.id, QString::number(ACCOUNT_ID));
    EXPECT_EQ(info.userName, QString("User"));
    EXPECT_EQ(info.profileUrl, QUrl("test://users/7"));
    EXPECT_EQ(info.avatarUrl, QUrl("test://avatars/7.png"));
    EXPECT_EQ(info.collectionUrl, QUrl("test://users/7/sheetmusic"));
}

TEST_F(Cloud_MuseScoreComServiceTest, Init_NoTokens_NoRequests)
{
    // [WHEN] The service is initialized without the tokens file
    initServiceWithoutTokens();

    // [THEN] Nothing is requested, the user is not authorized
    EXPECT_TRUE(m_pendingRequests.empty());
    EXPECT_FALSE(m_service->userAuthorized().val);
}

TEST_F(Cloud_MuseScoreComServiceTest, Init_InvalidAccountInfo_NotAuthorized)
{
    // [GIVEN] The server replies with an account without a name
    willReply(ACCOUNT_INFO_PATH, R"({ "id": 7 })");

    // [WHEN] The service is initialized
    m_service->init();
    processMessages();
    finishRequest(ACCOUNT_INFO_PATH);

    // [THEN] The user is not authorized
    EXPECT_FALSE(m_service->userAuthorized().val);
}

TEST_F(Cloud_MuseScoreComServiceTest, Init_TokenExpired_RefreshesTokensAndRetries)
{
    // [GIVEN] The tokens can be refreshed
    willReply(REFRESH_TOKENS_PATH, NEW_TOKENS_JSON);

    QByteArray savedTokens;
    ON_CALL(*m_fileSystem, writeFile(_, _))
    .WillByDefault(Invoke([&savedTokens](const io::path_t&, const ByteArray& data) {
        savedTokens = data.toQByteArray();
        return muse::make_ok();
    }));

    // [WHEN] The account info request fails because the access token has expired
    m_service->init();
    processMessages();
    finishRequest(ACCOUNT_INFO_PATH, TOKEN_EXPIRED_RET);

    // [THEN] The tokens are refreshed
    EXPECT_EQ(pendingRequest(REFRESH_TOKENS_PATH).verb, POST);

    // [WHEN] The tokens are received
    finishRequest(REFRESH_TOKENS_PATH);

    // [THEN] The new tokens are saved
    QJsonObject savedTokensObj = QJsonDocument::fromJson(savedTokens).object();
    EXPECT_EQ(savedTokensObj.value("access_token").toString(), NEW_ACCESS_TOKEN);
    EXPECT_EQ(savedTokensObj.value("refresh_token").toString(), NEW_REFRESH_TOKEN);

    // [THEN] The account info is requested again with the new access token
    const Request& retry = pendingRequest(ACCOUNT_INFO_PATH);
    EXPECT_EQ(retry.verb, GET);
    EXPECT_EQ(queryValue(retry.url, "access_token"), NEW_ACCESS_TOKEN);

    // [WHEN] The account info is received
    finishRequest(ACCOUNT_INFO_PATH);

    // [THEN] The user is authorized
    EXPECT_TRUE(m_service->userAuthorized().val);
}

// =============================================
// Tokens refreshing
// =============================================

TEST_F(Cloud_MuseScoreComServiceTest, UploadAudio_TokenExpired_RefreshesTokensAndRetries)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(REFRESH_TOKENS_PATH, NEW_TOKENS_JSON);

    // [WHEN] Upload audio
    ProgressPtr progress = m_service->uploadAudio(makeDevice("audio"), "mp3", QUrl("test://scores/100"));
    watch(progress);
    processMessages();

    const Request& request = pendingRequest(AUDIO_UPLOAD_PATH);
    EXPECT_EQ(request.verb, POST);
    EXPECT_EQ(queryValue(request.url, "access_token"), ACCESS_TOKEN);

    // [WHEN] The access token has expired
    finishRequest(AUDIO_UPLOAD_PATH, TOKEN_EXPIRED_RET);

    // [THEN] The tokens are refreshed
    finishRequest(REFRESH_TOKENS_PATH);

    // [THEN] The upload is retried with the new access token
    const Request& retry = pendingRequest(AUDIO_UPLOAD_PATH);
    EXPECT_EQ(retry.verb, POST);
    EXPECT_EQ(queryValue(retry.url, "access_token"), NEW_ACCESS_TOKEN);
    EXPECT_TRUE(m_progressResults.empty());

    // [WHEN] The retried upload succeeds
    finishRequest(AUDIO_UPLOAD_PATH);

    // [THEN] The upload is finished successfully
    EXPECT_TRUE(progressResult().ret);
}

TEST_F(Cloud_MuseScoreComServiceTest, UploadAudio_TokensRefreshFailed_NoRetry)
{
    // [GIVEN] The service is initialized
    initService();

    // [GIVEN] The access token has expired
    ProgressPtr progress = m_service->uploadAudio(makeDevice("audio"), "mp3", QUrl("test://scores/100"));
    watch(progress);
    processMessages();
    finishRequest(AUDIO_UPLOAD_PATH, TOKEN_EXPIRED_RET);

    // [WHEN] The tokens refreshing fails
    finishRequest(REFRESH_TOKENS_PATH, statusRet(400));

    // [THEN] The upload fails without a retry
    EXPECT_TRUE(m_pendingRequests.empty());
    EXPECT_FALSE(progressResult().ret);
}

TEST_F(Cloud_MuseScoreComServiceTest, UploadAudio_InvalidTokensReply_NoRetry)
{
    // [GIVEN] The service is initialized
    initService();

    // [GIVEN] The access token has expired
    willReply(REFRESH_TOKENS_PATH, "not a json");

    ProgressPtr progress = m_service->uploadAudio(makeDevice("audio"), "mp3", QUrl("test://scores/100"));
    watch(progress);
    processMessages();
    finishRequest(AUDIO_UPLOAD_PATH, TOKEN_EXPIRED_RET);

    // [WHEN] The server replies with invalid tokens
    finishRequest(REFRESH_TOKENS_PATH);

    // [THEN] The upload fails without a retry
    EXPECT_TRUE(m_pendingRequests.empty());
    EXPECT_EQ(progressResult().ret.code(), code(cloud::Err::UnknownError));
}

TEST_F(Cloud_MuseScoreComServiceTest, UploadAudio_TokensSavingFailed_NoRetry)
{
    // [GIVEN] The service is initialized
    initService();

    // [GIVEN] The tokens can't be saved
    willReply(REFRESH_TOKENS_PATH, NEW_TOKENS_JSON);
    ON_CALL(*m_fileSystem, writeFile(_, _))
    .WillByDefault(Return(muse::make_ret(Ret::Code::InternalError)));

    // [GIVEN] The access token has expired
    ProgressPtr progress = m_service->uploadAudio(makeDevice("audio"), "mp3", QUrl("test://scores/100"));
    watch(progress);
    processMessages();
    finishRequest(AUDIO_UPLOAD_PATH, TOKEN_EXPIRED_RET);

    // [WHEN] The tokens are received
    finishRequest(REFRESH_TOKENS_PATH);

    // [THEN] The upload fails without a retry
    EXPECT_TRUE(m_pendingRequests.empty());
    EXPECT_EQ(progressResult().ret.code(), static_cast<int>(Ret::Code::InternalError));
}

TEST_F(Cloud_MuseScoreComServiceTest, UploadAudio_OtherError_NoTokensRefresh)
{
    // [GIVEN] The service is initialized
    initService();

    // [WHEN] Upload audio fails for a reason other than an expired token
    ProgressPtr progress = m_service->uploadAudio(makeDevice("audio"), "mp3", QUrl("test://scores/100"));
    watch(progress);
    processMessages();
    finishRequest(AUDIO_UPLOAD_PATH, statusRet(500));

    // [THEN] The upload fails, the tokens are not refreshed
    EXPECT_TRUE(m_pendingRequests.empty());
    EXPECT_FALSE(progressResult().ret);
}

// =============================================
// Score info
// =============================================

TEST_F(Cloud_MuseScoreComServiceTest, DownloadScoreInfo_ParsesReply)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(SCORE_INFO_PATH, SCORE_INFO_JSON);

    //! NOTE: the method spins an event loop until the request is finished
    Request request;
    QTimer::singleShot(0, [this, &request]() {
        request = pendingRequest(SCORE_INFO_PATH);
        finishRequest(SCORE_INFO_PATH);
    });

    // [WHEN] Download the score info
    RetVal<ScoreInfo> info = m_service->downloadScoreInfo(SCORE_ID);

    // [THEN] The info is requested by the score id
    EXPECT_EQ(request.verb, GET);
    EXPECT_EQ(queryValue(request.url, "access_token"), ACCESS_TOKEN);
    EXPECT_EQ(queryValue(request.url, "score_id"), QString::number(SCORE_ID));

    // [THEN] The info is parsed
    ASSERT_TRUE(info.ret);
    EXPECT_EQ(info.val.id, SCORE_ID);
    EXPECT_EQ(info.val.revisionId, 3);
    EXPECT_EQ(info.val.title, QString("Title"));
    EXPECT_EQ(info.val.description, QString("Description"));
    EXPECT_EQ(info.val.license, QString("all-rights-reserved"));
    EXPECT_EQ(info.val.tags, QStringList({ "piano", "solo" }));
    EXPECT_EQ(info.val.visibility, Visibility::Unlisted);
    EXPECT_EQ(info.val.url, QString("test://scores/100"));
    EXPECT_EQ(info.val.owner.id, ACCOUNT_ID);
    EXPECT_EQ(info.val.owner.userName, QString("User"));
    EXPECT_EQ(info.val.owner.profileUrl, QUrl("test://users/7"));
}

TEST_F(Cloud_MuseScoreComServiceTest, DownloadScoreInfo_ByUrl_UsesIdFromUrl)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(SCORE_INFO_PATH, SCORE_INFO_JSON);

    Request request;
    QTimer::singleShot(0, [this, &request]() {
        request = pendingRequest(SCORE_INFO_PATH);
        finishRequest(SCORE_INFO_PATH);
    });

    // [WHEN] Download the score info by the score url
    RetVal<ScoreInfo> info = m_service->downloadScoreInfo(QUrl("test://scores/100"));

    // [THEN] The score id is taken from the url
    EXPECT_EQ(queryValue(request.url, "score_id"), QString::number(SCORE_ID));
    EXPECT_TRUE(info.ret);
}

TEST_F(Cloud_MuseScoreComServiceTest, DownloadScoreInfo_Failed_ReturnsCloudError)
{
    // [GIVEN] The service is initialized
    initService();

    QTimer::singleShot(0, [this]() {
        finishRequest(SCORE_INFO_PATH, statusRet(403));
    });

    // [WHEN] Download the info of a score the user has no access to
    RetVal<ScoreInfo> info = m_service->downloadScoreInfo(SCORE_ID);

    // [THEN] The error is returned
    EXPECT_EQ(info.ret.code(), code(cloud::Err::Status403_NotOwner));
}

TEST_F(Cloud_MuseScoreComServiceTest, DownloadScoreInfo_NoAccessToken_ReturnsError)
{
    // [GIVEN] The service is initialized without the tokens file
    initServiceWithoutTokens();

    // [WHEN] Download the score info
    RetVal<ScoreInfo> info = m_service->downloadScoreInfo(SCORE_ID);

    // [THEN] The error is returned without waiting for a request
    EXPECT_EQ(info.ret.code(), code(cloud::Err::AccessTokenIsEmpty));
}

// =============================================
// Scores list
// =============================================

TEST_F(Cloud_MuseScoreComServiceTest, DownloadScoresList_ParsesReply)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(SCORES_LIST_PATH, SCORES_LIST_JSON);

    // [WHEN] Download the first batch of scores
    std::optional<ScoresList> list;
    watch(m_service->downloadScoresList(10, 1), list);
    processMessages();

    // [THEN] The batch is requested
    const Request& request = pendingRequest(SCORES_LIST_PATH);
    EXPECT_EQ(request.verb, GET);
    EXPECT_EQ(queryValue(request.url, "access_token"), ACCESS_TOKEN);
    EXPECT_EQ(queryValue(request.url, "per-page"), QString("10"));
    EXPECT_EQ(queryValue(request.url, "page"), QString("1"));

    // [WHEN] The batch is received
    finishRequest(SCORES_LIST_PATH);

    // [THEN] The meta is parsed
    ASSERT_TRUE(list.has_value());
    EXPECT_EQ(list->meta.totalScoresCount, 12);
    EXPECT_EQ(list->meta.batchesCount, 2);
    EXPECT_EQ(list->meta.thisBatchNumber, 1);
    EXPECT_EQ(list->meta.scoresPerBatch, 10);

    // [THEN] The scores are parsed
    ASSERT_EQ(list->items.size(), 2u);

    const ScoresList::Item& converted = list->items.at(0);
    EXPECT_EQ(converted.id, 1);
    EXPECT_EQ(converted.title, QString("Converted"));
    EXPECT_EQ(converted.lastModified.toSecsSinceEpoch(), 1700000000);
    EXPECT_EQ(converted.fileSize, static_cast<size_t>(2048));
    EXPECT_EQ(converted.thumbnailUrl, QString("test://thumbnails/1.png"));
    EXPECT_EQ(converted.visibility, Visibility::Public);
    EXPECT_EQ(converted.viewCount, 5);

    ASSERT_TRUE(converted.conversion.has_value());
    EXPECT_EQ(converted.conversion->id, 9);
    EXPECT_EQ(converted.conversion->type, ConvertType::Audio2Score);
    EXPECT_EQ(converted.conversion->status, ConvertStatus::AwaitingReview);

    const ScoresList::Item& regular = list->items.at(1);
    EXPECT_EQ(regular.id, 2);
    EXPECT_EQ(regular.visibility, Visibility::Private);
    EXPECT_FALSE(regular.conversion.has_value());
}

TEST_F(Cloud_MuseScoreComServiceTest, DownloadScoresList_BatchNumberTooHigh_ReturnsNoScores)
{
    // [GIVEN] The service is initialized
    initService();

    // [GIVEN] The server replies with the last batch when the requested one doesn't exist
    willReply(SCORES_LIST_PATH, LAST_PAGE_SCORES_LIST_JSON);

    // [WHEN] Download a batch that doesn't exist
    std::optional<ScoresList> list;
    watch(m_service->downloadScoresList(10, 5), list);
    processMessages();
    finishRequest(SCORES_LIST_PATH);

    // [THEN] No scores are returned
    ASSERT_TRUE(list.has_value());
    EXPECT_TRUE(list->items.empty());
    EXPECT_EQ(list->meta.totalScoresCount, 12);
    EXPECT_EQ(list->meta.thisBatchNumber, 2);
}

TEST_F(Cloud_MuseScoreComServiceTest, DownloadScoresList_RequestFailed_Rejects)
{
    // [GIVEN] The service is initialized
    initService();

    // [WHEN] Download the scores, the request fails
    bool resolved = false;
    std::optional<int> rejectCode;
    m_service->downloadScoresList(10, 1).onResolve(this, [&resolved](const ScoresList&) {
        resolved = true;
    }).onReject(this, [&rejectCode](int code, const std::string&) {
        rejectCode = code;
    });
    processMessages();

    Ret error = statusRet(500);
    finishRequest(SCORES_LIST_PATH, error);

    // [THEN] The promise is rejected
    EXPECT_FALSE(resolved);
    ASSERT_TRUE(rejectCode.has_value());
    EXPECT_EQ(*rejectCode, error.code());
}

TEST_F(Cloud_MuseScoreComServiceTest, DownloadScoresList_InvalidReply_Rejects)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(SCORES_LIST_PATH, "not a json");

    // [WHEN] Download the scores, the server replies with garbage
    std::optional<int> rejectCode;
    m_service->downloadScoresList(10, 1).onReject(this, [&rejectCode](int code, const std::string&) {
        rejectCode = code;
    });
    processMessages();
    finishRequest(SCORES_LIST_PATH);

    // [THEN] The promise is rejected
    ASSERT_TRUE(rejectCode.has_value());
    EXPECT_EQ(*rejectCode, static_cast<int>(Ret::Code::InternalError));
}

TEST_F(Cloud_MuseScoreComServiceTest, DownloadScoresList_NoAccessToken_Rejects)
{
    // [GIVEN] The service is initialized without the tokens file
    initServiceWithoutTokens();

    // [WHEN] Download the scores without being signed in
    std::optional<int> rejectCode;
    m_service->downloadScoresList(10, 1).onReject(this, [&rejectCode](int code, const std::string&) {
        rejectCode = code;
    });
    processMessages();

    // [THEN] The promise is rejected without a request
    EXPECT_TRUE(m_pendingRequests.empty());
    ASSERT_TRUE(rejectCode.has_value());
    EXPECT_EQ(*rejectCode, code(cloud::Err::AccessTokenIsEmpty));
}

// =============================================
// Score downloading
// =============================================

TEST_F(Cloud_MuseScoreComServiceTest, DownloadScore_OwnScore)
{
    // [GIVEN] The service is initialized
    initService();

    // [WHEN] Download an own score
    ProgressPtr progress = m_service->downloadScore(SCORE_ID, makeDevice());
    watch(progress);
    processMessages();

    // [THEN] The score is requested by its id
    const Request& request = pendingRequest(SCORE_DOWNLOAD_PATH);
    EXPECT_EQ(request.verb, GET);
    EXPECT_EQ(queryValue(request.url, "access_token"), ACCESS_TOKEN);
    EXPECT_EQ(queryValue(request.url, "score_id"), QString::number(SCORE_ID));
    EXPECT_FALSE(QUrlQuery(request.url).hasQueryItem("h"));
    EXPECT_FALSE(QUrlQuery(request.url).hasQueryItem("secret"));

    // [WHEN] The score is downloaded
    finishRequest(SCORE_DOWNLOAD_PATH);

    // [THEN] The downloading is finished successfully
    EXPECT_TRUE(progressResult().ret);
}

TEST_F(Cloud_MuseScoreComServiceTest, DownloadScore_SharedScore)
{
    // [GIVEN] The service is initialized
    initService();

    // [WHEN] Download a score shared by a link
    ProgressPtr progress = m_service->downloadScore(SCORE_ID, makeDevice(), "hash", "secret");
    processMessages();

    // [THEN] The score is requested by its id, hash and secret
    const Request& request = pendingRequest(SCORE_DOWNLOAD_SHARED_PATH);
    EXPECT_EQ(request.verb, GET);
    EXPECT_EQ(queryValue(request.url, "score_id"), QString::number(SCORE_ID));
    EXPECT_EQ(queryValue(request.url, "h"), QString("hash"));
    EXPECT_EQ(queryValue(request.url, "secret"), QString("secret"));
}

TEST_F(Cloud_MuseScoreComServiceTest, DownloadScore_ReportsProgress)
{
    // [GIVEN] The service is initialized
    initService();

    ProgressPtr progress = m_service->downloadScore(SCORE_ID, makeDevice());

    int64_t current = 0;
    int64_t total = 0;
    progress->progressChanged().onReceive(this, [&current, &total](int64_t cur, int64_t tot, const std::string&) {
        current = cur;
        total = tot;
    });
    processMessages();

    // [WHEN] The network request progresses
    Progress requestProgress = pendingRequest(SCORE_DOWNLOAD_PATH).progress;
    requestProgress.progress(5, 10);
    processMessages();

    // [THEN] The progress is forwarded
    EXPECT_EQ(current, 5);
    EXPECT_EQ(total, 10);
}

TEST_F(Cloud_MuseScoreComServiceTest, DownloadScore_Failed_ReturnsCloudError)
{
    // [GIVEN] The service is initialized
    initService();

    // [WHEN] Download a score that doesn't exist
    ProgressPtr progress = m_service->downloadScore(SCORE_ID, makeDevice());
    watch(progress);
    processMessages();
    finishRequest(SCORE_DOWNLOAD_PATH, statusRet(404));

    // [THEN] The downloading fails
    EXPECT_EQ(progressResult().ret.code(), code(cloud::Err::Status404_NotFound));
}

// =============================================
// Score deleting
// =============================================

TEST_F(Cloud_MuseScoreComServiceTest, DeleteScore_Success)
{
    // [GIVEN] The service is initialized
    initService();

    // [WHEN] Delete a score
    std::optional<Ret> ret;
    watch(m_service->deleteScore(SCORE_ID), ret);
    processMessages();

    // [THEN] The deletion is requested
    const Request& request = pendingRequest(SCORE_DELETE_PATH);
    EXPECT_EQ(request.verb, DELETE_);
    EXPECT_EQ(queryValue(request.url, "access_token"), ACCESS_TOKEN);
    EXPECT_EQ(queryValue(request.url, "score_id"), QString::number(SCORE_ID));
    EXPECT_FALSE(ret.has_value());

    // [WHEN] The score is deleted
    finishRequest(SCORE_DELETE_PATH);

    // [THEN] The promise is resolved successfully
    ASSERT_TRUE(ret.has_value());
    EXPECT_TRUE(*ret);
}

TEST_F(Cloud_MuseScoreComServiceTest, DeleteScore_TokenExpired_RefreshesTokensAndRetries)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(REFRESH_TOKENS_PATH, NEW_TOKENS_JSON);

    // [GIVEN] Delete a score
    std::optional<Ret> ret;
    watch(m_service->deleteScore(SCORE_ID), ret);
    processMessages();

    // [WHEN] The access token has expired
    finishRequest(SCORE_DELETE_PATH, TOKEN_EXPIRED_RET);

    // [THEN] The tokens are refreshed
    finishRequest(REFRESH_TOKENS_PATH);

    // [THEN] The deletion is retried with the new access token
    const Request& retry = pendingRequest(SCORE_DELETE_PATH);
    EXPECT_EQ(retry.verb, DELETE_);
    EXPECT_EQ(queryValue(retry.url, "access_token"), NEW_ACCESS_TOKEN);
    EXPECT_FALSE(ret.has_value());

    // [WHEN] The retried deletion succeeds
    finishRequest(SCORE_DELETE_PATH);

    // [THEN] The promise is resolved successfully
    ASSERT_TRUE(ret.has_value());
    EXPECT_TRUE(*ret);
}

TEST_F(Cloud_MuseScoreComServiceTest, DeleteScore_Failed_ReturnsError)
{
    // [GIVEN] The service is initialized
    initService();

    // [WHEN] The deletion fails
    std::optional<Ret> ret;
    watch(m_service->deleteScore(SCORE_ID), ret);
    processMessages();
    finishRequest(SCORE_DELETE_PATH, statusRet(404));

    // [THEN] The error is returned
    ASSERT_TRUE(ret.has_value());
    EXPECT_EQ(ret->code(), code(cloud::Err::Status404_NotFound));
}

TEST_F(Cloud_MuseScoreComServiceTest, DeleteScore_UnknownStatusCode)
{
    // [GIVEN] The service is initialized
    initService();

    // [WHEN] The deletion fails with an unexpected status code
    std::optional<Ret> ret;
    watch(m_service->deleteScore(SCORE_ID), ret);
    processMessages();
    finishRequest(SCORE_DELETE_PATH, statusRet(418));

    // [THEN] The error keeps the status code
    ASSERT_TRUE(ret.has_value());
    EXPECT_EQ(ret->code(), code(cloud::Err::UnknownStatusCode));
    EXPECT_EQ(ret->data<int>("status", 0), 418);
}

TEST_F(Cloud_MuseScoreComServiceTest, DeleteScore_NetworkError_ReturnsError)
{
    // [GIVEN] The service is initialized
    initService();

    // [WHEN] The deletion fails without a status code
    std::optional<Ret> ret;
    watch(m_service->deleteScore(SCORE_ID), ret);
    processMessages();
    finishRequest(SCORE_DELETE_PATH, muse::network::make_ret(muse::network::Err::NetworkError));

    // [THEN] The network error is returned
    ASSERT_TRUE(ret.has_value());
    EXPECT_EQ(ret->code(), code(cloud::Err::NetworkError));
}

TEST_F(Cloud_MuseScoreComServiceTest, DeleteScore_RequestNotStarted_ReturnsError)
{
    // [GIVEN] The service is initialized
    initService();

    // [GIVEN] The network manager can't start requests
    m_requestError = muse::network::make_ret(muse::network::Err::FiledOpenIODeviceWrite);

    // [WHEN] Delete a score
    std::optional<Ret> ret;
    watch(m_service->deleteScore(SCORE_ID), ret);
    processMessages();

    // [THEN] The error is returned
    ASSERT_TRUE(ret.has_value());
    EXPECT_EQ(ret->code(), m_requestError->code());
}

TEST_F(Cloud_MuseScoreComServiceTest, DeleteScore_NoAccessToken_ReturnsError)
{
    // [GIVEN] The service is initialized without the tokens file
    initServiceWithoutTokens();

    // [WHEN] Delete a score without being signed in
    std::optional<Ret> ret;
    watch(m_service->deleteScore(SCORE_ID), ret);
    processMessages();

    // [THEN] The error is returned without a request
    EXPECT_TRUE(m_pendingRequests.empty());
    ASSERT_TRUE(ret.has_value());
    EXPECT_EQ(ret->code(), code(cloud::Err::AccessTokenIsEmpty));
}

// =============================================
// Score uploading
// =============================================

TEST_F(Cloud_MuseScoreComServiceTest, UploadScore_NewScore_Posts)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(SCORE_UPLOAD_PATH, SCORE_UPLOAD_JSON);

    // [WHEN] Upload a score that has no source url
    ProgressPtr progress = m_service->uploadScore(makeDevice("score"), "Title");
    watch(progress);
    processMessages();

    // [THEN] The score is posted without checking whether it exists
    const Request& request = pendingRequest(SCORE_UPLOAD_PATH);
    EXPECT_EQ(request.verb, POST);
    EXPECT_EQ(queryValue(request.url, "access_token"), ACCESS_TOKEN);

    // [WHEN] The score is uploaded
    finishRequest(SCORE_UPLOAD_PATH);

    // [THEN] The uploading is finished once, with the info about the uploaded score
    ASSERT_TRUE(progressResult().ret);

    ValMap result = progressResult().val.toMap();
    EXPECT_EQ(result["sourceUrl"].toQString(), QString("test://scores/100"));
    EXPECT_EQ(result["editUrl"].toQString(), QString("test://scores/100/edit"));
    EXPECT_EQ(result["revisionId"].toInt(), 4);
}

TEST_F(Cloud_MuseScoreComServiceTest, UploadScore_OwnScore_Puts)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(SCORE_INFO_PATH, SCORE_INFO_JSON);
    willReply(SCORE_UPLOAD_PATH, SCORE_UPLOAD_JSON);

    // [WHEN] Upload a score that has a source url
    ProgressPtr progress = m_service->uploadScore(makeDevice("score"), "Title", Visibility::Private, QUrl("test://scores/100"), 3);
    watch(progress);
    processMessages();

    // [THEN] The score info is requested first
    const Request& request = pendingRequest(SCORE_INFO_PATH);
    EXPECT_EQ(request.verb, GET);
    EXPECT_EQ(queryValue(request.url, "score_id"), QString::number(SCORE_ID));

    // [WHEN] The score is owned by the user
    finishRequest(SCORE_INFO_PATH);

    // [THEN] The existing score is updated
    EXPECT_EQ(pendingRequest(SCORE_UPLOAD_PATH).verb, PUT);

    // [WHEN] The score is uploaded
    finishRequest(SCORE_UPLOAD_PATH);

    // [THEN] The uploading is finished successfully
    EXPECT_TRUE(progressResult().ret);
}

TEST_F(Cloud_MuseScoreComServiceTest, UploadScore_ScoreOfOtherUser_Posts)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(SCORE_INFO_PATH, OTHER_USER_SCORE_INFO_JSON);
    willReply(SCORE_UPLOAD_PATH, SCORE_UPLOAD_JSON);

    // [WHEN] Upload a score whose source url points to a score of another user
    ProgressPtr progress = m_service->uploadScore(makeDevice("score"), "Title", Visibility::Private, QUrl("test://scores/100"));
    processMessages();
    finishRequest(SCORE_INFO_PATH);

    // [THEN] A new score is posted
    EXPECT_EQ(pendingRequest(SCORE_UPLOAD_PATH).verb, POST);
}

TEST_F(Cloud_MuseScoreComServiceTest, UploadScore_ScoreNotFoundOnServer_Posts)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(SCORE_UPLOAD_PATH, SCORE_UPLOAD_JSON);

    // [GIVEN] Upload a score that has a source url
    ProgressPtr progress = m_service->uploadScore(makeDevice("score"), "Title", Visibility::Private, QUrl("test://scores/100"));
    watch(progress);
    processMessages();

    // [WHEN] The score no longer exists on the server
    finishRequest(SCORE_INFO_PATH, statusRet(404));

    // [THEN] A new score is posted
    EXPECT_EQ(pendingRequest(SCORE_UPLOAD_PATH).verb, POST);

    // [WHEN] The score is uploaded
    finishRequest(SCORE_UPLOAD_PATH);

    // [THEN] The uploading is finished successfully
    EXPECT_TRUE(progressResult().ret);
}

TEST_F(Cloud_MuseScoreComServiceTest, UploadScore_ScoreInfoFailed_NoUpload)
{
    // [GIVEN] The service is initialized
    initService();

    // [WHEN] Upload a score, the score info request fails
    ProgressPtr progress = m_service->uploadScore(makeDevice("score"), "Title", Visibility::Private, QUrl("test://scores/100"));
    watch(progress);
    processMessages();
    finishRequest(SCORE_INFO_PATH, statusRet(500));

    // [THEN] The uploading fails, nothing is uploaded
    EXPECT_TRUE(m_pendingRequests.empty());
    EXPECT_EQ(progressResult().ret.code(), code(cloud::Err::Status500_InternalServerError));
}

TEST_F(Cloud_MuseScoreComServiceTest, UploadScore_Forbidden_NewScore)
{
    // [GIVEN] The service is initialized
    initService();

    // [WHEN] Posting a new score is forbidden
    ProgressPtr progress = m_service->uploadScore(makeDevice("score"), "Title");
    watch(progress);
    processMessages();
    finishRequest(SCORE_UPLOAD_PATH, statusRet(403));

    // [THEN] The uploading fails
    EXPECT_EQ(progressResult().ret.code(), code(cloud::Err::Status403_NotOwner));
}

TEST_F(Cloud_MuseScoreComServiceTest, UploadScore_Forbidden_OwnScore)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(SCORE_INFO_PATH, SCORE_INFO_JSON);

    // [WHEN] Updating an own score is forbidden
    ProgressPtr progress = m_service->uploadScore(makeDevice("score"), "Title", Visibility::Private, QUrl("test://scores/100"));
    watch(progress);
    processMessages();
    finishRequest(SCORE_INFO_PATH);
    finishRequest(SCORE_UPLOAD_PATH, statusRet(403));

    // [THEN] The uploading fails because the account is not activated
    EXPECT_EQ(progressResult().ret.code(), code(cloud::Err::Status403_AccountNotActivated));
}

TEST_F(Cloud_MuseScoreComServiceTest, UploadScore_NoSourceUrlInReply_Fails)
{
    // [GIVEN] The service is initialized
    initService();

    // [GIVEN] The server doesn't reply with the url of the uploaded score
    willReply(SCORE_UPLOAD_PATH, "{}");

    // [WHEN] Upload a score
    ProgressPtr progress = m_service->uploadScore(makeDevice("score"), "Title");
    watch(progress);
    processMessages();
    finishRequest(SCORE_UPLOAD_PATH);

    // [THEN] The uploading fails
    EXPECT_EQ(progressResult().ret.code(), code(cloud::Err::CouldNotReceiveSourceUrl));
}

// =============================================
// Convert: config
// =============================================

TEST_F(Cloud_MuseScoreComServiceTest, FetchConfig_ParsesReply)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(CONVERT_CONFIG_PATH, CONVERT_CONFIG_JSON);

    // [WHEN] Fetch the config
    std::optional<RetVal<ConvertConfig> > config;
    watch(m_service->fetchConfig(), config);
    processMessages();

    const Request& request = pendingRequest(CONVERT_CONFIG_PATH);
    EXPECT_EQ(request.verb, GET);

    finishRequest(CONVERT_CONFIG_PATH);

    // [THEN] The config is parsed
    ASSERT_TRUE(config.has_value());
    ASSERT_TRUE(config->ret);
    EXPECT_EQ(config->val.version, 3);

    const OmrConfig& omr = config->val.omr;
    EXPECT_EQ(omr.pdf.maxFileSizeBytes, 52428800);
    EXPECT_EQ(omr.pdf.maxFiles, 1);
    EXPECT_EQ(omr.pdf.maxPages, 50);
    EXPECT_EQ(omr.images.maxFileSizeBytes, 10485760);
    EXPECT_EQ(omr.images.maxFiles, 20);

    // [THEN] The MIME types are resolved to the extensions, the unknown ones fall back to their subtype
    EXPECT_TRUE(omr.images.allowedExtensions.contains("jpg"));
    EXPECT_TRUE(omr.images.allowedExtensions.contains("jpeg"));
    EXPECT_TRUE(omr.images.allowedExtensions.contains("png"));
    EXPECT_TRUE(omr.images.allowedExtensions.contains("x-nonexistent-format"));

    const Audio2ScoreConfig& audio2score = config->val.audio2score;
    EXPECT_EQ(audio2score.file.maxFileSizeBytes, 104857600);
    EXPECT_EQ(audio2score.file.maxFiles, 2);
    EXPECT_EQ(audio2score.file.allowedExtensions, QStringList({ "mp3", "wav" }));
    EXPECT_EQ(audio2score.link.maxLength, 2048);

    // [THEN] The unknown link sources are ignored
    EXPECT_TRUE(audio2score.link.allowedSources.testFlag(LinkSource::YouTube));
    EXPECT_TRUE(audio2score.link.allowedSources.testFlag(LinkSource::AudioCom));
}

TEST_F(Cloud_MuseScoreComServiceTest, FetchConfig_NoAccessToken_StillRequested)
{
    // [GIVEN] The service is initialized without the tokens file
    initServiceWithoutTokens();
    willReply(CONVERT_CONFIG_PATH, CONVERT_CONFIG_JSON);

    // [WHEN] Fetch the config without being signed in
    std::optional<RetVal<ConvertConfig> > config;
    watch(m_service->fetchConfig(), config);
    processMessages();

    // [THEN] The config is requested without an access token
    const Request& request = pendingRequest(CONVERT_CONFIG_PATH);
    EXPECT_FALSE(QUrlQuery(request.url).hasQueryItem("access_token"));

    finishRequest(CONVERT_CONFIG_PATH);

    ASSERT_TRUE(config.has_value());
    EXPECT_TRUE(config->ret);
}

TEST_F(Cloud_MuseScoreComServiceTest, FetchConfig_InvalidReply_Fails)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(CONVERT_CONFIG_PATH, "not a json");

    // [WHEN] Fetch the config, the server replies with garbage
    std::optional<RetVal<ConvertConfig> > config;
    watch(m_service->fetchConfig(), config);
    processMessages();
    finishRequest(CONVERT_CONFIG_PATH);

    // [THEN] The error is returned
    ASSERT_TRUE(config.has_value());
    EXPECT_EQ(config->ret.code(), static_cast<int>(Ret::Code::InternalError));
}

TEST_F(Cloud_MuseScoreComServiceTest, FetchConfig_Failed_ReturnsCloudError)
{
    // [GIVEN] The service is initialized
    initService();

    // [WHEN] Fetch the config, the request fails
    std::optional<RetVal<ConvertConfig> > config;
    watch(m_service->fetchConfig(), config);
    processMessages();
    finishRequest(CONVERT_CONFIG_PATH, statusRet(500));

    // [THEN] The error is returned
    ASSERT_TRUE(config.has_value());
    EXPECT_EQ(config->ret.code(), code(cloud::Err::Status500_InternalServerError));
}

// =============================================
// Convert: start
// =============================================

TEST_F(Cloud_MuseScoreComServiceTest, UploadToConvert_Success)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(CONVERT_PATH, CONVERT_RESULT_JSON);

    // [WHEN] Start a conversion
    ProgressPtr progress = m_service->uploadToConvert(makeConvertUploadData());
    watch(progress);
    processMessages();

    // [THEN] The files are posted
    const Request& request = pendingRequest(CONVERT_PATH);
    EXPECT_EQ(request.verb, POST);
    EXPECT_EQ(queryValue(request.url, "access_token"), ACCESS_TOKEN);

    // [WHEN] The files are uploaded
    finishRequest(CONVERT_PATH);

    // [THEN] The uploading is finished once, with the info about the conversion
    ASSERT_TRUE(progressResult().ret);

    ValMap result = progressResult().val.toMap();
    EXPECT_EQ(result["id"].toInt(), 11);
    EXPECT_EQ(result["type"].toQString(), QString("omr"));
    EXPECT_EQ(result["status"].toInt(), static_cast<int>(ConvertStatus::Processing));
}

TEST_F(Cloud_MuseScoreComServiceTest, UploadToConvert_ReportsProgress)
{
    // [GIVEN] The service is initialized
    initService();

    ProgressPtr progress = m_service->uploadToConvert(makeConvertUploadData());

    int64_t current = 0;
    int64_t total = 0;
    progress->progressChanged().onReceive(this, [&current, &total](int64_t cur, int64_t tot, const std::string&) {
        current = cur;
        total = tot;
    });
    processMessages();

    // [WHEN] The network request progresses
    Progress requestProgress = pendingRequest(CONVERT_PATH).progress;
    requestProgress.progress(5, 10);
    processMessages();

    // [THEN] The progress is forwarded
    EXPECT_EQ(current, 5);
    EXPECT_EQ(total, 10);
}

TEST_F(Cloud_MuseScoreComServiceTest, UploadToConvert_Canceled_CancelsRequest)
{
    // [GIVEN] The service is initialized, and the files are being uploaded
    initService();

    ProgressPtr progress = m_service->uploadToConvert(makeConvertUploadData());
    watch(progress);
    processMessages();

    // [WHEN] The uploading is canceled
    progress->cancel();
    processMessages();

    // [THEN] The request is canceled, and the uploading is finished once, as canceled
    EXPECT_TRUE(pendingRequest(CONVERT_PATH).progress.isCanceled());
    EXPECT_EQ(progressResult().ret.code(), static_cast<int>(Ret::Code::Cancel));
}

TEST_F(Cloud_MuseScoreComServiceTest, UploadToConvert_CanceledWhileRefreshingTokens_NoRetry)
{
    // [GIVEN] The service is initialized, and the access token has expired during the upload
    initService();

    willReply(REFRESH_TOKENS_PATH, NEW_TOKENS_JSON);

    ProgressPtr progress = m_service->uploadToConvert(makeConvertUploadData());
    watch(progress);
    processMessages();
    finishRequest(CONVERT_PATH, TOKEN_EXPIRED_RET);

    // [WHEN] The uploading is canceled before the tokens are refreshed
    progress->cancel();
    finishRequest(REFRESH_TOKENS_PATH);

    // [THEN] The upload is not retried, and the uploading is finished once, as canceled
    EXPECT_TRUE(findPendingRequest(CONVERT_PATH) == m_pendingRequests.end());
    EXPECT_EQ(progressResult().ret.code(), static_cast<int>(Ret::Code::Cancel));
}

TEST_F(Cloud_MuseScoreComServiceTest, UploadToConvert_Failed_ReturnsServerErrorCode)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(CONVERT_PATH, errorCodeJson("file_too_large"));

    // [WHEN] Start a conversion, the server rejects the files
    ProgressPtr progress = m_service->uploadToConvert(makeConvertUploadData());
    watch(progress);
    processMessages();
    finishRequest(CONVERT_PATH, statusRet(422));

    // [THEN] The uploading fails once, with the error code from the server
    EXPECT_EQ(progressResult().ret.code(), code(cloud::Err::Status422_ValidationFailed));
    EXPECT_EQ(convertErrorCode(progressResult().ret), ConvertErrorCode::FileTooLarge);
}

TEST_F(Cloud_MuseScoreComServiceTest, UploadToConvert_InvalidReply_Fails)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(CONVERT_PATH, "not a json");

    // [WHEN] Start a conversion, the server replies with garbage
    ProgressPtr progress = m_service->uploadToConvert(makeConvertUploadData());
    watch(progress);
    processMessages();
    finishRequest(CONVERT_PATH);

    // [THEN] The uploading fails
    EXPECT_EQ(progressResult().ret.code(), static_cast<int>(Ret::Code::InternalError));
}

TEST_F(Cloud_MuseScoreComServiceTest, UploadToConvert_NoAccessToken_Fails)
{
    // [GIVEN] The service is initialized without the tokens file
    initServiceWithoutTokens();

    // [WHEN] Start a conversion without being signed in
    ProgressPtr progress = m_service->uploadToConvert(makeConvertUploadData());
    watch(progress);
    processMessages();

    // [THEN] The uploading fails without a request
    EXPECT_TRUE(m_pendingRequests.empty());
    EXPECT_EQ(progressResult().ret.code(), code(cloud::Err::AccessTokenIsEmpty));
}

// =============================================
// Convert: queue
// =============================================

TEST_F(Cloud_MuseScoreComServiceTest, FetchQueue_ParsesReply)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(CONVERT_QUEUE_PATH, CONVERT_QUEUE_JSON);

    // [WHEN] Fetch the queue
    std::optional<RetVal<ConvertQueueList> > queue;
    watch(m_service->fetchQueue(), queue);
    processMessages();

    const Request& request = pendingRequest(CONVERT_QUEUE_PATH);
    EXPECT_EQ(request.verb, GET);
    EXPECT_EQ(queryValue(request.url, "access_token"), ACCESS_TOKEN);

    finishRequest(CONVERT_QUEUE_PATH);

    // [THEN] The queue is parsed
    ASSERT_TRUE(queue.has_value());
    ASSERT_TRUE(queue->ret);
    ASSERT_EQ(queue->val.size(), 3u);

    const ConvertQueueItem& done = queue->val.at(0);
    EXPECT_EQ(done.id, 1);
    EXPECT_EQ(done.type, ConvertType::Omr);
    EXPECT_EQ(done.status, ConvertStatus::Done);
    EXPECT_EQ(done.filename, QString("score.pdf"));
    EXPECT_TRUE(done.link.isEmpty());
    ASSERT_TRUE(done.scoreId.has_value());
    EXPECT_EQ(*done.scoreId, 55);
    EXPECT_EQ(done.dataCreated.toSecsSinceEpoch(), 1700000000);
    EXPECT_EQ(done.dataUpdated.toSecsSinceEpoch(), 1700000100);
    EXPECT_EQ(done.errorCode, ConvertErrorCode::Unknown);

    const ConvertQueueItem& failed = queue->val.at(1);
    EXPECT_EQ(failed.id, 2);
    EXPECT_EQ(failed.type, ConvertType::Audio2Score);
    EXPECT_EQ(failed.status, ConvertStatus::Failed);
    EXPECT_EQ(failed.link, QString("test://audio/abc"));
    EXPECT_FALSE(failed.scoreId.has_value());
    EXPECT_EQ(failed.errorCode, ConvertErrorCode::TooComplex);

    const ConvertQueueItem& unknown = queue->val.at(2);
    EXPECT_EQ(unknown.id, 3);
    EXPECT_EQ(unknown.type, ConvertType::Unknown);
    EXPECT_EQ(unknown.status, ConvertStatus::Unknown);
    EXPECT_FALSE(unknown.scoreId.has_value());
}

TEST_F(Cloud_MuseScoreComServiceTest, FetchQueue_Failed_ReturnsServerErrorCode)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(CONVERT_QUEUE_PATH, errorCodeJson("rate_limited"));

    // [WHEN] Fetch the queue too often
    std::optional<RetVal<ConvertQueueList> > queue;
    watch(m_service->fetchQueue(), queue);
    processMessages();
    finishRequest(CONVERT_QUEUE_PATH, statusRet(429));

    // [THEN] The error is returned with the error code from the server
    ASSERT_TRUE(queue.has_value());
    EXPECT_EQ(queue->ret.code(), code(cloud::Err::Status429_RateLimitExceeded));
    EXPECT_EQ(convertErrorCode(queue->ret), ConvertErrorCode::RateLimited);
    EXPECT_TRUE(queue->val.empty());
}

TEST_F(Cloud_MuseScoreComServiceTest, FetchQueue_NoAccessToken_Fails)
{
    // [GIVEN] The service is initialized without the tokens file
    initServiceWithoutTokens();

    // [WHEN] Fetch the queue without being signed in
    std::optional<RetVal<ConvertQueueList> > queue;
    watch(m_service->fetchQueue(), queue);
    processMessages();

    // [THEN] The error is returned without a request
    EXPECT_TRUE(m_pendingRequests.empty());
    ASSERT_TRUE(queue.has_value());
    EXPECT_EQ(queue->ret.code(), code(cloud::Err::AccessTokenIsEmpty));
}

// =============================================
// Convert: review
// =============================================

TEST_F(Cloud_MuseScoreComServiceTest, SubmitReview_Success)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(CONVERT_REVIEW_PATH, R"({ "id": 11, "type": "audio2score", "status": "done" })");

    // [WHEN] Submit a review
    std::optional<RetVal<ConvertResult> > result;
    watch(m_service->submitReview(ConvertType::Audio2Score, 11, ReviewRating::Good), result);
    processMessages();

    // [THEN] The review is posted
    const Request& request = pendingRequest(CONVERT_REVIEW_PATH);
    EXPECT_EQ(request.verb, POST);
    EXPECT_EQ(queryValue(request.url, "access_token"), ACCESS_TOKEN);

    finishRequest(CONVERT_REVIEW_PATH);

    // [THEN] The updated conversion is returned
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->ret);
    EXPECT_EQ(result->val.id, 11);
    EXPECT_EQ(result->val.type, ConvertType::Audio2Score);
    EXPECT_EQ(result->val.status, ConvertStatus::Done);
}

TEST_F(Cloud_MuseScoreComServiceTest, SubmitReview_Failed_ReturnsServerErrorCode)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(CONVERT_REVIEW_PATH, errorCodeJson("no_need_review"));

    // [WHEN] Submit a review for a conversion that doesn't need it
    std::optional<RetVal<ConvertResult> > result;
    watch(m_service->submitReview(ConvertType::Omr, 11, ReviewRating::Bad, "Comment"), result);
    processMessages();
    finishRequest(CONVERT_REVIEW_PATH, statusRet(400));

    // [THEN] The error is returned with the error code from the server
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->ret.code(), code(cloud::Err::Status400_InvalidRequest));
    EXPECT_EQ(convertErrorCode(result->ret), ConvertErrorCode::NoNeedReview);
}

TEST_F(Cloud_MuseScoreComServiceTest, SubmitReviewComment_Success)
{
    // [GIVEN] The service is initialized
    initService();

    // [WHEN] Submit a review comment
    std::optional<Ret> ret;
    watch(m_service->submitReviewComment(ConvertType::Omr, 11, "Comment"), ret);
    processMessages();

    // [THEN] The comment is posted
    const Request& request = pendingRequest(CONVERT_COMMENT_PATH);
    EXPECT_EQ(request.verb, POST);
    EXPECT_EQ(queryValue(request.url, "access_token"), ACCESS_TOKEN);

    finishRequest(CONVERT_COMMENT_PATH);

    ASSERT_TRUE(ret.has_value());
    EXPECT_TRUE(*ret);
}

TEST_F(Cloud_MuseScoreComServiceTest, SubmitReviewComment_Failed_ReturnsServerErrorCode)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(CONVERT_COMMENT_PATH, errorCodeJson("comment_required"));

    // [WHEN] Submit an empty review comment
    std::optional<Ret> ret;
    watch(m_service->submitReviewComment(ConvertType::Omr, 11, QString()), ret);
    processMessages();
    finishRequest(CONVERT_COMMENT_PATH, statusRet(422));

    // [THEN] The error is returned with the error code from the server
    ASSERT_TRUE(ret.has_value());
    EXPECT_EQ(ret->code(), code(cloud::Err::Status422_ValidationFailed));
    EXPECT_EQ(convertErrorCode(*ret), ConvertErrorCode::CommentRequired);
}

// =============================================
// Convert: delete
// =============================================

TEST_F(Cloud_MuseScoreComServiceTest, DeleteConversion_Success)
{
    // [GIVEN] The service is initialized
    initService();

    // [WHEN] Delete a conversion
    std::optional<Ret> ret;
    watch(m_service->deleteConversion(ConvertType::Audio2Score, 5), ret);
    processMessages();

    // [THEN] The deletion is requested by the conversion type and id
    const Request& request = pendingRequest(CONVERT_PATH);
    EXPECT_EQ(request.verb, DELETE_);
    EXPECT_EQ(queryValue(request.url, "access_token"), ACCESS_TOKEN);
    EXPECT_EQ(queryValue(request.url, "type"), QString("audio2score"));
    EXPECT_EQ(queryValue(request.url, "id"), QString("5"));

    finishRequest(CONVERT_PATH);

    ASSERT_TRUE(ret.has_value());
    EXPECT_TRUE(*ret);
}

TEST_F(Cloud_MuseScoreComServiceTest, DeleteConversion_Failed_ReturnsServerErrorCode)
{
    // [GIVEN] The service is initialized
    initService();

    willReply(CONVERT_PATH, errorCodeJson("bad_params"));

    // [WHEN] Delete a conversion that doesn't exist
    std::optional<Ret> ret;
    watch(m_service->deleteConversion(ConvertType::Omr, 5), ret);
    processMessages();
    finishRequest(CONVERT_PATH, statusRet(400));

    // [THEN] The error is returned with the error code from the server
    ASSERT_TRUE(ret.has_value());
    EXPECT_EQ(ret->code(), code(cloud::Err::Status400_InvalidRequest));
    EXPECT_EQ(convertErrorCode(*ret), ConvertErrorCode::BadParams);
}
