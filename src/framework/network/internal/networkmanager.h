/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#pragma once

#include <QObject>

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "network/inetworkconfiguration.h"
#include "network/inetworkmanager.h"

class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;

namespace muse::network {
class NetworkManager : public QObject, public INetworkManager, public Injectable, public async::Asyncable
{
    Q_OBJECT

    Inject<INetworkConfiguration> configuration = { this };

public:
    explicit NetworkManager(QObject* parent = nullptr);

    RetVal<Progress> get(const QUrl& url, IncomingDevicePtr incomingData, const RequestHeaders& headers = RequestHeaders()) override;
    RetVal<Progress> head(const QUrl& url, const RequestHeaders& headers = RequestHeaders()) override;
    RetVal<Progress> post(const QUrl& url, OutgoingDeviceVar outgoingData, IncomingDevicePtr incomingData,
                          const RequestHeaders& headers = RequestHeaders()) override;
    RetVal<Progress> put(const QUrl& url, OutgoingDeviceVar outgoingData, IncomingDevicePtr incomingData,
                         const RequestHeaders& headers = RequestHeaders()) override;
    RetVal<Progress> patch(const QUrl& url, OutgoingDeviceVar outgoingData, IncomingDevicePtr incomingData,
                           const RequestHeaders& headers = RequestHeaders()) override;
    RetVal<Progress> del(const QUrl& url, IncomingDevicePtr incomingData, const RequestHeaders& headers = RequestHeaders()) override;

private:
    enum RequestType {
        GET_REQUEST,
        HEAD_REQUEST,
        POST_REQUEST,
        PUT_REQUEST,
        PATCH_REQUEST,
        DELETE_REQUEST
    };

    RetVal<Progress> execRequest(RequestType requestType, const QUrl& url, IncomingDevicePtr incomingData, OutgoingDeviceVar outgoingData,
                                 const RequestHeaders& headers);

    QNetworkRequest prepareRequest(const QUrl& url, const RequestHeaders& headers) const;
    QNetworkReply* sendRequest(RequestType type, const QNetworkRequest& request, const OutgoingDeviceVar& device);

    struct RequestData {
        IncomingDevicePtr incomingData;
        OutgoingDeviceVar outgoingData;
        QNetworkReply* reply = nullptr;
        Progress progress;
    };

    QNetworkAccessManager* m_manager = nullptr;
    std::map<size_t /*requestId*/, RequestData> m_requestDataMap;
};
}
