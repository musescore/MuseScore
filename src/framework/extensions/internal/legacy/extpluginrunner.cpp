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
#include "extpluginrunner.h"

#include <QQmlComponent>

#include "api/qmlpluginapi.h"

#include "log.h"

using namespace mu::extensions;
using namespace mu::extensions::legacy;

mu::Ret ExtPluginRunner::run(const Manifest& m)
{
    //! NOTE We create extension UI using a separate engine to control what we provide,
    //! making it easier to maintain backward compatibility and stability.
    QQmlComponent component = QQmlComponent(engine()->qmlEngine(), m.qmlFilePath.toQString());
    if (!component.isReady()) {
        LOGE() << "Failed to load QML file: " << m.qmlFilePath
               << ", from extension: " << m.uri.toString();
        LOGE() << component.errorString();
        return make_ret(Ret::Code::UnknownError, component.errorString().toStdString());
    }

    QObject* obj = component.create();
    if (!obj) {
        LOGE() << "Failed to create QML Object: " << m.qmlFilePath
               << ", from extension: " << m.uri.toString();
        return make_ret(Ret::Code::UnknownError);
    }

    QmlPluginApi* plugin = qobject_cast<QmlPluginApi*>(obj);
    if (!plugin) {
        LOGE() << "Qml Object not MuseScore plugin: " << m.qmlFilePath
               << ", from extension: " << m.uri.toString();
        return make_ret(Ret::Code::UnknownError);
    }

    plugin->runPlugin();

    return make_ok();
}
