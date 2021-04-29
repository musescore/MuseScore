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
#ifndef MU_DOCK_DOCKBASE_H
#define MU_DOCK_DOCKBASE_H

#include <QQuickItem>

#include "../docktypes.h"

namespace KDDockWidgets {
class DockWidgetQuick;
}

namespace mu::dock {
class DockBase : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)

    Q_PROPERTY(int minimumWidth READ minimumWidth WRITE setMinimumWidth NOTIFY minimumSizeChanged)
    Q_PROPERTY(int minimumHeight READ minimumHeight WRITE setMinimumHeight NOTIFY minimumSizeChanged)
    Q_PROPERTY(int maximumWidth READ maximumWidth WRITE setMaximumWidth NOTIFY maximumSizeChanged)
    Q_PROPERTY(int maximumHeight READ maximumHeight WRITE setMaximumHeight NOTIFY maximumSizeChanged)

    Q_PROPERTY(Qt::DockWidgetAreas allowedAreas READ allowedAreas WRITE setAllowedAreas NOTIFY allowedAreasChanged)

    Q_PROPERTY(bool floating READ floating NOTIFY floatingChanged)

public:
    explicit DockBase(QQuickItem* parent = nullptr);

    QString title() const;

    int minimumWidth() const;
    int minimumHeight() const;
    int maximumWidth() const;
    int maximumHeight() const;

    QSize preferredSize() const;

    Qt::DockWidgetAreas allowedAreas() const;

    virtual void init();

    void close();

    bool floating() const;

public slots:
    void setTitle(const QString& title);

    virtual void setMinimumWidth(int width);
    virtual void setMinimumHeight(int height);
    virtual void setMaximumWidth(int width);
    virtual void setMaximumHeight(int height);

    void setAllowedAreas(Qt::DockWidgetAreas areas);

    void setFloating(bool floating);

signals:
    void titleChanged();
    void minimumSizeChanged();
    void maximumSizeChanged();
    void allowedAreasChanged();

    void closed();

    void floatingChanged();

protected:
    friend class DockWindow;

    virtual DockType type() const = 0;

    void componentComplete() override;

    KDDockWidgets::DockWidgetQuick* dockWidget() const;

private slots:
    void resize();

private:
    void applySizeConstraints();
    void listenFloatingChanges();

    int m_minimumWidth = 0;
    int m_minimumHeight = 0;
    int m_maximumWidth = 0;
    int m_maximumHeight = 0;

    QString m_title;
    Qt::DockWidgetAreas m_allowedAreas = Qt::NoDockWidgetArea;
    KDDockWidgets::DockWidgetQuick* m_dockWidget = nullptr;
    bool m_floating = false;
};
}

#endif // MU_DOCK_DOCKBASE_H
