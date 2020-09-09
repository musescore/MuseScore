//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "cloudmanager.h"
#include "cloudmanager_p.h"
#include "log.h"

#include "modularity/ioc.h"

#ifdef USE_WEBENGINE
#include <QWebEngineCookieStore>
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#define qrand() QRandomGenerator::global()->generate()
#endif

namespace Ms {
extern QString dataPath;

ApiInfo* ApiInfo::_instance = nullptr;
const QUrl ApiInfo::REGISTER_URL(ApiInfo::REGISTER_PAGE);
const QUrl ApiInfo::LOGIN_URL(ApiInfo::LOGIN_PAGE);
const QUrl ApiInfo::LOGIN_SUCCESS_URL(ApiInfo::LOGIN_SUCCESS_PAGE);

//---------------------------------------------------------
//   ApiInfo:apiInfoLocation
//---------------------------------------------------------

QString ApiInfo::apiInfoLocation()
{
    return mu::io::path(globalConfiguration()->dataPath() + "/api.dat").toQString();
}

//---------------------------------------------------------
//   ApiInfo:genClientId
//---------------------------------------------------------

QByteArray ApiInfo::genClientId()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    QByteArray qtGeneratedId(QSysInfo::machineUniqueId());
    if (!qtGeneratedId.isEmpty()) {
        return qtGeneratedId;
    }
#endif
    long long randId = qrand();
    constexpr size_t randBytes = sizeof(decltype(qrand()));
    qDebug() << "randBytes =" << randBytes << "sizeof(randId)" << sizeof(randId);
    for (size_t bytes = randBytes; bytes < sizeof(randId); bytes += randBytes) {
        randId <<= 8 * randBytes;
        randId += qrand();
    }
    qDebug() << randId << QString::number(randId, 2) << QString::number(randId, 16);

    return QString::number(randId, 16).toLatin1();
}

//---------------------------------------------------------
//   ApiInfo:createInstance
//---------------------------------------------------------

void ApiInfo::createInstance()
{
    if (_instance) {
        return;
    }

    QFile f(apiInfoLocation());
    QByteArray clientId;
    if (f.open(QIODevice::ReadOnly)) {
        const QByteArray saveData = f.readAll();
        // ToDo for Qt 5.15: QJsonDocument::fromBinaryData vs. CBOR format ??
        const QJsonDocument d(QJsonDocument::fromBinaryData(saveData));
        QJsonObject saveObject = d.object();
        clientId = saveObject["clientId"].toString().toLatin1();
        f.close();
    } else {
        clientId = genClientId();
        // Save the generated ID
        if (f.open(QIODevice::WriteOnly)) {
            QJsonObject saveObject;
            saveObject["clientId"] = QString(clientId);
            QJsonDocument saveDoc(saveObject);
            // ToDo for Qt 5.15: QJsonDocument::toBinaryData vs. CBOR format ??
            f.write(saveDoc.toBinaryData());
            f.close();
        }
    }

    QByteArray apiKey("0b19809bab331d70fb9983a0b9866290");
    _instance = new ApiInfo(clientId, apiKey);
}

//---------------------------------------------------------
//   ApiInfo::getOsInfo
//---------------------------------------------------------

QString ApiInfo::getOsInfo()
{
    QStringList info;
    info << QSysInfo::kernelType() << QSysInfo::kernelVersion()
         << QSysInfo::productType() << QSysInfo::productVersion()
         << QSysInfo::currentCpuArchitecture();
    return info.join(' ');
}

//---------------------------------------------------------
//   ApiInfo
//---------------------------------------------------------

ApiInfo::ApiInfo(const QByteArray _clientId, const QByteArray _apiKey)
    : CLIENT_ID(_clientId),
    API_KEY(_apiKey),
    USER_AGENT(QString(USER_AGENT_TEMPLATE).arg(VERSION).arg(BUILD_NUMBER).arg(getOsInfo()).toLatin1())
{
    LOGD() << "clientId: %s" << CLIENT_ID.constData();
    LOGD() << "apiKey: %s" << API_KEY.constData();
    LOGD() << "userAgent: %s" << USER_AGENT.constData();
}

