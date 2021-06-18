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
#ifndef MU_NETWORK_NETWORKMANAGER_H
#define MU_NETWORK_NETWORKMANAGER_H

#include "inetworkmanager.h"

class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;

namespace mu::network {
class NetworkManager : public QObject, public INetworkManager
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject* parent = nullptr);
    ~NetworkManager() override;

    Ret get(const QUrl& url, IncomingDevice* incommingData, const RequestHeaders& headers = RequestHeaders()) override;
    Ret head(const QUrl& url, const RequestHeaders& headers = RequestHeaders()) override;
    Ret post(const QUrl& url, OutgoingDevice* outgoingData, IncomingDevice* incommingData,
             const RequestHeaders& headers = RequestHeaders()) override;
    Ret put(const QUrl& url, OutgoingDevice* outgoingData, IncomingDevice* incommingData,
            const RequestHeaders& headers = RequestHeaders()) override;
    Ret del(const QUrl& url, IncomingDevice* incommingData, const RequestHeaders& headers = RequestHeaders()) override;

    framework::ProgressChannel progressChannel() const override;

    void abort() override;

signals:
    void aborted();

private:
    enum RequestType {
        GET_REQUEST,
        HEAD_REQUEST,
        POST_REQUEST,
        PUT_REQUEST,
        DELETE_REQUEST
    };

    Ret execRequest(RequestType requestType, const QUrl& url, IncomingDevice* incommingData = nullptr,
                    OutgoingDevice* outgoingData = nullptr, const RequestHeaders& headers = RequestHeaders());

    QNetworkReply* receiveReply(RequestType requestType, const QNetworkRequest& request, OutgoingDevice* outgoingData = nullptr);

    bool openDevice(io::Device* device, QIODevice::OpenModeFlag flags);
    void closeDevice(io::Device* device);

    bool isAborted() const;

    void prepareReplyReceive(QNetworkReply* reply, IncomingDevice* incommingData);
    void prepareReplyTransmit(QNetworkReply* reply);

    Ret waitForReplyFinished(QNetworkReply* reply, int timeoutMs);
    Ret errorFromReply(int err);

private:
    QNetworkAccessManager* m_manager = nullptr;
    IncomingDevice* m_incommingData = nullptr;
    QNetworkReply* m_reply = nullptr;
    framework::ProgressChannel m_progressCh;

    bool m_isAborted = false;
};
}

#endif // MU_NETWORK_NETWORKMANAGER_H
