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
#include <QCoreApplication>

using namespace mu;

std::string mu::trc(const char* context, const char* key, const char* disambiguation, int n)
{
    return QCoreApplication::translate(context, key, disambiguation, n).toStdString();
}

QString mu::qtrc(const char* context, const char* key, const char* disambiguation, int n)
{
    return QCoreApplication::translate(context, key, disambiguation, n);
}

QString mu::qtrc(const char* context, const String& key, const char* disambiguation, int n)
{
    ByteArray utf8 = key.toUtf8();
    return QCoreApplication::translate(context, utf8.constChar(), disambiguation, n);
}

QString mu::qtrc(const char* context, const String& key, const String& disambiguation, int n)
{
    ByteArray keyutf8 = key.toUtf8();
    ByteArray disutf8 = disambiguation.toUtf8();
    return QCoreApplication::translate(context, keyutf8.constChar(), disutf8.empty() ? nullptr : disutf8.constChar(), n);
}
