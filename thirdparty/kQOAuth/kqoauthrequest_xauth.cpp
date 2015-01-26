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
#include <QtDebug>

#include "kqoauthrequest_xauth_p.h"
#include "kqoauthrequest_xauth.h"

/**
 * Private d_ptr implementations.
 */
KQOAuthRequest_XAuthPrivate::KQOAuthRequest_XAuthPrivate()
{

}

KQOAuthRequest_XAuthPrivate::~KQOAuthRequest_XAuthPrivate()
{
}

/**
 * Public implementations.
 */
KQOAuthRequest_XAuth::KQOAuthRequest_XAuth(QObject *parent) :
        KQOAuthRequest(parent),
        d_ptr(new KQOAuthRequest_XAuthPrivate)
{
}

bool KQOAuthRequest_XAuth::isValid() const {
    // An xAuth can never request temporary credentials.
    if (requestType() == KQOAuthRequest::TemporaryCredentials) {
        qWarning() << "XAuth request cannot be of type KQOAuthRequest::TemporaryCredentials. Aborting.";
        return false;
    }

    // Access token must always be retrieved using the POST HTTP method.
    if (requestType() == KQOAuthRequest::AccessToken
        && httpMethod() != KQOAuthRequest::POST) {

        qWarning() << "Access tokens must be fetched using the POST HTTP method. Aborting.";

        return false;
    }

    if (!xauth_parameters_set) {
        qWarning() << "No XAuth parameters set. Aborting.";
        return false;
    }

    // And then check the validity of the XAuth request.
    // Provided by the base class as a protected method for us.
    return validateXAuthRequest();
}

void KQOAuthRequest_XAuth::setXAuthLogin(const QString &username,
                                         const QString &password) {

    if (username.isEmpty() || password.isEmpty()) {
        qWarning() << "Username or password cannot be empty. Aborting.";
        return;
    }

    xauth_parameters_set = true;

    KQOAuthParameters xauthParams;
    xauthParams.insert("x_auth_username", username);
    xauthParams.insert("x_auth_password", password);
    xauthParams.insert("x_auth_mode", "client_auth");

    setAdditionalParameters(xauthParams);
}

