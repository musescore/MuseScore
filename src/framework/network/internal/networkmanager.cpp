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

using namespace muse;
using namespace muse::network;

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

Ret NetworkManager::get(const QUrl& url, IncomingDevice* incomingData, const RequestHeaders& headers)
{
    return execRequest(GET_REQUEST, url, incomingData, nullptr, headers);
}

Ret NetworkManager::head(const QUrl& url, const RequestHeaders& headers)
{
    return execRequest(HEAD_REQUEST, url, nullptr, nullptr, headers);
}

Ret NetworkManager::post(const QUrl& url, OutgoingDevice* outgoingData, IncomingDevice* incomingData, const RequestHeaders& headers)
{
    return execRequest(POST_REQUEST, url, incomingData, outgoingData, headers);
}

Ret NetworkManager::put(const QUrl& url, OutgoingDevice* outgoingData, IncomingDevice* incomingData, const RequestHeaders& headers)
{
    return execRequest(PUT_REQUEST, url, incomingData, outgoingData, headers);
}

Ret NetworkManager::patch(const QUrl& url, OutgoingDevice* outgoingData, IncomingDevice* incomingData, const RequestHeaders& headers)
{
    return execRequest(PATCH_REQUEST, url, incomingData, outgoingData, headers);
}

Ret NetworkManager::del(const QUrl& url, IncomingDevice* incomingData, const RequestHeaders& headers)
{
    return execRequest(DELETE_REQUEST, url, incomingData, nullptr, headers);
}

Ret NetworkManager::execRequest(RequestType requestType, const QUrl& url, IncomingDevice* incomingData, OutgoingDevice* outgoingData,
                                const RequestHeaders& headers)
{
    if (outgoingData && outgoingData->device()) {
        if (!openDevice(outgoingData->device(), QIODevice::ReadOnly)) {
            return make_ret(Err::FiledOpenIODeviceRead);
        }
    }

    if (incomingData) {
        if (!openDevice(incomingData, QIODevice::WriteOnly)) {
            return make_ret(Err::FiledOpenIODeviceWrite);
        }
        m_incomingData = incomingData;
    }

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, true);

    for (QNetworkRequest::KnownHeaders knownHeader: headers.knownHeaders.keys()) {
        request.setHeader(knownHeader, headers.knownHeaders[knownHeader]);
    }

    for (const QByteArray& rawHeader: headers.rawHeaders.keys()) {
        request.setRawHeader(rawHeader, headers.rawHeaders[rawHeader]);
    }

    m_progress.start();

    QNetworkReply* reply = receiveReply(requestType, request, outgoingData);

    if (outgoingData) {
        prepareReplyTransmit(reply);
    }

    if (incomingData) {
        prepareReplyReceive(reply, m_incomingData);
    }

    Ret ret = waitForReplyFinished(reply, NET_TIMEOUT_MS);
    if (!ret) {
        LOGE() << ret.toString();
    }

    m_progress.finish(ret);

    if (outgoingData && outgoingData->device()) {
        closeDevice(outgoingData->device());
    }

    closeDevice(m_incomingData);
    m_incomingData = nullptr;

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
    case PATCH_REQUEST: {
        if (outgoingData->device()) {
            return m_manager->sendCustomRequest(request, "PATCH", outgoingData->device());
        } else if (outgoingData->multiPart()) {
            return m_manager->sendCustomRequest(request, "PATCH", outgoingData->multiPart());
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

Progress NetworkManager::progress() const
{
    return m_progress;
}

void NetworkManager::abort()
{
    if (m_reply) {
        m_reply->abort();
    }

    m_isAborted = true;
    m_progress.finish(make_ret(Err::Abort));
}

bool NetworkManager::openDevice(QIODevice* device, QIODevice::OpenModeFlag flags)
{
    IF_ASSERT_FAILED(device) {
        return false;
    }

    if (device->isOpen()) {
        device->close();
    }

    return device->open(flags);
}

void NetworkManager::closeDevice(QIODevice* device)
{
    if (device && device->isOpen()) {
        device->close();
    }
}

bool NetworkManager::isAborted() const
{
    return m_isAborted;
}

void NetworkManager::prepareReplyReceive(QNetworkReply* reply, IncomingDevice* incomingData)
{
    if (incomingData) {
        connect(reply, &QNetworkReply::downloadProgress, this, [this](const qint64 curr, const qint64 total) {
            m_progress.progress(curr, total, "");
        });

        connect(reply, &QNetworkReply::readyRead, this, [this]() {
            IF_ASSERT_FAILED(m_incomingData) {
                return;
            }

            QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
            IF_ASSERT_FAILED(reply) {
                return;
            }

            m_incomingData->write(reply->readAll());
        });
    }
}

void NetworkManager::prepareReplyTransmit(QNetworkReply* reply)
{
    connect(reply, &QNetworkReply::uploadProgress, [this](const qint64 curr, const qint64 total) {
        m_progress.progress(curr, total, "");
    });
}

Ret NetworkManager::waitForReplyFinished(QNetworkReply* reply, int timeoutMs)
{
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    m_isAborted = false;

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

    return errorFromReply(reply);
}

Ret NetworkManager::errorFromReply(const QNetworkReply* reply) const
{
    if (!reply) {
        return make_ret(Err::NetworkError);
    }

    Ret ret = muse::make_ok();

    if (reply->error() != QNetworkReply::NoError) {
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
