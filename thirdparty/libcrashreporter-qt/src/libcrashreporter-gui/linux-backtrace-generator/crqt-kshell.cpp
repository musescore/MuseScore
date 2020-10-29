/*
    This file is part of the KDE libraries

    Copyright (c) 2003,2007 Oswald Buddenhagen <ossi@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "crqt-kshell.h"
#include "crqt-kshell_p.h"

#include <QtCore/QDir>

QString KShell::homeDir(const QString &user)
{
    return QDir::homePath();
}

QString KShell::joinArgs(const QStringList &args)
{
    QString ret;
    for (QStringList::ConstIterator it = args.begin(); it != args.end(); ++it) {
        if (!ret.isEmpty()) {
            ret.append(QLatin1Char(' '));
        }
        ret.append(quoteArg(*it));
    }
    return ret;
}

#ifdef Q_OS_WIN
# define ESCAPE '^'
#else
# define ESCAPE '\\'
#endif

QString KShell::tildeExpand(const QString &fname)
{
    if (!fname.isEmpty() && fname[0] == QLatin1Char('~')) {
        int pos = fname.indexOf(QLatin1Char('/'));
        if (pos < 0) {
            return homeDir(fname.mid(1));
        }
        QString ret = homeDir(fname.mid(1, pos - 1));
        if (!ret.isNull()) {
            ret += fname.midRef(pos);
        }
        return ret;
    } else if (fname.length() > 1 && fname[0] == QLatin1Char(ESCAPE) && fname[1] == QLatin1Char('~')) {
        return fname.mid(1);
    }
    return fname;
}
