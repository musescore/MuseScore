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
#include "accessibilitymodule.h"

#include <QtQml>
#include <QAccessible>

#include "modularity/ioc.h"
#include "log.h"

using namespace mu::accessibility;
using namespace mu::framework;

static void accessibility_init_qrc()
{
    //Q_INIT_RESOURCE(accessibility);
}

static QAccessibleInterface* muAccessibleFactory(const QString& classname, QObject* object)
{
    LOGI() << "classname: " << classname;
//    if (classname == QLatin1String("QQuickWindow")) {
//        return new QAccessibleQuickWindow(qobject_cast<QQuickWindow*>(object));
//    } else if (classname == QLatin1String("QQuickItem")) {
//        QQuickItem* item = qobject_cast<QQuickItem*>(object);
//        Q_ASSERT(item);
//        QQuickItemPrivate* itemPrivate = QQuickItemPrivate::get(item);
//        if (!itemPrivate->isAccessible) {
//            return nullptr;
//        }
//        return new QAccessibleQuickItem(item);
//    }
    return nullptr;
}

std::string AccessibilityModule::moduleName() const
{
    return "accessibility";
}

void AccessibilityModule::registerExports()
{
}

void AccessibilityModule::resolveImports()
{
}

void AccessibilityModule::registerResources()
{
    accessibility_init_qrc();
}

void AccessibilityModule::registerUiTypes()
{
}

void AccessibilityModule::onInit(const IApplication::RunMode&)
{
    QAccessible::installFactory(muAccessibleFactory);
}

void AccessibilityModule::onDeinit()
{
}
