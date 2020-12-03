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

#include "dockview.h"

#include <QQmlEngine>
#include <QQmlContext>
#include <QWidget>

#include "log.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

using namespace mu::dock;

DockView::DockView(QQuickItem* parent)
    : QQuickItem(parent)
{
    setFlag(QQuickItem::ItemHasContents, true);

    connect(this, &DockView::colorChanged, this, &DockView::updateStyle);
}

DockView::~DockView()
{
}

void DockView::componentComplete()
{
    QQuickItem::componentComplete();

    if (objectName().isEmpty()) {
        LOGE() << "not set objectName for " << this;
        Q_ASSERT(!objectName().isEmpty());
    }

    if (_content) {
        QQmlEngine* engine = nullptr;
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
        engine = framework::ioc()->resolve<framework::IUiEngine>("appshell")->qmlEngine();
#else
        engine = _content->engine();
#endif

        _view = new QQuickView(engine, nullptr);
        _view->setResizeMode(QQuickView::SizeRootObjectToView);
        _widget = QWidget::createWindowContainer(_view, nullptr, Qt::Widget);
        _widget->setObjectName("w_" + objectName());

        _view->resize(width(), height());
        _widget->resize(_view->size());

        QQmlContext* ctx = QQmlEngine::contextForObject(this);
        QQuickItem* obj = qobject_cast<QQuickItem*>(_content->create(ctx));
        _view->setContent(QUrl(), _content, obj);
    }

    onComponentCompleted();
}

void DockView::onWidgetEvent(QEvent* e)
{
    if (QEvent::Resize == e->type()) {
        QResizeEvent* re = static_cast<QResizeEvent*>(e);
        setSize(QSizeF(re->size()));
    }
}

QWidget* DockView::view() const
{
    return _widget;
}

QQmlComponent* DockView::content() const
{
    return _content;
}

void DockView::setContent(QQmlComponent* component)
{
    if (_content == component) {
        return;
    }

    _content = component;
    emit contentChanged();
}

QColor DockView::color() const
{
    return _color;
}

void DockView::setColor(QColor color)
{
    if (_color == color) {
        return;
    }

    _color = color;
    emit colorChanged(_color);
}
