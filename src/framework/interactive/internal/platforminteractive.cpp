/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and it under the terms of the GNU General Public License version 3 as
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

 #include "platforminteractive.h"

 #include <QDesktopServices>

 #ifdef Q_OS_MAC
 #include "platform/macos/macosinteractivehelper.h"
 #elif defined(Q_OS_WIN)
 #include <QDir>
 #include <QProcess>
 #include "platform/win/wininteractivehelper.h"
 #endif

using namespace muse;

Ret PlatformInteractive::openUrl(const std::string& url) const
{
    return openUrl(QUrl(QString::fromStdString(url)));
}

Ret PlatformInteractive::openUrl(const QUrl& url) const
{
    return QDesktopServices::openUrl(url);
}

Ret PlatformInteractive::isAppExists(const std::string& appIdentifier) const
{
#ifdef Q_OS_MACOS
    return MacOSInteractiveHelper::isAppExists(appIdentifier);
#else
    NOT_IMPLEMENTED;
    UNUSED(appIdentifier);
    return false;
#endif
}

Ret PlatformInteractive::canOpenApp(const UriQuery& uri) const
{
#ifdef Q_OS_MACOS
    return MacOSInteractiveHelper::canOpenApp(uri);
#else
    NOT_IMPLEMENTED;
    UNUSED(uri);
    return false;
#endif
}

async::Promise<Ret> PlatformInteractive::openApp(const UriQuery& uri) const
{
#ifdef Q_OS_MACOS
    return MacOSInteractiveHelper::openApp(uri);
#elif defined(Q_OS_WIN)
    return WinInteractiveHelper::openApp(uri);
#else
    UNUSED(uri);
    return async::Promise<Ret>([](auto, auto reject) {
        Ret ret = make_ret(Ret::Code::NotImplemented);
        return reject(ret.code(), ret.text());
    });
#endif
}

Ret PlatformInteractive::revealInFileBrowser(const io::path_t& filePath) const
{
#ifdef Q_OS_MACOS
    if (MacOSInteractiveHelper::revealInFinder(filePath)) {
        return true;
    }
#elif defined(Q_OS_WIN)
    QString command = QLatin1String("explorer /select,%1").arg(QDir::toNativeSeparators(filePath.toQString()));
    if (QProcess::startDetached(command, QStringList())) {
        return true;
    }
#endif
    io::path_t dirPath = io::dirpath(filePath);
    return openUrl(QUrl::fromLocalFile(dirPath.toQString()));
}