//---------------------------------------------------------
//   ApiInfo::getUpdateScoreInfoUrl
//---------------------------------------------------------

QUrl ApiInfo::getUpdateScoreInfoUrl(const QString& scoreId, const QString& accessToken, bool newScore,
                                    const QString& customPath)
{
    QUrl url(MSCORE_HOST);
    url.setPath(customPath.isEmpty() ? DEFAULT_UPDATE_SCORE_INFO_PATH : customPath);

    QUrlQuery query;
    query.addQueryItem("id", scoreId);
    query.addQueryItem("newScore", QString::number(newScore));

#ifdef USE_WEBENGINE
    query.addQueryItem("_token", accessToken);
#else
    Q_UNUSED(accessToken);   // we'll be redirected to a browser, don't put access token there
#endif

    url.setQuery(query);

    return url;
}

//---------------------------------------------------------
//   LoginManager
//---------------------------------------------------------

CloudManager::CloudManager(QAction* uploadAudioMenuAction, QProgressDialog* progress, QObject* parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this)),
    m_uploadAudioMenuAction(uploadAudioMenuAction),
    m_progressDialog(progress)
{
    m_progressDialog->setWindowFlags(Qt::WindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowTitleHint));
    m_progressDialog->setWindowModality(Qt::NonModal);
    m_progressDialog->reset();   // required for Qt 5.5, see QTBUG-47042
}

CloudManager::CloudManager(QObject* parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this))
{
}

//---------------------------------------------------------
//   save
//---------------------------------------------------------

bool CloudManager::save()
{
    if (m_accessToken.isEmpty() && m_refreshToken.isEmpty()) {
        return true;
    }
    mu::io::path credPath = globalConfiguration()->dataPath() + "/cred.dat";
    QFile saveFile(credPath.toQString());
    if (!saveFile.open(QIODevice::WriteOnly)) {
        return false;
    }
    QJsonObject saveObject;
    saveObject["accessToken"] = m_accessToken;
    saveObject["refreshToken"] = m_refreshToken;
    QJsonDocument saveDoc(saveObject);
    // ToDo for Qt 5.15: QJsonDocument::toBinaryData vs. CBOR format ??
    saveFile.write(saveDoc.toBinaryData());
    saveFile.close();
    return true;
}

//---------------------------------------------------------
//   init
//---------------------------------------------------------

