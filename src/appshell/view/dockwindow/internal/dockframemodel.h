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

#ifndef MU_DOCK_DOCKFRAMEMODEL_H
#define MU_DOCK_DOCKFRAMEMODEL_H

#include <QQuickItem>

namespace mu::dock {
class DockFrameModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQuickItem * frame READ frame WRITE setFrame NOTIFY frameChanged)
    Q_PROPERTY(bool titleBarVisible READ titleBarVisible NOTIFY titleBarVisibleChanged)
    Q_PROPERTY(QObject * navigationSection READ navigationSection NOTIFY navigationSectionChanged)
    Q_PROPERTY(QString currentDockUniqueName READ currentDockUniqueName NOTIFY currentDockUniqueNameChanged)

public:
    explicit DockFrameModel(QObject* parent = nullptr);

    QQuickItem* frame() const;
    bool titleBarVisible() const;

    QObject* navigationSection() const;

    QString currentDockUniqueName() const;

public slots:
    void setFrame(QQuickItem* item);

signals:
    void frameChanged(QQuickItem* frame);
    void titleBarVisibleChanged(bool visible);
    void navigationSectionChanged();
    void currentDockUniqueNameChanged();

private:
    void listenChangesInFrame();
    void setTitleBarVisible(bool visible);

    QObject* currentNavigationSection() const;
    void updateNavigationSection();

    QQuickItem* m_frame = nullptr;
    bool m_titleBarVisible = false;
    QObject* m_navigationSection = nullptr;
};
}

#endif // MU_DOCK_DOCKFRAMEMODEL_H
