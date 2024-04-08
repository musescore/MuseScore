/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MUSE_NETWORK_NETWORKMANAGER_H
#define MUSE_NETWORK_NETWORKMANAGER_H

#include "inetworkmanager.h"

#include <QIODevice>

class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;

namespace muse::network {
class NetworkManager : public QObject, public INetworkManager
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject* parent = nullptr);
    ~NetworkManager() override;

    Ret get(const QUrl& url, IncomingDevice* incomingData, const RequestHeaders& headers = RequestHeaders()) override;
    Ret head(const QUrl& url, const RequestHeaders& headers = RequestHeaders()) override;
    Ret post(const QUrl& url, OutgoingDevice* outgoingData, IncomingDevice* incomingData,
             const RequestHeaders& headers = RequestHeaders()) override;
    Ret put(const QUrl& url, OutgoingDevice* outgoingData, IncomingDevice* incomingData,
            const RequestHeaders& headers = RequestHeaders()) override;
    Ret patch(const QUrl& url, OutgoingDevice* outgoingData, IncomingDevice* incomingData,
              const RequestHeaders& headers = RequestHeaders()) override;
    Ret del(const QUrl& url, IncomingDevice* incomingData, const RequestHeaders& headers = RequestHeaders()) override;

    Progress progress() const override;

    void abort() override;

private:
    enum RequestType {
        GET_REQUEST,
        HEAD_REQUEST,
        POST_REQUEST,
        PUT_REQUEST,
        PATCH_REQUEST,
        DELETE_REQUEST
    };

    Ret execRequest(RequestType requestType, const QUrl& url, IncomingDevice* incomingData = nullptr,
                    OutgoingDevice* outgoingData = nullptr, const RequestHeaders& headers = RequestHeaders());

    QNetworkReply* receiveReply(RequestType requestType, const QNetworkRequest& request, OutgoingDevice* outgoingData = nullptr);

    bool openDevice(QIODevice* device, QIODevice::OpenModeFlag flags);
    void closeDevice(QIODevice* device);

    bool isAborted() const;

    void prepareReplyReceive(QNetworkReply* reply, IncomingDevice* incomingData);
    void prepareReplyTransmit(QNetworkReply* reply);

    Ret waitForReplyFinished(QNetworkReply* reply, int timeoutMs);
    Ret errorFromReply(const QNetworkReply* reply) const;

private:
    QNetworkAccessManager* m_manager = nullptr;
    IncomingDevice* m_incomingData = nullptr;
    QNetworkReply* m_reply = nullptr;
    Progress m_progress;

    bool m_isAborted = false;
};
}

#endif // MUSE_NETWORK_NETWORKMANAGER_H
