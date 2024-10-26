/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "View.h"
#include "../Window_p.h"
#include "core/View_p.h"
#include "core/layouting/Item_p.h"
#include "ViewWrapper_p.h"

#include <QTabBar>
#include <QTabWidget>
#include <QMainWindow>
#include <QRubberBand>
#include <QLineEdit>

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtWidgets;

// clazy:excludeall=missing-qobject-macro

template<>
View<QWidget>::View(Core::Controller *controller, Core::ViewType type,
                    QWidget *parent, Qt::WindowFlags windowFlags)
    : QWidget(parent, windowFlags)
    , View_qt(controller, type, this)
{
}

template<>
View<QTabBar>::View(Core::Controller *controller, Core::ViewType type,
                    QWidget *parent, Qt::WindowFlags)
    : QTabBar(parent)
    , View_qt(controller, type, this)
{
}

template<>
View<QTabWidget>::View(Core::Controller *controller, Core::ViewType type,
                       QWidget *parent, Qt::WindowFlags)
    : QTabWidget(parent)
    , View_qt(controller, type, this)
{
}

template<>
View<QMainWindow>::View(Core::Controller *controller, Core::ViewType type,
                        QWidget *parent, Qt::WindowFlags)
    : QMainWindow(parent)
    , View_qt(controller, type, this)
{
}

template<>
View<QRubberBand>::View(Core::Controller *controller, Core::ViewType type,
                        QWidget *parent, Qt::WindowFlags)
    : QRubberBand(QRubberBand::Rectangle, parent)
    , View_qt(controller, type, this)
{
}

template<>
View<QLineEdit>::View(Core::Controller *controller, Core::ViewType type,
                      QWidget *parent, Qt::WindowFlags)
    : QLineEdit(parent)
    , View_qt(controller, type, this)
{
}

template<class T>
std::shared_ptr<Core::Window> View<T>::window() const
{
    if (QWidget *root = QWidget::window()) {
        if (root->window()) {
            return std::shared_ptr<Core::Window>(new Window(root));
        }
    }

    return {};
}

template<class T>
void View<T>::setMaximumSize(QSize sz)
{
    if (sz != QWidget::maximumSize()) {
        T::setMaximumSize(sz);
        d->layoutInvalidated.emit();
    }
}

template<class T>
bool View<T>::event(QEvent *e)
{
    if (e->type() == QEvent::LayoutRequest)
        d->layoutInvalidated.emit();

    return T::event(e);
}

template<class T>
void View<T>::closeEvent(QCloseEvent *ev)
{
    d->requestClose(ev);
}

template<class T>
void View<T>::setMinimumSize(QSize sz)
{
    if (sz != QWidget::minimumSize()) {
        QWidget::setMinimumSize(sz);
        d->layoutInvalidated.emit();
    }
}


template<class T>
std::shared_ptr<Core::View> View<T>::childViewAt(QPoint localPos) const
{
    if (QWidget *child = QWidget::childAt(localPos))
        return ViewWrapper::create(child);

    return {};
}

template<class T>
std::shared_ptr<Core::View> View<T>::rootView() const
{
    if (auto w = QWidget::window()) {
        return ViewWrapper::create(w);
    }

    return {};
}

template<class T>
std::shared_ptr<Core::View> View<T>::parentView() const
{
    if (QWidget *p = QWidget::parentWidget()) {
        return ViewWrapper::create(p);
    }

    return {};
}

template<class T>
std::shared_ptr<Core::View> View<T>::asWrapper()
{
    return ViewWrapper::create(this);
}

/* static */
template<class T>
QVector<std::shared_ptr<Core::View>> View<T>::childViewsFor(const QWidget *parent)
{
    QVector<std::shared_ptr<Core::View>> result;
    const QObjectList &children = parent->children();
    result.reserve(children.size());
    for (QObject *child : children) {
        if (auto widget = qobject_cast<QWidget *>(child)) {
            result.push_back(ViewWrapper::create(widget));
        }
    }

    return result;
}

namespace KDDockWidgets::QtWidgets {

template class View<QWidget>;
template class View<QMainWindow>;
template class View<QLineEdit>;
template class View<QRubberBand>;
template class View<QTabWidget>;
template class View<QTabBar>;

QSize boundedMaxSize(QSize min, QSize max)
{
    // Max should be bigger than min, but not bigger than the hardcoded max
    max = max.boundedTo(Core::Item::hardcodedMaximumSize);

    // 0 interpreted as not having max
    if (max.width() <= 0)
        max.setWidth(Core::Item::hardcodedMaximumSize.width());
    if (max.height() <= 0)
        max.setHeight(Core::Item::hardcodedMaximumSize.height());

    max = max.expandedTo(min);

    return max;
}

}
