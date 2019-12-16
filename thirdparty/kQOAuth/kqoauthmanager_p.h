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
#ifndef KQOAUTHMANAGER_P_H
#define KQOAUTHMANAGER_P_H

#include "kqoauthauthreplyserver.h"
#include "kqoauthrequest.h"

class KQOAuthManagerPrivate {

public:
    KQOAuthManagerPrivate(KQOAuthManager *parent);
    ~KQOAuthManagerPrivate();

    QList< QPair<QString, QString> > createQueryParams(const KQOAuthParameters &requestParams);
    QMultiMap<QString, QString> createTokensFromResponse(QByteArray reply);
    bool setSuccessfulRequestToken(const QMultiMap<QString, QString> &request);
    bool setSuccessfulAuthorized(const QMultiMap<QString, QString> &request);
    void emitTokens();
    bool setupCallbackServer();

    KQOAuthManager::KQOAuthError error;
    KQOAuthRequest *r;                  // This request is used to cache the user sent request.
    KQOAuthRequest *opaqueRequest;       // This request is used to creating opaque convenience requests for the user.
    KQOAuthManager * const q_ptr;

    /**
     * The items below are needed in order to store the state of the manager and
     * by that be able to do convenience operations for the user.
     */
    KQOAuthRequest::RequestType currentRequestType;

    // Variables we store here for opaque request handling.
    // NOTE: The variables are labeled the same for both access token request
    //       and protected resource access.
    QString requestToken;
    QString requestTokenSecret;
    QString consumerKey;
    QString consumerKeySecret;
    QString requestVerifier;
    KQOAuthRequest::RequestSignatureMethod signatureMethod;

    KQOAuthAuthReplyServer *callbackServer;

    bool hasTemporaryToken;
    bool isVerified;
    bool isAuthorized;
    bool autoAuth;
    bool handleAuthPageOpening;
    QNetworkAccessManager *networkManager;
    bool managerUserSet;
    QMap<QNetworkReply*, int> requestIds;

    QMap<KQOAuthRequest*, QNetworkReply*> requestMap;

    Q_DECLARE_PUBLIC(KQOAuthManager);
};

#endif // KQOAUTHMANAGER_P_H
