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
#ifndef MUSE_EXTENSIONS_APIV1_EXTAPIV1_H
#define MUSE_EXTENSIONS_APIV1_EXTAPIV1_H

#include <QObject>
#include <QMap>

#include "modularity/ioc.h"
#include "api/iapiregister.h"
#include "api/iapiengine.h"

namespace muse::extensions::apiv1 {
class ExtApiV1 : public QObject, public Injectable
{
    Q_OBJECT

    Q_PROPERTY(QJSValue log READ log CONSTANT)
    Q_PROPERTY(QJSValue engraving READ engraving CONSTANT)

    Q_PROPERTY(QJSValue websocket READ websocket CONSTANT)
    Q_PROPERTY(QJSValue websocketserver READ websocketserver CONSTANT)

    Inject<muse::api::IApiRegister> apiRegister = { this };

public:

    ExtApiV1(muse::api::IApiEngine* engine, QObject* parent);
    ~ExtApiV1();

    void setup(QJSValue globalObj);

    QJSValue log() const { return api("api.log"); }
    QJSValue engraving() const { return api("api.engraving.v1"); }
    QJSValue websocket() const { return api("api.websocket"); }
    QJSValue websocketserver() const { return api("api.websocketserver"); }

    static void registerQmlTypes();

private:

    QJSValue api(const std::string& name) const;

    struct Api
    {
        muse::api::ApiObject* obj = nullptr;
        bool isNeedDelete = false;
        QJSValue jsval;
    };

    muse::api::IApiEngine* m_engine = nullptr;
    mutable QMap<std::string, Api> m_apis;
};
}
#endif // MUSE_EXTENSIONS_APIV1_EXTAPIV1_H
