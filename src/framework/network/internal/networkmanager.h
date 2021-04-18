//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
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

    Ret get(const QUrl& url, system::IODevice* incommingData) override;
    Ret head(const QUrl& url) override;
    Ret post(const QUrl& url, system::IODevice* outgoingData, system::IODevice* incommingData) override;
    Ret put(const QUrl& url, system::IODevice* outgoingData, system::IODevice* incommingData) override;
    Ret del(const QUrl& url, system::IODevice* incommingData) override;

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

    Ret execRequest(RequestType requestType, const QUrl& url, system::IODevice* incommingData = nullptr,
                    system::IODevice* outgoingData = nullptr);
    QNetworkReply* receiveReply(RequestType requestType, const QNetworkRequest& request, system::IODevice* outgoingData = nullptr);

    bool openIoDevice(system::IODevice* device, QIODevice::OpenModeFlag flags);
    void closeIoDevice(system::IODevice* device);

    bool isAborted() const;

    void prepareReplyReceive(QNetworkReply* reply, system::IODevice* incommingData);
    void prepareReplyTransmit(QNetworkReply* reply);

    Ret waitForReplyFinished(QNetworkReply* reply, int timeoutMs);
    Ret errorFromReply(int err);

private:
    QNetworkAccessManager* m_manager = nullptr;
    system::IODevice* m_incommingData = nullptr;
    QNetworkReply* m_reply = nullptr;
    framework::ProgressChannel m_progressCh;

    bool m_isAborted = false;
};
}

#endif // MU_NETWORK_NETWORKMANAGER_H
