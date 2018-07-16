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

#ifndef __LOGINMANAGER_H__
#define __LOGINMANAGER_H__

#include "thirdparty/kQOAuth/kqoauthmanager.h"

namespace Ms {

//---------------------------------------------------------
//   LoginDialog
//---------------------------------------------------------

class LoginManager : public QObject
      {
      Q_OBJECT

      static const int MAX_UPLOAD_TRY_COUNT = 5;

      KQOAuthManager* _oauthManager = nullptr;
      QAction* _uploadAudioMenuAction = nullptr;
      QString _consumerKey = 0;
      QString _consumerSecret = 0;
      QString _accessToken = 0;
      QString _accessTokenSecret = 0;
      QString _userName = 0;
      int _uid = -1;

      QString _mediaUrl;
      QFile* _mp3File;
      int _uploadTryCount = 0;

      QProgressDialog* _progressDialog;

   signals:
      void loginError(const QString& error);
      void loginSuccess();
      void getUserError(const QString& error);
      void getUserSuccess();
      void getScoreError(const QString& error);
      void getScoreSuccess(const QString &title, const QString &description, bool priv, const QString& license, const QString& tags, const QString& url);
      void uploadError(const QString& error);
      void uploadSuccess(const QString& url, const QString& nid, const QString& vid);
      void tryLoginSuccess();
      void displaySuccess();

   private slots:
      void onAccessTokenRequestReady(QByteArray ba);
      void onAccessTokenReceived(QString token, QString tokenSecret);
      void onGetUserRequestReady(QByteArray ba);
      void onGetScoreRequestReady(QByteArray ba);
      void onAuthorizedRequestDone();
      void onUploadRequestReady(QByteArray ba);
      void onGetMediaUrlRequestReady(QByteArray ba);

      void uploadMedia();
      void mediaUploadFinished();
      void mediaUploadProgress(qint64, qint64);

      void onTryLoginSuccess();
      void onTryLoginError(const QString&);

   public slots:
      void tryLogin();

   public:
      LoginManager(QAction* uploadAudioMenuAction, QObject* parent = 0);
      void login(QString login, QString password);
      void upload(const QString& path, int nid, const QString& title, const QString& description, const QString& priv, const QString& license, const QString& tags, const QString& changes);
      bool hasAccessToken();
      void getUser();
      void getScore(int nid);
      void getMediaUrl(const QString& nid, const QString& vid, const QString& format);

      bool save();
      bool load();

      bool logout();

      QString userName() { return _userName; }
      int uid()          { return _uid; }
      };
}

#endif

