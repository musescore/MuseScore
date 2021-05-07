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
#ifndef MU_UICOMPONENTS_POPUPWINDOW_QQUICKVIEW_H
#define MU_UICOMPONENTS_POPUPWINDOW_QQUICKVIEW_H

#include <QObject>
#include <QQuickView>
#include "ipopupwindow.h"

#include "modularity/ioc.h"
#include "ui/imainwindow.h"

namespace mu::uicomponents {
class PopupWindow_QQuickView : public QObject, public IPopupWindow
{
    Q_OBJECT

    INJECT(uicomponents, ui::IMainWindow, mainWindow)

public:
    explicit PopupWindow_QQuickView(QObject* parent = nullptr);
    ~PopupWindow_QQuickView();

    void init(QQmlEngine* engine, std::shared_ptr<ui::IUiConfiguration> uiConfiguration, bool isDialogMode) override;

    void setContent(QQuickItem* item) override;

    void show(QPoint p) override;
    void hide() override;

    QWindow* qWindow() const override;
    bool isVisible() const override;
    QRect geometry() const override;

    void forceActiveFocus() override;

    void setOnHidden(const std::function<void()>& callback) override;

private:

    bool eventFilter(QObject* watched, QEvent* event) override;

    QQuickView* m_view = nullptr;
    std::function<void()> m_onHidden;
};
}
#endif // POPUPWINDOW_QQUICKVIEW_H
