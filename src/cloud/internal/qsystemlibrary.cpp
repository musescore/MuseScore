/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qsystemlibrary_p.h"
#include <QtCore/qvarlengtharray.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qfileinfo.h>

#include "log.h"
#include "defer.h"

/*!

    \internal
    \class QSystemLibrary
    \inmodule QtCore

    The purpose of this class is to load only libraries that are located in
    well-known and trusted locations on the filesystem. It does not suffer from
    the security problem that QLibrary has, therefore it will never search in
    the current directory.

    The search order is the same as the order in DLL Safe search mode Windows,
    except that we don't search:
    * The current directory
    * The 16-bit system directory. (normally \c{c:\windows\system})
    * The Windows directory.  (normally \c{c:\windows})

    This means that the effective search order is:
    1. Application path.
    2. System libraries path.
    3. Trying all paths inside the PATH environment variable.

    Note, when onlySystemDirectory is true it will skip 1) and 3).

    DLL Safe search mode is documented in the "Dynamic-Link Library Search
    Order" document on MSDN.
*/

QT_BEGIN_NAMESPACE

#if defined(Q_OS_WINRT)
HINSTANCE QSystemLibrary::load(const wchar_t *libraryName, bool onlySystemDirectory /* = true */)
{
    Q_UNUSED(onlySystemDirectory);
    return ::LoadPackagedLibrary(libraryName, 0);
}
#else

#if !defined(QT_BOOTSTRAPPED)
extern QString qAppFileName();
#endif

static QString qSystemDirectory()
{
    QVarLengthArray<wchar_t, MAX_PATH> fullPath;

    UINT retLen = ::GetSystemDirectory(fullPath.data(), MAX_PATH);
    if (retLen > MAX_PATH) {
        fullPath.resize(retLen);
        retLen = ::GetSystemDirectory(fullPath.data(), retLen);
    }
    // in some rare cases retLen might be 0
    return QString::fromWCharArray(fullPath.constData(), int(retLen));
}

HINSTANCE QSystemLibrary::load(const wchar_t *libraryName, bool onlySystemDirectory /* = true */)
{
    LOGI() << "load start " << QString::fromWCharArray(libraryName);
    DEFER {
        LOGI() << "load finish " << QString::fromWCharArray(libraryName);
    };
    QStringList searchOrder;

#if !defined(QT_BOOTSTRAPPED)
    if (!onlySystemDirectory)
        searchOrder << QFileInfo(qAppFileName()).path();
#endif
    searchOrder << qSystemDirectory();

    if (!onlySystemDirectory) {
        const QString PATH(QLatin1String(qgetenv("PATH").constData()));
        searchOrder << PATH.split(QLatin1Char(';'), Qt::SkipEmptyParts);
    }
    QString fileName = QString::fromWCharArray(libraryName);
    fileName.append(QLatin1String(".dll"));

    // Start looking in the order specified
    for (int i = 0; i < searchOrder.count(); ++i) {
        QString fullPathAttempt = searchOrder.at(i);
        if (!fullPathAttempt.endsWith(QLatin1Char('\\'))) {
            fullPathAttempt.append(QLatin1Char('\\'));
        }
        {
            fullPathAttempt.append(fileName);
            LOGI() << "looking at " << fullPathAttempt;
            HINSTANCE inst = ::LoadLibrary(reinterpret_cast<const wchar_t *>(fullPathAttempt.utf16()));
            LOGI() << "GetLastError: " << GetLastError();
            if (inst != 0) {
                LOGI() << "found";
                return inst;
            }
        }

        {
            fullPathAttempt.replace("/", "\\");
            LOGI() << "looking at " << fullPathAttempt;
            HINSTANCE inst = ::LoadLibrary(reinterpret_cast<const wchar_t *>(fullPathAttempt.utf16()));
            LOGI() << "GetLastError: " << GetLastError();
            if (inst != 0) {
                LOGI() << "found";
                return inst;
            }
        }

    }
    return 0;

}

#endif // Q_OS_WINRT

QT_END_NAMESPACE
