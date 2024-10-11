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

#ifndef MUSE_UI_IUIENGINE_H
#define MUSE_UI_IUIENGINE_H

#include <QString>

#include "modularity/imoduleinterface.h"

#include "uitypes.h"

class QQmlEngine;
class QQmlApplicationEngine;

namespace muse::ui {
class IUiEngine : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IUiEngine)

public:
    virtual ~IUiEngine() = default;

    virtual void updateTheme() = 0;
    virtual QQmlApplicationEngine* qmlAppEngine() const = 0;
    virtual QQmlEngine* qmlEngine() const = 0;
    virtual void quit() = 0;
    virtual void clearComponentCache() = 0;

    virtual GraphicsApi graphicsApi() const = 0;
    virtual QString graphicsApiName() const = 0;

    virtual void addSourceImportPath(const QString& path) = 0;
};
}

#endif // MUSE_UI_UIENGINEMODULE_H