bool CloudManager::init()
{
    mu::io::path credPath = globalConfiguration()->dataPath() + "/cred.dat";
    QFile loadFile(credPath.toQString());
    if (!loadFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    QByteArray saveData = loadFile.readAll();
    // ToDo for Qt 5.15: QJsonDocument::fromBinaryData vs. CBOR format ??
    QJsonDocument loadDoc(QJsonDocument::fromBinaryData(saveData));
    QJsonObject saveObject = loadDoc.object();
    m_accessToken = saveObject["accessToken"].toString();
    m_refreshToken = saveObject["refreshToken"].toString();
    loadFile.close();
    return true;
}

void CloudManager::createAccount()
{
#ifdef USE_WEBENGINE
    showWebViewDialog(ApiInfo::REGISTER_URL);
#endif
}

//---------------------------------------------------------
//   onReplyFinished
//---------------------------------------------------------

void CloudManager::onReplyFinished(ApiRequest* request, RequestType requestType)
{
    if (!request) {
        return;
    }
    QNetworkReply* reply = request->reply();
    if (!reply) {
        return;
    }

    const int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (code == HTTP_UNAUTHORIZED && requestType != RequestType::LOGIN && requestType != RequestType::LOGIN_REFRESH) {
        if (request->retryCount() < MAX_REFRESH_LOGIN_RETRY_COUNT) {
            ApiRequest* refreshRequest = buildLoginRefreshRequest();
            refreshRequest->setParent(this);
            connect(refreshRequest, &ApiRequest::replyFinished, this,
                    [this, request, requestType](ApiRequest* refreshRequest) {
                    m_accessToken.clear();
                    onReplyFinished(refreshRequest, RequestType::LOGIN_REFRESH);
                    if (!m_accessToken.isEmpty()) {
                        // try to execute the request once more with the new token
                        request->setToken(m_accessToken);
                        request->executeRequest(m_networkManager);
                    } else {
                        handleReply(request->reply(), requestType);
                        request->deleteLater();
                    }
                });
            refreshRequest->executeRequest(m_networkManager);
            return;
        }
    }

    handleReply(reply, requestType);

    request->deleteLater();
}

//---------------------------------------------------------
//   handleReply
//---------------------------------------------------------

void CloudManager::handleReply(QNetworkReply* reply, RequestType requestType)
{
    if (!reply) {
        return;
    }

    const int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    QByteArray ba(reply->readAll());
    QJsonObject obj;
    if (!ba.isEmpty()) {
        QJsonDocument jsonResponse = QJsonDocument::fromJson(ba);
        if (jsonResponse.isObject()) {
            obj = jsonResponse.object();
        }
    }

    switch (requestType) {
    case RequestType::LOGIN:
        onLoginReply(reply, code, obj);
        break;
    case RequestType::LOGIN_REFRESH:
        onLoginRefreshReply(reply, code, obj);
        break;
    case RequestType::GET_USER_INFO:
        onGetUserReply(reply, code, obj);
        break;
    case RequestType::GET_SCORE_INFO:
        onGetScoreInfoReply(reply, code, obj);
        break;
    case RequestType::UPLOAD_SCORE:
        onUploadReply(reply, code, obj);
        break;
    case RequestType::GET_MEDIA_URL:
        onGetMediaUrlReply(reply, code, obj);
        break;
    }
}

//---------------------------------------------------------
//   getErrorString
//---------------------------------------------------------

QString CloudManager::getErrorString(QNetworkReply* reply, const QJsonObject& obj)
{
    const QString err = reply ? reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString() : tr("Error");
    const QString msg = obj["message"].toString();
    return QString("%1 (%2)").arg(err).arg(msg);
}

/*------- TRY LOGIN ROUTINES ----------------------------*/
/*  Try to get user information, if error,               */
/*  display login form until quit or successful login    */

//---------------------------------------------------------
//   tryLogin
//---------------------------------------------------------

void CloudManager::tryLogin()
{
    disconnect(this, SIGNAL(loginSuccess()), this, SLOT(tryLogin()));
    connect(this, SIGNAL(getUserSuccess()), this, SLOT(onTryLoginSuccess()));
    connect(this, SIGNAL(getUserError(QString)), this, SLOT(onTryLoginError(QString)));
    getUser();
}

//---------------------------------------------------------
//   onTryLoginSuccess
//---------------------------------------------------------

void CloudManager::onTryLoginSuccess()
{
    disconnect(this, SIGNAL(getUserSuccess()), this, SLOT(onTryLoginSuccess()));
    disconnect(this, SIGNAL(getUserError(QString)), this, SLOT(onTryLoginError(QString)));
    emit tryLoginSuccess();
}

//---------------------------------------------------------
//   onTryLoginError
//---------------------------------------------------------

void CloudManager::onTryLoginError(const QString& error)
{
    Q_UNUSED(error);
    disconnect(this, SIGNAL(getUserSuccess()), this, SLOT(onTryLoginSuccess()));
    disconnect(this, SIGNAL(getUserError(QString)), this, SLOT(onTryLoginError(QString)));
    connect(this, SIGNAL(loginSuccess()), this, SLOT(tryLogin()));
    logout();
#ifdef USE_WEBENGINE
    showWebViewDialog(ApiInfo::LOGIN_URL);
#else
    emit loginDialogRequested();
#endif
}

/*------- END - TRY LOGIN ROUTINES ----------------------------*/

//---------------------------------------------------------
//   clearHttpCacheOnRenderFinish
//---------------------------------------------------------

#ifdef USE_WEBENGINE
static void clearHttpCacheOnRenderFinish(QWebEngineView* webView)
{
    QWebEnginePage* page = webView->page();
    QWebEngineProfile* profile = page->profile();

    // workaround for the crashes sometimes happening in Chromium on macOS with Qt 5.12
    QObject::connect(webView, &QWebEngineView::renderProcessTerminated, webView,
                     [profile, webView](QWebEnginePage::RenderProcessTerminationStatus terminationStatus, int exitCode)
        {
            qDebug() << "Login page loading terminated" << terminationStatus << " " << exitCode;
            profile->clearHttpCache();
            webView->show();
        });
}

#endif
//---------------------------------------------------------
//   loginInteractive
//---------------------------------------------------------

#ifdef USE_WEBENGINE
void CloudManager::showWebViewDialog(const QUrl& url)
{
    QWebEngineView* webView = new QWebEngineView;
    webView->setWindowModality(Qt::ApplicationModal);
    webView->setAttribute(Qt::WA_DeleteOnClose);

    QWebEnginePage* page = webView->page();
    QWebEngineProfile* profile = page->profile();
    // TODO: logout in editor does not log out in web view
    profile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    profile->setRequestInterceptor(new ApiWebEngineRequestInterceptor(profile));

    clearHttpCacheOnRenderFinish(webView);

    connect(page, &QWebEnginePage::loadFinished, this, [this, page, webView](
                bool ok) {
            if (!ok) {
                return;
            }
            constexpr QUrl::FormattingOptions cmpOpt = QUrl::RemoveQuery | QUrl::RemoveFragment | QUrl::StripTrailingSlash;
            if (!page->url().matches(ApiInfo::LOGIN_SUCCESS_URL, cmpOpt)) {
                return;
            }

            page->runJavaScript("JSON.stringify(muGetAuthInfo())", [this, page, webView](const QVariant& v) {
                onLoginReply(nullptr, HTTP_OK, QJsonDocument::fromJson(v.toString().toUtf8()).object());
                // We have retrieved an access token, do not remain logged
                // in with web view profile.
                page->profile()->cookieStore()->deleteAllCookies();
                webView->close();
            });
        });

    webView->load(url);
    webView->show();
}

#endif

//---------------------------------------------------------
//   login
//---------------------------------------------------------

void CloudManager::login(QString login, QString password)
{
    if (login.isEmpty() || password.isEmpty()) {
        return;
    }

    ApiRequest* r = new ApiRequest(this);
    r->setPath("/auth/login")
    .setMethod(ApiRequest::HTTP_PUT)
    .addPostParameter("field", login)
    .addPostParameter("password", password);

    connect(r, &ApiRequest::replyFinished, this, [this](ApiRequest* r) {
            onReplyFinished(r, RequestType::LOGIN);
        });

    r->executeRequest(m_networkManager);
}

//---------------------------------------------------------
//   buildLoginRefreshRequest
//---------------------------------------------------------

ApiRequest* CloudManager::buildLoginRefreshRequest() const
{
    ApiRequest* r = new ApiRequest();
    r->setPath("/auth/refresh")
    .setMethod(ApiRequest::HTTP_POST)
    .addGetParameter("device_id", ApiInfo::instance().CLIENT_ID)
    .addPostParameter("refresh_token", m_refreshToken);

    return r;
}

//---------------------------------------------------------
//   onLoginReply
//---------------------------------------------------------

void CloudManager::onLoginReply(QNetworkReply* reply, int code, const QJsonObject& obj)
{
    if (code == HTTP_OK) {
        m_accessToken = obj["token"].toString();
        m_refreshToken = obj["refresh_token"].toString();
        if (!m_accessToken.isEmpty()) {
            emit loginSuccess();
        } else {
            emit loginError(tr("Wrong response from the server"));
        }
    } else {
        emit loginError(getErrorString(reply, obj));
    }

    save();
}

//---------------------------------------------------------
//   onLoginRefreshReply
//---------------------------------------------------------

void CloudManager::onLoginRefreshReply(QNetworkReply* reply, int code, const QJsonObject& obj)
{
    Q_UNUSED(reply);
    if (code == HTTP_OK) {
        m_accessToken = obj["token"].toString();
        m_refreshToken = obj["refresh_token"].toString();
    } else {
        m_accessToken.clear();
        m_refreshToken.clear();
    }

    save();
}

//---------------------------------------------------------
//   getUser
//---------------------------------------------------------

void CloudManager::getUser()
{
    if (m_accessToken.isEmpty() && m_refreshToken.isEmpty()) {
        emit getUserError("getUser - No token");
        return;
    }

    ApiRequest* r = new ApiRequest(this);
    r->setPath("/user/me")
    .setMethod(ApiRequest::HTTP_GET)
    .setToken(m_accessToken);

    connect(r, &ApiRequest::replyFinished, this, [this](ApiRequest* r) {
            onReplyFinished(r, RequestType::GET_USER_INFO);
        });

    r->executeRequest(m_networkManager);
}

//---------------------------------------------------------
//   onGetUserReply
//---------------------------------------------------------

void CloudManager::onGetUserReply(QNetworkReply* reply, int code, const QJsonObject& user)
{
//       qDebug() << "onGetUserReply" << code << reply->errorString();
    if (code == HTTP_OK) {
        if (user.value("name") != QJsonValue::Undefined) {
            m_accountInfo.id = user.value("id").toString().toInt();
            m_accountInfo.userName = user.value("name").toString();
            QString profileUrl = user.value("permalink").toString();
            m_accountInfo.profileUrl = QUrl(profileUrl);
            m_accountInfo.avatarUrl = QUrl(user.value("avatar_url").toString());
            m_accountInfo.sheetmusicUrl = QUrl(profileUrl + "/sheetmusic");

            emit getUserSuccess();
        } else {
            emit getUserError(tr("Wrong response from the server"));
        }
    } else {
        emit getUserError(tr("Error while getting user info: %1").arg(getErrorString(reply, user)));
    }
}

//---------------------------------------------------------
//   getScore
//---------------------------------------------------------

void CloudManager::getScoreInfo(int nid)
{
    if (m_accessToken.isEmpty() && m_refreshToken.isEmpty()) {
        emit getScoreError("getScore - No token");
        return;
    }

    ApiRequest* r = new ApiRequest(this);
    r->setPath("/score/full-info")
    .setMethod(ApiRequest::HTTP_GET)
    .setToken(m_accessToken)
    .addGetParameter("score_id", QString::number(nid));

    connect(r, &ApiRequest::replyFinished, this, [this](ApiRequest* r) {
            onReplyFinished(r, RequestType::GET_SCORE_INFO);
        });

    r->executeRequest(m_networkManager);
}

//---------------------------------------------------------
//   onGetScoreInfoReply
//---------------------------------------------------------

void CloudManager::onGetScoreInfoReply(QNetworkReply* reply, int code, const QJsonObject& score)
{
    if (code == HTTP_OK) {
        if (score.value("user") != QJsonValue::Undefined) {
            QJsonObject user = score.value("user").toObject();
            QString title = score.value("title").toString();
            QString description = score.value("description").toString();
            QString sharing = score.value("sharing").toString();
            QString license = score.value("license").toString();
            QString tags = score.value("tags").toString();
            QString url = score.value("custom_url").toString();
            if (user.value("uid") != QJsonValue::Undefined) {
                int uid = user.value("uid").toString().toInt();
                if (uid == m_accountInfo.id) {
                    emit getScoreSuccess(title, description, (sharing == "private"), license, tags, url);
                } else {
                    emit getScoreError("");
                }
            } else {
                emit getScoreError("");
            }
        } else {
            emit getScoreError("");
        }
    } else {
        emit getScoreError(getErrorString(reply, score));
    }
}

//---------------------------------------------------------
//   getMediaUrl
//---------------------------------------------------------

void CloudManager::getMediaUrl(const QString& nid, const QString& vid, const QString& encoding)
{
    Q_UNUSED(encoding);
    ApiRequest* r = new ApiRequest(this);
    r->setPath("/score/audio")
    .setMethod(ApiRequest::HTTP_GET)
    .setToken(m_accessToken)
    .addGetParameter("score_id", nid)
    .addGetParameter("revision_id", vid);

    connect(r, &ApiRequest::replyFinished, this, [this](ApiRequest* r) {
            onReplyFinished(r, RequestType::GET_MEDIA_URL);
        });

    r->executeRequest(m_networkManager);
}

//---------------------------------------------------------
//   onGetMediaUrlReply
//---------------------------------------------------------
void CloudManager::onGetMediaUrlReply(QNetworkReply* reply, int code, const QJsonObject& response)
{
    if (code == HTTP_OK) {
        QJsonValue urlValue = response.value("url");
        if (urlValue.isString()) {
            m_mediaUrl = urlValue.toString();
            QString mp3Path = QDir::tempPath() + QString("/temp_%1.mp3").arg(qrand() % 100000);
            m_mp3File = new QFile(mp3Path);

            constexpr int mp3Bitrate = 128;

            if (mp3Exporter()->saveCurrentScoreMp3(mp3Path, mp3Bitrate)) {
                m_uploadTryCount = 0;
                uploadMedia();
            }
        }
    } else { // TODO: handle request error properly
        qWarning("%s", getErrorString(reply, response).toUtf8().constData());
    }
}

//---------------------------------------------------------
//   uploadMedia
//---------------------------------------------------------

void CloudManager::uploadMedia()
{
    if (m_mediaUrl.isEmpty()) {
        m_progressDialog->hide();
        m_uploadAudioMenuAction->setEnabled(true);
        return;
    }
    if (!m_mp3File->exists()) {
        emit mediaUploadSuccess();
        return;
    }
    if (m_mp3File->open(QIODevice::ReadOnly)) {   // probably cancelled, no error handling
        QNetworkRequest request;
        request.setUrl(QUrl(m_mediaUrl));
        m_progressDialog->reset();
        m_progressDialog->setLabelText(tr("Uploading…"));
        m_progressDialog->setCancelButtonText(tr("Cancel"));
        m_progressDialog->show();
        m_uploadTryCount++;
        m_uploadAudioMenuAction->setEnabled(false);
        QNetworkReply* reply = m_networkManager->put(request, m_mp3File);
        connect(m_progressDialog, SIGNAL(canceled()), reply, SLOT(abort()));
        connect(reply, SIGNAL(finished()), this, SLOT(mediaUploadFinished()));
        connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(mediaUploadProgress(qint64,qint64)));
    }
}

