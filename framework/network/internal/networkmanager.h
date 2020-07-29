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
#ifndef MU_FRAMEWORK_NETWORKMANAGER_H
#define MU_FRAMEWORK_NETWORKMANAGER_H

#include "inetworkmanager.h"

class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;

namespace mu {
namespace framework {
class NetworkManager : public QObject, public INetworkManager
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject* parent = nullptr);
    ~NetworkManager() override;

    Ret get(const QUrl& url, QIODevice* incommingData) override;
    Ret head(const QUrl &url) override;
    Ret post(const QUrl &url, QIODevice* outgoingData, QIODevice* incommingData) override;
    Ret put(const QUrl &url, QIODevice* outgoingData, QIODevice* incommingData) override;
    Ret del(const QUrl &url, QIODevice* incommingData) override;

    async::Channel<Progress> downloadProgressChannel() const override;
    async::Channel<Progress> uploadProgressChannel() const override;

    void abort() override;

signals:
    void aborted();

private:
    enum class RequestType {
        GET,
        HEAD,
        POST,
        PUT,
        DELETE
    };

    Ret execRequest(RequestType requestType, const QUrl& url, QIODevice* incommingData = nullptr, QIODevice* outgoingData = nullptr);
    QNetworkReply* receiveReply(RequestType requestType, const QNetworkRequest& request, QIODevice* outgoingData = nullptr);

    bool openIoDevice(QIODevice* device, QIODevice::OpenModeFlag flags);
    void closeIoDevice(QIODevice* device);

    bool isAborted() const;

    void prepareReplyReceive(QNetworkReply* reply, QIODevice* incommingData);
    void prepareReplyTransmit(QNetworkReply* reply);

    Ret waitForReplyFinished(QNetworkReply* reply, int timeoutMs);
    Ret errorFromReply(int err);

private:
    QNetworkAccessManager* m_manager = nullptr;
    QIODevice* m_incommingData = nullptr;
    QNetworkReply* m_reply = nullptr;
    async::Channel<Progress> m_downloadProgressCh;
    async::Channel<Progress> m_uploadProgressCh;

    bool m_isAborted = false;
};
}
}

#endif // MU_FRAMEWORK_NETWORKMANAGER_H
