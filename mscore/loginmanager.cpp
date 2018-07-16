
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
#include "musescore.h"
#include "libmscore/score.h"
#include "preferences.h"
#include "kQOAuth/kqoauthrequest.h"
#include "kQOAuth/kqoauthrequest_xauth.h"

namespace Ms {

extern QString dataPath;

static const char* MUSESCORE_HOST = "api.musescore.com";

//---------------------------------------------------------
//   LoginManager
//---------------------------------------------------------

LoginManager::LoginManager(QObject* parent)
 : QObject(parent)
      {
      _oauthManager = new KQOAuthManager(this);
      connect(_oauthManager, SIGNAL(accessTokenReceived(QString, QString)),
            this, SLOT(onAccessTokenReceived(QString, QString)));
      connect(_oauthManager, SIGNAL(authorizedRequestDone()),
            this, SLOT(onAuthorizedRequestDone()));
      QByteArray ba;
      ba.resize(32);
      ba[0] = 0x68; ba[1] = 0x74; ba[2] = 0x55; ba[3] = 0x38;
      ba[4] = 0x48; ba[5] = 0x45; ba[6] = 0x4c; ba[7] = 0x45;
      ba[8] = 0x4d; ba[9] = 0x47; ba[10] = 0x43; ba[11] = 0x55;
      ba[12] = 0x6e; ba[13] = 0x6f; ba[14] = 0x53; ba[15] = 0x54;
      ba[16] = 0x38; ba[17] = 0x67; ba[18] = 0x6b; ba[19] = 0x78;
      ba[20] = 0x34; ba[21] = 0x77; ba[22] = 0x33; ba[23] = 0x69;
      ba[24] = 0x52; ba[25] = 0x63; ba[26] = 0x64; ba[27] = 0x6e;
      ba[28] = 0x41; ba[29] = 0x6a; ba[30] = 0x37; ba[31] = 0x51;
      _consumerKey = QString(ba);
      ba[0] = 0x52; ba[1] = 0x50; ba[2] = 0x75; ba[3] = 0x32;
      ba[4] = 0x79; ba[5] = 0x52; ba[6] = 0x69; ba[7] = 0x52;
      ba[8] = 0x6f; ba[9] = 0x58; ba[10] = 0x53; ba[11] = 0x41;
      ba[12] = 0x48; ba[13] = 0x6d; ba[14] = 0x4a; ba[15] = 0x6f;
      ba[16] = 0x6b; ba[17] = 0x61; ba[18] = 0x62; ba[19] = 0x59;
      ba[20] = 0x35; ba[21] = 0x37; ba[22] = 0x59; ba[23] = 0x74;
      ba[24] = 0x66; ba[25] = 0x51; ba[26] = 0x5a; ba[27] = 0x36;
      ba[28] = 0x77; ba[29] = 0x35; ba[30] = 0x69; ba[31] = 0x77;
      _consumerSecret  = QString(ba);
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
      if (_accessToken.isEmpty() || _accessTokenSecret.isEmpty())
            return true;
      QFile saveFile(dataPath + "/cred.dat");
      if (!saveFile.open(QIODevice::WriteOnly))
            return false;
      QJsonObject saveObject;
      saveObject["accessToken"] = _accessToken;
      saveObject["accessTokenSecret"] = _accessTokenSecret;
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
      _accessTokenSecret = saveObject["accessTokenSecret"].toString();
      loadFile.close();
      return true;
      }

//---------------------------------------------------------
//   onAuthorizedRequestDone
//---------------------------------------------------------

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
      mscore->showLoginDialog();
      }
/*------- END - TRY LOGIN ROUTINES ----------------------------*/

//---------------------------------------------------------
//   login
//---------------------------------------------------------

void LoginManager::login(QString login, QString password)
      {
      if(login == "" || password == "")
           return;

      connect(_oauthManager, SIGNAL(requestReady(QByteArray)),
                this, SLOT(onAccessTokenRequestReady(QByteArray)), Qt::UniqueConnection);

      KQOAuthRequest_XAuth *oauthRequest = new KQOAuthRequest_XAuth(this);
      oauthRequest->initRequest(KQOAuthRequest::AccessToken, QUrl(QString("https://%1/oauth/access_token").arg(MUSESCORE_HOST)));
      oauthRequest->setConsumerKey(_consumerKey);
      oauthRequest->setConsumerSecretKey(_consumerSecret);
      oauthRequest->setXAuthLogin(login, password);
      _oauthManager->executeRequest(oauthRequest);
     }

//---------------------------------------------------------
//   onAccessTokenReceived
//---------------------------------------------------------

void LoginManager::onAccessTokenReceived(QString token, QString tokenSecret)
      {
      //qDebug() << "Access token received: " << token << tokenSecret;
      _accessToken = token;
      _accessTokenSecret = tokenSecret;
      disconnect(_oauthManager, SIGNAL(requestReady(QByteArray)), this, SLOT(onAccessTokenRequestReady(QByteArray)));
      emit loginSuccess();
      }

