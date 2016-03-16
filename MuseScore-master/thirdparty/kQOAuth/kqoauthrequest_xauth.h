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
#ifndef KQOAUTHREQUEST_XAUTH_H
#define KQOAUTHREQUEST_XAUTH_H

#include "kqoauthrequest.h"

class KQOAuthRequest_XAuthPrivate;
class KQOAuthRequest_XAuth : public KQOAuthRequest
{
    Q_OBJECT
public:
    KQOAuthRequest_XAuth(QObject *parent = 0);

    /**
     * These methods can be overridden in child classes which are different types of
     * OAuth requests.
     */
    // Validate the request of this type.
    bool isValid() const;

    // Give the xAuth specific parameters.
    void setXAuthLogin(const QString &username = "",
                       const QString &password = "");

private:
    KQOAuthRequest_XAuthPrivate * const d_ptr;
    bool xauth_parameters_set;
};

#endif // KQOAUTHREQUEST_XAUTH_H
