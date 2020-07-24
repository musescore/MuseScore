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
#include "networkmanager.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QEventLoop>
#include <QUrl>
#include <QIODevice>

#include "log.h"
#include "networkerrors.h"

using namespace mu;
using namespace mu::framework;

static const int TIMEOUT_MS = 60000;

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

    delete m_manager;
}

Ret NetworkManager::get(const QUrl& url, QIODevice* incommingData)
{
    if (incommingData) {
        if (!openIoDevice(incommingData, QIODevice::WriteOnly)) {
            return make_ret(Err::FiledOpenIODeviceWrite);
        }
        m_incommingData = incommingData;
    }

    QNetworkRequest request(url);
    QNetworkReply* reply = m_manager->get(request);
    prepareReplyReceive(reply, m_incommingData);
    Ret ret = execRequest(reply);

    closeIoDevice(m_incommingData);
    m_incommingData = nullptr;

    return ret;
}

async::Channel<Progress> NetworkManager::downloadProgressChannel() const
{
    return m_downloadProgressCh;
}

void NetworkManager::abort()
{
    if (m_reply) {
        m_reply->abort();
    }

    m_isAborted = true;
    emit aborted();
}

bool NetworkManager::openIoDevice(QIODevice* device, QIODevice::OpenModeFlag flags)
{
    IF_ASSERT_FAILED(device) {
        return false;
    }

    if (device->isOpen()) {
        device->close();
    }

    return device->open(flags);
}

void NetworkManager::closeIoDevice(QIODevice* device)
{
    if (device && device->isOpen()) {
        device->close();
    }
}

bool NetworkManager::isAborted() const
{
    return m_isAborted;
}

void NetworkManager::prepareReplyReceive(QNetworkReply* reply, QIODevice* incommingData)
{
    if (incommingData) {
        connect(reply, &QNetworkReply::downloadProgress, this, [this](const qint64 curr, const qint64 total) {
            m_downloadProgressCh.send(Progress(curr, total));
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

Ret NetworkManager::execRequest(QNetworkReply* reply)
{
    Ret ret = waitForReplyFinished(reply, TIMEOUT_MS);
    if (!ret) {
        LOGE() << "Error exec request, error" << ret.code() << ret.text();
    }

    return ret;
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
