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
#ifndef KQOAUTHAUTHREPLYSERVER_H
#define KQOAUTHAUTHREPLYSERVER_H

#include <QTcpServer>

#include "kqoauthglobals.h"

class KQOAuthAuthReplyServerPrivate;
class KQOAuthAuthReplyServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit KQOAuthAuthReplyServer(QObject *parent);
    ~KQOAuthAuthReplyServer();

Q_SIGNALS:
    void verificationReceived(QMultiMap<QString, QString>);


private:
    KQOAuthAuthReplyServerPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(KQOAuthAuthReplyServer);
    Q_DISABLE_COPY(KQOAuthAuthReplyServer);


};

#endif // KQOAUTHAUTHREPLYSERVER_H
