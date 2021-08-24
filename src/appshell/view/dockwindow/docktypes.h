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

#ifndef MU_DOCK_DOCKTYPES_H
#define MU_DOCK_DOCKTYPES_H

#include <QObject>
#include <QVariant>

namespace mu::dock {
inline const char* CONTEXT_MENU_MODEL_PROPERTY("contextMenuModel");
inline const char* DOCK_PANEL_PROPERY("dockPanel");

enum class DockType {
    Undefined = -1,
    Panel,
    PanelDockingHolder,
    ToolBar,
    ToolBarDockingHolder,
    StatusBar,
    Central
};

struct DockProperties
{
    DockType type = DockType::Undefined;
    Qt::DockWidgetAreas allowedAreas = Qt::NoDockWidgetArea;
};

inline void writePropertiesToObject(const DockProperties& properties, QObject& obj)
{
    QObject* propertiesObj = new QObject(&obj);
    propertiesObj->setObjectName("properties");
    propertiesObj->setProperty("dockType", QVariant::fromValue(static_cast<int>(properties.type)));
    propertiesObj->setProperty("allowedAreas", QVariant::fromValue(static_cast<int>(properties.allowedAreas)));
}

inline DockProperties readPropertiesFromObject(const QObject* obj)
{
    if (!obj) {
        return DockProperties();
    }

    QObject* properties = obj->findChild<QObject*>("properties");
    if (!properties) {
        return DockProperties();
    }

    DockProperties result;
    result.type = static_cast<DockType>(properties->property("dockType").toInt());
    result.allowedAreas = static_cast<Qt::DockWidgetAreas>(properties->property("allowedAreas").toInt());

    return result;
}
}

#endif // MU_DOCK_DOCKTYPES_H
