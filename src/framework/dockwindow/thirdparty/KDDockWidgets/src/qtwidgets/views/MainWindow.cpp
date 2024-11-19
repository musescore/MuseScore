/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @file
 * @brief QMainWindow wrapper to enable KDDockWidgets support.
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#include "MainWindow.h"
#include "Config.h"
#include "qtwidgets/ViewFactory.h"
#include "ViewWrapper_p.h"
#include "core/DockRegistry_p.h"
#include "core/DropArea.h"
#include "core/MainWindow.h"
#include "core/MainWindow_p.h"
#include "core/Group.h"
#include "core/SideBar.h"
#include "core/Window_p.h"
#include "core/DockRegistry.h"

#include "core/Logging_p.h"

#include <QPainter>
#include <QScreen>
#include <QVBoxLayout>
#include <QWindow>
#include <QTimer>

// clazy:excludeall=ctor-missing-parent-argument,missing-qobject-macro

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtWidgets;

namespace KDDockWidgets {

char const *const s_centralWidgetObjectName = "MyCentralWidget";

class MyCentralWidget : public QWidget
{
public:
    explicit MyCentralWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setObjectName(QLatin1String(s_centralWidgetObjectName));
    }

    ~MyCentralWidget() override;
};
}

class MainWindow::Private
{
public:
    explicit Private(MainWindow *qq)
        : q(qq)
        , m_controller(qq->mainWindow())
        , m_supportsAutoHide(Config::self().flags() & Config::Flag_AutoHideSupport)
        , m_centralWidget(new MyCentralWidget(qq))
        , m_layout(new QHBoxLayout(m_centralWidget)) // 1 level of indirection so we can add some
                                                     // margins
    {
    }

    ~Private()
    {
    }

    void updateMargins()
    {
        const qreal factor = QtWidgets::logicalDpiFactor(q);
        m_layout->setContentsMargins(m_centerWidgetMargins * factor);
    }

    void setupCentralLayout()
    {
        m_layout->setSpacing(0);
        updateMargins();

        if (m_supportsAutoHide) {
            m_layout->addWidget(
                View_qt::asQWidget(m_controller->sideBar(SideBarLocation::West)->view()));
            auto innerVLayout = new QVBoxLayout();
            innerVLayout->setSpacing(0);
            innerVLayout->setContentsMargins(0, 0, 0, 0);
            innerVLayout->addWidget(
                View_qt::asQWidget(m_controller->sideBar(SideBarLocation::North)));
            innerVLayout->addWidget(View_qt::asQWidget(m_controller->layout()));
            innerVLayout->addWidget(
                View_qt::asQWidget(m_controller->sideBar(SideBarLocation::South)));
            m_layout->addLayout(innerVLayout);
            m_layout->addWidget(View_qt::asQWidget(m_controller->sideBar(SideBarLocation::East)));
        } else {
            m_layout->addWidget(View_qt::asQWidget(m_controller->layout()->view()));
        }

        q->QMainWindow::setCentralWidget(m_centralWidget);
    }

    bool onlySupportsQDockWidgets() const
    {
        return m_controller && (m_controller->options() & MainWindowOption_QDockWidgets);
    }

    bool needsManualInit() const
    {
        return m_controller && (m_controller->options() & MainWindowOption_ManualInit);
    }

    /// Sanity-check to see if someone called QMainWindow::setCentralWidget() directly
    void sanityCheckCentralWidget() const
    {
        if (auto cw = q->centralWidget()) {
            if (cw->objectName() != QLatin1String(s_centralWidgetObjectName)) {
                qWarning() << "MainWindow: Expected our own central widget, not " << cw->objectName();
            }
        }
    }

    MainWindow *const q;
    Core::MainWindow *const m_controller;
    const bool m_supportsAutoHide;
    MyCentralWidget *const m_centralWidget;
    QHBoxLayout *const m_layout;
    QMargins m_centerWidgetMargins = { 1, 5, 1, 1 };

