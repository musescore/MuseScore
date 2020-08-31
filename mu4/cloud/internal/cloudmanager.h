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

#ifndef MU_CLOUD_CLOUDMANAGER_H
#define MU_CLOUD_CLOUDMANAGER_H

#include "config.h"

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"
#include "../imp3exporter.h"
#include "../cloudtypes.h"

namespace Ms {
class ApiRequest;

//---------------------------------------------------------
//   LoginManager
//---------------------------------------------------------

class CloudManager : public QObject
{
    Q_OBJECT

    INJECT(cloud, mu::cloud::IMp3Exporter, mp3Exporter)
    INJECT(cloud, mu::framework::IGlobalConfiguration, globalConfiguration)

    enum class RequestType
    {
        LOGIN,
        LOGIN_REFRESH,
        GET_USER_INFO,
        GET_SCORE_INFO,
        UPLOAD_SCORE,
        GET_MEDIA_URL,
    };

    static constexpr int MAX_UPLOAD_TRY_COUNT = 5;
    static constexpr int MAX_REFRESH_LOGIN_RETRY_COUNT = 2;

    QNetworkAccessManager* m_networkManager = nullptr;

    QAction* m_uploadAudioMenuAction = nullptr;
    QString m_accessToken;
    QString m_refreshToken;

    mu::cloud::AccountInfo m_accountInfo;

    QString m_updateScoreDataPath;

    QString m_mediaUrl;
    QFile* m_mp3File = nullptr;
    int m_uploadTryCount = 0;

    QProgressDialog* m_progressDialog = nullptr;

    void onReplyFinished(ApiRequest*, RequestType);
    void handleReply(QNetworkReply*, RequestType);
    static QString getErrorString(QNetworkReply*, const QJsonObject&);

    void onGetUserReply(QNetworkReply* reply, int code, const QJsonObject& replyContent);
    void onLoginReply(QNetworkReply* reply, int code, const QJsonObject& replyContent);
    void onLoginRefreshReply(QNetworkReply* reply, int code, const QJsonObject& replyContent);
    void onUploadReply(QNetworkReply* reply, int code, const QJsonObject& replyContent);
    void onGetScoreInfoReply(QNetworkReply* reply, int code, const QJsonObject& replyContent);
    void onGetMediaUrlReply(QNetworkReply* reply, int code, const QJsonObject& replyContent);

    ApiRequest* buildLoginRefreshRequest() const;

    bool save();

#ifdef USE_WEBENGINE
    void showWebViewDialog(const QUrl& url);
#endif

signals:
    void loginError(const QString& error);
    void loginSuccess();
    void getUserError(const QString& error);
    void getUserSuccess();
    void getScoreError(const QString& error);
    void getScoreSuccess(const QString& title, const QString& description, bool priv, const QString& license,const QString& tags,
                         const QString& url);
    void uploadError(const QString& error);
    void uploadSuccess(const QString& url, const QString& nid, const QString& vid);
    void tryLoginSuccess();
    void mediaUploadSuccess();

    void loginDialogRequested();

private slots:
    void uploadMedia();
    void mediaUploadFinished();
    void mediaUploadProgress(qint64, qint64);

    void onTryLoginSuccess();
    void onTryLoginError(const QString&);

public slots:
    void tryLogin();

public:
    explicit CloudManager(QObject* parent = nullptr);

    CloudManager(QAction* uploadAudioMenuAction, QProgressDialog* progress, QObject* parent = 0);

    bool init();
    void createAccount();
    void getUser();
    void login(QString login, QString password);
    bool logout();

    void upload(const QString& path, int nid, const QString& title);
    void updateScoreData(const QString& nid, bool newScore);
    void getScoreInfo(int nid);
    void getMediaUrl(const QString& nid, const QString& vid, const QString& format);

    mu::cloud::AccountInfo accountInfo() const { return m_accountInfo; }
};
}

#endif
