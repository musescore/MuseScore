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
//    Contains information necessary for making a request:
//    request - the request object itself
//    data    - data of the request's body (for POST, PUT)
//---------------------------------------------------------

struct ApiRequest
      {
      QNetworkRequest request;
      QByteArray data;
      };

//---------------------------------------------------------
//   ApiRequestBuilder
//---------------------------------------------------------

class ApiRequestBuilder
      {
      QUrl _url;
      QUrlQuery _urlQuery;
      QUrlQuery _bodyQuery;

   public:
      ApiRequestBuilder() : _url(ApiInfo::apiHost) {}
      ApiRequestBuilder& setPath(const QString& path) { _url.setPath(ApiInfo::apiRoot + path); return *this; }
      ApiRequestBuilder& addGetParameter(const QString& key, const QString& val)  { _urlQuery.addQueryItem(key, val); return *this; }
      ApiRequestBuilder& addPostParameter(const QString& key, const QString& val) { _bodyQuery.addQueryItem(key, val); return *this; }
      ApiRequestBuilder& setToken(const QString& token) { addGetParameter("token", token); return *this; }

      ApiRequest build() const;
      };

//---------------------------------------------------------
//   ApiWebEngineRequestInterceptor
//---------------------------------------------------------

class ApiWebEngineRequestInterceptor : public QWebEngineUrlRequestInterceptor
      {
      Q_OBJECT
   public:
      ApiWebEngineRequestInterceptor(QObject* parent) : QWebEngineUrlRequestInterceptor(parent) {}
      void interceptRequest(QWebEngineUrlRequestInfo& info) override;
      };

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
