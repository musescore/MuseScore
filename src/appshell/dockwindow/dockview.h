//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef MU_DOCK_DOCKVIEW_H
#define MU_DOCK_DOCKVIEW_H

#include <QQuickItem>
#include <QQmlComponent>
#include <QQuickView>

namespace mu {
namespace dock {
class EventsWatcher;
class DockView : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QQmlComponent * content READ content WRITE setContent NOTIFY contentChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

    Q_CLASSINFO("DefaultProperty", "content")

public:
    explicit DockView(QQuickItem* parent = nullptr);
    ~DockView();

    QQmlComponent* content() const;
    QColor color() const;

    QWidget* view() const;

public slots:
    void setContent(QQmlComponent* component);
    void setColor(QColor color);

signals:
    void sourceChanged();
    void contentChanged();
    void colorChanged(QColor color);

private slots:
    void onWidgetEvent(QEvent* e);

protected:
    virtual void onComponentCompleted() {}
    virtual void updateStyle() {}

private:

    void componentComplete() override;

    QQuickView* _view = nullptr;
    QWidget* _widget = nullptr;
    QQmlComponent* _content = nullptr;
    EventsWatcher* _eventsWatcher = nullptr;
    QColor _color;
};
}
}

#endif // MU_DOCK_DOCKVIEW_H
