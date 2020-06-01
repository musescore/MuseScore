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

#include "avsomrlocalinstaller.h"

#include <QtConcurrent>
#include <QFutureWatcher>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QBuffer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "thirdparty/qzip/qzipreader_p.h"

#include "avslog.h"

namespace {
static const QString AVS_RELEASE_LATEST_API("https://api.github.com/repos/musescore/omr-local/releases/latest");

#ifdef Q_OS_WIN

static const QString RELEASE_PREFIX("_windows_");

#elif defined(Q_OS_MAC)

static const QString RELEASE_PREFIX("_macos_");

#else

static const QString RELEASE_PREFIX("_linux_");

#endif
}

using namespace Ms::Avs;

AvsOmrLocalInstaller::AvsOmrLocalInstaller(const QString& avsHomePath)
    : _avsHomePath(avsHomePath)
{
    _loop = new QEventLoop();
}

//---------------------------------------------------------
//   installBackground
//---------------------------------------------------------

void AvsOmrLocalInstaller::installBackground()
{
    IF_ASSERT(!_watcher) {
        LOGE() << "already run installing";
        return;
    }

    const ReleaseInfo& info = loadReleaseInfo();
    if (!info.isValid()) {
        LOGW() << "failed load release info";
        return;
    }

    _watcher = new QFutureWatcher<bool>();
    QObject::connect(_watcher, &QFutureWatcher<bool>::finished, [this]() {
        if (_loop->isRunning()) {
            _loop->quit();
        }

        _watcher->deleteLater();
        _watcher = nullptr;
    });

    QFuture<bool> future = QtConcurrent::run(this, &AvsOmrLocalInstaller::doInstallAvs, info.url);
    _watcher->setFuture(future);
}

//---------------------------------------------------------
//   waitForFinished
//---------------------------------------------------------

void AvsOmrLocalInstaller::waitForFinished()
{
    if (!_watcher) {
        return;
    }

    _loop->exec();
}

//---------------------------------------------------------
//   doInstallAvs
//---------------------------------------------------------

bool AvsOmrLocalInstaller::doInstallAvs(const QString& url)
{
    LOGI() << "try load avs, url: " << url;

    QByteArray avsZipPack;
    bool ok = getData(&avsZipPack, url, "application/zip");
    if (!ok) {
        LOGE() << "failed load avs, url: " << url;
        return false;
    }
    LOGI() << "success loaded avs, url: " << url;

    ok = unpackAvs(&avsZipPack, _avsHomePath);
    if (!ok) {
        LOGE() << "failed unpack avs, path: " << _avsHomePath;
        return false;
    }
    LOGI() << "success unpack avs, path: " << _avsHomePath;

    return true;
}

//---------------------------------------------------------
//   loadReleaseInfo
//---------------------------------------------------------

const AvsOmrLocalInstaller::ReleaseInfo& AvsOmrLocalInstaller::loadReleaseInfo() const
{
    if (_info.isValid()) {
        return _info;
    }

    doLoadReleaseInfo(&_info, AVS_RELEASE_LATEST_API);
    return _info;
}

//---------------------------------------------------------
//   doLoadReleaseInfo
//---------------------------------------------------------

bool AvsOmrLocalInstaller::doLoadReleaseInfo(ReleaseInfo* info, const QString& url) const
{
    IF_ASSERT(info) {
        return false;
    }

    QByteArray json_data;
    bool ok = getData(&json_data, url, "application/json");
    if (!ok) {
        LOGE() << "failed get data, url: " << url;
        return false;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(json_data, &err);
    if (err.error != QJsonParseError::NoError) {
        LOGE() << "failed parse json, err: " << err.errorString() << ", json: " << json_data;
        return false;
    }

    QJsonObject infoObj = doc.object();
    info->tag = infoObj.value("tag_name").toString();
    QJsonArray assetsArr = infoObj.value("assets").toArray();
    for (const auto& av : assetsArr) {
        QJsonObject ao = av.toObject();
        QString name = ao.value("name").toString();
        if (!name.contains(RELEASE_PREFIX)) {
            continue;
        }

        info->url = ao.value("browser_download_url").toString();
        break;
    }

    if (info->tag.isEmpty() || info->url.isEmpty()) {
        return false;
    }

    return true;
}

namespace {
//---------------------------------------------------------
//   execReq
//---------------------------------------------------------

static QNetworkReply* execReq(QNetworkAccessManager& net, QNetworkRequest req)
{
    QNetworkReply* reply = net.get(req);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status >= 300 && status < 399) {   // redirect
        QString newurl = reply->rawHeader("Location");
        req.setUrl(newurl);
        return execReq(net, req);     // recursion
    }

    return reply;
}
}

//---------------------------------------------------------
//   getData
//---------------------------------------------------------

bool AvsOmrLocalInstaller::getData(QByteArray* data, const QString& url, const QByteArray& mime) const
{
    QNetworkRequest req;
    req.setUrl(QUrl(url));
    req.setRawHeader("accept", mime);
    QNetworkAccessManager net;

    QNetworkReply* reply = execReq(net, req);
    if (reply->error() != QNetworkReply::NoError) {
        LOGE() << "reply error: " << reply->errorString();
        reply->close();
        return false;
    }

    if (data) {
        *data = reply->readAll();
    }

    reply->close();
    return true;
}

//---------------------------------------------------------
//   unpackAvs
//---------------------------------------------------------

bool AvsOmrLocalInstaller::unpackAvs(QByteArray* avsZipPack, const QString& path)
{
    bool ok = cleanDir(path);
    if (!ok) {
        LOGE() << "failed clean avs dir: " << path;
        return false;
    }

    QBuffer buf(avsZipPack);
    MQZipReader zip(&buf);

    ok = zip.extractAll(path);
    if (!ok) {
        LOGE() << "failed unpack avs, path: " << path;
        return false;
    }

    return true;
}

//---------------------------------------------------------
//   cleanDir
//---------------------------------------------------------

bool AvsOmrLocalInstaller::cleanDir(const QString& dirPath)
{
    return QDir(dirPath).removeRecursively();
}
