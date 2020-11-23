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

#include "loginmanager.h"
#include "loginmanager_p.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "preferences.h"

#ifdef USE_WEBENGINE
#include <QWebEngineCookieStore>
#endif

namespace Ms {

extern QString dataPath;

ApiInfo* ApiInfo::_instance = nullptr;
const QUrl ApiInfo::loginUrl(ApiInfo::loginPage);
const QUrl ApiInfo::loginSuccessUrl(ApiInfo::loginSuccessPage);

//---------------------------------------------------------
//   ApiInfo:apiInfoLocation
//---------------------------------------------------------

QString ApiInfo::apiInfoLocation()
      {
      return dataPath + "/api.dat";
      }

//---------------------------------------------------------
//   ApiInfo:genClientId
//---------------------------------------------------------

QByteArray ApiInfo::genClientId()
      {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
      QByteArray qtGeneratedId(QSysInfo::machineUniqueId());
      if (!qtGeneratedId.isEmpty())
            return qtGeneratedId;
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
      if (_instance)
            return;

      QFile f(apiInfoLocation());
      QByteArray clientId;
      if (f.open(QIODevice::ReadOnly)) {
            const QByteArray saveData = f.readAll();
            const QJsonDocument d(QJsonDocument::fromBinaryData(saveData));
            QJsonObject saveObject = d.object();
            clientId = saveObject["clientId"].toString().toLatin1();
            f.close();
            }
      else {
            clientId = genClientId();
            // Save the generated ID
            if (f.open(QIODevice::WriteOnly)) {
                  QJsonObject saveObject;
                  saveObject["clientId"] = QString(clientId);
                  QJsonDocument saveDoc(saveObject);
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
      info  << QSysInfo::kernelType() << QSysInfo::kernelVersion()
            << QSysInfo::productType() << QSysInfo::productVersion()
            << QSysInfo::currentCpuArchitecture();
      return info.join(' ');
      }

//---------------------------------------------------------
//   ApiInfo
//---------------------------------------------------------

ApiInfo::ApiInfo(const QByteArray _clientId, const QByteArray _apiKey)
   : clientId(_clientId),
   apiKey(_apiKey),
   userAgent(QString(userAgentTemplate).arg(VERSION, BUILD_NUMBER, getOsInfo()).toLatin1())
      {
      if (MScore::debugMode) {
            qWarning("clientId: %s", clientId.constData());
            qWarning("apiKey: %s", apiKey.constData());
            qWarning("userAgent: %s", userAgent.constData());
            }
      }

//---------------------------------------------------------
//   ApiInfo::getUpdateScoreInfoUrl
//---------------------------------------------------------

QUrl ApiInfo::getUpdateScoreInfoUrl(const QString& scoreId, const QString& accessToken, bool newScore, const QString& customPath)
      {
      QUrl url(mscoreHost);
      url.setPath(customPath.isEmpty() ? defaultUpdateScoreInfoPath : customPath);

      QUrlQuery query;
      query.addQueryItem("id", scoreId);
      query.addQueryItem("newScore", QString::number(newScore));

#ifdef USE_WEBENGINE
      query.addQueryItem("_token", accessToken);
#else
      Q_UNUSED(accessToken); // we'll be redirected to a browser, don't put access token there
#endif

      url.setQuery(query);

      return url;
      }

//---------------------------------------------------------
//   LoginManager
//---------------------------------------------------------

LoginManager::LoginManager(QAction* uploadAudioMenuAction, QObject* parent)
 : QObject(parent), _networkManager(new QNetworkAccessManager(this)), m_asyncWait(new AsyncWait(this)),
   _uploadAudioMenuAction(uploadAudioMenuAction)
      {
      _progressDialog = new QProgressDialog(mscore);
      _progressDialog->setWindowFlags(Qt::WindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowTitleHint));
      _progressDialog->setWindowModality(Qt::NonModal);
      _progressDialog->reset(); // required for Qt 5.5, see QTBUG-47042
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

bool LoginManager::save()
      {
      if (_accessToken.isEmpty() && _refreshToken.isEmpty())
            return true;
      QFile saveFile(dataPath + "/cred.dat");
      if (!saveFile.open(QIODevice::WriteOnly))
            return false;
      QJsonObject saveObject;
      saveObject["accessToken"] = _accessToken;
      saveObject["refreshToken"] = _refreshToken;
      QJsonDocument saveDoc(saveObject);
      saveFile.write(saveDoc.toBinaryData());
      saveFile.close();
      return true;
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

bool LoginManager::load()
      {
      QFile loadFile(dataPath + "/cred.dat");
      if (!loadFile.open(QIODevice::ReadOnly))
            return false;
      QByteArray saveData = loadFile.readAll();
      QJsonDocument loadDoc(QJsonDocument::fromBinaryData(saveData));
      QJsonObject saveObject = loadDoc.object();
      _accessToken = saveObject["accessToken"].toString();
      _refreshToken = saveObject["refreshToken"].toString();
      loadFile.close();
      return true;
      }

//---------------------------------------------------------
//   onReplyFinished
//---------------------------------------------------------

void LoginManager::onReplyFinished(ApiRequest* request, RequestType requestType)
      {
      if (!request)
            return;
      QNetworkReply* reply = request->reply();
      if (!reply)
            return;

      const int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

      if (code == HTTP_UNAUTHORIZED && requestType != RequestType::LOGIN && requestType != RequestType::LOGIN_REFRESH) {
            if (request->retryCount() < MAX_REFRESH_LOGIN_RETRY_COUNT) {
                  ApiRequest* refreshRequest = buildLoginRefreshRequest();
                  refreshRequest->setParent(this);
                  connect(refreshRequest, &ApiRequest::replyFinished, this, [this, request, requestType](ApiRequest* refreshRequest) {
                        _accessToken.clear();
                        onReplyFinished(refreshRequest, RequestType::LOGIN_REFRESH);
                        if (!_accessToken.isEmpty()) {
                              // try to execute the request once more with the new token
                              request->setToken(_accessToken);
                              request->executeRequest(_networkManager);
                              }
                        else {
                              handleReply(request->reply(), requestType);
                              request->deleteLater();
                              }
                        });
                  refreshRequest->executeRequest(_networkManager);
                  return;
                  }
            }

      handleReply(reply, requestType);

      request->deleteLater();
      }

//---------------------------------------------------------
//   handleReply
//---------------------------------------------------------

void LoginManager::handleReply(QNetworkReply* reply, RequestType requestType)
      {
      if (!reply)
            return;

      const int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

      QByteArray ba(reply->readAll());
      QJsonObject obj;
      if (!ba.isEmpty()) {
            QJsonDocument jsonResponse = QJsonDocument::fromJson(ba);
            if (jsonResponse.isObject())
                  obj = jsonResponse.object();
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

QString LoginManager::getErrorString(QNetworkReply* reply, const QJsonObject& obj)
      {
      const QString err = reply ? reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString() : tr("Error");
      const QString msg = obj["message"].toString();
      return QString("%1 (%2)").arg(err, msg);
      }

/*------- TRY LOGIN ROUTINES ----------------------------*/
/*  Try to get user information, if error,               */
/*  display login form until quit or successful login    */

//---------------------------------------------------------
//   tryLogin
//---------------------------------------------------------

void LoginManager::tryLogin()
      {
      disconnect(this, SIGNAL(loginSuccess()), this, SLOT(tryLogin()));
      connect(this, SIGNAL(getUserSuccess()), this, SLOT(onTryLoginSuccess()));
      connect(this, SIGNAL(getUserError(QString)), this, SLOT(onTryLoginError(QString)));
      getUser();
      }

//---------------------------------------------------------
//   onTryLoginSuccess
//---------------------------------------------------------

void LoginManager::onTryLoginSuccess()
      {
      disconnect(this, SIGNAL(getUserSuccess()), this, SLOT(onTryLoginSuccess()));
      disconnect(this, SIGNAL(getUserError(QString)), this, SLOT(onTryLoginError(QString)));
      emit tryLoginSuccess();
      }

//---------------------------------------------------------
//   onTryLoginError
//---------------------------------------------------------

void LoginManager::onTryLoginError(const QString& error)
      {
      Q_UNUSED(error);
      disconnect(this, SIGNAL(getUserSuccess()), this, SLOT(onTryLoginSuccess()));
      disconnect(this, SIGNAL(getUserError(QString)), this, SLOT(onTryLoginError(QString)));
      connect(this, SIGNAL(loginSuccess()), this, SLOT(tryLogin()));
      logout();
#ifdef USE_WEBENGINE
      loginInteractive();
#else
      mscore->showLoginDialog();
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
      QObject::connect(webView, &QWebEngineView::renderProcessTerminated, webView, [profile, webView](QWebEnginePage::RenderProcessTerminationStatus terminationStatus, int exitCode)
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
void LoginManager::loginInteractive()
      {
#if defined(WIN_PORTABLE)
      QWebEngineProfile* defaultProfile = QWebEngineProfile::defaultProfile();
      defaultProfile->setCachePath(QDir::cleanPath(QString("%1/../../../Data/settings/QWebEngine").arg(QCoreApplication::applicationDirPath())));
      defaultProfile->setPersistentStoragePath(QDir::cleanPath(QString("%1/../../../Data/settings/QWebEngine").arg(QCoreApplication::applicationDirPath())));
#endif
      QWebEngineView* webView = new QWebEngineView;
      webView->setWindowModality(Qt::ApplicationModal);
      webView->setAttribute(Qt::WA_DeleteOnClose);

      QWebEnginePage* page = webView->page();
      QWebEngineProfile* profile = page->profile();
      // TODO: logout in editor does not log out in web view
      profile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
#if defined(WIN_PORTABLE)
      profile->setCachePath(QDir::cleanPath(QString("%1/../../../Data/settings/QWebEngine").arg(QCoreApplication::applicationDirPath())));
      profile->setPersistentStoragePath(QDir::cleanPath(QString("%1/../../../Data/settings/QWebEngine").arg(QCoreApplication::applicationDirPath())));
#endif
      profile->setRequestInterceptor(new ApiWebEngineRequestInterceptor(profile));

      clearHttpCacheOnRenderFinish(webView);

      connect(page, &QWebEnginePage::loadFinished, this, [this, page, webView](bool ok) {
            if (!ok)
                  return;
            constexpr QUrl::FormattingOptions cmpOpt = QUrl::RemoveQuery | QUrl::RemoveFragment | QUrl::StripTrailingSlash;
            if (!page->url().matches(ApiInfo::loginSuccessUrl, cmpOpt))
                  return;

            page->runJavaScript("JSON.stringify(muGetAuthInfo())", [this, page, webView](const QVariant& v) {
                  onLoginReply(nullptr, HTTP_OK, QJsonDocument::fromJson(v.toString().toUtf8()).object());
                  // We have retrieved an access token, do not remain logged
                  // in with web view profile.
                  page->profile()->cookieStore()->deleteAllCookies();
                  webView->close();
                  });
            });

      webView->load(ApiInfo::loginUrl);
      webView->show();
      }
#endif

//---------------------------------------------------------
//   login
//---------------------------------------------------------

void LoginManager::login(QString login, QString password)
      {
      if(login.isEmpty() || password.isEmpty())
           return;

      ApiRequest* r = new ApiRequest(this);
      r->setPath("/auth/login")
         .setMethod(ApiRequest::HTTP_PUT)
         .addPostParameter("field", login)
         .addPostParameter("password", password);

      connect(r, &ApiRequest::replyFinished, this, [this](ApiRequest* r) {
            onReplyFinished(r, RequestType::LOGIN);
            });

      r->executeRequest(_networkManager);
      }

//---------------------------------------------------------
//   buildLoginRefreshRequest
//---------------------------------------------------------

ApiRequest* LoginManager::buildLoginRefreshRequest() const
      {
      ApiRequest* r = new ApiRequest();
      r->setPath("/auth/refresh")
         .setMethod(ApiRequest::HTTP_POST)
         .addGetParameter("device_id", ApiInfo::instance().clientId)
         .addPostParameter("refresh_token", _refreshToken);

      return r;
      }

//---------------------------------------------------------
//   onLoginReply
//---------------------------------------------------------

void LoginManager::onLoginReply(QNetworkReply* reply, int code, const QJsonObject& obj)
      {
      if (code == HTTP_OK) {
            _accessToken = obj["token"].toString();
            _refreshToken = obj["refresh_token"].toString();
            if (!_accessToken.isEmpty())
                  emit loginSuccess();
            else
                  emit loginError(tr("Wrong response from the server"));
            }
      else
            emit loginError(getErrorString(reply, obj));

      save();
      }

//---------------------------------------------------------
//   onLoginRefreshReply
//---------------------------------------------------------

void LoginManager::onLoginRefreshReply(QNetworkReply* reply, int code, const QJsonObject& obj)
      {
      Q_UNUSED(reply);
      if (code == HTTP_OK) {
            _accessToken = obj["token"].toString();
            _refreshToken = obj["refresh_token"].toString();
            }
      else {
            _accessToken.clear();
            _refreshToken.clear();
            }

      save();
      }

//---------------------------------------------------------
//   getUser
//---------------------------------------------------------

void LoginManager::getUser()
      {
      if (_accessToken.isEmpty() && _refreshToken.isEmpty()) {
            emit getUserError("getUser - No token");
            return;
            }

      ApiRequest* r = new ApiRequest(this);
      r->setPath("/user/me")
         .setMethod(ApiRequest::HTTP_GET)
         .setToken(_accessToken);

      connect(r, &ApiRequest::replyFinished, this, [this](ApiRequest* r) {
            onReplyFinished(r, RequestType::GET_USER_INFO);
            });

      r->executeRequest(_networkManager);
      }

//---------------------------------------------------------
//   onGetUserReply
//---------------------------------------------------------

void LoginManager::onGetUserReply(QNetworkReply* reply, int code, const QJsonObject& user)
      {
//       qDebug() << "onGetUserReply" << code << reply->errorString();
      if (code == HTTP_OK) {
            if (user.value("name") != QJsonValue::Undefined) {
                  _userName = user.value("name").toString();
                  _uid = user.value("id").toString().toInt();
                  _avatar = QUrl(user.value("avatar_url").toString());
                  qInfo() << "Logged in as" << _userName;
                  emit getUserSuccess();
                  }
            else
                  emit getUserError(tr("Wrong response from the server"));
            }
      else
            emit getUserError(tr("Error while getting user info: %1").arg(getErrorString(reply, user)));
      }

//---------------------------------------------------------
//   syncGetUser
//---------------------------------------------------------

bool LoginManager::syncGetUser()
      {
      connect(this, &LoginManager::getUserSuccess, m_asyncWait, &AsyncWait::success);
      connect(this, &LoginManager::getUserError, m_asyncWait, &AsyncWait::failure);
      getUser();
      bool success = (m_asyncWait->exec() == 0);
      disconnect(this, &LoginManager::getUserSuccess, m_asyncWait, &AsyncWait::success);
      disconnect(this, &LoginManager::getUserError, m_asyncWait, &AsyncWait::failure);
      if (!success) {
            qWarning() << m_asyncWait->errorMsg();
            }
      return success;
      }

//---------------------------------------------------------
//   getScore
//---------------------------------------------------------

void LoginManager::getScoreInfo(int nid)
      {
      if (_accessToken.isEmpty() && _refreshToken.isEmpty()) {
            emit getScoreError("getScore - No token");
            return;
            }

      ApiRequest* r = new ApiRequest(this);
      r->setPath("/score/full-info")
         .setMethod(ApiRequest::HTTP_GET)
         .setToken(_accessToken)
         .addGetParameter("score_id", QString::number(nid));

      connect(r, &ApiRequest::replyFinished, this, [this](ApiRequest* r) {
            onReplyFinished(r, RequestType::GET_SCORE_INFO);
            });

      r->executeRequest(_networkManager);
      }

//---------------------------------------------------------
//   onGetScoreInfoReply
//---------------------------------------------------------

void LoginManager::onGetScoreInfoReply(QNetworkReply* reply, int code, const QJsonObject& score)
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
                        if (uid == _uid) {
                              _scoreTitle = title;
                              _nid = score.value("id").toString().toInt();
                              emit getScoreSuccess(title, description, (sharing == "private"), license, tags, url);
                              }
                        else {
                              emit getScoreError("");
                              }
                        }
                  else {
                       emit getScoreError("");
                       }
                  }
            else {
                  emit getScoreError("");
                  }
            }
      else
            emit getScoreError(getErrorString(reply, score));
      }

//---------------------------------------------------------
//   syncGetScoreInfo
//---------------------------------------------------------

bool LoginManager::syncGetScoreInfo(int nid)
{
      connect(this, &LoginManager::getScoreSuccess, m_asyncWait, &AsyncWait::success);
      connect(this, &LoginManager::getScoreError, m_asyncWait, &AsyncWait::failure);
      getScoreInfo(nid);
      bool success = (m_asyncWait->exec() == 0);
      disconnect(this, &LoginManager::getScoreSuccess, m_asyncWait, &AsyncWait::success);
      disconnect(this, &LoginManager::getScoreError, m_asyncWait, &AsyncWait::failure);
      if (!success) {
            qWarning() << m_asyncWait->errorMsg();
            }
      return success;
}

//---------------------------------------------------------
//   getMediaUrl
//---------------------------------------------------------

void LoginManager::getMediaUrl(const QString& nid, const QString& vid, const QString& encoding)
      {
      Q_UNUSED(encoding);
      ApiRequest* r = new ApiRequest(this);
      r->setPath("/score/audio")
         .setMethod(ApiRequest::HTTP_GET)
         .setToken(_accessToken)
         .addGetParameter("score_id", nid)
         .addGetParameter("revision_id", vid);

      connect(r, &ApiRequest::replyFinished, this, [this](ApiRequest* r) {
            onReplyFinished(r, RequestType::GET_MEDIA_URL);
            });

      r->executeRequest(_networkManager);
      }

//---------------------------------------------------------
//   onGetMediaUrlReply
//---------------------------------------------------------
void LoginManager::onGetMediaUrlReply(QNetworkReply* reply, int code, const QJsonObject& response)
      {
      if (code == HTTP_OK) {
            QJsonValue urlValue = response.value("url");
            if (urlValue.isString()) {
                  _mediaUrl = urlValue.toString();
                  QString mp3Path = QDir::tempPath() + QString("/temp_%1.mp3").arg(qrand() % 100000);
                  _mp3File = new QFile(mp3Path);
                  Score* score = mscore->currentScore()->masterScore();
                  int br = preferences.getInt(PREF_EXPORT_MP3_BITRATE);
                  preferences.setPreference(PREF_EXPORT_MP3_BITRATE, 128);
                  if (mscore->saveMp3(score, mp3Path)) { // no else, error handling is done in saveMp3
                        _uploadTryCount = 0;
                        uploadMedia();
                        }
                  preferences.setPreference(PREF_EXPORT_MP3_BITRATE, br);
                  }
            }
      else // TODO: handle request error properly
            qWarning("%s", getErrorString(reply, response).toUtf8().constData());
      }

//---------------------------------------------------------
//   uploadMedia
//---------------------------------------------------------

void LoginManager::uploadMedia()
      {
      if (_mediaUrl.isEmpty()) {
            _progressDialog->hide();
            _uploadAudioMenuAction->setEnabled(true);
            return;
            }
      if (!_mp3File->exists()) {
            emit mediaUploadSuccess();
            return;
            }
      if (_mp3File->open(QIODevice::ReadOnly)) { // probably cancelled, no error handling
            QNetworkRequest request;
            request.setUrl(QUrl(_mediaUrl));
            _progressDialog->reset();
            _progressDialog->setLabelText(tr("Uploading…"));
            _progressDialog->setCancelButtonText(tr("Cancel"));
            _progressDialog->show();
            _uploadTryCount++;
            _uploadAudioMenuAction->setEnabled(false);
            QNetworkReply *reply = mscore->networkManager()->put(request, _mp3File);
            connect(_progressDialog, SIGNAL(canceled()), reply, SLOT(abort()));
            connect(reply, SIGNAL(finished()), this, SLOT(mediaUploadFinished()));
            connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(mediaUploadProgress(qint64, qint64)));
            }
      }

//---------------------------------------------------------
//   mediaUploadFinished
//---------------------------------------------------------

void LoginManager::mediaUploadFinished()
      {
      _uploadAudioMenuAction->setEnabled(true);
      QNetworkReply* reply = static_cast<QNetworkReply*>(QObject::sender());
      int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
      QNetworkReply::NetworkError e = reply->error();
      reply->deleteLater();
      _progressDialog->hide();
      _progressDialog->reset();
      if ((statusCode == 200 && reply->error() == QNetworkReply::NoError) || _progressDialog->wasCanceled()) {
            _mp3File->remove();
            delete _mp3File;
            _mediaUrl = "";
            emit mediaUploadSuccess();
            }
      else if (e == QNetworkReply::RemoteHostClosedError && _uploadTryCount < MAX_UPLOAD_TRY_COUNT) {
            uploadMedia();
            }
      else {
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

void LoginManager::mediaUploadProgress(qint64 progress, qint64 total)
      {
      if (!_progressDialog->wasCanceled()) {
            _progressDialog->setMinimum(0);
            _progressDialog->setMaximum(total);
            _progressDialog->setValue(progress);
            }
      }

//---------------------------------------------------------
//   upload
//---------------------------------------------------------

void LoginManager::upload(const QString& path, int nid, const QString& title)
      {
      qDebug() << "file upload" << nid;

      ApiRequest* r = new ApiRequest(this);
      r->setPath("/score/upload-light")
         .setToken(_accessToken);

      QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType, /* parent */ r);

      QHttpPart filePart;
      filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
      QString contentDisposition = QString("form-data; name=\"score_data\"; filename=\"temp_%1.mscz\"").arg(qrand() % 100000);
      filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(contentDisposition));
      QFile *file = new QFile(path);
      file->open(QIODevice::ReadOnly);
      filePart.setBodyDevice(file);
      file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart
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

      if (nid > 0) // score exists, update
            r->setMethod(ApiRequest::HTTP_PUT);
      else // score doesn't exist, post a new score
            r->setMethod(ApiRequest::HTTP_POST);

      connect(r, &ApiRequest::replyFinished, this, [this](ApiRequest* r) {
            onReplyFinished(r, RequestType::UPLOAD_SCORE);
            });

      r->executeRequest(_networkManager);
      }

//---------------------------------------------------------
//   onUploadReply
//---------------------------------------------------------

void LoginManager::onUploadReply(QNetworkReply* reply, int code, const QJsonObject& obj)
      {
      qDebug() << "onUploadReply" << code << reply->errorString();
      if (code == HTTP_OK) {
            constexpr const char* pathKey = "_webview_url";
            _updateScoreDataPath = obj.contains(pathKey) ? obj.value(pathKey).toString() : QString();

            if (obj.value("permalink") != QJsonValue::Undefined) {
                  emit uploadSuccess(obj.value("permalink").toString(), obj.value("id").toString(), obj.value("vid").toString());
                  }
            else {
                  emit uploadError(tr("An error occurred during the file transfer. Please try again"));
                  }
            }
      else
            emit uploadError(tr("Cannot upload: %1").arg(getErrorString(reply, obj)));
      }

//---------------------------------------------------------
//   syncGetScoreInfo
//---------------------------------------------------------

bool LoginManager::syncUpload(const QString& path, int nid, const QString& title)
      {
      connect(this, &LoginManager::uploadSuccess, m_asyncWait, &AsyncWait::success);
      connect(this, &LoginManager::uploadError, m_asyncWait, &AsyncWait::failure);
      upload(path, nid, title);
      bool success = (m_asyncWait->exec() == 0);
      disconnect(this, &LoginManager::uploadSuccess, m_asyncWait, &AsyncWait::success);
      disconnect(this, &LoginManager::uploadError, m_asyncWait, &AsyncWait::failure);
      if (!success) {
            qWarning() << m_asyncWait->errorMsg();
            }
      return success;
      }

//---------------------------------------------------------
//   updateScoreData
//---------------------------------------------------------

void LoginManager::updateScoreData(const QString& nid, bool newScore)
      {
      const QUrl url(ApiInfo::getUpdateScoreInfoUrl(nid, _accessToken, newScore, _updateScoreDataPath));
#ifdef USE_WEBENGINE
#if defined(WIN_PORTABLE)
      QWebEngineProfile* defaultProfile = QWebEngineProfile::defaultProfile();
      defaultProfile->setCachePath(QDir::cleanPath(QString("%1/../../../Data/settings/QWebEngine").arg(QCoreApplication::applicationDirPath())));
      defaultProfile->setPersistentStoragePath(QDir::cleanPath(QString("%1/../../../Data/settings/QWebEngine").arg(QCoreApplication::applicationDirPath())));
#endif
      QWebEngineView* webView = new QWebEngineView;
      webView->setWindowModality(Qt::ApplicationModal);
      webView->setAttribute(Qt::WA_DeleteOnClose);

      QWebEnginePage* page = webView->page();
      QWebEngineProfile* profile = page->profile();

      profile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
#if defined(WIN_PORTABLE)
      profile->setCachePath(QDir::cleanPath(QString("%1/../../../Data/settings/QWebEngine").arg(QCoreApplication::applicationDirPath())));
      profile->setPersistentStoragePath(QDir::cleanPath(QString("%1/../../../Data/settings/QWebEngine").arg(QCoreApplication::applicationDirPath())));
#endif
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
//   hasAccessToken
//---------------------------------------------------------

bool LoginManager::hasAccessToken()
      {
      return !_accessToken.isEmpty();
      }

//---------------------------------------------------------
//   logout
//---------------------------------------------------------

bool LoginManager::logout()
      {
      if (!_accessToken.isEmpty()) {
            ApiRequest* r = new ApiRequest(this);
            r->setPath("/auth/login")
               .setMethod(ApiRequest::HTTP_DELETE)
               .setToken(_accessToken);

            connect(r, &ApiRequest::replyFinished, r, &ApiRequest::deleteLater); // we don't need the reply info here
            r->executeRequest(_networkManager);
            }

      _accessToken.clear();
      _refreshToken.clear();
      QFile loadFile(dataPath + "/cred.dat");
      if (!loadFile.exists())
            return true;
      return loadFile.remove();
      }

//---------------------------------------------------------
//   ApiRequest::setToken
//---------------------------------------------------------

ApiRequest& ApiRequest::setToken(const QString& token)
      {
      const QString tokenKey("token");
      _urlQuery.removeQueryItem(tokenKey);
      if (!token.isEmpty())
            _urlQuery.addQueryItem(tokenKey, token);
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
      r.setHeader(QNetworkRequest::UserAgentHeader, apiInfo.userAgent);
      r.setRawHeader(apiInfo.clientIdHeader, apiInfo.clientId);
      r.setRawHeader(apiInfo.apiKeyHeader, apiInfo.apiKey);

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
                  if (_multipart)
                        _reply = networkManager->post(request, _multipart);
                  else
                        _reply = networkManager->post(request, data);
                  break;
            case HTTP_PUT:
                  if (_multipart)
                        _reply = networkManager->put(request, _multipart);
                  else
                        _reply = networkManager->put(request, data);
                  break;
            case HTTP_DELETE:
                  _reply = networkManager->deleteResource(request);
                  break;
            };

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
      request.setHttpHeader("User-Agent", apiInfo.userAgent);
      request.setHttpHeader(apiInfo.clientIdHeader, apiInfo.clientId);
      request.setHttpHeader(apiInfo.apiKeyHeader, apiInfo.apiKey);
      }
#endif
}
