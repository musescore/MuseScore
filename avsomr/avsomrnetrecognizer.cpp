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

#include "avsomrnetrecognizer.h"

#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QHttpPart>
#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonParseError>

#include "avslog.h"

namespace {
static const QString HOST_API_OMR("https://musescore.com/api/omr");
static const int STATUS_OK{ 200 };
static const int STATUS_NOT_FOUND{ 404 };
static const int STATUS_PROCESSING{ 411 };
static const int STATUS_NOT_SUPPORTED{ 422 };

static const int CHECK_COUNT{ 1000 };
static const int CHECK_INTERVAL_MS{ 300 };
}

using namespace Ms::Avs;

AvsOmrNetRecognizer::AvsOmrNetRecognizer()
{
    _net = new QNetworkAccessManager();
}

AvsOmrNetRecognizer::~AvsOmrNetRecognizer()
{
    delete _net;
}

//---------------------------------------------------------
//   type
//---------------------------------------------------------

QString AvsOmrNetRecognizer::type() const
{
    return "cloud";
}

//---------------------------------------------------------
//   isAvailable
//---------------------------------------------------------

bool AvsOmrNetRecognizer::isAvailable() const
{
    //! TODO add allow check
    return true;
}

//---------------------------------------------------------
//   recognize
//---------------------------------------------------------

bool AvsOmrNetRecognizer::recognize(const QString& filePath, QByteArray* avsFileData, const OnStep& onStep)
{
    QString getUrl;
    LOGI() << "begin send file: " << filePath;

    auto step = [&onStep](Step::Type t, uint16_t perc, uint16_t percMax, Ret ret = Ret::Ok) {
                    if (!ret) {
                        LOGE() << "failed step: " << t << ", err: " << ret.formatedText();
                    } else {
                        LOGI() << "success step: " << t << ", err: " << ret.formatedText();
                    }

                    if (onStep) {
                        onStep(Step(t, perc, percMax, ret));
                    }
                };

    step(Step::PrepareStart, 1, 10);
    Ret ret = send(filePath, &getUrl);
    step(Step::PrepareFinish, 10, 10, ret);
    if (!ret) {
        return false;
    }

    step(Step::ProcessingStart, 11, 90);
    QString fileUrl;
    ret = check(getUrl, &fileUrl);
    step(Step::ProcessingFinish, 90, 90, ret);
    if (!ret) {
        return false;
    }

    step(Step::LoadStart, 91, 100);
    ret = load(fileUrl, avsFileData);
    step(Step::LoadFinish, 100, 100, ret);
    if (!ret) {
        return false;
    }

    return true;
}

//---------------------------------------------------------
//   send
//---------------------------------------------------------

Ret AvsOmrNetRecognizer::netRetToRet(const NetRet& nr) const
{
    if (nr.httpStatus < 100) {
        return Ret::NetworkError;
    }

    if (nr.httpStatus >= 200 && nr.httpStatus < 300) {
        return Ret::Ok;
    } else if (nr.httpStatus == STATUS_NOT_SUPPORTED) {
        return Ret::FileNotSupported;
    }

    return Ret::ServerError;
}

//---------------------------------------------------------
//   send
//---------------------------------------------------------

Ret AvsOmrNetRecognizer::send(const QString& filePath, QString* getUrl)
{
    QFile* file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        LOGE() << "failed open file: " << filePath;
        delete file;
        return Ret::FailedReadFile;
    }

    //request
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart part;
    part.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    part.setHeader(QNetworkRequest::ContentDispositionHeader,
                   QString("form-data; name=\"file\"; filename=\"%1\"").arg(QFileInfo(filePath).fileName()));

    part.setBodyDevice(file);
    file->setParent(multiPart);   // we cannot delete the file now, so delete it with the multiPart

    multiPart->append(part);

    QNetworkRequest req;
    req.setUrl(QUrl(HOST_API_OMR));
    req.setRawHeader("accept", "application/json");
    QNetworkReply* rep = _net->post(req, multiPart);
    multiPart->setParent(rep);

    QByteArray ba;
    NetRet netRet = doExecReply(rep, &ba);

    LOGI() << "send status: " << netRet.httpStatus << ", data: " << ba;

    if (netRet.httpStatus != STATUS_OK) {
        LOGE() << "failed send status: " << netRet.httpStatus << ", file: " << filePath;
        return netRetToRet(netRet);
    }

    QVariantMap info = parseInfo(ba);
    if (getUrl) {
        *getUrl = info.value("url").toString();
    }

    return Ret::Ok;
}

//---------------------------------------------------------
//   doExecReply
//---------------------------------------------------------

AvsOmrNetRecognizer::NetRet AvsOmrNetRecognizer::doExecReply(QNetworkReply* reply, QByteArray* ba)
{
    NetRet ret;
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    ret.replyError = static_cast<int>(reply->error());
    ret.httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (ba) {
        *ba = reply->readAll();
    }

    reply->close();
    delete reply;

    return ret;
}

//---------------------------------------------------------
//   parseInfo
//---------------------------------------------------------

QVariantMap AvsOmrNetRecognizer::parseInfo(const QByteArray& ba) const
{
    QVariantMap info;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(ba, &err);
    if (err.error != QJsonParseError::NoError) {
        LOGE() << "failed parse reply, err: " << err.errorString();
        return QVariantMap();
    }

    QJsonObject replyObj = doc.object();
    QJsonObject infoObj = replyObj.value("info").toObject();
    info["url"] = infoObj.value("url").toString();

    return info;
}

//---------------------------------------------------------
//   check
//---------------------------------------------------------

Ret AvsOmrNetRecognizer::check(const QString& url, QString* fileUrl)
{
    NetRet netRet;
    QByteArray data;

    for (size_t t = 0; t < CHECK_COUNT; ++t) {
        netRet = doCheck(url, &data);
        LOGI() << "[" << t << "] status: " << netRet.httpStatus;
        if (netRet.httpStatus != STATUS_PROCESSING) {
            break;
        }

        // sleep
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(CHECK_INTERVAL_MS);
        loop.exec();
    }

    if (netRet.httpStatus != STATUS_OK) {
        return netRetToRet(netRet);
    }

    QVariantMap info = parseInfo(data);
    if (fileUrl) {
        *fileUrl = info.value("url").toString();
    }

    return Ret::Ok;
}

//---------------------------------------------------------
//   doCheck
//---------------------------------------------------------

AvsOmrNetRecognizer::NetRet AvsOmrNetRecognizer::doCheck(const QString& url, QByteArray* ba)
{
    QNetworkRequest req;
    req.setUrl(QUrl(url));
    QNetworkReply* reply = _net->get(req);
    return doExecReply(reply, ba);
}

//---------------------------------------------------------
//   load
//---------------------------------------------------------

Ret AvsOmrNetRecognizer::load(const QString& url, QByteArray* ba)
{
    QNetworkRequest req;
    req.setUrl(QUrl(url));
    QNetworkReply* reply = _net->get(req);

    NetRet netRet = doExecReply(reply, ba);
    return netRetToRet(netRet);
}