//---------------------------------------------------------
//   mediaUploadFinished
//---------------------------------------------------------

void CloudManager::mediaUploadFinished()
{
    m_uploadAudioMenuAction->setEnabled(true);
    QNetworkReply* reply = static_cast<QNetworkReply*>(QObject::sender());
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QNetworkReply::NetworkError e = reply->error();
    reply->deleteLater();
    m_progressDialog->hide();
    m_progressDialog->reset();
    if ((statusCode == 200 && reply->error() == QNetworkReply::NoError) || m_progressDialog->wasCanceled()) {
        m_mp3File->remove();
        delete m_mp3File;
        m_mediaUrl = "";
        emit mediaUploadSuccess();
    } else if (e == QNetworkReply::RemoteHostClosedError && m_uploadTryCount < MAX_UPLOAD_TRY_COUNT) {
        uploadMedia();
    } else {
        qDebug() << "error uploading media" << e;
        QMessageBox::warning(0,
                             tr("Upload Error"),
                             tr("Sorry, MuseScore couldn't upload the audio file. Error %1").arg(e),
                             QString(), QString());
    }
}

//---------------------------------------------------------
//   mediaUploadProgress
//---------------------------------------------------------

void CloudManager::mediaUploadProgress(qint64 progress, qint64 total)
{
    if (!m_progressDialog->wasCanceled()) {
        m_progressDialog->setMinimum(0);
        m_progressDialog->setMaximum(total);
        m_progressDialog->setValue(progress);
    }
}

