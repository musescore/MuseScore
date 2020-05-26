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

#ifndef AVS_AVSOMRLOCALINSTALLER_H
#define AVS_AVSOMRLOCALINSTALLER_H

#include <functional>
#include <QString>
#include <QByteArray>

template<typename T>
class QFutureWatcher;

class QEventLoop;

namespace Ms {
namespace Avs {
class AvsOmrLocalInstaller
{
private:

    friend class AvsOmrLocal;

    AvsOmrLocalInstaller(const QString& avsHomePath);

    struct ReleaseInfo {
        QString tag;
        QString url;
        bool isValid() const { return !tag.isEmpty() && !url.isEmpty(); }
    };

    const ReleaseInfo& loadReleaseInfo() const;

    void installBackground();
    void waitForFinished();

private:

    bool doLoadReleaseInfo(ReleaseInfo* info, const QString& url) const;
    bool doInstallAvs(const QString& url);
    bool getData(QByteArray* data, const QString& url, const QByteArray& mime) const;
    bool unpackAvs(QByteArray* avsZipPack, const QString& path);
    bool cleanDir(const QString& dirPath);

    QString _avsHomePath;
    mutable ReleaseInfo _info;
    QFutureWatcher<bool>* _watcher{ nullptr };
    QEventLoop* _loop{ nullptr };
};
} // Avs
} // Ms

#endif // AVS_AVSOMRLOCALINSTALLER_H
