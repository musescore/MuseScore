/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#include "autobotutils.h"

#include "log.h"

namespace mu::autobot {
QJSValue toQJSValue(const JsonValue& jv, api::IApiEngine* e)
{
    if (jv.isNull()) {
        return QJSValue(QJSValue::NullValue);
    } else if (jv.isBool()) {
        return QJSValue(jv.toBool());
    } else if (jv.isNumber()) {
        return QJSValue(jv.toDouble());
    } else if (jv.isString()) {
        return QJSValue(QString::fromStdString(jv.toStdString()));
    }

    IF_ASSERT_FAILED(e) {
        NOT_SUPPORTED;
        return QJSValue();
    }

    if (jv.isObject()) {
        QJSValue obj = e->newObject();
        JsonObject jo = jv.toObject();
        std::vector<std::string> keys = jo.keys();
        for (const std::string& k : keys) {
            obj.setProperty(QString::fromStdString(k), toQJSValue(jo.value(k), e));
        }
        return obj;
    } else if (jv.isArray()) {
        JsonArray ja = jv.toArray();
        QJSValue arr = e->newArray(ja.size());
        for (size_t i = 0; i < ja.size(); ++i) {
            arr.setProperty(quint32(i), toQJSValue(ja.at(i), e));
        }
        return arr;
    }

    NOT_SUPPORTED;
    return QJSValue();
}
}