//---------------------------------------------------------
//   upload
//---------------------------------------------------------

void CloudManager::upload(const QString& path, int nid, const QString& title)
{
    qDebug() << "file upload" << nid;

    ApiRequest* r = new ApiRequest(this);
    r->setPath("/score/upload-light")
    .setToken(m_accessToken);

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType, /* parent */ r);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    QString contentDisposition = QString("form-data; name=\"score_data\"; filename=\"temp_%1.mscz\"").arg(
        qrand() % 100000);
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(contentDisposition));
    QFile* file = new QFile(path);
    file->open(QIODevice::ReadOnly);
    filePart.setBodyDevice(file);
    file->setParent(multiPart);   // we cannot delete the file now, so delete it with the multiPart
    multiPart->append(filePart);

    if (nid > 0) {
        QHttpPart idPart;
        idPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"score_id\""));
        idPart.setBody(QString::number(nid).toLatin1());
        multiPart->append(idPart);
    }

    QHttpPart titlePart;
    titlePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"title\""));
    titlePart.setBody(title.toUtf8());
    multiPart->append(titlePart);

    r->setMultiPartData(multiPart);

    if (nid > 0) { // score exists, update
        r->setMethod(ApiRequest::HTTP_PUT);
    } else { // score doesn't exist, post a new score
        r->setMethod(ApiRequest::HTTP_POST);
    }

    connect(r, &ApiRequest::replyFinished, this, [this](ApiRequest* r) {
            onReplyFinished(r, RequestType::UPLOAD_SCORE);
        });

    r->executeRequest(m_networkManager);
}