    KDBindings::ScopedConnection groupCountChangedConnection;
};

MyCentralWidget::~MyCentralWidget() = default;

MainWindow::MainWindow(const QString &uniqueName, MainWindowOptions options,
                       QWidget *parent, Qt::WindowFlags flags)
    : View<QMainWindow>(new Core::MainWindow(this, uniqueName, options),
                        Core::ViewType::MainWindow, parent, flags)
    , MainWindowViewInterface(static_cast<Core::MainWindow *>(controller()))
    , d(new Private(this))
{
    if (options & MainWindowOption_QDockWidgets)
        return;

    // Disable QWidgetAnimator. We don't use QDockWidget, but QWidgetAnimator will appear in stack traces
    // which is unneeded.
    QMainWindow::setDockOptions({});

    m_mainWindow->init(uniqueName);

    const bool requiresManualInit = options & MainWindowOption_ManualInit;
    if (!requiresManualInit)
        d->setupCentralLayout();

    const bool isWindow = !parentWidget() || (flags & Qt::Window);
    if (isWindow) {
        // Update our margins when logical dpi changes.
        // QWidget doesn't have any screenChanged signal, so we need to use QWindow::screenChanged.
        // Note #1: Someone might be using this main window embedded into another main window, in
        // which case it will never have a QWindow, so guard it with isWindow. Note #2: We don't use
        // QWidget::isWindow() as that will always be true since QMainWindow sets it. Anyone wanting
        // or not wanting this immediate create() needs to pass a parent/flag pair that makes sense.
        // For example, some people might want to add this main window into a layout and avoid the
        // create(), so they pass a parent, with null flag.

        create(); // ensure QWindow exists
        window()->onScreenChanged(this, [](QObject *context, auto window) {
            if (auto mw = qobject_cast<MainWindow *>(context))
                mw->updateMargins(); // logical dpi might have changed
            DockRegistry::self()->dptr()->windowChangedScreen.emit(window);
        });
    }

    QTimer::singleShot(0, this, [this] {
        d->sanityCheckCentralWidget();
    });

    d->groupCountChangedConnection = m_mainWindow->d->groupCountChanged.connect([this](int count) {
        Q_EMIT groupCountChanged(count);
    });
}

MainWindow::~MainWindow()
{
    d->sanityCheckCentralWidget();
    delete d;
}

void MainWindow::setCentralWidget(QWidget *w)
{
    QMainWindow::setCentralWidget(w);
}

QMargins MainWindow::centerWidgetMargins() const
{
    return d->m_centerWidgetMargins;
}

void MainWindow::setCenterWidgetMargins(QMargins margins)
{
    if (d->m_centerWidgetMargins == margins)
        return;
    d->m_centerWidgetMargins = margins;
    d->updateMargins();
}

QRect MainWindow::centralAreaGeometry() const
{
    return centralWidget()->geometry();
}

void MainWindow::setContentsMargins(int left, int top, int right, int bottom)
{
    QMainWindow::setContentsMargins(left, top, right, bottom);
}

void MainWindow::setPersistentCentralWidget(QWidget *widget)
{
    m_mainWindow->setPersistentCentralView(QtWidgets::ViewWrapper::create(widget));
}

QWidget *MainWindow::persistentCentralWidget() const
{
    auto view = m_mainWindow->persistentCentralView();
    return View_qt::asQWidget(view.get());
}

QHBoxLayout *MainWindow::internalLayout() const
{
    return d->m_layout;
}

void MainWindow::updateMargins()
{
    d->updateMargins();
}

void MainWindow::setCentralWidget_legacy(QWidget *widget)
{
    if (d->onlySupportsQDockWidgets()) {
        QMainWindow::setCentralWidget(widget);
    } else {
        qFatal("MainWindow::setCentralWidget_legacy: Legacy QDockWidgets are not supported without MainWindowOption_QDockWidgets");
    }
}

