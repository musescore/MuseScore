/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "networkmanager.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

#include "networkerrors.h"

using namespace muse;
using namespace muse::network;
using namespace muse::async;

static bool openDevice(QIODevicePtr& device, QIODevice::OpenModeFlag flags)
{
    IF_ASSERT_FAILED(device) {
        return false;
    }

    if (device->isOpen()) {
        device->close();
    }

    return device->open(flags);
}

static void closeDevice(QIODevicePtr& device)
{
    if (device && device->isOpen()) {
        device->close();
    }
}

static Ret retFromReply(const QNetworkReply* reply)
{
    if (!reply) {
        return make_ret(Err::NetworkError);
    }

    Ret ret = muse::make_ok();

    if (reply->error() == QNetworkReply::TimeoutError) {
        ret = make_ret(Err::Timeout);
    } else if (reply->error() == QNetworkReply::OperationCanceledError) {
        ret = make_ret(Err::Abort);
    } else if (reply->error() != QNetworkReply::NoError) {
        ret.setCode(static_cast<int>(Err::NetworkError));
    }

    QString errorString = reply->errorString();
    if (!errorString.isEmpty()) {
        ret.setText(errorString.toStdString());
    }

    QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (status.isValid()) {
        ret.setData("status", status.toInt());
    }

    return ret;
}

NetworkManager::NetworkManager(QObject* parent)
    : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
    m_manager->setTransferTimeout(); // Use Qt's default timeout (30s)
}

RetVal<Progress> NetworkManager::get(const QUrl& url, IncomingDevicePtr incomingData, const RequestHeaders& headers)
{
    return execRequest(GET_REQUEST, url, incomingData, NoOutgoingDevice(), headers);
}

RetVal<Progress> NetworkManager::head(const QUrl& url, const RequestHeaders& headers)
{
    return execRequest(HEAD_REQUEST, url, nullptr, NoOutgoingDevice(), headers);
}

RetVal<Progress> NetworkManager::post(const QUrl& url, OutgoingDeviceVar outgoingData, IncomingDevicePtr incomingData,
                                      const RequestHeaders& headers)
{
    return execRequest(POST_REQUEST, url, incomingData, outgoingData, headers);
}

RetVal<Progress> NetworkManager::put(const QUrl& url, OutgoingDeviceVar outgoingData, IncomingDevicePtr incomingData,
                                     const RequestHeaders& headers)
{
    return execRequest(PUT_REQUEST, url, incomingData, outgoingData, headers);
}

RetVal<Progress> NetworkManager::patch(const QUrl& url, OutgoingDeviceVar outgoingData, IncomingDevicePtr incomingData,
                                       const RequestHeaders& headers)
{
    return execRequest(PATCH_REQUEST, url, incomingData, outgoingData, headers);
}

RetVal<Progress> NetworkManager::del(const QUrl& url, IncomingDevicePtr incomingData, const RequestHeaders& headers)
{
    return execRequest(DELETE_REQUEST, url, incomingData, NoOutgoingDevice(), headers);
}

