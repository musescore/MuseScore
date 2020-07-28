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
class QNetworkReply;

namespace mu {
namespace framework {
class NetworkManager : public QObject, public INetworkManager
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject* parent = nullptr);
    virtual ~NetworkManager() override;

    Ret get(const QUrl& url, QIODevice* incommingData) override;

    async::Channel<Progress> downloadProgressChannel() const override;

    void abort() override;

signals:
    void aborted();

private:
    bool openIoDevice(QIODevice* device, QIODevice::OpenModeFlag flags);
    void closeIoDevice(QIODevice* device);

    bool isAborted() const;

    void prepareReplyReceive(QNetworkReply* reply, QIODevice* incommingData);

    Ret execRequest(QNetworkReply* reply);
    Ret waitForReplyFinished(QNetworkReply* reply, int timeoutMs);
    Ret errorFromReply(int err);

private:
    QNetworkAccessManager* m_manager = nullptr;
    QIODevice* m_incommingData = nullptr;
    QNetworkReply* m_reply = nullptr;
    async::Channel<Progress> m_downloadProgressCh;

    bool m_isAborted = false;
};
}
}

#endif // MU_FRAMEWORK_NETWORKMANAGER_H
