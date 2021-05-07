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
#ifndef MU_UICOMPONENTS_IPOPUPWINDOW_H
#define MU_UICOMPONENTS_IPOPUPWINDOW_H

#include <memory>
#include <QQmlEngine>
#include <QQuickItem>
#include "ui/iuiconfiguration.h"

namespace mu::uicomponents {
class IPopupWindow
{
public:
    virtual ~IPopupWindow() = default;

    virtual void init(QQmlEngine* engine, std::shared_ptr<ui::IUiConfiguration> uiConfiguration, bool isDialogMode) = 0;

    virtual void setContent(QQuickItem* item) = 0;

    virtual void show(QPoint p) = 0;
    virtual void hide() = 0;

    virtual QWindow* qWindow() const = 0;
    virtual bool isVisible() const = 0;
    virtual QRect geometry() const = 0;

    virtual void forceActiveFocus() = 0;

    virtual void setOnHidden(const std::function<void()>& callback) = 0;
};
}
#endif // MU_UICOMPONENTS_IPOPUPWINDOW_H
