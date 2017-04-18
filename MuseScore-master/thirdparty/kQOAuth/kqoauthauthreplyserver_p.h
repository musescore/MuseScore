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

// Note this class shouldn't be copied or used and the implementation might change later.
#ifndef KQOAUTHAUTHREPLYSERVER_P_H
#define KQOAUTHAUTHREPLYSERVER_P_H

#include "kqoauthauthreplyserver.h"
#include <QMultiMap>
#include <QString>

class KQOAuthAuthReplyServerPrivate: public QObject
{
    Q_OBJECT
public:
    KQOAuthAuthReplyServerPrivate( KQOAuthAuthReplyServer * parent );
    ~KQOAuthAuthReplyServerPrivate();
    QMultiMap<QString, QString> parseQueryParams(QByteArray *sdata);

public Q_SLOTS:
    void onIncomingConnection();
    void onBytesReady();

public:
    KQOAuthAuthReplyServer * q_ptr;
    Q_DECLARE_PUBLIC(KQOAuthAuthReplyServer);
    QTcpSocket *socket;

};

#endif // KQOAUTHAUTHREPLYSERVER_P_H
