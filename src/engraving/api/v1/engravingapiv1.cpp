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

#include "engravingapiv1.h"

#include <QJSValueIterator>

#include "qmlpluginapi.h"

#include "log.h"

using namespace mu::engraving::apiv1;

EngravingApiV1::EngravingApiV1(mu::api::IApiEngine* e)
    : mu::api::ApiObject(e)
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
    QJSValue self = engine()->newQObject(this);

    // Objects properties
    globalObj.setProperty("curScore", self.property("curScore"));

    // Enums properties
    globalObj.setProperty("Element", self.property("Element"));

    // Functions
    globalObj.setProperty("cmd", self.property("cmd"));
    globalObj.setProperty("quit", self.property("quit"));
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

Score* EngravingApiV1::curScore() const
{
    return api()->curScore();
}

Enum* EngravingApiV1::elementEnum() const
{
    return api()->get_elementTypeEnum();
}

void EngravingApiV1::cmd(const QString& code)
{
    api()->cmd(code);
}

void EngravingApiV1::quit()
{
    api()->quit();
}
