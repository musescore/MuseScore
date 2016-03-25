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
#include <QtCore>
#include <QDesktopServices>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif
#include <QNetworkCookie>

#include "kqoauthmanager.h"
#include "kqoauthmanager_p.h"


////////////// Private d_ptr implementation ////////////////

KQOAuthManagerPrivate::KQOAuthManagerPrivate(KQOAuthManager *parent) :
    error(KQOAuthManager::NoError) ,
    r(0) ,
    opaqueRequest(new KQOAuthRequest) ,
    q_ptr(parent) ,
    callbackServer(new KQOAuthAuthReplyServer(parent)) ,
    isVerified(false) ,
    isAuthorized(false) ,
    autoAuth(false),
    handleAuthPageOpening(true),
    networkManager(new QNetworkAccessManager),
    managerUserSet(false)
{

}

KQOAuthManagerPrivate::~KQOAuthManagerPrivate() {
    delete opaqueRequest;
    opaqueRequest = 0;

    if (!managerUserSet) {
        delete networkManager;
        networkManager = 0;
    }
}

QList< QPair<QString, QString> > KQOAuthManagerPrivate::createQueryParams(const KQOAuthParameters &requestParams) {
    QList<QString> requestKeys = requestParams.keys();
    QList<QString> requestValues = requestParams.values();

    QList< QPair<QString, QString> > result;
    for(int i=0; i<requestKeys.size(); i++) {
        result.append( qMakePair(requestKeys.at(i),
                                 requestValues.at(i))
                      );
    }

    return result;
}

QMultiMap<QString, QString> KQOAuthManagerPrivate::createTokensFromResponse(QByteArray reply) {
    QMultiMap<QString, QString> result;
    QString replyString(reply);

    QStringList parameterPairs = replyString.split('&', QString::SkipEmptyParts);
    foreach (const QString &parameterPair, parameterPairs) {
        QStringList parameter = parameterPair.split('=');
        result.insert(parameter.value(0), parameter.value(1));
    }

    return result;
}

bool KQOAuthManagerPrivate::setSuccessfulRequestToken(const QMultiMap<QString, QString> &request) {
    if (currentRequestType == KQOAuthRequest::TemporaryCredentials) {
        hasTemporaryToken = (!QString(request.value("oauth_token")).isEmpty() && !QString(request.value("oauth_token_secret")).isEmpty());
    } else {
        return false;
    }

    if (hasTemporaryToken) {
        requestToken = QUrl::fromPercentEncoding( QString(request.value("oauth_token")).toLocal8Bit() );
        requestTokenSecret =  QUrl::fromPercentEncoding( QString(request.value("oauth_token_secret")).toLocal8Bit() );
    }

    return hasTemporaryToken;
}

bool KQOAuthManagerPrivate::setSuccessfulAuthorized(const QMultiMap<QString, QString> &request ) {
    if (currentRequestType == KQOAuthRequest::AccessToken) {
        isAuthorized = (!QString(request.value("oauth_token")).isEmpty() && !QString(request.value("oauth_token_secret")).isEmpty());
    } else {
        return false;
    }

    if (isAuthorized) {
        requestToken = QUrl::fromPercentEncoding( QString(request.value("oauth_token")).toLocal8Bit() );
        requestTokenSecret =  QUrl::fromPercentEncoding( QString(request.value("oauth_token_secret")).toLocal8Bit() );
    }

    return isAuthorized;
}

void KQOAuthManagerPrivate::emitTokens() {
    Q_Q(KQOAuthManager);

    if (this->requestToken.isEmpty() || this->requestTokenSecret.isEmpty()) {
        error = KQOAuthManager::RequestUnauthorized;
    }

    if (currentRequestType == KQOAuthRequest::TemporaryCredentials) {
        // Signal that we are ready to use the protected resources.
        emit q->temporaryTokenReceived(this->requestToken, this->requestTokenSecret);
    }

    if (currentRequestType == KQOAuthRequest::AccessToken) {
        // Signal that we are ready to use the protected resources.
        emit q->accessTokenReceived(this->requestToken, this->requestTokenSecret);
    }

    emit q->receivedToken(this->requestToken, this->requestTokenSecret);
}

bool KQOAuthManagerPrivate::setupCallbackServer() {
    return callbackServer->listen();
}


/////////////// Public implementation ////////////////

