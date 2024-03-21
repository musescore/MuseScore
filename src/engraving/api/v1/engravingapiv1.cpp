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

#include "qmlpluginapi.h"

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

void EngravingApiV1::__setup(const QJSValueList& args)
{
    IF_ASSERT_FAILED(args.size() == 1) {
        return;
    }
    QJSValue globalObj = args.at(0);
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
    return m_api->curScore();
}
