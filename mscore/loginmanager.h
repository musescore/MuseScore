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
      
      KQOAuthManager* _oauthManager;
      QString _consumerKey = 0;
      QString _consumerSecret = 0;
      QString _accessToken = 0;
      QString _accessTokenSecret = 0;
      QString _userName = 0;
      int _uid = -1;

   signals:
      void loginError(const QString& error);
      void loginSuccess();
      void getUserError(const QString& error);
      void getUserSuccess();
      void getScoreError(const QString& error);
      void getScoreSuccess(const QString &title, const QString &description, bool priv, const QString& license, const QString& tags);
      void uploadError(const QString& error);
      void uploadSuccess(const QString& url);
      void tryLoginSuccess();

   private slots:
      void onAccessTokenRequestReady(QByteArray ba);
      void onAccessTokenReceived(QString token, QString tokenSecret);
      void onGetUserRequestReady(QByteArray ba);
      void onGetScoreRequestReady(QByteArray ba);
      void onAuthorizedRequestDone();
      void onUploadRequestReady(QByteArray ba);

      void onTryLoginSuccess();
      void onTryLoginError(const QString&);

   public slots:
      void tryLogin();
   
   public:
      LoginManager(QObject* parent = 0);
      void login(QString login, QString password);
      void upload(const QString& path, int nid, const QString& title, const QString& description, const QString& priv, const QString& license, const QString& tags);
      bool hasAccessToken();
      void getUser();
      void getScore(int nid);

      bool save();
      bool load();

      bool logout();

      QString userName() { return _userName; }
      int uid()          { return _uid; }
      };
}

#endif

