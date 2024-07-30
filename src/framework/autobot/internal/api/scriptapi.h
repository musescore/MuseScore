/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MUSE_AUTOBOT_API_SCRIPTAPI_H
#define MUSE_AUTOBOT_API_SCRIPTAPI_H

#include <QObject>
#include <QMap>

#include "modularity/ioc.h"
#include "api/iapiregister.h"
#include "api/iapiengine.h"

namespace muse::autobot {
class ScriptApi : public QObject, public Injectable
{
    Q_OBJECT
    Q_PROPERTY(QJSValue log READ log CONSTANT)
    Q_PROPERTY(QJSValue autobot READ autobot CONSTANT)
    Q_PROPERTY(QJSValue dispatcher READ dispatcher CONSTANT)
    Q_PROPERTY(QJSValue navigation READ navigation CONSTANT)
    Q_PROPERTY(QJSValue context READ context CONSTANT)
    Q_PROPERTY(QJSValue shortcuts READ shortcuts CONSTANT)
    Q_PROPERTY(QJSValue interactive READ interactive CONSTANT)
    Q_PROPERTY(QJSValue keyboard READ keyboard CONSTANT)
    Q_PROPERTY(QJSValue accessibility READ accessibility CONSTANT)
    Q_PROPERTY(QJSValue process READ process CONSTANT)
    Q_PROPERTY(QJSValue filesystem READ filesystem CONSTANT)

    Inject<muse::api::IApiRegister> apiRegister = { this };

public:
    ScriptApi(muse::api::IApiEngine* engine, QObject* parent);
    ~ScriptApi();

    QJSValue log() const { return api("api.log"); }
    QJSValue autobot() const { return api("api.autobot"); }
    QJSValue dispatcher() const { return api("api.dispatcher"); }
    QJSValue navigation() const { return api("api.navigation"); }
    QJSValue context() const { return api("api.context"); }
    QJSValue shortcuts() const { return api("api.shortcuts"); }
    QJSValue interactive() const { return api("api.interactive"); }
    QJSValue keyboard() const { return api("api.keyboard"); }
    QJSValue accessibility() const { return api("api.accessibility"); }
    QJSValue process() const { return api("api.process"); }
    QJSValue filesystem() const { return api("api.filesystem"); }

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

#endif // MUSE_AUTOBOT_API_SCRIPTAPI_H
