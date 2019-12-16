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
#ifndef KQOAUTHMANAGER_H
#define KQOAUTHMANAGER_H

#include <QObject>
#include <QMultiMap>
#include <QNetworkReply>

#include "kqoauthrequest.h"

class KQOAuthRequest;
class KQOAuthManagerThread;
class KQOAuthManagerPrivate;
class QNetworkAccessManager;
class QUrl;
class QByteArray;
class KQOAuthManager : public QObject
{
    Q_OBJECT
public:

    enum KQOAuthError {
        NoError,                    // No error
        NetworkError,               // Network error: timeout, cannot connect.
        RequestEndpointError,       // Request endpoint is not valid.
        RequestValidationError,     // Request is not valid: some parameter missing?
        RequestUnauthorized,        // Authorization error: trying to access a resource without tokens.
        ContentOperationNotPermittedError,// Authentication is required, key or auth is invalid
        ContentNotFound,            // Content is not found
        RequestError,               // The given request to KQOAuthManager is invalid: NULL?,
        ManagerError                // Manager error, cannot use for sending requests.
    };

    explicit KQOAuthManager(QObject *parent = 0);
    ~KQOAuthManager();

    KQOAuthError lastError();

    /**
     * The manager executes the given request. It takes the HTTP parameters from the
     * request and uses QNetworkAccessManager to submit the HTTP request to the net.
     * When the request is done it will emit signal requestReady(QByteArray networkReply).
     * NOTE: At the moment there is no timeout for the request.
     */
    void executeRequest(KQOAuthRequest *request);    
    void executeAuthorizedRequest(KQOAuthRequest *request, int id);
    /**
     * Indicates to the user that KQOAuthManager should handle user authorization by
     * opening the user's default browser and parsing the reply from the service.
     * By setting the parameter to true, KQOAuthManager will store intermediate results
     * of the OAuth 1.0 process in its own opaque request. This information is used in
     * the user authorization process and also when calling sendAuthorizedRequest().
     * NOTE: You need to set this to true if you want to use getUserAccessTokens() or
     *       sendAuthorizedRequest().
     */
    void setHandleUserAuthorization(bool set);

    /** Indicates whether the KQOAuthManager should launch the browser with the user
     * authorization page itself.
     *
     * If set to true (the default), the KQOAuthManager uses QDesktopServices::openUrl()
     * for opening the browser. Otherwise it emits the authorizationPageRequested()
     * signal which must then be handled by the calling code.
     */
    void setHandleAuthorizationPageOpening(bool set);

    /**
     * Returns true if the KQOAuthManager has retrieved the oauth_token value. Otherwise
     * return false.
     */
    bool hasTemporaryToken();
    /**
     * Returns true if the user has authorized us to use the protected resources. Otherwise
     * returns false.
     * NOTE: In order for KQOAuthManager to know if the user has authorized us to use the
     *       protected resources, KQOAuthManager must be in control of the user authorization
     *       process. Hence, this returns true if setHandleUserAuthorization() is set to true
     *       and the user is authorized with getUserAuthorization().
     */
    bool isVerified();
    /**
     * Returns true if KQOAuthManager has the access token and hence can access the protected
     * resources. Otherwise returns false.
     * NOTE: In order for KQOAuthManager to know if we have access to protected resource
     *       KQOAuthManager must be in control of the user authorization process and requesting
     *       the acess token. Hence, this returns true if setHandleUserAuthorization() is set to true
     *       and the user is authorized with getUserAuthorization() and the access token must be retrieved
     *       with getUserAccessTokens.
     */
    bool isAuthorized();

    /**
     * This is a convenience API for authorizing the user.
     * The call will open the user's default browser, setup a local HTTP server and parse the reply from the
     * service after the user has authorized us to access protected resources. If the user authorizes
     * us to access protected resources, the verifier token is stored in KQOAuthManager for further use.
     * In order to use this method, you must set setHandleUserAuthorization() to true.
     */
    void getUserAuthorization(QUrl authorizationEndpoint);
    /**
     * This is a convenience API for retrieving the access token in exchange for the temporary token and the
     * verifier.
     * This call will create a KQOAuthRequest and use the previously stored temporary token and verifier to
     * exchange for the access token, which will be used to access the protected resources.
     * Note that in order to use this method, KQOAuthManager must be in control of the user authorization process.
     * Set setHandleUserAuthorization() to true and retrieve user authorization with void getUserAuthorization.
     */
    void getUserAccessTokens(QUrl accessTokenEndpoint);
    /**
     * This is a conveience API for setting the token verifier.
     * If setHandleUserAuthorization() is set to false you need to call this function before calling
     * getUserAccessTokens()
     */
    void verifyToken(const QString &token, const QString &verifier);
    /**
     * Sends a request to the protected resources. Parameters for the request are service specific and
     * are given to the 'requestParameters' as parameters.
     * Note that in order to use this method, KQOAuthManager must be in control of the user authorization process.
     * Set setHandleUserAuthorization() to true and retrieve user authorization with void getUserAuthorization.
     */
    void sendAuthorizedRequest(QUrl requestEndpoint, const KQOAuthParameters &requestParameters);

    /**
     * Sets a custom QNetworkAccessManager to handle network requests. This method can be useful if the
     * application is using some proxy settings for example.
     * The application is responsible for deleting this manager. KQOAuthManager will not delete any
     * previously given manager.
     * If the manager is NULL, the manager will not be set and the KQOAuthManager::Error.
     * If no manager is given, KQOAuthManager will use the default one it will create by itself.
     */
    void setNetworkManager(QNetworkAccessManager *manager);

    /**
     * Returns the given QNetworkAccessManager. Returns NULL if none is given.
     */
    QNetworkAccessManager* networkManager() const;

Q_SIGNALS:
    // This signal will be emitted after each request has got a reply.
    // Parameter is the raw response from the service.
    void requestReady(QByteArray networkReply);

    void authorizedRequestReady(QByteArray networkReply, int id);

    // This signal will be emitted when the authorization page should be opened if
    // setHandleAuthorizationPageOpening() is set to false.
    void authorizationPageRequested(QUrl pageUrl);

    // This signal will be emited when we have an request tokens available
    // (either temporary resource tokens, or authorization tokens).
    void receivedToken(QString oauth_token, QString oauth_token_secret);   // oauth_token, oauth_token_secret

    // This signal is emited when temporary tokens are returned from the service.
    // Note that this signal is also emited in case temporary tokens are not available.
    void temporaryTokenReceived(QString oauth_token, QString oauth_token_secret);   // oauth_token, oauth_token_secret

    // This signal is emited when the user has authenticated the application to
    // communicate with the protected resources. Next we need to exchange the
    // temporary tokens for access tokens.
    // Note that this signal is also emited if user denies access.
    void authorizationReceived(QString oauth_token, QString oauth_verifier); // oauth_token, oauth_verifier

    // This signal is emited when access tokens are received from the service. We are
    // ready to start communicating with the protected resources.
    void accessTokenReceived(QString oauth_token, QString oauth_token_secret);  // oauth_token, oauth_token_secret

    // This signal is emited when the authorized request is done.
    // This ends the kQOAuth interactions.
    void authorizedRequestDone();

private Q_SLOTS:
    void onRequestReplyReceived();
    void onAuthorizedRequestReplyReceived();
    void onVerificationReceived(QMultiMap<QString, QString> response);
    void slotError(QNetworkReply::NetworkError error);
    void requestTimeout();

private:
    KQOAuthManagerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(KQOAuthManager);
    Q_DISABLE_COPY(KQOAuthManager);

};

#endif // KQOAUTHMANAGER_H
