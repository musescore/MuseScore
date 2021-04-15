/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/

#ifndef MU_DOCK_DOCKVIEW_H
#define MU_DOCK_DOCKVIEW_H

#include "modularity/ioc.h"
#include "framework/ui/iuiconfiguration.h"

#include <QQuickItem>
#include <QQmlComponent>
#include <QQuickView>

namespace mu::dock {
class EventsWatcher;
class DockView : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QQmlComponent * content READ content WRITE setContent NOTIFY contentChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleEdited)

    Q_CLASSINFO("DefaultProperty", "content")

    INJECT(appshell, ui::IUiConfiguration, uiConfiguration)

public:
    explicit DockView(QQuickItem* parent = nullptr);

    Q_INVOKABLE void forceActiveFocus();

    QQmlComponent* content() const;
    QColor color() const;
    QColor borderColor() const;
    virtual bool visible() const;

    QWidget* view() const;

public slots:
    void setContent(QQmlComponent* component);
    void setColor(QColor color);
    void setBorderColor(QColor color);
    virtual void setVisible(bool value);

signals:
    void sourceChanged();
    void contentChanged();
    void colorChanged(QColor color);
    void borderColorChanged(QColor color);
    void positionChanged(const QPointF& pos);
    void visibleEdited(bool visible);

protected slots:
    virtual void onWidgetEvent(QEvent* event);

protected:
    virtual void onComponentCompleted() {}
    virtual void updateStyle() {}

private:
    void componentComplete() override;

    QQuickView* m_view = nullptr;
    QWidget* m_widget = nullptr;
    QQmlComponent* m_content = nullptr;
    EventsWatcher* m_eventsWatcher = nullptr;
    QColor m_color;
    QColor m_borderColor;
};
}

#endif // MU_DOCK_DOCKVIEW_H
