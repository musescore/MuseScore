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
#include "kQOAuth/kqoauthrequest.h"
#include "kQOAuth/kqoauthrequest_xauth.h"

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
   userAgent(QString(userAgentTemplate).arg(VERSION).arg(BUILD_NUMBER).arg(getOsInfo()).toLatin1())
      {
      if (MScore::debugMode) {
            qWarning("clientId: %s", clientId.constData());
            qWarning("apiKey: %s", apiKey.constData());
            qWarning("userAgent: %s", userAgent.constData());
            }
      }

//---------------------------------------------------------
//   LoginManager
//---------------------------------------------------------

LoginManager::LoginManager(QAction* uploadAudioMenuAction, QObject* parent)
 : QObject(parent), _networkManager(new QNetworkAccessManager(this)),
   _uploadAudioMenuAction(uploadAudioMenuAction)
      {
      load();
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
      if (_accessToken.isEmpty() || _refreshToken.isEmpty())
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

void LoginManager::onReplyFinished(QNetworkReply* reply, RequestType requestType)
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

      reply->deleteLater();
      }

//---------------------------------------------------------
//   getErrorString
//---------------------------------------------------------

QString LoginManager::getErrorString(QNetworkReply* reply, const QJsonObject& obj)
      {
      const QString err = reply ? reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString() : tr("Error");
      const QString msg = obj["message"].toString();
      return QString("%1 (%2)").arg(err).arg(msg);
      }

//---------------------------------------------------------
//   onAuthorizedRequestDone
//---------------------------------------------------------
#if 0
void LoginManager::onAuthorizedRequestDone()
      {
      if (_oauthManager->lastError() == KQOAuthManager::NetworkError)
            QMessageBox::critical(0, tr("Network error"), tr("Please check your Internet connection"));
      else if (_oauthManager->lastError() == KQOAuthManager::ContentOperationNotPermittedError)
            QMessageBox::critical(0, tr("Please upgrade"), tr("Your MuseScore version is too old to use this feature.\n"
                                                              "%1Please upgrade first%2.")
                                  .arg("<a href=\"https://musescore.org\">")
                                  .arg("</a>")
                                  .replace("\n", "<br/>"));
      // don't do that, it will logout user if score is private and already known
      //else if (_oauthManager->lastError() == KQOAuthManager::RequestUnauthorized){
      //      logout();
      //      mscore->showLoginDialog();
      //      }
      }
#endif

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
//   loginInteractive
//---------------------------------------------------------

#ifdef USE_WEBENGINE
void LoginManager::loginInteractive()
      {
      QWebEngineView* webView = new QWebEngineView;
      webView->setWindowModality(Qt::ApplicationModal);
      webView->setAttribute(Qt::WA_DeleteOnClose);

      QWebEnginePage* page = webView->page();
      QWebEngineProfile* profile = page->profile();
      // TODO: logout in editor does not log out in web view
      profile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
      profile->setRequestInterceptor(new ApiWebEngineRequestInterceptor(profile));

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

      ApiRequest r = ApiRequestBuilder()
         .setPath("/auth/login")
         .addPostParameter("field", login)
         .addPostParameter("password", password)
         .build();

      QNetworkReply* reply = _networkManager->put(r.request, r.data);
      connect(reply, &QNetworkReply::finished, this, [this, reply] {
            onReplyFinished(reply, RequestType::LOGIN);
            });
      }

//---------------------------------------------------------
//   onLoginSuccessReply
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
      }

//---------------------------------------------------------
//   onAccessTokenReceived
//---------------------------------------------------------
#if 0
void LoginManager::onAccessTokenReceived(QString token, QString tokenSecret)
      {
      //qDebug() << "Access token received: " << token << tokenSecret;
      _accessToken = token;
      _accessTokenSecret = tokenSecret;
      disconnect(_oauthManager, SIGNAL(requestReady(QByteArray)), this, SLOT(onAccessTokenRequestReady(QByteArray)));
      emit loginSuccess();
      }
#endif

