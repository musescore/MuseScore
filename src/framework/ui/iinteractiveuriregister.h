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
#ifndef MU_UI_IINTERACTIVEURIREGISTER_H
#define MU_UI_IINTERACTIVEURIREGISTER_H

#include <QVariantMap>
#include <QDialog>

#include "modularity/imoduleexport.h"
#include "global/uri.h"
#include "uitypes.h"

namespace mu::ui {
class IInteractiveUriRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IInteractiveUriRegister)

public:
    virtual ~IInteractiveUriRegister() = default;

    virtual void registerUri(const Uri& uri, const ContainerMeta& meta) = 0;
    virtual ContainerMeta meta(const Uri& uri) const = 0;

    // userfull
    void registerQmlUri(const Uri& uri, const QString& qmlPath)
    {
        registerUri(uri, ContainerMeta(ContainerType::Type::QmlDialog, qmlPath));
    }

    void registerWidgetUri(const Uri& uri, int widgetMetaTypeId)
    {
        registerUri(uri, ContainerMeta(ContainerType::Type::QWidgetDialog, widgetMetaTypeId));
    }
};
}

#endif // MU_UI_IINTERACTIVEURIREGISTER_H