//---------------------------------------------------------
//   onUploadReply
//---------------------------------------------------------

void CloudManager::onUploadReply(QNetworkReply* reply, int code, const QJsonObject& obj)
{
    qDebug() << "onUploadReply" << code << reply->errorString();
    if (code == HTTP_OK) {
        constexpr const char* pathKey = "_webview_url";
        m_updateScoreDataPath = obj.contains(pathKey) ? obj.value(pathKey).toString() : QString();

        if (obj.value("permalink") != QJsonValue::Undefined) {
            emit uploadSuccess(obj.value("permalink").toString(), obj.value("id").toString(), obj.value(
                                   "vid").toString());
        } else {
            emit uploadError(tr("An error occurred during the file transfer. Please try again"));
        }
    } else {
        emit uploadError(tr("Cannot upload: %1").arg(getErrorString(reply, obj)));
    }
}

//---------------------------------------------------------
//   updateScoreData
//---------------------------------------------------------

void CloudManager::updateScoreData(const QString& nid, bool newScore)
{
    const QUrl url(ApiInfo::getUpdateScoreInfoUrl(nid, m_accessToken, newScore, m_updateScoreDataPath));
#ifdef USE_WEBENGINE
    QWebEngineView* webView = new QWebEngineView;
    webView->setWindowModality(Qt::ApplicationModal);
    webView->setAttribute(Qt::WA_DeleteOnClose);

    QWebEnginePage* page = webView->page();
    QWebEngineProfile* profile = page->profile();

    profile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    profile->setRequestInterceptor(new ApiWebEngineRequestInterceptor(profile));

    connect(page, &QWebEnginePage::windowCloseRequested, webView, &QWebEngineView::close);

    clearHttpCacheOnRenderFinish(webView);

    webView->load(url);
    webView->show();
#else
    QDesktopServices::openUrl(url);
#endif
}

