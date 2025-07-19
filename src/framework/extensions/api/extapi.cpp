/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#include "extapi.h"

#include "log.h"

using namespace muse::extensions::api;

ExtApi::ExtApi(muse::api::IApiEngine* engine, QObject* parent)
    : QObject(parent), Injectable(engine->iocContext()), m_engine(engine)
{
}

ExtApi::~ExtApi()
{
    for (auto& a : m_apis) {
        if (a.isNeedDelete) {
            delete a.obj;
        }
    }
}

QJSValue ExtApi::api(const std::string& name) const
{
    if (!apiRegister()) {
        return QJSValue();
    }

    Api a = m_apis.value(name);
    if (!a.jsval.isUndefined()) {
        return a.jsval;
    }

    auto api = apiRegister()->createApi(name, m_engine);
    a.obj = api.first;
    a.isNeedDelete = api.second;
    if (!a.obj) {
        LOGW() << "Not allowed api: " << name;
        return QJSValue();
    }

    a.jsval = m_engine->newQObject(a.obj);

    m_apis[name] = a;

    return a.jsval;
}
