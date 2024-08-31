/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "engravingapiv1.h"

#include <QJSValueIterator>

#include "qmlpluginapi.h"

#include "log.h"

using namespace mu::engraving::apiv1;

EngravingApiV1::EngravingApiV1(muse::api::IApiEngine* e)
    : muse::api::ApiObject(e)
{
}

EngravingApiV1::~EngravingApiV1()
{
    if (m_selfApi) {
        delete m_api;
    }
}

//! NOTE Don't remove please
// static void dump(QString name, QJSValue val)
// {
//     LOGDA() << "val: " << name;
//     QJSValueIterator it(val);
//     while (it.hasNext()) {
//         it.next();
//         LOGDA() << it.name() << ": " << it.value().toString();
//     }
// }

void EngravingApiV1::setup(QJSValue globalObj)
{
    //! NOTE API v1 provides not only one global `api` object,
    //! but also a number of others, for example `curScore`,
    //! this is the legacy of the Qml plugin.

    QJSValue self = engine()->newQObject(this);

    static const QSet<QString> ignore = { "objectName", "objectNameChanged" };

    QJSValueIterator it(self);
    while (it.hasNext()) {
        it.next();

        if (ignore.contains(it.name())) {
            continue;
        }

        LOGDA() << it.name() << ": " << it.value().toString();
        globalObj.setProperty(it.name(), it.value());
    }
}

void EngravingApiV1::setApi(PluginAPI* api)
{
    m_api = api;
}

PluginAPI* EngravingApiV1::api() const
{
    if (!m_api) {
        m_api = new PluginAPI();
        m_selfApi = true;
    }

    return m_api;
}