KQOAuthManager::KQOAuthManager(QObject *parent) :
    QObject(parent) ,
    d_ptr(new KQOAuthManagerPrivate(this))
{

    qsrand(QTime::currentTime().msec());  // We need to seed the nonce random number with something.
                                          // However, we cannot do this while generating the nonce since
                                          // we might get the same seed. So initializing here should be fine.
}

KQOAuthManager::~KQOAuthManager()
{
    delete d_ptr;
}

void KQOAuthManager::executeRequest(KQOAuthRequest *request) {
    Q_D(KQOAuthManager);

    d->r = request;

    if (request == 0) {
        qWarning() << "Request is NULL. Cannot proceed.";
        d->error = KQOAuthManager::RequestError;
        return;
    }

    if (!request->requestEndpoint().isValid()) {
        qWarning() << "Request endpoint URL is not valid. Cannot proceed.";
        d->error = KQOAuthManager::RequestEndpointError;
        return;
    }

    if (!request->isValid()) {
        qWarning() << "Request is not valid. Cannot proceed.";
        d->error = KQOAuthManager::RequestValidationError;
        return;
    }

    d->currentRequestType = request->requestType();

    QNetworkRequest networkRequest;
    networkRequest.setUrl( request->requestEndpoint() );

    if (d->autoAuth && d->currentRequestType == KQOAuthRequest::TemporaryCredentials) {
        d->setupCallbackServer();
        connect(d->callbackServer, SIGNAL(verificationReceived(QMultiMap<QString, QString>)),
                this, SLOT( onVerificationReceived(QMultiMap<QString, QString>)));

        QString serverString = "http://localhost:";
        serverString.append(QString::number(d->callbackServer->serverPort()));
        request->setCallbackUrl(QUrl(serverString));
    }

    // And now fill the request with "Authorization" header data.
    QList<QByteArray> requestHeaders = request->requestParameters();
    QByteArray authHeader;

    bool first = true;
    foreach (const QByteArray header, requestHeaders) {
        if (!first) {
            authHeader.append(", ");
        } else {
            authHeader.append("OAuth ");
            first = false;
        }

        authHeader.append(header);
    }
    networkRequest.setRawHeader("Authorization", authHeader);

    if (request->httpMethod() == KQOAuthRequest::GET) {
        // Get the requested additional params as a list of pairs we can give QUrl
        QList< QPair<QString, QString> > urlParams = d->createQueryParams(request->additionalParameters());

        // Take the original URL and append the query params to it.
        QUrl urlWithParams = networkRequest.url();
#if QT_VERSION < 0x050000
        urlWithParams.setQueryItems(urlParams);
#else
        QUrlQuery query;
        query.setQueryItems(urlParams);
        urlWithParams.setQuery(query);
#endif
        networkRequest.setUrl(urlWithParams);

        // Submit the request including the params.
        QNetworkReply *reply = d->networkManager->get(networkRequest);
        connect(reply, SIGNAL(finished()),
            this, SLOT(onRequestReplyReceived()));
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                 this, SLOT(slotError(QNetworkReply::NetworkError)));
        d->requestMap.insert( request, reply );

    } else if (request->httpMethod() == KQOAuthRequest::POST) {
        QNetworkReply *reply;
        if (request->contentType() == "application/x-www-form-urlencoded") {
          networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, request->contentType());
          reply = d->networkManager->post(networkRequest, request->requestBody());
        } else if (request->contentType() == "multipart/form-data") {
          reply = d->networkManager->post(networkRequest, request->httpMultiPart());
        } else {
          networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, request->contentType());
          reply = d->networkManager->post(networkRequest, request->rawData());
        }
        connect(reply, SIGNAL(finished()),
            this, SLOT(onRequestReplyReceived()), Qt::UniqueConnection);
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                 this, SLOT(slotError(QNetworkReply::NetworkError)));
        d->requestMap.insert( request, reply );
    }

    d->r->requestTimerStart();
}

