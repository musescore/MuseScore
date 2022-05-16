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

#ifndef MU_UICOMPONENTS_POPUPVIEWCLOSECONTROLLER_H
#define MU_UICOMPONENTS_POPUPVIEWCLOSECONTROLLER_H

#include <QObject>
#include <QQuickItem>

#include "async/notification.h"

#include "modularity/ioc.h"
#include "ui/imainwindow.h"

namespace mu::uicomponents {
class PopupViewCloseController : public QObject
{
    Q_OBJECT

    INJECT(uicomponents, ui::IMainWindow, mainWindow)

    Q_ENUMS(ClosePolicy)

public:
    explicit PopupViewCloseController(QObject* parent = nullptr);
    ~PopupViewCloseController() override = default;

    enum ClosePolicy {
        NoAutoClose = 0,
        CloseOnPressOutsideParent
    };

    void init();

    ClosePolicy closePolicy() const;
    void setClosePolicy(ClosePolicy closePolicy);

    virtual void setActive(bool active);

    QQuickItem* parentItem() const;
    void setParentItem(QQuickItem* parentItem);

    QWindow* popupWindow() const;
    void setWindow(QWindow* window);

    void setPopupHasFocus(bool hasFocus);

    async::Notification closeNotification() const;

private slots:
    void onApplicationStateChanged(Qt::ApplicationState state);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

    void doFocusOut();

    bool isMouseWithinBoundaries(const QPoint& mousePos) const;

private:
    void notifyAboutClose();

    bool m_active = false;

    ClosePolicy m_closePolicy = ClosePolicy::CloseOnPressOutsideParent;
    QQuickItem* m_parentItem = nullptr;
    QWindow* m_popupWindow = nullptr;

    bool m_popupHasFocus = true;

    async::Notification m_closeNotification;
};
}

Q_DECLARE_METATYPE(mu::uicomponents::PopupViewCloseController::ClosePolicy);

#endif // MU_UICOMPONENTS_POPUPVIEWCLOSECONTROLLER_H