//---------------------------------------------------------
//   logout
//---------------------------------------------------------

bool CloudManager::logout()
{
    if (!m_accessToken.isEmpty()) {
        ApiRequest* r = new ApiRequest(this);
        r->setPath("/auth/login")
        .setMethod(ApiRequest::HTTP_DELETE)
        .setToken(m_accessToken);

        connect(r, &ApiRequest::replyFinished, r, &ApiRequest::deleteLater);     // we don't need the reply info here
        r->executeRequest(m_networkManager);
    }

    m_accessToken.clear();
    m_refreshToken.clear();
    mu::io::path credPath = globalConfiguration()->dataPath() + "/cred.dat";
    QFile loadFile(credPath.toQString());
    if (!loadFile.exists()) {
        return true;
    }
    return loadFile.remove();
}

//---------------------------------------------------------
//   ApiRequest::setToken
//---------------------------------------------------------

ApiRequest& ApiRequest::setToken(const QString& token)
{
    const QString tokenKey("token");
    _urlQuery.removeQueryItem(tokenKey);
    if (!token.isEmpty()) {
        _urlQuery.addQueryItem(tokenKey, token);
    }
    return *this;
}

//---------------------------------------------------------
//   ApiRequest::buildRequest
//---------------------------------------------------------

QNetworkRequest ApiRequest::buildRequest() const
{
    QNetworkRequest r;

    QUrl url(_url);
    url.setQuery(_urlQuery);
    r.setUrl(url);
    r.setRawHeader("Accept", "application/json");
    const ApiInfo& apiInfo = ApiInfo::instance();
    r.setHeader(QNetworkRequest::UserAgentHeader, apiInfo.USER_AGENT);
    r.setRawHeader(apiInfo.CLIENT_ID_HEADER, apiInfo.CLIENT_ID);
    r.setRawHeader(apiInfo.API_KEY_HEADER, apiInfo.API_KEY);

    return r;
}