RetVal<Progress> NetworkManager::execRequest(RequestType requestType, const QUrl& url,
                                             IncomingDevicePtr incomingData,
                                             OutgoingDeviceVar outgoingData,
                                             const RequestHeaders& headers)
{
    if (std::holds_alternative<QIODevicePtr>(outgoingData)) {
        if (!openDevice(std::get<QIODevicePtr>(outgoingData), QIODevice::ReadOnly)) {
            return RetVal<Progress>::make_ret((int)Err::FiledOpenIODeviceRead);
        }
    }

    if (incomingData) {
        if (!openDevice(incomingData, QIODevice::WriteOnly)) {
            return RetVal<Progress>::make_ret((int)Err::FiledOpenIODeviceWrite);
        }
    }

    const QNetworkRequest request = prepareRequest(url, headers);
    QNetworkReply* reply = sendRequest(requestType, request, outgoingData);
    IF_ASSERT_FAILED(reply) {
        return RetVal<Progress>::make_ret((int)Err::NetworkError);
    }

    size_t requestId = 0;
    if (!m_requestDataMap.empty()) {
        requestId = m_requestDataMap.rbegin()->first + 1;
    }

    RequestData& requestData = m_requestDataMap[requestId];
    requestData.incomingData = incomingData;
    requestData.outgoingData = outgoingData;
    requestData.reply = reply;
    requestData.progress.start();
    requestData.progress.canceled().onNotify(this, [this, requestId]() {
        m_requestDataMap[requestId].reply->abort();
    });

    if (!std::holds_alternative<NoOutgoingDevice>(outgoingData)) {
        connect(reply, &QNetworkReply::uploadProgress, this, [this, requestId](qint64 curr, qint64 total) {
            m_requestDataMap[requestId].progress.progress(curr, total);
        });
    }

    if (incomingData) {
        connect(reply, &QNetworkReply::downloadProgress, this, [this, requestId](qint64 curr, qint64 total) {
            m_requestDataMap[requestId].progress.progress(curr, total);
        });

        connect(reply, &QNetworkReply::readyRead, this, [this, requestId]() {
            RequestData& data = m_requestDataMap[requestId];
            data.incomingData->write(data.reply->readAll());
        });
    }

    connect(reply, &QNetworkReply::finished, this, [this, requestId]() {
        auto it = m_requestDataMap.find(requestId);
        IF_ASSERT_FAILED(it != m_requestDataMap.end()) {
            return;
        }

        RequestData& data = it->second;
        data.reply->disconnect();
        data.progress.canceled().disconnect(this);

        if (std::holds_alternative<QIODevicePtr>(data.outgoingData)) {
            closeDevice(std::get<QIODevicePtr>(data.outgoingData));
        }

        closeDevice(data.incomingData);

        const Ret ret = retFromReply(data.reply);
        Progress progress = data.progress;
        m_requestDataMap.erase(it);
        progress.finish(ret);
    });

    return RetVal<Progress>::make_ok(requestData.progress);
}

QNetworkRequest NetworkManager::prepareRequest(const QUrl& url, const RequestHeaders& headers) const
{
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, true);

    RequestHeaders _headers = headers;
    if (_headers.isEmpty()) {
        _headers = configuration()->defaultHeaders();
    }

    for (auto it = _headers.knownHeaders.cbegin(); it != _headers.knownHeaders.cend(); ++it) {
        request.setHeader(it.key(), it.value());
    }

    for (auto it = _headers.rawHeaders.cbegin(); it != _headers.rawHeaders.cend(); ++it) {
        request.setRawHeader(it.key(), it.value());
    }

    return request;
}

QNetworkReply* NetworkManager::sendRequest(RequestType type, const QNetworkRequest& request, const OutgoingDeviceVar& device)
{
    switch (type) {
    case GET_REQUEST: return m_manager->get(request);
    case HEAD_REQUEST: return m_manager->head(request);
    case DELETE_REQUEST: return m_manager->deleteResource(request);
    case PUT_REQUEST: {
        if (std::holds_alternative<QIODevicePtr>(device)) {
            return m_manager->put(request, std::get<QIODevicePtr>(device).get());
        } else if (std::holds_alternative<QHttpMultiPartPtr>(device)) {
            return m_manager->put(request, std::get<QHttpMultiPartPtr>(device).get());
        }
        break;
    }
    case PATCH_REQUEST: {
        if (std::holds_alternative<QIODevicePtr>(device)) {
            return m_manager->sendCustomRequest(request, "PATCH", std::get<QIODevicePtr>(device).get());
        } else if (std::holds_alternative<QHttpMultiPartPtr>(device)) {
            return m_manager->sendCustomRequest(request, "PATCH", std::get<QHttpMultiPartPtr>(device).get());
        }
        break;
    }
    case POST_REQUEST: {
        if (std::holds_alternative<QIODevicePtr>(device)) {
            return m_manager->post(request, std::get<QIODevicePtr>(device).get());
        } else if (std::holds_alternative<QHttpMultiPartPtr>(device)) {
            return m_manager->post(request, std::get<QHttpMultiPartPtr>(device).get());
        }
        break;
    }
    }

    UNREACHABLE;
    return nullptr;
}