//---------------------------------------------------------
//   onAccessTokenRequestReady
//---------------------------------------------------------
#if 0
void LoginManager::onAccessTokenRequestReady(QByteArray ba)
      {
      //qDebug() << "onAccessTokenRequestReady" << ba;
      if (_oauthManager->lastError() == KQOAuthManager::RequestUnauthorized) { // 401/406

            QJsonDocument jsonResponse = QJsonDocument::fromJson(ba);
            QJsonArray array = jsonResponse.array();
            QString message = tr("Unsuccessful login. Please try again.");
            if (array.size() > 0) {
                 QJsonObject o = array.at(0).toObject();
                 if (o.value("code") != QJsonValue::Undefined) {
                 	   QString code = o["code"].toString();
                     if (code == "USER_AUTHENTICATION_FAILED") {
                           message = tr("Sorry, wrong email address, username or password. Please check again. %1Have you forgotten your password%2?")
                                       .arg("<a href=\"https://musescore.com/user/password\">")
                                       .arg("</a>");
                           }
                     else if (code == "USER_DENIED") {
                           message = tr("This account has been blocked.");
                           }
                     else if (code == "USER_NOT_ACTIVATED") {
                           message = tr("Your account has not been activated yet. Please check your mailbox to activate your account or %1request a new activation email%2.")
                                       .arg("<a href=\"https://musescore.com/user/resendregistrationpassword\">")
                                       .arg("</a>");
                           }
                     else if (code == "USER_TIMESTAMP_EXPIRED") {
                           message = tr("The local time on your device is not set right. Please check it and adjust. It's advised to set the time/timezone to automatic. If you still can't log in, %1contact us%2.")
                                       .arg("<a href=\"https://musescore.com/contact?category=Login%20problems\">")
                                       .arg("</a>");
                           }
                     }
                 }
                 emit loginError(message);
            }
      else if (_oauthManager->lastError() == KQOAuthManager::NetworkError) {
            QMessageBox::critical(0, tr("Network error"), tr("Please check your Internet connection"));
            }
      else if (_oauthManager->lastError() == KQOAuthManager::ContentOperationNotPermittedError) {
            QMessageBox::critical(0, tr("Please upgrade"), tr("Your MuseScore version is too old to use this feature.\n"
                                                              "%1Please upgrade first%2.")
                                  .arg("<a href=\"https://musescore.org\">")
                                  .arg("</a>")
                                  .replace("\n", "<br/>"));
            }
      }
#endif

//---------------------------------------------------------
//   getUser
//---------------------------------------------------------

void LoginManager::getUser()
      {
      if (_accessToken.isEmpty() || _refreshToken.isEmpty()) {
            emit getUserError("getUser - No token");
            return;
            }

      ApiRequest r = ApiRequestBuilder()
         .setPath("/user/me")
         .setToken(_accessToken)
         .build();

      QNetworkReply* reply = _networkManager->get(r.request);
      connect(reply, &QNetworkReply::finished, this, [this, reply] {
            onReplyFinished(reply, RequestType::GET_USER_INFO);
            });
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
                  emit getUserSuccess();
                  }
            else
                  emit getUserError(tr("Wrong response from the server"));
            }
      else
            emit getUserError(tr("Error while getting user info: %1").arg(getErrorString(reply, user)));
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

      ApiRequest r = ApiRequestBuilder()
         .setPath("/score/full-info")
         .setToken(_accessToken)
         .addGetParameter("score_id", QString::number(nid))
         .build();

      QNetworkReply* reply = _networkManager->get(r.request);
      connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            onReplyFinished(reply, RequestType::GET_SCORE_INFO);
            });
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
                        if (uid == _uid)
                              emit getScoreSuccess(title, description, (sharing == "private"), license, tags, url);
                        else
                              emit getScoreError("");
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
//   getMediaUrl
//---------------------------------------------------------

void LoginManager::getMediaUrl(const QString& nid, const QString& vid, const QString& encoding)
      {
      Q_UNUSED(encoding);
      ApiRequest r = ApiRequestBuilder()
         .setPath("/score/audio")
         .setToken(_accessToken)
         .addGetParameter("score_id", nid)
         .addGetParameter("revision_id", vid)
         .build();

      QNetworkReply* reply = _networkManager->get(r.request);
      connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            onReplyFinished(reply, RequestType::GET_MEDIA_URL);
            });
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
#if 0
      disconnect(_oauthManager, SIGNAL(requestReady(QByteArray)),
            this, SLOT(onGetMediaUrlRequestReady(QByteArray)));
      QJsonDocument jsonResponse = QJsonDocument::fromJson(ba);
      QJsonObject response = jsonResponse.object();
      QJsonValue urlValue = response.value("url");
      if (urlValue.isString()) {
            _mediaUrl = response.value("url").toString();
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
#endif
      }

