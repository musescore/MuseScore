/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <QString>
#include <QMetaType>

namespace muse::interactive {
struct ContainerMeta
{
    enum Type {
        Undefined = 0,
        PrimaryPage,
        QmlDialog,
        QWidgetDialog
    };

    Type type = Type::Undefined;
    QString qmlModule;
    QString qmlPath;
    int widgetMetaTypeId = QMetaType::UnknownType;

    ContainerMeta() = default;

    ContainerMeta(Type type)
        : type(type) {}
    ContainerMeta(Type type, QString qmlPath)
        : type(type), qmlPath(qmlPath) {}
    ContainerMeta(Type type, QString qmlModule, QString qmlPath)
        : type(type), qmlModule(qmlModule), qmlPath(qmlPath) {}
    ContainerMeta(Type type, int widgetMetaTypeId)
        : type(type), widgetMetaTypeId(widgetMetaTypeId) {}
};
}