//---------------------------------------------------------
//   onAccessTokenRequestReady
//---------------------------------------------------------

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

//---------------------------------------------------------
//   getUser
//---------------------------------------------------------

void LoginManager::getUser()
      {
      //qDebug() << "getUser";
      if (_accessToken.isEmpty() || _accessTokenSecret.isEmpty()) {
            emit getUserError("getUser - No token");
            return;
            }
      KQOAuthRequest * oauthRequest = new KQOAuthRequest();
      oauthRequest->initRequest(KQOAuthRequest::AuthorizedRequest, QUrl(QString("https://%1/services/rest/me.json").arg(MUSESCORE_HOST)));
      oauthRequest->setHttpMethod(KQOAuthRequest::GET);
      oauthRequest->setConsumerKey(_consumerKey);
      oauthRequest->setConsumerSecretKey(_consumerSecret);
      oauthRequest->setToken(_accessToken);
      oauthRequest->setTokenSecret(_accessTokenSecret);

      connect(_oauthManager, SIGNAL(requestReady(QByteArray)),
            this, SLOT(onGetUserRequestReady(QByteArray)));

      _oauthManager->executeRequest(oauthRequest);
      }

//---------------------------------------------------------
//   onGetUserRequestReady
//---------------------------------------------------------

void LoginManager::onGetUserRequestReady(QByteArray ba)
      {
      //qDebug() << "onGetUserRequestReady" << ba;
      disconnect(_oauthManager, SIGNAL(requestReady(QByteArray)),
            this, SLOT(onGetUserRequestReady(QByteArray)));
      if (_oauthManager->lastError() == KQOAuthManager::NoError) {
            QJsonDocument jsonResponse = QJsonDocument::fromJson(ba);
            QJsonObject user = jsonResponse.object();
            if (user.value("name") != QJsonValue::Undefined) {
            	_userName = user.value("name").toString();
                  _uid = user.value("id").toString().toInt();
                  emit getUserSuccess();
                  }
            else {
                  emit getUserError(tr("Error while getting user info. Please try again"));
                  }
            }
      else if (_oauthManager->lastError() != KQOAuthManager::NetworkError) {
            emit getUserError(tr("Error while getting user info: %1").arg(_oauthManager->lastError()));
            }

      }

//---------------------------------------------------------
//   getScore
//---------------------------------------------------------

void LoginManager::getScore(int nid)
      {
      //qDebug() << "getScore";
      if (_accessToken.isEmpty() || _accessTokenSecret.isEmpty()) {
            emit getScoreError("getScore - No token");
            return;
            }
      KQOAuthRequest * oauthRequest = new KQOAuthRequest();
      oauthRequest->initRequest(KQOAuthRequest::AuthorizedRequest, QUrl(QString("https://%1/services/rest/score/%2.json").arg(MUSESCORE_HOST).arg(nid)));
      oauthRequest->setHttpMethod(KQOAuthRequest::GET);
      oauthRequest->setConsumerKey(_consumerKey);
      oauthRequest->setConsumerSecretKey(_consumerSecret);
      oauthRequest->setToken(_accessToken);
      oauthRequest->setTokenSecret(_accessTokenSecret);

      connect(_oauthManager, SIGNAL(requestReady(QByteArray)),
            this, SLOT(onGetScoreRequestReady(QByteArray)));

      _oauthManager->executeRequest(oauthRequest);
      }

//---------------------------------------------------------
//   getScore
//---------------------------------------------------------

void LoginManager::getMediaUrl(const QString& nid, const QString& vid, const QString& encoding)
      {
      KQOAuthRequest * oauthRequest = new KQOAuthRequest();
      QString url = QString("https://%1/services/rest/signedurl/%2/%3/%4.json").arg(MUSESCORE_HOST).arg(nid).arg(vid).arg(encoding);
      oauthRequest->initRequest(KQOAuthRequest::AuthorizedRequest, QUrl(url));
      oauthRequest->setHttpMethod(KQOAuthRequest::GET);
      oauthRequest->setConsumerKey(_consumerKey);
      oauthRequest->setConsumerSecretKey(_consumerSecret);
      oauthRequest->setToken(_accessToken);
      oauthRequest->setTokenSecret(_accessTokenSecret);

      connect(_oauthManager, SIGNAL(requestReady(QByteArray)),
            this, SLOT(onGetMediaUrlRequestReady(QByteArray)));

      _oauthManager->executeRequest(oauthRequest);
      }

//---------------------------------------------------------
//   onGetUserRequestReady
//---------------------------------------------------------

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

//---------------------------------------------------------
//   onGetMediaUrlRequestReady
//---------------------------------------------------------

