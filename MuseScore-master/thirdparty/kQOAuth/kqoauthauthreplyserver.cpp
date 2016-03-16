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
#include <QTcpSocket>
#include <QStringList>
#include <QUrl>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

#include "kqoauthauthreplyserver.h"
#include "kqoauthauthreplyserver_p.h"

KQOAuthAuthReplyServerPrivate::KQOAuthAuthReplyServerPrivate(KQOAuthAuthReplyServer *parent):
    q_ptr(parent)
{

}

KQOAuthAuthReplyServerPrivate::~KQOAuthAuthReplyServerPrivate()
{

}

void KQOAuthAuthReplyServerPrivate::onIncomingConnection() {
    Q_Q(KQOAuthAuthReplyServer);

    socket = q->nextPendingConnection();
    connect(socket, SIGNAL(readyRead()),
            this, SLOT(onBytesReady()), Qt::UniqueConnection);
}

void KQOAuthAuthReplyServerPrivate::onBytesReady() {
    Q_Q(KQOAuthAuthReplyServer);
    
    QByteArray reply;
    QByteArray content;
    content.append("<HTML></HTML>");

    reply.append("HTTP/1.0 200 OK \r\n");
    reply.append("Content-Type: text/html; charset=\"utf-8\"\r\n");
    reply.append(QString("Content-Length: %1\r\n").arg(content.size()));
    reply.append("\r\n");
    reply.append(content);
    socket->write(reply);
    
    QByteArray data = socket->readAll();
    QMultiMap<QString, QString> queryParams = parseQueryParams(&data);

    socket->disconnectFromHost();
    q->close();
    emit q->verificationReceived(queryParams);
}

QMultiMap<QString, QString> KQOAuthAuthReplyServerPrivate::parseQueryParams(QByteArray *data) {
    QString splitGetLine = QString(*data).split("\r\n").first();   // Retrieve the first line with query params.
    splitGetLine.remove("GET ");                                   // Clean the line from GET
    splitGetLine.remove("HTTP/1.1");                               // From HTTP
    splitGetLine.remove("\r\n");                                   // And from rest.
    splitGetLine.prepend("http://localhost");                      // Now, make it a URL

    QUrl getTokenUrl(splitGetLine);
#if QT_VERSION < 0x050000
    QList< QPair<QString, QString> > tokens = getTokenUrl.queryItems();  // Ask QUrl to do our work.
#else
    QList< QPair<QString, QString> > tokens = QUrlQuery(getTokenUrl.query()).queryItems();  // Ask QUrl to do our work.
#endif

    QMultiMap<QString, QString> queryParams;
    QPair<QString, QString> tokenPair;
    foreach (tokenPair, tokens) {
        queryParams.insert(tokenPair.first.trimmed(), tokenPair.second.trimmed());
    }

    return queryParams;
}



KQOAuthAuthReplyServer::KQOAuthAuthReplyServer(QObject *parent) :
    QTcpServer(parent),
    d_ptr( new KQOAuthAuthReplyServerPrivate(this) )
{
    Q_D(KQOAuthAuthReplyServer);

    connect(this, SIGNAL(newConnection()),
            d, SLOT(onIncomingConnection()));
}

KQOAuthAuthReplyServer::~KQOAuthAuthReplyServer()
{
    delete d_ptr;
}


