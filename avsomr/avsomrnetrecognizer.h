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

#ifndef AVS_AVSOMRNETRECOGNIZER_H
#define AVS_AVSOMRNETRECOGNIZER_H

#include <QString>
#include <QByteArray>
#include <QVariantMap>

#include "iavsomrrecognizer.h"

class QNetworkAccessManager;
class QNetworkReply;

namespace Ms {
namespace Avs {
class AvsOmrNetRecognizer : public IAvsOmrRecognizer
{
public:
    AvsOmrNetRecognizer();
    ~AvsOmrNetRecognizer() override;

    QString type() const override;
    bool isAvailable() const override;
    bool recognize(const QString& filePath, QByteArray* avsFileData, const OnStep& onStep) override;

private:

    Ret send(const QString& filePath, QString* getUrl);
    Ret check(const QString& url, QString* fileUrl);
    Ret load(const QString& url, QByteArray* ba);

    struct NetRet
    {
        int httpStatus{ 0 };
        int replyError{ 0 };
    };

    Ret netRetToRet(const NetRet& nr) const;

    NetRet doExecReply(QNetworkReply* reply, QByteArray* ba);
    NetRet doCheck(const QString& url, QByteArray* ba);

    QVariantMap parseInfo(const QByteArray& ba) const;

    QNetworkAccessManager* _net{ nullptr };
};
} // Avs
} // Ms

#endif // AVS_AVSOMRNETRECOGNIZER_H