//---------------------------------------------------------
//   ApiRequest::executeRequest
//---------------------------------------------------------

void ApiRequest::executeRequest(QNetworkAccessManager* networkManager)
{
    ++_retryCount;
    QNetworkRequest request(buildRequest());
    const QByteArray data(_bodyQuery.toString().toLatin1());

    switch (_method) {
    case HTTP_GET:
        _reply = networkManager->get(request);
        break;
    case HTTP_POST:
        if (_multipart) {
            _reply = networkManager->post(request, _multipart);
        } else {
            _reply = networkManager->post(request, data);
        }
        break;
    case HTTP_PUT:
        if (_multipart) {
            _reply = networkManager->put(request, _multipart);
        } else {
            _reply = networkManager->put(request, data);
        }
        break;
    case HTTP_DELETE:
        _reply = networkManager->deleteResource(request);
        break;
    }

    _reply->setParent(this);
    connect(_reply, &QNetworkReply::finished, this, [this]() { emit replyFinished(this); });
}

//---------------------------------------------------------
//   ApiWebEngineRequestInterceptor::interceptRequest
//    Sets the appropriate API headers for requests to
//    musescore.com
//---------------------------------------------------------

#ifdef USE_WEBENGINE
void ApiWebEngineRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo& request)
{
    const ApiInfo& apiInfo = ApiInfo::instance();
    request.setHttpHeader("User-Agent", apiInfo.USER_AGENT);
    request.setHttpHeader(apiInfo.CLIENT_ID_HEADER, apiInfo.CLIENT_ID);
    request.setHttpHeader(apiInfo.API_KEY_HEADER, apiInfo.API_KEY);
}

#endif
}