void MainWindow::addDockWidget_legacy(Qt::DockWidgetArea area, QDockWidget *dockwidget)
{
    if (d->onlySupportsQDockWidgets()) {
        QMainWindow::addDockWidget(area, dockwidget);
    } else {
        qFatal("MainWindow::addDockWidget_legacy: Legacy QDockWidgets are not supported without MainWindowOption_QDockWidgets");
    }
}

void MainWindow::addDockWidget_legacy(Qt::DockWidgetArea area, QDockWidget *dockwidget,
                                      Qt::Orientation orientation)
{
    if (d->onlySupportsQDockWidgets()) {
        QMainWindow::addDockWidget(area, dockwidget, orientation);
    } else {
        qFatal("MainWindow::addDockWidget_legacy: Legacy QDockWidgets are not supported without MainWindowOption_QDockWidgets");
    }
}

bool MainWindow::restoreDockWidget_legacy(QDockWidget *dockwidget)
{
    if (d->onlySupportsQDockWidgets()) {
        return QMainWindow::restoreDockWidget(dockwidget);
    } else {
        qFatal("MainWindow::restoreDockWidget_legacy: Legacy QDockWidgets are not supported without MainWindowOption_QDockWidgets");
        return {};
    }
}

void MainWindow::removeDockWidget_legacy(QDockWidget *dockwidget)
{
    if (d->onlySupportsQDockWidgets()) {
        QMainWindow::removeDockWidget(dockwidget);
    } else {
        qFatal("MainWindow::removeDockWidget_legacy: Legacy QDockWidgets are not supported without MainWindowOption_QDockWidgets");
    }
}

Qt::DockWidgetArea MainWindow::dockWidgetArea_legacy(QDockWidget *dockwidget) const
{
    if (d->onlySupportsQDockWidgets()) {
        return QMainWindow::dockWidgetArea(dockwidget);
    } else {
        qFatal("MainWindow::dockWidgetArea_legacy: Legacy QDockWidgets are not supported without MainWindowOption_QDockWidgets");
        return {};
    }
}

void MainWindow::resizeDocks_legacy(const QList<QDockWidget *> &docks,
                                    const QList<int> &sizes, Qt::Orientation orientation)
{
    if (d->onlySupportsQDockWidgets()) {
        QMainWindow::resizeDocks(docks, sizes, orientation);
    } else {
        qFatal("MainWindow::resizeDocks_legacy: Legacy QDockWidgets are not supported without MainWindowOption_QDockWidgets");
    }
}

void MainWindow::tabifyDockWidget_legacy(QDockWidget *first, QDockWidget *second)
{
    if (d->onlySupportsQDockWidgets()) {
        QMainWindow::tabifyDockWidget(first, second);
    } else {
        qFatal("MainWindow::tabifyDockWidget_legacy: Legacy QDockWidgets are not supported without MainWindowOption_QDockWidgets");
    }
}

QList<QDockWidget *> MainWindow::tabifiedDockWidgets_legacy(QDockWidget *dockwidget) const
{
    if (d->onlySupportsQDockWidgets()) {
        return QMainWindow::tabifiedDockWidgets(dockwidget);
    } else {
        qFatal("MainWindow::tabifiedDockWidgets_legacy: Legacy QDockWidgets are not supported without MainWindowOption_QDockWidgets");
        return {};
    }
}

void MainWindow::splitDockWidget_split_legacy(QDockWidget *after, QDockWidget *dockwidget,
                                              Qt::Orientation orientation)
{
    if (d->onlySupportsQDockWidgets()) {
        QMainWindow::splitDockWidget(after, dockwidget, orientation);
    } else {
        qFatal("MainWindow::splitDockWidget_split_legacy: Legacy QDockWidgets are not supported without MainWindowOption_QDockWidgets");
    }
}

void MainWindow::manualInit()
{
    if (d->needsManualInit()) {
        delete centralWidget();
        d->setupCentralLayout();
    } else {
        qFatal("MainWindow::manualInit requires MainWindowOption_ManualInit");
    }
}
