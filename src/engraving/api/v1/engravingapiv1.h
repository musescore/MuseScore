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
#ifndef MU_ENGRAVING_APIV1_ENGRAVINGAPIV1_H
#define MU_ENGRAVING_APIV1_ENGRAVINGAPIV1_H

#include "global/api/apiobject.h"

#include "score.h"

namespace mu::engraving::apiv1 {
class PluginAPI;
class EngravingApiV1 : public mu::api::ApiObject
{
    Q_PROPERTY(apiv1::Score * curScore READ curScore CONSTANT)

public:
    EngravingApiV1(mu::api::IApiEngine* e);
    ~EngravingApiV1();

    Q_INVOKABLE void __setup(const QJSValueList& args);

    void setApi(PluginAPI* api);
    PluginAPI* api() const;

    apiv1::Score* curScore() const;

private:
    mutable PluginAPI* m_api = nullptr;
    mutable bool m_selfApi = false;
};
}

#endif // MU_ENGRAVING_APIV1_ENGRAVINGAPIV1_H
