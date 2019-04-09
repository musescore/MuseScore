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

#ifndef __LOGINMANAGER_P_H__
#define __LOGINMANAGER_P_H__

#include "config.h"

namespace Ms {

//---------------------------------------------------------
//   ApiInfo
//---------------------------------------------------------

class ApiInfo
      {
      static ApiInfo* _instance;

      ApiInfo(QByteArray clientId, QByteArray apiKey);
      ApiInfo(const ApiInfo&) = delete;
      ApiInfo& operator=(const ApiInfo&) = delete;

      static QByteArray genClientId();
      static void createInstance();

      static QString apiInfoLocation();
      static QString getOsInfo();

   public:
      static const ApiInfo& instance()
            {
            if (!_instance)
                  createInstance();
            return *_instance;
            }

      static constexpr const char* apiHost = "https://api.musescore.com/";
      static constexpr const char* apiRoot = "/v2";
      static constexpr const char* clientIdHeader = "X-MS-CLIENT-ID";
      static constexpr const char* apiKeyHeader = "X-MS-API-KEY";
      static constexpr const char* userAgentTemplate = "MS_EDITOR/%1.%2 (%3)";

      static constexpr const char* loginPage = "https://musescore.com/user/auth/webview";
      static constexpr const char* loginSuccessPage = "https://musescore.com/user/auth/webview/success";

      static const QUrl loginUrl;
      static const QUrl loginSuccessUrl;

      const QByteArray clientId;
      const QByteArray apiKey;
      const QByteArray userAgent;
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
         : QObject(parent), _url(ApiInfo::apiHost) {}
      ApiRequest& setMethod(Method m) { _method = m; return *this; }
      ApiRequest& setPath(const QString& path) { _url.setPath(ApiInfo::apiRoot + path); return *this; }
      ApiRequest& addGetParameter(const QString& key, const QString& val)  { _urlQuery.addQueryItem(key, val); return *this; }
      ApiRequest& addPostParameter(const QString& key, const QString& val) { _bodyQuery.addQueryItem(key, val); return *this; }
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
      ApiWebEngineRequestInterceptor(QObject* parent) : QWebEngineUrlRequestInterceptor(parent) {}
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
