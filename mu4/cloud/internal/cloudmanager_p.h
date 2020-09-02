//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef MU_CLOUD_CLOUDMANAGER_P_H
#define MU_CLOUD_CLOUDMANAGER_P_H

#include "config.h"
#include "modularity/ioc.h"
#include "iglobalconfiguration.h"

namespace Ms {
//---------------------------------------------------------
//   ApiInfo
//---------------------------------------------------------

class ApiInfo
{
    INJECT_STATIC(cloud, mu::framework::IGlobalConfiguration, globalConfiguration)

    static ApiInfo* _instance;

    ApiInfo(QByteArray cliendId, QByteArray apiKey);
    ApiInfo(const ApiInfo&) = delete;
    ApiInfo& operator=(const ApiInfo&) = delete;

    static QByteArray genClientId();
    static void createInstance();

    static QString apiInfoLocation();
    static QString getOsInfo();

    static constexpr const char* MSCORE_HOST = "https://musescore.com";
    static constexpr const char* DEFAULT_UPDATE_SCORE_INFO_PATH = "/score/manage/upload/update";

public:
    static const ApiInfo& instance()
    {
        if (!_instance) {
            createInstance();
        }
        return *_instance;
    }

    static constexpr const char* API_HOST = "https://api.musescore.com/";
    static constexpr const char* API_ROOT = "/v2";
    static constexpr const char* CLIENT_ID_HEADER = "X-MS-CLIENT-ID";
    static constexpr const char* API_KEY_HEADER = "X-MS-API-KEY";
    static constexpr const char* USER_AGENT_TEMPLATE = "MS_EDITOR/%1.%2 (%3)";

    static constexpr const char* REGISTER_PAGE = "https://musescore.com/user/register?webview";
    static constexpr const char* LOGIN_PAGE = "https://musescore.com/user/auth/webview";
    static constexpr const char* LOGIN_SUCCESS_PAGE = "https://musescore.com/user/auth/webview/success";

    static QUrl getUpdateScoreInfoUrl(const QString& scoreId, const QString& accessToken, bool newScore,const QString& customPath);

    static const QUrl REGISTER_URL;
    static const QUrl LOGIN_URL;
    static const QUrl LOGIN_SUCCESS_URL;

    const QByteArray CLIENT_ID;
    const QByteArray API_KEY;
    const QByteArray USER_AGENT;
};

//---------------------------------------------------------
//   ApiRequest
//---------------------------------------------------------

class ApiRequest : public QObject
{
    Q_OBJECT
public:
    enum Method {
        HTTP_GET,
        HTTP_POST,
        HTTP_PUT,
        HTTP_DELETE
    };

private:
    QUrl _url;
    QUrlQuery _urlQuery;
    QUrlQuery _bodyQuery;
    QHttpMultiPart* _multipart = nullptr;

    QNetworkReply* _reply = nullptr;

    Method _method = HTTP_GET;

    int _retryCount = 0;

    QNetworkRequest buildRequest() const;

signals:
    void replyFinished(ApiRequest*);

public:
    ApiRequest(QObject* parent = nullptr)
        : QObject(parent), _url(ApiInfo::API_HOST) {}
    ApiRequest& setMethod(Method m) { _method = m; return *this; }
    ApiRequest& setPath(const QString& path)
    {
        _url.setPath(ApiInfo::API_ROOT + path);
        return *this;
    }

    ApiRequest& addGetParameter(const QString& key, const QString& val)
    {
        _urlQuery.addQueryItem(key, val);
        return *this;
    }

    ApiRequest& addPostParameter(const QString& key, const QString& val)
    {
        _bodyQuery.addQueryItem(key, val);
        return *this;
    }

    ApiRequest& setMultiPartData(QHttpMultiPart* m) { _multipart = m; m->setParent(this); return *this; }
    ApiRequest& setToken(const QString& token);

    void executeRequest(QNetworkAccessManager* mgr);
    QNetworkReply* reply() { return _reply; }
    const QNetworkReply* reply() const { return _reply; }

    int retryCount() const { return _retryCount; }
};

//---------------------------------------------------------
//   ApiWebEngineRequestInterceptor
//---------------------------------------------------------

#ifdef USE_WEBENGINE
class ApiWebEngineRequestInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT
public:
    ApiWebEngineRequestInterceptor(QObject* parent)
        : QWebEngineUrlRequestInterceptor(parent) {}
    void interceptRequest(QWebEngineUrlRequestInfo& info) override;
};
#endif

//---------------------------------------------------------
//   HttpStatus
//---------------------------------------------------------

enum HttpStatus
{
    HTTP_OK = 200,
    HTTP_UNAUTHORIZED = 401,
    HTTP_NOT_FOUND = 404
};
}
#endif
