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

#include <QObject>
#include <QWindow>

#include "modularity/ioc.h"
#include "../iinteractiveprovider.h"

#include "framework/ui/imainwindow.h"

namespace mu::ui {
class MainWindowProvider : public QObject, public IMainWindow
{
    Q_OBJECT

    Q_PROPERTY(QWindow * window READ qWindow WRITE setWindow NOTIFY windowChanged)
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
    Q_PROPERTY(bool fileModified READ fileModified WRITE setFileModified NOTIFY fileModifiedChanged)

    INJECT(ui, IInteractiveProvider, interactiveProvider)

public:
    explicit MainWindowProvider(QObject* parent = nullptr);

    QWindow* qWindow() const override;

    QString filePath() const;
    virtual bool fileModified() const;

    void requestShowOnBack() override;
    void requestShowOnFront() override;

    bool isFullScreen() const override;
    void toggleFullScreen() override;
    QScreen* screen() const override;

signals:
    void windowChanged();
    void filePathChanged();
    void fileModifiedChanged();

protected:
    virtual void init();

    QWindow* m_window = nullptr;

    QWindow::Visibility m_windowVisibility = QWindow::AutomaticVisibility;

private slots: // Should only be used from QML
    void setWindow(QWindow* window);
    void setFilePath(const QString& filePath);
    virtual void setFileModified(bool modified);
};
}

#endif // MU_DOCK_MAINWINDOWPROVIDER_H
