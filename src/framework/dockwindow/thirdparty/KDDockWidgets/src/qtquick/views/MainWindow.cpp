/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "MainWindow.h"
#include "core/Layout.h"
#include "core/MainWindow.h"
#include "core/Window_p.h"
#include "core/Logging_p.h"
#include "core/DockRegistry.h"
#include "core/layouting/Item_p.h"

#include "kddockwidgets/qtquick/Platform.h"
#include "kddockwidgets/qtquick/views/View.h"

#include <QQuickItem>
#include <QDebug>
#include <QTimer>

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtQuick;

namespace KDDockWidgets {
class MainWindow::Private
{
public:
    explicit Private(MainWindow *qq)
        : q(qq)
    {
    }

    void onLayoutGeometryUpdated()
    {
        const QSize minSz = q->minSize();
        const bool mainWindowIsTooSmall = minSz.expandedTo(q->Core::View::size()) != q->Core::View::size();
        if (mainWindowIsTooSmall) {
            if (q->isRootView()) {
                // If we're a top-level, let's go ahead and resize the QWindow
                // any other case is too complex for QtQuick as there's no layout propagation.
                q->window()->resize(minSz.width(), minSz.height());
            }
        }
    }

    MainWindow *const q;
    QMetaObject::Connection layoutGeometryChangedConnection;
};
}


MainWindow::MainWindow(const QString &uniqueName, MainWindowOptions options,
                       QQuickItem *parent, Qt::WindowFlags flags)
    : View(new Core::MainWindow(this, uniqueName, options), Core::ViewType::MainWindow, parent,
           flags)
    , MainWindowViewInterface(static_cast<Core::MainWindow *>(View::controller()))
    , d(new Private(this))
{
    m_mainWindow->init(uniqueName);
    makeItemFillParent(this);

    Core::Layout *lw = m_mainWindow->layout();
    auto layoutView = asView_qtquick(lw->view());
    makeItemFillParent(layoutView);

    // MainWindowQuick has the same constraints as Layout, so just forward the signal
    d->layoutGeometryChangedConnection = connect(layoutView, &View::geometryUpdated, this,
                                                 [this] {
                Q_EMIT geometryUpdated();
                d->onLayoutGeometryUpdated(); });

    {
        // This block silences a benign layouting constraints warning.
        // During initialization, QtQuick will evaluate the width and height bindings separately,
        // meaning our first Layout::setSize() might have height=0 still, as we're processing the
        // width binding.

        auto timer = new QTimer(this);
        timer->setSingleShot(true);
        timer->start();
        Core::Item::s_silenceSanityChecks = true;
        timer->callOnTimeout([] { Core::Item::s_silenceSanityChecks = false; });
    }
}

MainWindow::~MainWindow()
{
    // early disconnect to avoid rentrancy
    disconnect(d->layoutGeometryChangedConnection);

    if (isRootView()) {
        if (auto window = this->window()) {
            QObject::setParent(nullptr);
            window->destroy();
        }
    }

    delete d;
}

QSize MainWindow::minSize() const
{
    return m_mainWindow->layout()->layoutMinimumSize();
}

QSize MainWindow::maxSizeHint() const
{
    return m_mainWindow->layout()->layoutMaximumSizeHint();
}

QMargins MainWindow::centerWidgetMargins() const
{
    qDebug() << Q_FUNC_INFO << "SideBar hasn't been implemented yet";
    return {};
}

QRect MainWindow::centralAreaGeometry() const
{
    qFatal("Not implemented");
    return {};
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void MainWindow::setContentsMargins(int left, int top, int right, int bottom)
{
    Q_UNUSED(left);
    Q_UNUSED(right);
    Q_UNUSED(top);
    Q_UNUSED(bottom);
    qDebug() << Q_FUNC_INFO << "not implemented";
}

void MainWindow::setPersistentCentralView(const QString &qmlFilename)
{
    QQuickItem *guest = QtQuick::View::createItem(plat()->qmlEngine(), qmlFilename);
    if (guest) {
        // kddw deals in Core::View, so put it inside one
        auto wrapper = QtQuick::View::asQQuickWrapper(guest);
        m_mainWindow->setPersistentCentralView(wrapper);
    } else {
        KDDW_ERROR("setPersistentCentralView: Failed to create item for {}", qmlFilename);
    }
}
