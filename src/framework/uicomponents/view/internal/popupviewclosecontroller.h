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

#ifndef MUSE_UICOMPONENTS_POPUPVIEWCLOSECONTROLLER_H
#define MUSE_UICOMPONENTS_POPUPVIEWCLOSECONTROLLER_H

#include <QObject>
#include <QQuickItem>

#include "async/notification.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "ui/imainwindow.h"
#include "ui/iinteractiveprovider.h"

namespace muse::uicomponents {
class PopupViewCloseController : public QObject, public muse::Injectable, public async::Asyncable
{
    Q_OBJECT

    muse::Inject<ui::IMainWindow> mainWindow = { this };
    muse::Inject<ui::IInteractiveProvider> interactiveProvider = { this };

public:
    explicit PopupViewCloseController(const muse::modularity::ContextPtr& iocCtx, QObject* parent = nullptr);
    ~PopupViewCloseController() override = default;

    void init();

    bool active() const;
    void setActive(bool active);

    QQuickItem* parentItem() const;
    void setParentItem(QQuickItem* parentItem);

    QWindow* popupWindow() const;
    void setWindow(QWindow* window);

    void setIsCloseOnPressOutsideParent(bool close);

    async::Notification closeNotification() const;

private slots:
    void onApplicationStateChanged(Qt::ApplicationState state);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

    void doFocusOut(const QPointF& mousePos);
    virtual void doUpdateEventFilters();

    bool isMouseWithinBoundaries(const QPointF& mousePos) const;

    void notifyAboutClose();

private:
    QWindow* parentWindow() const;

    bool m_active = false;

    QQuickItem* m_parentItem = nullptr;
    QWindow* m_popupWindow = nullptr;

    bool m_isCloseOnPressOutsideParent = false;

    async::Notification m_closeNotification;
};
}

#endif // MUSE_UICOMPONENTS_POPUPVIEWCLOSECONTROLLER_H
