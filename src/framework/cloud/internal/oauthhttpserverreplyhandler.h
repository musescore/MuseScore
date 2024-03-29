/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MU_CLOUD_OAUTHHTTPSERVERREPLYHANDLER_H
#define MU_CLOUD_OAUTHHTTPSERVERREPLYHANDLER_H

#include <QOAuthOobReplyHandler>

#include <QHostAddress>

class QUrlQuery;

namespace mu::cloud {
class OAuthHttpServerReplyHandler : public QOAuthOobReplyHandler
{
    Q_OBJECT

public:
    explicit OAuthHttpServerReplyHandler(QObject* parent = nullptr);
    explicit OAuthHttpServerReplyHandler(quint16 port, QObject* parent = nullptr);
    explicit OAuthHttpServerReplyHandler(const QHostAddress& address, quint16 port, QObject* parent = nullptr);
    ~OAuthHttpServerReplyHandler();

    QString callback() const override;

    QString callbackPath() const;
    void setCallbackPath(const QString& path);

    quint16 port() const;

    bool listen(const QHostAddress& address = QHostAddress::Any, quint16 port = 0);
    void close();
    bool isListening() const;

    void setRedirectUrl(const QUrl& url);

private:
    class Impl;

    std::unique_ptr<Impl> m_impl;
};
}

#endif // MU_CLOUD_OAUTHHTTPSERVERREPLYHANDLER_H
