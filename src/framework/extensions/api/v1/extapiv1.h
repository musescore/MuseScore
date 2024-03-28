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
#ifndef MU_EXTENSIONS_APIV1_EXTAPIV1_H
#define MU_EXTENSIONS_APIV1_EXTAPIV1_H

#include <QObject>
#include <QMap>

#include "modularity/ioc.h"
#include "api/iapiregister.h"
#include "api/iapiengine.h"

namespace mu::extensions::apiv1 {
class ExtApiV1 : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QJSValue log READ log CONSTANT)
    Q_PROPERTY(QJSValue engraving READ engraving CONSTANT)

    Inject<mu::api::IApiRegister> apiRegister;

public:

    ExtApiV1(mu::api::IApiEngine* engine, QObject* parent);

    void setup(QJSValue globalObj);

    QJSValue log() const { return api("api.log"); }
    QJSValue engraving() const { return api("api.engraving.v1"); }

    static void registerQmlTypes();

private:

    QJSValue api(const std::string& name) const;

    struct Api
    {
        mu::api::ApiObject* obj = nullptr;
        QJSValue jsval;
    };

    mu::api::IApiEngine* m_engine = nullptr;
    mutable QMap<std::string, Api> m_apis;
};
}
#endif // MU_EXTENSIONS_APIV1_EXTAPIV1_H