void KQOAuthManager::executeAuthorizedRequest(KQOAuthRequest *request, int id) {
    Q_D(KQOAuthManager);

    d->r = request;

    if (request == 0) {
        qWarning() << "Request is NULL. Cannot proceed.";
        d->error = KQOAuthManager::RequestError;
        return;
    }

    if (!request->requestEndpoint().isValid()) {
        qWarning() << "Request endpoint URL is not valid. Cannot proceed.";
        d->error = KQOAuthManager::RequestEndpointError;
        return;
    }

    if (!request->isValid()) {
        qWarning() << "Request is not valid. Cannot proceed.";
        d->error = KQOAuthManager::RequestValidationError;
        return;
    }

    d->currentRequestType = request->requestType();

    QNetworkRequest networkRequest;
    networkRequest.setUrl( request->requestEndpoint() );

    if ( d->currentRequestType != KQOAuthRequest::AuthorizedRequest){
        qWarning() << "Not Authorized Request. Cannot proceed";
        d->error = KQOAuthManager::RequestError;
        return;
    }


    // And now fill the request with "Authorization" header data.
    QList<QByteArray> requestHeaders = request->requestParameters();
    QByteArray authHeader;

    bool first = true;
    foreach (const QByteArray header, requestHeaders) {
        if (!first) {
            authHeader.append(", ");
        } else {
            authHeader.append("OAuth ");
            first = false;
        }

        authHeader.append(header);
    }
    networkRequest.setRawHeader("Authorization", authHeader);

    QNetworkReply *reply = 0;

    if (request->httpMethod() == KQOAuthRequest::POST) {

        networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, request->contentType());

        /*
        qDebug() << networkRequest.rawHeaderList();
        qDebug() << networkRequest.rawHeader("Authorization");
        qDebug() << networkRequest.rawHeader("Content-Type");
        */

        if (request->contentType() == "application/x-www-form-urlencoded") {
          reply = d->networkManager->post(networkRequest, request->requestBody());
        } else if (request->contentType() == "multipart/form-data") {
          reply = d->networkManager->post(networkRequest, request->httpMultiPart());
        } else {
          reply = d->networkManager->post(networkRequest, request->rawData());
        }
        connect(reply, SIGNAL(finished(QNetworkReply *)),
            this, SLOT(onAuthorizedRequestReplyReceived(QNetworkReply*)), Qt::UniqueConnection);
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                 this, SLOT(slotError(QNetworkReply::NetworkError)));
        connect(request, SIGNAL(requestTimedout()),
                 this, SLOT(requestTimeout()));
        d->requestMap.insert( request, reply );
    } else {
        // Get the requested additional params as a list of pairs we can give QUrl
        QList< QPair<QString, QString> > urlParams = d->createQueryParams(request->additionalParameters());

        // Take the original URL and append the query params to it.
        QUrl urlWithParams = networkRequest.url();
#if QT_VERSION < 0x050000
        urlWithParams.setQueryItems(urlParams);
#else
        QUrlQuery query;
        query.setQueryItems(urlParams);
        urlWithParams.setQuery(query);
#endif
        networkRequest.setUrl(urlWithParams);

        // Submit the request including the params.
        if (request->httpMethod() == KQOAuthRequest::GET)
            reply = d->networkManager->get(networkRequest);
        else if (request->httpMethod() == KQOAuthRequest::HEAD)
            reply = d->networkManager->head(networkRequest);
        else if (request->httpMethod() == KQOAuthRequest::DELETE)
            reply = d->networkManager->deleteResource(networkRequest);

        connect(reply, SIGNAL(finished()),
            this, SLOT(onAuthorizedRequestReplyReceived()));
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                 this, SLOT(slotError(QNetworkReply::NetworkError)));
        connect(request, SIGNAL(requestTimedout()),
                 this, SLOT(requestTimeout()));
        d->requestMap.insert( request, reply );
    }
    d->requestIds.insert(reply, id);
    d->r->requestTimerStart();
}


void KQOAuthManager::setHandleUserAuthorization(bool set) {
    Q_D(KQOAuthManager);

    d->autoAuth = set;
}

void KQOAuthManager::setHandleAuthorizationPageOpening(bool set) {
    Q_D(KQOAuthManager);

    d->handleAuthPageOpening = set;
}

bool KQOAuthManager::hasTemporaryToken() {
    Q_D(KQOAuthManager);

    return d->hasTemporaryToken;
}

bool KQOAuthManager::isVerified() {
    Q_D(KQOAuthManager);

    return d->isVerified;
}

bool KQOAuthManager::isAuthorized() {
    Q_D(KQOAuthManager);

    return d->isAuthorized;
}

KQOAuthManager::KQOAuthError KQOAuthManager::lastError() {
    Q_D(KQOAuthManager);

    return d->error;
}

void KQOAuthManager::setNetworkManager(QNetworkAccessManager *manager) {
    Q_D(KQOAuthManager);

    if (manager == 0) {
        d->error = KQOAuthManager::ManagerError;
        return;
    }

    if (!d->managerUserSet) {
        delete d->networkManager;
    }

    d->managerUserSet = true;
    d->networkManager = manager;
}

