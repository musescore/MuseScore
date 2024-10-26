/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Window_p.h"
#include "views/View.h"

#include <QWindow>
#include <QQuickWindow>
#include <QDebug>

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtQuick;

Window::~Window()
{
}

inline Core::View *topMostKDDWView(QQuickItem *parent)
{
    if (!parent)
        return {};

    if (auto v = qobject_cast<QtQuick::View *>(parent))
        return v;

    const auto children = parent->childItems();
    for (QQuickItem *item : children) {
        if (auto v = topMostKDDWView(item))
            return v;
    }

    return nullptr;
}

std::shared_ptr<Core::View> Window::rootView() const
{
    if (auto quickwindow = qobject_cast<QQuickWindow *>(m_window)) {
        auto contentItem = quickwindow->contentItem();
        if (Core::View *view = topMostKDDWView(contentItem)) {
            // This block is for retrocompatibility with 1.x. For QtQuick the topmost "widget" is a
            // KDDW known widget and not any arbitrary user QtQuickItem.
            return view->asWrapper();
        } else {
            const auto children = contentItem->childItems();
            Q_ASSERT(!children.isEmpty());
            return QtQuick::View::asQQuickWrapper(children.first());
        }
    } else {
        qWarning() << Q_FUNC_INFO << "Expected QQuickView";
    }

    qWarning() << Q_FUNC_INFO << "Window does not have a root";
    return {};
}

Core::Window::Ptr Window::transientParent() const
{
    if (QWindow *w = m_window->transientParent())
        return Core::Window::Ptr(new Window(w));

    return nullptr;
}

void Window::setVisible(bool is)
{
    QtCommon::Window::setVisible(is);
    if (auto root = rootView())
        root->setVisible(is);
}

bool Window::supportsHonouringLayoutMinSize() const
{
    // If this method returns true, then Item.cpp will be strict and issue qWarnings
    // whenever the window is resized lower than the layout's min-size.

    if (auto view = rootView()) {
        // For floating window we have full control. While for anything else we don't know the
        // disposition of the users main.qml
        return view->is(Core::ViewType::FloatingWindow);
    }

    return false;
}
