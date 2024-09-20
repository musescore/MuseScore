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

#include "../../api/v1/ipluginapiv1.h"
#include "../../extensionserrors.h"

#include "log.h"

using namespace muse;
using namespace muse::extensions;
using namespace muse::extensions::legacy;
using namespace muse::extensions::apiv1;

Ret ExtPluginRunner::run(const Action& action)
{
    io::path_t qmlPath = action.path;

    //! NOTE We create extension UI using a separate engine to control what we provide,
    //! making it easier to maintain backward compatibility and stability.
    QQmlComponent component = QQmlComponent(engine()->qmlEngineApiV1(), qmlPath.toQString());
    if (!component.isReady()) {
        LOGE() << "Failed to load QML file: " << qmlPath;
        LOGE() << component.errorString();
        return make_ret(Err::ExtLoadError);
    }

    QObject* obj = component.create();
    if (!obj) {
        LOGE() << "Failed to create QML Object: " << qmlPath;
        return make_ret(Err::ExtLoadError);
    }

    IPluginApiV1* plugin = dynamic_cast<IPluginApiV1*>(obj);
    if (!plugin) {
        LOGE() << "Qml Object not MuseScore plugin: " << qmlPath;
        return make_ret(Err::ExtBadFormat);
    }

    plugin->runPlugin();

    return muse::make_ok();
}
