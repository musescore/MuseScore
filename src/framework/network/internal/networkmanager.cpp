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
#include "networkmanager.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QEventLoop>
#include <QUrl>

#include "log.h"
#include "networkerrors.h"

using namespace mu;
using namespace mu::network;
using namespace mu::framework;
using namespace mu::io;

static constexpr int NET_TIMEOUT_MS = 60000;

NetworkManager::NetworkManager(QObject* parent)
    : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
}

NetworkManager::~NetworkManager()
{
    if (m_reply) {
        m_reply->abort();
    }
}

Ret NetworkManager::get(const QUrl& url, IncomingDevice* incommingData, const RequestHeaders& headers)
{
    return execRequest(GET_REQUEST, url, incommingData, nullptr, headers);
}

Ret NetworkManager::head(const QUrl& url, const RequestHeaders& headers)
{
    return execRequest(HEAD_REQUEST, url, nullptr, nullptr, headers);
}

Ret NetworkManager::post(const QUrl& url, OutgoingDevice* outgoingData, IncomingDevice* incommingData, const RequestHeaders& headers)
{
    return execRequest(POST_REQUEST, url, incommingData, outgoingData, headers);
}

Ret NetworkManager::put(const QUrl& url, OutgoingDevice* outgoingData, IncomingDevice* incommingData, const RequestHeaders& headers)
{
    return execRequest(PUT_REQUEST, url, incommingData, outgoingData, headers);
}

Ret NetworkManager::del(const QUrl& url, IncomingDevice* incommingData, const RequestHeaders& headers)
{
    return execRequest(DELETE_REQUEST, url, incommingData, nullptr, headers);
}

Ret NetworkManager::execRequest(RequestType requestType, const QUrl& url, IncomingDevice* incommingData, OutgoingDevice* outgoingData,
                                const RequestHeaders& headers)
{
    if (outgoingData && outgoingData->device()) {
        if (!openDevice(outgoingData->device(), Device::ReadOnly)) {
            return make_ret(Err::FiledOpenIODeviceRead);
        }
    }

    if (incommingData) {
        if (!openDevice(incommingData, Device::WriteOnly)) {
            return make_ret(Err::FiledOpenIODeviceWrite);
        }
        m_incommingData = incommingData;
    }

    QNetworkRequest request(url);

    for (QNetworkRequest::KnownHeaders knownHeader: headers.knownHeaders.keys()) {
        request.setHeader(knownHeader, headers.knownHeaders[knownHeader]);
    }

    for (const QByteArray& rawHeader: headers.rawHeaders.keys()) {
        request.setRawHeader(rawHeader, headers.rawHeaders[rawHeader]);
    }

    QNetworkReply* reply = receiveReply(requestType, request, outgoingData);

    if (outgoingData) {
        prepareReplyTransmit(reply);
    }

    if (incommingData) {
        prepareReplyReceive(reply, m_incommingData);
    }

    Ret ret = waitForReplyFinished(reply, NET_TIMEOUT_MS);
    if (!ret) {
        LOGE() << ret.toString();
    }

    if (outgoingData && outgoingData->device()) {
        closeDevice(outgoingData->device());
    }

    closeDevice(m_incommingData);
    m_incommingData = nullptr;

    return ret;
}

QNetworkReply* NetworkManager::receiveReply(RequestType requestType, const QNetworkRequest& request, OutgoingDevice* outgoingData)
{
    switch (requestType) {
    case GET_REQUEST: return m_manager->get(request);
    case HEAD_REQUEST: return m_manager->head(request);
    case DELETE_REQUEST: return m_manager->deleteResource(request);
    case PUT_REQUEST: {
        if (outgoingData->device()) {
            return m_manager->put(request, outgoingData->device());
        } else if (outgoingData->multiPart()) {
            return m_manager->put(request, outgoingData->multiPart());
        }
        break;
    }
    case POST_REQUEST: {
        if (outgoingData->device()) {
            return m_manager->post(request, outgoingData->device());
        } else if (outgoingData->multiPart()) {
            return m_manager->post(request, outgoingData->multiPart());
        }
        break;
    }
    }

    return nullptr;
}

ProgressChannel NetworkManager::progressChannel() const
{
    return m_progressCh;
}

void NetworkManager::abort()
{
    if (m_reply) {
        m_reply->abort();
    }
    m_isAborted = true;
    emit aborted();
}

bool NetworkManager::openDevice(Device* device, Device::OpenModeFlag flags)
{
    IF_ASSERT_FAILED(device) {
        return false;
    }

    if (device->isOpen()) {
        device->close();
    }

    return device->open(flags);
}

void NetworkManager::closeDevice(Device* device)
{
    if (device && device->isOpen()) {
        device->close();
    }
}

bool NetworkManager::isAborted() const
{
    return m_isAborted;
}

void NetworkManager::prepareReplyReceive(QNetworkReply* reply, IncomingDevice* incommingData)
{
    if (incommingData) {
        connect(reply, &QNetworkReply::downloadProgress, this, [this](const qint64 curr, const qint64 total) {
            m_progressCh.send(Progress(curr, total));
        });

        connect(reply, &QNetworkReply::readyRead, this, [this]() {
            IF_ASSERT_FAILED(m_incommingData) {
                return;
            }

            QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
            IF_ASSERT_FAILED(reply) {
                return;
            }

            m_incommingData->write(reply->readAll());
        });
    }
}

void NetworkManager::prepareReplyTransmit(QNetworkReply* reply)
{
    connect(reply, &QNetworkReply::uploadProgress, [this](const qint64 curr, const qint64 total) {
        m_progressCh.send(Progress(curr, total));
    });
}

Ret NetworkManager::waitForReplyFinished(QNetworkReply* reply, int timeoutMs)
{
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);

    bool isTimeout = false;
    connect(&timeoutTimer, &QTimer::timeout, this, [this, &isTimeout]() {
        isTimeout = true;
        abort();
    });

    auto restartTimeoutTimer = [&timeoutTimer, &isTimeout](qint64, qint64) {
        if (!isTimeout) {
            timeoutTimer.start();
        }
    };

    connect(reply, &QNetworkReply::downloadProgress, this, restartTimeoutTimer);
    connect(reply, &QNetworkReply::uploadProgress, this, restartTimeoutTimer);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    m_reply = reply;
    timeoutTimer.start(timeoutMs);
    loop.exec();
    m_reply = nullptr;

    if (isTimeout) {
        return make_ret(Err::Timeout);
    }

    if (isAborted()) {
        return make_ret(Err::Abort);
    }

    if (reply) {
        return errorFromReply(reply->error());
    }

    return errorFromReply(QNetworkReply::ServiceUnavailableError);
}

Ret NetworkManager::errorFromReply(int err)
{
    if (err == QNetworkReply::NoError) {
        return make_ret(Err::NoError);
    }
    if (err >= QNetworkReply::ContentAccessDenied && err <= QNetworkReply::UnknownContentError) {
        return make_ret(Err::NoError);
    }

    switch (err) {
    case QNetworkReply::HostNotFoundError:
        return make_ret(Err::HostNotFound);
    case QNetworkReply::RemoteHostClosedError:
        return make_ret(Err::HostClosed);
    }

    return make_ret(Err::NetworkError);
}
