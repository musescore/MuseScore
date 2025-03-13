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
#include "translation.h"

#ifndef NO_QT_SUPPORT
#include <QCoreApplication>
#include <QString>
#endif

#include "types/bytearray.h"
#include "types/string.h"

#include "log.h"

using namespace muse;

static String translate(const char* context, const char* key, const char* disambiguation, int n)
{
#ifndef NO_QT_SUPPORT
    return String::fromQString(QCoreApplication::translate(context, key, disambiguation, n));
#else
    UNUSED(context);
    UNUSED(disambiguation);
    UNUSED(n);
    return String::fromUtf8(key);
#endif
}

std::string muse::trc(const char* context, const char* key, const char* disambiguation, int n)
{
    return translate(context, key, disambiguation, n).toStdString();
}

#ifndef NO_QT_SUPPORT
QString muse::qtrc(const char* context, const char* key, const char* disambiguation, int n)
{
    return QCoreApplication::translate(context, key, disambiguation, n);
}

QString muse::qtrc(const char* context, const String& key, const char* disambiguation, int n)
{
    ByteArray utf8 = key.toUtf8();
    return QCoreApplication::translate(context, utf8.constChar(), disambiguation, n);
}

QString muse::qtrc(const char* context, const String& key, const String& disambiguation, int n)
{
    ByteArray keyutf8 = key.toUtf8();
    ByteArray disutf8 = disambiguation.toUtf8();
    return QCoreApplication::translate(context, keyutf8.constChar(), disutf8.empty() ? nullptr : disutf8.constChar(), n);
}

#endif

String muse::mtrc(const char* context, const char* key, const char* disambiguation, int n)
{
    return translate(context, key, disambiguation, n);
}

String muse::mtrc(const char* context, const String& key, const char* disambiguation, int n)
{
    ByteArray keyutf8 = key.toUtf8();
    return translate(context, keyutf8.constChar(), disambiguation, n);
}

String muse::mtrc(const char* context, const String& key, const String& disambiguation, int n)
{
    ByteArray keyutf8 = key.toUtf8();
    ByteArray disutf8 = disambiguation.toUtf8();
    return translate(context, keyutf8.constChar(), disutf8.empty() ? nullptr : disutf8.constChar(), n);
}