QNetworkAccessManager * KQOAuthManager::networkManager() const {
    Q_D(const KQOAuthManager);

    if (d->managerUserSet) {
        return d->networkManager;
    } else {
        return NULL;
    }

}


//////////// Public convenience API /////////////

void KQOAuthManager::getUserAuthorization(QUrl authorizationEndpoint) {
    Q_D(KQOAuthManager);

    if (!d->hasTemporaryToken) {
        qWarning() << "No temporary tokens retreieved. Cannot get user authorization.";
        d->error = KQOAuthManager::RequestUnauthorized;
        return;
    }

    if (!authorizationEndpoint.isValid()) {
        qWarning() << "Authorization endpoint not valid. Cannot proceed.";
        d->error = KQOAuthManager::RequestEndpointError;
        return;
    }

    d->error = KQOAuthManager::NoError;

    QPair<QString, QString> tokenParam = qMakePair(QString("oauth_token"), QString(d->requestToken));
    QUrl openWebPageUrl(authorizationEndpoint.toString(), QUrl::StrictMode);

#if QT_VERSION < 0x050000
    openWebPageUrl.addQueryItem(tokenParam.first, tokenParam.second);
#else
    QUrlQuery query(openWebPageUrl);
    query.addQueryItem(tokenParam.first, tokenParam.second);
    openWebPageUrl.setQuery(query);
#endif

    if (d->handleAuthPageOpening) {
        // Open the user's default browser to the resource authorization page provided
        // by the service.
        QDesktopServices::openUrl(openWebPageUrl);
    } else {
        emit authorizationPageRequested(openWebPageUrl);
    }
}

void KQOAuthManager::getUserAccessTokens(QUrl accessTokenEndpoint) {
    Q_D(KQOAuthManager);

    if (!d->isVerified) {
        qWarning() << "Not verified. Cannot get access tokens.";
        d->error = KQOAuthManager::RequestUnauthorized;
        return;
    }

    if (!accessTokenEndpoint.isValid()) {
        qWarning() << "Endpoint for access token exchange is not valid. Cannot proceed.";
        d->error = KQOAuthManager::RequestEndpointError;
        return;
    }

    d->error = KQOAuthManager::NoError;

    d->opaqueRequest->clearRequest();
    d->opaqueRequest->initRequest(KQOAuthRequest::AccessToken, accessTokenEndpoint);
    d->opaqueRequest->setToken(d->requestToken);
    d->opaqueRequest->setTokenSecret(d->requestTokenSecret);
    d->opaqueRequest->setVerifier(d->requestVerifier);
    d->opaqueRequest->setConsumerKey(d->consumerKey);
    d->opaqueRequest->setConsumerSecretKey(d->consumerKeySecret);
    d->opaqueRequest->setSignatureMethod(d->signatureMethod);

    executeRequest(d->opaqueRequest);
}

void KQOAuthManager::verifyToken(const QString &token, const QString &verifier) {
    QMultiMap<QString, QString> params;
    params.insert("oauth_token", token);
    params.insert("oauth_verifier", verifier);
    onVerificationReceived(params);
}

void KQOAuthManager::sendAuthorizedRequest(QUrl requestEndpoint, const KQOAuthParameters &requestParameters) {
    Q_D(KQOAuthManager);

    if (!d->isAuthorized) {
        qWarning() << "No access tokens retrieved. Cannot send authorized requests.";
        d->error = KQOAuthManager::RequestUnauthorized;
        return;
    }

    if (!requestEndpoint.isValid()) {
        qWarning() << "Endpoint for authorized request is not valid. Cannot proceed.";
        d->error = KQOAuthManager::RequestEndpointError;
        return;
    }

    d->error = KQOAuthManager::NoError;

    d->opaqueRequest->clearRequest();
    d->opaqueRequest->initRequest(KQOAuthRequest::AuthorizedRequest, requestEndpoint);
    d->opaqueRequest->setAdditionalParameters(requestParameters);
    d->opaqueRequest->setToken(d->requestToken);
    d->opaqueRequest->setTokenSecret(d->requestTokenSecret);
    d->opaqueRequest->setConsumerKey(d->consumerKey);
    d->opaqueRequest->setConsumerSecretKey(d->consumerKeySecret);

    executeRequest(d->opaqueRequest);
}


