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

#ifndef MU_DOCK_MAINWINDOWPROVIDER_H
#define MU_DOCK_MAINWINDOWPROVIDER_H

#include "framework/ui/imainwindow.h"

#include <QObject>

namespace mu::dock {
class MainWindowProvider : public QObject, public ui::IMainWindow
{
    Q_OBJECT

    Q_PROPERTY(QWindow * window READ qWindow WRITE setWindow NOTIFY windowChanged)

public:
    explicit MainWindowProvider(QObject* parent = nullptr);

    QMainWindow* qMainWindow() const override;
    QWindow* qWindow() const override;

    void requestShowOnBack() override;
    void requestShowOnFront() override;

    bool isFullScreen() const override;
    void toggleFullScreen() override;
    const QScreen* screen() const override;

    void requestChangeToolBarOrientation(const QString& toolBarName, framework::Orientation orientation) override;
    async::Channel<QString, framework::Orientation> changeToolBarOrientationRequested() const override;

    void requestShowToolBarDockingHolder(const QPoint& globalPos) override;
    async::Channel<QPoint> showToolBarDockingHolderRequested() const override;

    void requestShowPanelDockingHolder(const QPoint& globalPos) override;
    async::Channel<QPoint> showPanelDockingHolderRequested() const override;

    void requestHideAllDockingHolders() override;
    async::Notification hideAllDockingHoldersRequested() const override;

signals:
    void windowChanged();

protected:
    virtual void init();

    QWindow* m_window = nullptr;

private:
    void setWindow(QWindow* window); // Should only be used from QML

    async::Channel<QString, framework::Orientation> m_dockOrientationChanged;
    async::Channel<QPoint> m_showToolBarDockingHolderRequested;
    async::Channel<QPoint> m_showPanelDockingHolderRequested;
    async::Notification m_hideAllHoldersRequested;
};
}

#endif // MU_DOCK_MAINWINDOWPROVIDER_H
