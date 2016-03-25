/**
 * KQOAuth - An OAuth authentication library for Qt.
 *
 * Author: Johan Paul (johan.paul@gmail.com)
 *         http://www.johanpaul.com
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  KQOAuth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with KQOAuth.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef KQOAUTHREQUEST_H
#define KQOAUTHREQUEST_H

#include <QObject>
#include <QUrl>
#include <QMultiMap>
#include <QHttpMultiPart>

#include "kqoauthglobals.h"

typedef QMultiMap<QString, QString> KQOAuthParameters;

class KQOAuthRequestPrivate;
class KQOAuthRequest : public QObject
{
    Q_OBJECT
public:
    explicit KQOAuthRequest(QObject *parent = 0);
    ~KQOAuthRequest();

    enum RequestType {
        TemporaryCredentials = 0,
        AccessToken,
        AuthorizedRequest
    };

    enum RequestSignatureMethod {
        PLAINTEXT = 0,
        HMAC_SHA1
    };
#undef DELETE
    enum RequestHttpMethod {
        GET = 0,
        POST,
        HEAD,
        DELETE
    };

    /**
     * These methods can be overridden in child classes which are different types of
     * OAuth requests.
     */
    // Validate the request of this type.
    virtual bool isValid() const;

    /**
     * These methods are OAuth request type specific and not overridden in child
     * classes.
     * NOTE: Refactorting still a TODO
     */
    // Initialize the request of this type.
    void initRequest(KQOAuthRequest::RequestType type, const QUrl &requestEndpoint);

    void setConsumerKey(const QString &consumerKey);
    void setConsumerSecretKey(const QString &consumerSecretKey);

    // Mandatory methods for acquiring a request token
    void setCallbackUrl(const QUrl &callbackUrl);

    // Mandator methods for acquiring a access token
    void setTokenSecret(const QString &tokenSecret);
    void setToken(const QString &token);
    void setVerifier(const QString &verifier);

    // Request signature method to use - HMAC_SHA1 currently only supported
    void setSignatureMethod(KQOAuthRequest::RequestSignatureMethod = KQOAuthRequest::HMAC_SHA1);

    // Request's HTTP method.
    void setHttpMethod(KQOAuthRequest::RequestHttpMethod = KQOAuthRequest::POST);
    KQOAuthRequest::RequestHttpMethod httpMethod() const;

    // Sets the timeout for this request. If the timeout expires, the signal "requestTimedout" will be
    // emitted.  The KQOAuthManager will then call the abort() function from QNetworkReply associated with this request
    // 0 = If set to zero, timeout is disabled.
    // TODO: Do we need some request ID now?
    void setTimeout(int timeoutMilliseconds);

    // Additional optional parameters to the request.
    void setAdditionalParameters(const KQOAuthParameters &additionalParams);
    KQOAuthParameters additionalParameters() const;
    QList<QByteArray> requestParameters();  // This will return all request's parameters in the raw format given
                                            // to the QNetworkRequest.
    QByteArray requestBody() const;         // This will return the POST body as given to the QNetworkRequest.

    KQOAuthRequest::RequestType requestType() const;
    QUrl requestEndpoint() const;

    void setContentType(const QString &contentType);
    QString contentType();

    void setRawData(const QByteArray &rawData);
    QByteArray rawData();

    void setHttpMultiPart(QHttpMultiPart* httpMultiPart);
    QHttpMultiPart* httpMultiPart();

    void clearRequest();

    // Enable verbose debug output for request content.
    void setEnableDebugOutput(bool enabled);

Q_SIGNALS:
    // This signal is emited if the request is not completed before the request's timeout
    // value has expired.
    void requestTimedout();

protected:
    bool validateXAuthRequest() const;

private:
    KQOAuthRequestPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(KQOAuthRequest);
    Q_DISABLE_COPY(KQOAuthRequest);

    // These classes are only for the internal use of KQOAuthManager so it can
    // work with the opaque request.
    QString consumerKeyForManager() const;
    QString consumerKeySecretForManager() const;
    KQOAuthRequest::RequestSignatureMethod requestSignatureMethodForManager() const;
    QUrl callbackUrlForManager() const;

    // This method is for timeout handling by the KQOAuthManager.
    void requestTimerStart();
    void requestTimerStop();

    friend class KQOAuthManager;
#ifdef UNIT_TEST
    friend class Ut_KQOAuth;
#endif
};

#endif // KQOAUTHREQUEST_H
