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
#include "extapiv1.h"

#include <QQmlEngine>

#include "messagedialog.h"
#include "filedialog.h"
#include "qqmlsettings_p.h"
#include "util.h"
#include "iapiv1object.h"

#include "log.h"

using namespace muse::extensions::apiv1;

void ExtApiV1::registerQmlTypes()
{
    qmlRegisterType<MsProcess>("MuseScore", 3, 0, "QProcess");
    qmlRegisterType<FileIO, 1>("FileIO",    3, 0, "FileIO");

    qmlRegisterUncreatableType<StandardButton>("MuseScore", 3, 0, "StandardButton", "Cannot create an enumeration");
    qmlRegisterType<MessageDialog>("MuseScore", 3, 0, "MessageDialog");
    qmlRegisterType<QQmlSettings>("MuseScore", 3, 0, "Settings");
    qmlRegisterType<FileDialog>("MuseScore", 3, 0, "FileDialog");
}

ExtApiV1::ExtApiV1(muse::api::IApiEngine* engine, QObject* parent)
    : QObject(parent), Injectable(engine->iocContext()), m_engine(engine)
{
}

ExtApiV1::~ExtApiV1()
{
    for (auto& a : m_apis) {
        if (a.isNeedDelete) {
            delete a.obj;
        }
    }
}

void ExtApiV1::setup(QJSValue globalObj)
{
    QJSValue engApiVal = engraving();
    if (engApiVal.isNull()) {
        LOGE() << "not found api.engraving.v1";
        return;
    }

    QObject* engObj = engApiVal.toQObject();
    if (!engObj) {
        LOGE() << "api.engraving.v1 is not QObject";
        return;
    }

    IApiV1Object* engApiV1 = dynamic_cast<IApiV1Object*>(engObj);
    if (!engApiV1) {
        LOGE() << "api.engraving.v1 is not IApiV1Object";
        return;
    }

    engApiV1->setup(globalObj);
}

QJSValue ExtApiV1::api(const std::string& name) const
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
