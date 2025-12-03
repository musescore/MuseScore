/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#ifndef MUSE_API_APIUTILS_H
#define MUSE_API_APIUTILS_H

#include <string>
#include <vector>
#include <QString>
#include <QJSValue>

#include "apitypes.h"
#include "iapiengine.h"

namespace muse::api {
inline std::vector<std::string> toStdVector(const QStringList& l)
{
    std::vector<std::string> v;
    v.reserve(l.size());
    for (const QString& s : l) {
        v.push_back(s.toStdString());
    }
    return v;
}

inline QJSValue enumToJsValue(IApiEngine* engine, const QMetaEnum& meta, EnumType type)
{
    QJSValue enumObj = engine->newObject();

    for (int i = 0; i < meta.keyCount(); ++i) {
        QString key = QString::fromLatin1(meta.key(i));
        if (type == EnumType::String) {
            enumObj.setProperty(key, key);
        } else {
            int val = meta.value(i);
            enumObj.setProperty(key, val);
        }
    }

    QJSValue frozenObj = engine->freeze(enumObj);
    return frozenObj;
}
}

#endif // MUSE_API_APIUTILS_H
