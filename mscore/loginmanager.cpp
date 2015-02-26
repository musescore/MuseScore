
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
#include "kQOAuth/kqoauthrequest.h"
#include "kQOAuth/kqoauthrequest_xauth.h"

namespace Ms {

extern QString dataPath;

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
      ba[0] = 0x52; ba[1] = 0x61; ba[2] = 0x70; ba[3] = 0x33;
      ba[4] = 0x74; ba[5] = 0x52; ba[6] = 0x58; ba[7] = 0x70;
      ba[8] = 0x62; ba[9] = 0x43; ba[10] = 0x70; ba[11] = 0x39;
      ba[12] = 0x36; ba[13] = 0x35; ba[14] = 0x46; ba[15] = 0x4a;
      ba[16] = 0x64; ba[17] = 0x5a; ba[18] = 0x46; ba[19] = 0x75;
      ba[20] = 0x54; ba[21] = 0x35; ba[22] = 0x5a; ba[23] = 0x63;
      ba[24] = 0x42; ba[25] = 0x73; ba[26] = 0x4b; ba[27] = 0x67;
      ba[28] = 0x4a; ba[29] = 0x7a; ba[30] = 0x4c; ba[31] = 0x44;
      _consumerKey = QString(ba);
      ba[0] = 0x35; ba[1] = 0x39; ba[2] = 0x33; ba[3] = 0x61; 
      ba[4] = 0x57; ba[5] = 0x6f; ba[6] = 0x51; ba[7] = 0x73;
      ba[8] = 0x77; ba[9] = 0x73; ba[10] = 0x50; ba[11] = 0x44;
      ba[12] = 0x56; ba[13] = 0x48; ba[14] = 0x37; ba[15] = 0x4c;
      ba[16] = 0x58; ba[17] = 0x76; ba[18] = 0x6e; ba[19] = 0x61;
      ba[20] = 0x51; ba[21] = 0x71; ba[22] = 0x34; ba[23] = 0x4b;
      ba[24] = 0x45; ba[25] = 0x5a; ba[26] = 0x42; ba[27] = 0x4a;
      ba[28] = 0x64; ba[29] = 0x74; ba[30] = 0x4e; ba[31] = 0x74;
      _consumerSecret  = QString(ba);
      load();
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
            QMessageBox::critical(0, tr("Network error"), tr("Please check your internet connection"));
      else if (_oauthManager->lastError() == KQOAuthManager::ContentOperationNotPermittedError)
            QMessageBox::critical(0, tr("Please upgrade"), tr("Your MuseScore version is too old to use this feature.<br/> <a href=\"%1\">Please upgrade first</a>.").arg("http://musescore.org"));
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
      oauthRequest->initRequest(KQOAuthRequest::AccessToken, QUrl("https://api.musescore.com/oauth/access_token"));
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
                           message = tr("Sorry, wrong email address, username or password. Please check again. <a href=\"%1\">Have you forgotten your password</a>?").arg("https://musescore.com/user/password");
                           }
                     else if (code == "USER_DENIED") {
                           message = tr("This account has been blocked.");
                           }
                     else if (code == "USER_NOT_ACTIVATED") {
                           message = tr("Your account has not been activated yet. Please check your mailbox to activate your account or <a href=\"%1\">request a new activation email</a>.").arg("https://musescore.com/user/resendregistrationpassword");
                           }
                     }
                 }
                 emit loginError(message);
            }
      else if (_oauthManager->lastError() == KQOAuthManager::NetworkError) {
            QMessageBox::critical(0, tr("Network error"), tr("Please check your internet connection"));
            }
      else if (_oauthManager->lastError() == KQOAuthManager::ContentOperationNotPermittedError) {
            QMessageBox::critical(0, tr("Please upgrade"), tr("Your MuseScore version is too old to use this feature.<br/> <a href=\"%1\">Please upgrade first</a>.").arg("http://musescore.org"));
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
      oauthRequest->initRequest(KQOAuthRequest::AuthorizedRequest, QUrl("https://api.musescore.com/services/rest/me.json"));
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
      oauthRequest->initRequest(KQOAuthRequest::AuthorizedRequest, QUrl(QString("https://api.musescore.com/services/rest/score/%1.json").arg(nid)));
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
//   upload
//---------------------------------------------------------

void LoginManager::upload(const QString &path, int nid, const QString &title, const QString &description, const QString& priv, const QString& license, const QString& tags)
      {
      //qDebug() << "file upload";
      KQOAuthRequest *oauthRequest = new KQOAuthRequest(this);
      QUrl url("https://api.musescore.com/services/rest/score.json");
      if (nid > 0)
            url = QUrl(QString("https://api.musescore.com/services/rest/score/%1/update.json").arg(nid));
      oauthRequest->initRequest(KQOAuthRequest::AuthorizedRequest, url);
      oauthRequest->setConsumerKey(_consumerKey);
      oauthRequest->setConsumerSecretKey(_consumerSecret);
      oauthRequest->setToken(_accessToken);
      oauthRequest->setTokenSecret(_accessTokenSecret);

      oauthRequest->setContentType("multipart/form-data");

      QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

      QHttpPart filePart;
      filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
      filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"score_data\"; filename=\"temp.mscz\""));
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
                  emit uploadSuccess(score.value("permalink").toString());
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

