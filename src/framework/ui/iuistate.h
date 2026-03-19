/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

 #pragma once

#include <QByteArray>

 #include "modularity/imoduleinterface.h"
 #include "global/async/notification.h"
 #include "global/types/retval.h"

 #include "uiaction.h"

namespace muse::ui {
class IUiState : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(IUiState)

public:
    virtual ~IUiState() = default;

    virtual QByteArray windowGeometry() const = 0;
    virtual void setWindowGeometry(const QByteArray& state) = 0;
    virtual async::Notification windowGeometryChanged() const = 0;

    virtual ValNt<QByteArray> pageState(const QString& pageName) const = 0;
    virtual void setPageState(const QString& pageName, const QByteArray& state) = 0;

    virtual ToolConfig toolConfig(const QString& toolName, const ToolConfig& defaultConfig) const = 0;
    virtual void setToolConfig(const QString& toolName, const ToolConfig& config) = 0;
    virtual async::Notification toolConfigChanged(const QString& toolName) const = 0;

    virtual QString uiItemState(const QString& itemName) const = 0;
    virtual void setUiItemState(const QString& itemName, const QString& value) = 0;
    virtual async::Notification uiItemStateChanged(const QString& itemName) const = 0;

    virtual bool isVisible(const QString& key, bool def = true) const = 0;
    virtual void setIsVisible(const QString& key, bool val) = 0;
    virtual async::Notification isVisibleChanged(const QString& key) const = 0;
};
}
