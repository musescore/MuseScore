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

    Q_PROPERTY(DockLocation location READ location WRITE setLocation NOTIFY locationChanged)

public:
    explicit DockBase(QQuickItem* parent = nullptr);

    enum class DockLocation {
        Undefined = -1,
        Left,
        Right,
        Center,
        Top,
        Bottom
    };
    Q_ENUM(DockLocation)

    QString title() const;

    int minimumWidth() const;
    int minimumHeight() const;
    int maximumWidth() const;
    int maximumHeight() const;
    QSize preferredSize() const;

    Qt::DockWidgetAreas allowedAreas() const;

    bool floating() const;

    virtual void init();

    bool isShown() const;
    void show();
    void hide();
    void toggle();

    DockLocation location() const;

public slots:
    void setTitle(const QString& title);

    virtual void setMinimumWidth(int width);
    virtual void setMinimumHeight(int height);
    virtual void setMaximumWidth(int width);
    virtual void setMaximumHeight(int height);

    void setAllowedAreas(Qt::DockWidgetAreas areas);

    void setFloating(bool floating);

    void setLocation(DockLocation location);

signals:
    void titleChanged();
    void minimumSizeChanged();
    void maximumSizeChanged();
    void allowedAreasChanged();

    void floatingChanged();

    void locationChanged(DockLocation location);

protected:
    friend class DockWindow;

    virtual DockType type() const;

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
    DockLocation m_location = DockLocation::Undefined;
};
}

#endif // MU_DOCK_DOCKBASE_H