void LoginManager::onGetMediaUrlRequestReady(QByteArray ba)
      {
      disconnect(_oauthManager, SIGNAL(requestReady(QByteArray)),
            this, SLOT(onGetMediaUrlRequestReady(QByteArray)));
      QJsonDocument jsonResponse = QJsonDocument::fromJson(ba);
      QJsonObject response = jsonResponse.object();
      QJsonValue urlValue = response.value("url");
      if (urlValue.isString()) {
            _mediaUrl = response.value("url").toString();
            QString mp3Path = QDir::tempPath() + QString("/temp_%1.mp3").arg(qrand() % 100000);
            _mp3File = new QFile(mp3Path);
            Score* score = mscore->currentScore()->rootScore();
            int br = preferences.exportMp3BitRate;
            preferences.exportMp3BitRate = 128;
            if (mscore->saveMp3(score, mp3Path)) { // no else, error handling is done in saveMp3
                  _uploadTryCount = 0;
                  uploadMedia();
                  }
            preferences.exportMp3BitRate = br;
            }
      }

//---------------------------------------------------------
//   uploadMedia
//---------------------------------------------------------

void LoginManager::uploadMedia()
      {
      if (_mediaUrl.isEmpty()) {
            _progressDialog->hide();
            return;
            }
      if (!_mp3File->exists()) {
            emit displaySuccess();
            return;
            }
      if (_mp3File->open(QIODevice::ReadOnly)) { // probably cancelled, no error handling
            QNetworkRequest request;
            request.setUrl(QUrl(_mediaUrl));
            request.setHeader(QNetworkRequest::KnownHeaders::ContentLengthHeader, _mp3File->size());
            _progressDialog->reset();
            _progressDialog->setLabelText(tr("Uploading..."));
            _progressDialog->setCancelButtonText(tr("Cancel"));
            _progressDialog->show();
            _uploadTryCount++;
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
      QNetworkReply* reply = static_cast<QNetworkReply*>(QObject::sender());
      int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
      QNetworkReply::NetworkError e = reply->error();
      reply->deleteLater();
      _progressDialog->hide();
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
      //qDebug() << "file upload";
      KQOAuthRequest *oauthRequest = new KQOAuthRequest(this);
      QUrl url(QString("https://%1/services/rest/score.json").arg(MUSESCORE_HOST));
      if (nid > 0)
            url = QUrl(QString("https://%1/services/rest/score/%2/update.json").arg(MUSESCORE_HOST).arg(nid));
      oauthRequest->initRequest(KQOAuthRequest::AuthorizedRequest, url);
      oauthRequest->setConsumerKey(_consumerKey);
      oauthRequest->setConsumerSecretKey(_consumerSecret);
      oauthRequest->setToken(_accessToken);
      oauthRequest->setTokenSecret(_accessTokenSecret);

      oauthRequest->setContentType("multipart/form-data");

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

      if (nid > 0) {
            QHttpPart changesPart;
            changesPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"revision_log\""));
            changesPart.setBody(changes.toUtf8());
            multiPart->append(changesPart);
      }

      connect(_oauthManager, SIGNAL(requestReady(QByteArray)),
            this, SLOT(onUploadRequestReady(QByteArray)));
      oauthRequest->setHttpMultiPart(multiPart);
      _oauthManager->executeRequest(oauthRequest);
     }

//---------------------------------------------------------
//   onUploadRequestReady
//---------------------------------------------------------

void LoginManager::onUploadRequestReady(QByteArray ba)
      {
      disconnect(_oauthManager, SIGNAL(requestReady(QByteArray)),
            this, SLOT(onUploadRequestReady(QByteArray)));
      //qDebug() << "onUploadRequestReady" << ba;
      if (_oauthManager->lastError() == KQOAuthManager::NoError) {
            QJsonDocument jsonResponse = QJsonDocument::fromJson(ba);
            QJsonObject score = jsonResponse.object();
            if (score.value("permalink") != QJsonValue::Undefined) {
                  emit uploadSuccess(score.value("permalink").toString(), score.value("id").toString(), score.value("vid").toString());
                  }
            else {
                  emit uploadError(tr("An error occurred during the file transfer. Please try again"));
                  }
            }
      else {
            emit uploadError(tr("Cannot upload: %1").arg(_oauthManager->lastError()));
            }
      }

//---------------------------------------------------------
//   hasAccessToken
//---------------------------------------------------------

bool LoginManager::hasAccessToken()
      {
      return !_accessTokenSecret.isEmpty() && !_accessToken.isEmpty();
      }

//---------------------------------------------------------
//   logout
//---------------------------------------------------------

bool LoginManager::logout()
      {
      _accessToken = "";
      _accessTokenSecret = "";
      QFile loadFile(dataPath + "/cred.dat");
      if (!loadFile.exists())
      	return true;
      return loadFile.remove();
      }

}