/////////////// Private slots //////////////////

void KQOAuthManager::onRequestReplyReceived() {
    Q_D(KQOAuthManager);
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    QNetworkReply::NetworkError networkError = reply->error();
    switch (networkError) {
    case QNetworkReply::NoError:
        d->error = KQOAuthManager::NoError;
        break;

    case QNetworkReply::ContentAccessDenied:
    case QNetworkReply::UnknownContentError:
    case QNetworkReply::AuthenticationRequiredError:
        d->error = KQOAuthManager::RequestUnauthorized;
        break;
    case QNetworkReply::ContentNotFoundError:
        d->error = KQOAuthManager::ContentNotFound;
        break;
    case QNetworkReply::ContentOperationNotPermittedError:
        d->error = KQOAuthManager::ContentOperationNotPermittedError;
        break;
    default:
        d->error = KQOAuthManager::NetworkError;
        break;
    }

    // Let's disconnect this slot first
    /*
    disconnect(d->networkManager, SIGNAL(finished(QNetworkReply *)),
            this, SLOT(onRequestReplyReceived(QNetworkReply *)));
    */

    // Read the content of the reply from the network.
    QByteArray networkReply = reply->readAll();

    d->r = d->requestMap.key(reply);
    if( d->r ) {
        d->requestMap.remove(d->r);
        disconnect(d->r, SIGNAL(requestTimedout()),
                this, SLOT(requestTimeout()));
        // Stop any timer we have set on the request.
        d->r->requestTimerStop();
        d->currentRequestType = d->r->requestType();
    }

    QVariant v = reply->header(QNetworkRequest::SetCookieHeader);
    
    /*QList<QNetworkCookie> c = qvariant_cast<QList<QNetworkCookie> >(v);
    foreach (QNetworkCookie cookie , c)
            qDebug() << cookie.name() << cookie.value();*/

    // We need to emit the signal even if we got an error.
    if (d->error != KQOAuthManager::NoError) {
        reply->deleteLater();
        emit requestReady(networkReply);
        //d->emitTokens();
        return;
    }

    // Just don't do anything if we didn't get anything useful.
    if(networkReply.isEmpty()) {
        reply->deleteLater();
        return;
    }
    QMultiMap<QString, QString> responseTokens;

    responseTokens = d->createTokensFromResponse(networkReply);
    d->opaqueRequest->clearRequest();
    d->opaqueRequest->setHttpMethod(KQOAuthRequest::POST);   // XXX FIXME: Convenient API does not support GET
    emit requestReady(networkReply);
    if (!d->isAuthorized || !d->isVerified) {
        if (d->setSuccessfulRequestToken(responseTokens)) {
            //qDebug() << "Successfully got request tokens.";
            d->consumerKey = d->r->consumerKeyForManager();
            d->consumerKeySecret = d->r->consumerKeySecretForManager();
            d->signatureMethod = d->r->requestSignatureMethodForManager();
            d->opaqueRequest->setSignatureMethod(KQOAuthRequest::HMAC_SHA1);
            d->opaqueRequest->setCallbackUrl(d->r->callbackUrlForManager());

            d->emitTokens();

        } else if (d->setSuccessfulAuthorized(responseTokens)) {
              //qDebug() << "Successfully got access tokens.";
              d->opaqueRequest->setSignatureMethod(KQOAuthRequest::HMAC_SHA1);
              d->emitTokens();
          } else if (d->currentRequestType == KQOAuthRequest::AuthorizedRequest) {
                emit authorizedRequestDone();
            }
    }

    reply->deleteLater();           // We need to clean this up, after the event processing is done.
}