//---------------------------------------------------------
//   onGetUserRequestReady
//---------------------------------------------------------
#if 0
void LoginManager::onGetScoreRequestReady(QByteArray ba)
      {
      //qDebug() << "onGetScoreRequestReady" << ba;
      //qDebug() << _oauthManager->lastError();
      disconnect(_oauthManager, SIGNAL(requestReady(QByteArray)),
            this, SLOT(onGetScoreRequestReady(QByteArray)));
      if (_oauthManager->lastError() == KQOAuthManager::NoError) {
            QJsonDocument jsonResponse = QJsonDocument::fromJson(ba);
            QJsonObject score = jsonResponse.object();
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
                        if (uid == _uid)
                              emit getScoreSuccess(title, description, (sharing == "private"), license, tags, url);
                        else
                              emit getScoreError("");
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
#endif

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
            emit displaySuccess();
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
            emit displaySuccess();
            }
      else if (e == QNetworkReply::RemoteHostClosedError && _uploadTryCount < MAX_UPLOAD_TRY_COUNT) {
            uploadMedia();
            }
      else {
            qDebug() << "error uploading media" << e;
            QMessageBox::warning(0,
                     tr("Upload Error"),
                     tr("Sorry, MuseScore couldn't upload the audio file. Error %1").arg(e),
                     QString::null, QString::null);
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

void LoginManager::upload(const QString &path, int nid, const QString &title, const QString &description, const QString& priv, const QString& license, const QString& tags, const QString& changes)
      {
#if ! 0 // see further down
      Q_UNUSED(changes);
#endif
      qDebug() << "file upload" << nid;
//       KQOAuthRequest *oauthRequest = new KQOAuthRequest(this);
//       QUrl url(QString("https://%1/services/rest/score.json").arg(MUSESCORE_HOST));
//       if (nid > 0)
//             url = QUrl(QString("https://%1/services/rest/score/%2/update.json").arg(MUSESCORE_HOST).arg(nid));

      ApiRequest r = ApiRequestBuilder()
         .setPath("/score/upload")
         .setToken(_accessToken)
         .build();

      QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

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
            qDebug() << "added idPart";
            idPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"score_id\""));
            idPart.setBody(QString::number(nid).toLatin1()); // TODO: check
            multiPart->append(idPart);
            }

      QHttpPart titlePart;
      titlePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"title\""));
      titlePart.setBody(title.toUtf8());
      multiPart->append(titlePart);

      QHttpPart descriptionPart;
      descriptionPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"description\""));
      descriptionPart.setBody(description.toUtf8());
      multiPart->append(descriptionPart);

      QHttpPart privatePart;
      privatePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"private\""));
      privatePart.setBody(priv.toUtf8());
      multiPart->append(privatePart);

      QHttpPart licensePart;
      licensePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"license\""));
      licensePart.setBody(license.toUtf8());
      multiPart->append(licensePart);

      QHttpPart tagsPart;
      tagsPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"tags\""));
      tagsPart.setBody(tags.toUtf8());
      multiPart->append(tagsPart);

#if 0 // TODO: what is this and is this now supported?
      if (nid > 0) {
            QHttpPart changesPart;
            changesPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"revision_log\""));
            changesPart.setBody(changes.toUtf8());
            multiPart->append(changesPart);
      }
#endif

      // TODO: "uri" parameter?
      QNetworkReply* reply;
      if (nid > 0) // score exists, update
            reply = _networkManager->put(r.request, multiPart);
      else // score doesn't exist, post a new score
            reply = _networkManager->post(r.request, multiPart);

      connect(reply, &QNetworkReply::finished, this, [this, reply] {
            onReplyFinished(reply, RequestType::UPLOAD_SCORE);
            });
     }

//---------------------------------------------------------
//   onUploadReply
//---------------------------------------------------------

void LoginManager::onUploadReply(QNetworkReply* reply, int code, const QJsonObject& obj)
      {
      qDebug() << "onUploadReply" << code << reply->errorString();
      if (code == HTTP_OK) {
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
            ApiRequest r = ApiRequestBuilder()
               .setPath("/auth/login")
               .setToken(_accessToken)
               .build();

            QNetworkReply* reply = _networkManager->deleteResource(r.request);
            connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater); // we don't need the reply info here
            }

      _accessToken.clear();
      _refreshToken.clear();
      QFile loadFile(dataPath + "/cred.dat");
      if (!loadFile.exists())
            return true;
      return loadFile.remove();
      }

//---------------------------------------------------------
//   ApiRequestBuilder::build
//---------------------------------------------------------

ApiRequest ApiRequestBuilder::build() const
      {
      ApiRequest r;

      QUrl url(_url);
      url.setQuery(_urlQuery);
      r.request.setUrl(url);
      r.request.setRawHeader("Accept", "application/json");
      const ApiInfo& apiInfo = ApiInfo::instance();
      r.request.setHeader(QNetworkRequest::UserAgentHeader, apiInfo.userAgent);
      r.request.setRawHeader(apiInfo.clientIdHeader, apiInfo.clientId);
      r.request.setRawHeader(apiInfo.apiKeyHeader, apiInfo.apiKey);

      r.data = _bodyQuery.toString().toLatin1();

      return r;
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
