//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_FRAMEWORK_UITYPES_H
#define MU_FRAMEWORK_UITYPES_H

#include <QString>
#include <QMetaType>

#include "ret.h"

namespace mu {
namespace framework {
enum class ContainerType
{
    Undefined = 0,
    Qml,
    QWidget
};

struct ContainerMeta
{
    ContainerType type;
    QString qmlPath;
    int widgetMetaTypeId = QMetaType::UnknownType;

    ContainerMeta() = default;

    ContainerMeta(const ContainerType& type, const QString& qmlPath)
        : type(type), qmlPath(qmlPath) {}

    ContainerMeta(const ContainerType& type, int widgetMetaTypeId)
        : type(type), widgetMetaTypeId(widgetMetaTypeId) {}
};
}
}

#endif // MU_FRAMEWORK_UIERRORS_H