void KQOAuthManager::onAuthorizedRequestReplyReceived() {
    Q_D(KQOAuthManager);
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    QNetworkReply::NetworkError networkError = reply->error();
    switch (networkError) {
    case QNetworkReply::NoError:
        d->error = KQOAuthManager::NoError;
        break;

    case QNetworkReply::ContentAccessDenied:
    case QNetworkReply::UnknownContentError:
    case QNetworkReply::AuthenticationRequiredError:
        d->error = KQOAuthManager::RequestUnauthorized;
        break;
    case QNetworkReply::ContentNotFoundError:
        d->error = KQOAuthManager::ContentNotFound;
        break;
    case QNetworkReply::ContentOperationNotPermittedError:
        d->error = KQOAuthManager::ContentOperationNotPermittedError;
        break;
    default:
        d->error = KQOAuthManager::NetworkError;
        break;
    }

    /*
    disconnect(d->networkManager, SIGNAL(finished(QNetworkReply *)),
            this, SLOT(onAuthorizedRequestReplyReceived(QNetworkReply *)));
    */

    // Read the content of the reply from the network.
    QByteArray networkReply = reply->readAll();

    int id = d->requestIds.take(reply);
    d->r = d->requestMap.key(reply);
    if( d->r ) {
        d->requestMap.remove(d->r);
        disconnect(d->r, SIGNAL(requestTimedout()),
                this, SLOT(requestTimeout()));

        // Stop any timer we have set on the request.
        d->r->requestTimerStop();
        d->currentRequestType = d->r->requestType();
    }

    // Just don't do anything if we didn't get anything useful.
    if(networkReply.isEmpty()) {
        reply->deleteLater();
        return;
    }

    // We need to emit the signal even if we got an error.
    if (d->error != KQOAuthManager::NoError) {
        qWarning() << "Network reply error";
        return;
    }


    d->opaqueRequest->clearRequest();
    d->opaqueRequest->setHttpMethod(KQOAuthRequest::POST);   // XXX FIXME: Convenient API does not support GET
    if (d->currentRequestType == KQOAuthRequest::AuthorizedRequest) {
                emit authorizedRequestDone();
     }

    emit authorizedRequestReady(networkReply, id);
    reply->deleteLater();
}


void KQOAuthManager::onVerificationReceived(QMultiMap<QString, QString> response) {
    Q_D(KQOAuthManager);

    QString token = response.value("oauth_token");
    QString verifier = response.value("oauth_verifier");
    if (verifier.isEmpty()) {
        d->error = KQOAuthManager::RequestUnauthorized;
    }

    verifier = QUrl::fromPercentEncoding(verifier.toUtf8());     // We get the raw URL response here so we need to convert it back
                                                                 // to plain string so we can percent encode it again later in requests.

    if (d->error == KQOAuthManager::NoError) {
        d->requestVerifier = verifier;
        d->isVerified = true;
    }

    emit authorizationReceived(token, verifier);
}

void KQOAuthManager::slotError(QNetworkReply::NetworkError error) {
    Q_UNUSED(error)
    //qDebug() << error;
    
    Q_D(KQOAuthManager);

    switch (error) {
    case QNetworkReply::NoError:
        d->error = KQOAuthManager::NoError;
        break;
    case QNetworkReply::ContentAccessDenied:
    case QNetworkReply::UnknownContentError:
    case QNetworkReply::AuthenticationRequiredError:
        d->error = KQOAuthManager::RequestUnauthorized;
        break;
    case QNetworkReply::ContentNotFoundError:
        d->error = KQOAuthManager::ContentNotFound;
        break;
    case QNetworkReply::ContentOperationNotPermittedError:
        d->error = KQOAuthManager::ContentOperationNotPermittedError;
        break;
    default:
        d->error = KQOAuthManager::NetworkError;
        break;
    }
    QByteArray emptyResponse;
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    //qDebug() << "STATUS" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    d->r = d->requestMap.key(reply);
    d->currentRequestType = d->r->requestType();
    if( d->requestIds.contains(reply) ) {
        int id = d->requestIds.value(reply);
        emit authorizedRequestReady(emptyResponse, id);
        reply->deleteLater();
    }
    else if ( d->currentRequestType == KQOAuthRequest::AuthorizedRequest) {
        // does this signal always have to be emitted if there is an error
        // or can is it only valid for KQOAuthRequest::AuthorizedRequest?
        if (error != QNetworkReply::ContentAccessDenied &&
            error != QNetworkReply::AuthenticationRequiredError)
             emit requestReady(emptyResponse);
        
        emit authorizedRequestDone();
    }
    else {
         if (error != QNetworkReply::ContentAccessDenied &&
             error!= QNetworkReply::AuthenticationRequiredError)
             emit requestReady(emptyResponse);
        }
}


void KQOAuthManager::requestTimeout() {
    Q_D(KQOAuthManager);
    KQOAuthRequest *request = qobject_cast<KQOAuthRequest *>(sender());
    if( d->requestMap.contains(request)) {
        qWarning() << "KQOAuthManager::requestTimeout: Calling abort";
        d->requestMap.value(request)->abort();
    }
    else
        qWarning() << "KQOAuthManager::requestTimeout: The KQOAuthRequest was not found";
}
