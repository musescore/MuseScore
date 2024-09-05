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
#ifndef MUSE_EXTENSIONS_API_EXTAPI_H
#define MUSE_EXTENSIONS_API_EXTAPI_H

#include <QObject>
#include <QMap>

#include "modularity/ioc.h"
#include "api/iapiregister.h"
#include "api/iapiengine.h"

namespace muse::extensions::api {
//! NOTE Used for qml and scripts
class ExtApi : public QObject, public Injectable
{
    Q_OBJECT
    Q_PROPERTY(QJSValue log READ log CONSTANT)
    Q_PROPERTY(QJSValue interactive READ interactive CONSTANT)
    Q_PROPERTY(QJSValue theme READ theme CONSTANT)

    Q_PROPERTY(QJSValue engraving READ engraving CONSTANT)

    Q_PROPERTY(QJSValue converter READ converter CONSTANT)

    //! NOTE Providing these APIs requires approval
    //Q_PROPERTY(QJSValue shortcuts READ shortcuts CONSTANT)
    //Q_PROPERTY(QJSValue navigation READ navigation CONSTANT)
    //Q_PROPERTY(QJSValue keyboard READ keyboard CONSTANT)
    //Q_PROPERTY(QJSValue accessibility READ accessibility CONSTANT)

    //! NOTE This is autobot context, should be rework and make general context
    //Q_PROPERTY(QJSValue context READ context CONSTANT)

    //! ATTENTION
    //! Don't add these APIs here.
    //! We do not control the authors of extensions;
    //! using these APIs they can deliberately or accidentally harm the user's system.
    //Q_PROPERTY(QJSValue process READ process CONSTANT)
    //Q_PROPERTY(QJSValue filesystem READ filesystem CONSTANT)

    Q_PROPERTY(QJSValue websocket READ websocket CONSTANT)
    Q_PROPERTY(QJSValue websocketserver READ websocketserver CONSTANT)

    Inject<muse::api::IApiRegister> apiRegister = { this };

public:
    ExtApi(muse::api::IApiEngine* engine, QObject* parent);
    ~ExtApi();

    QJSValue log() const { return api("api.log"); }
    QJSValue context() const { return api("api.context"); }
    QJSValue interactive() const { return api("api.interactive"); }
    QJSValue theme() const { return api("api.theme"); }

    QJSValue engraving() const { return api("api.engraving.v1"); }

    QJSValue converter() const { return api("api.converter"); }

    QJSValue dispatcher() const { return api("api.dispatcher"); }
    QJSValue navigation() const { return api("api.navigation"); }
    QJSValue shortcuts() const { return api("api.shortcuts"); }
    QJSValue keyboard() const { return api("api.keyboard"); }
    QJSValue accessibility() const { return api("api.accessibility"); }

    QJSValue websocket() const { return api("api.websocket"); }
    QJSValue websocketserver() const { return api("api.websocketserver"); }

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

#endif // MUSE_EXTENSIONS_API_EXTAPI_H
