/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "kddockwidgets/KDDockWidgets.h"
#include "Platform.h"
#include "views/View.h"
#include "kddockwidgets/qtquick/views/MainWindow.h"
#include "kddockwidgets/core/MainWindow.h"
#include "Helpers_p.h"
#include "core/View_p.h"
#include "core/Logging_p.h"

#include <QGuiApplication>
#include <QQmlEngine>
#include <QQuickStyle>
#include <QQuickItem>
#include <QQuickView>
#include <QtTest/QTest>

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtQuick;

#ifdef DOCKS_TESTING_METHODS

namespace KDDockWidgets::QtQuick {

class TestView : public QtQuick::View
{
public:
    explicit TestView(Core::CreateViewOptions opts, QQuickItem *parent)
        : QtQuick::View(nullptr, Core::ViewType::None, parent)
        , m_opts(opts)
    {
        setMinimumSize(opts.minSize);
        setMaximumSize(opts.maxSize);
    }

    ~TestView();

private:
    Core::CreateViewOptions m_opts;
};

TestView::~TestView() = default;

inline QCoreApplication *createCoreApplication(int &argc, char **argv, bool defaultToOffscreenQPA)
{
    if (defaultToOffscreenQPA)
        QtCommon::Platform_qt::maybeSetOffscreenQPA(argc, argv);

    return new QGuiApplication(argc, argv);
}

}

Platform::Platform(int &argc, char **argv, bool defaultToOffscreenQPA)
    : Platform_qt(createCoreApplication(argc, argv, defaultToOffscreenQPA))
    , m_qquickHelpers(new QtQuickHelpers())
{
    init();
}

void Platform::tests_initPlatform_impl()
{
    Platform_qt::tests_initPlatform_impl();

    QQuickStyle::setStyle(QStringLiteral("Material")); // so we don't load KDE plugins
    plat()->setQmlEngine(new QQmlEngine());
}

void Platform::tests_deinitPlatform_impl()
{
    delete m_qmlEngine;
    auto windows = qGuiApp->topLevelWindows();
    while (!windows.isEmpty()) {
        delete windows.first();
        windows = qGuiApp->topLevelWindows();
    }

    Platform_qt::tests_deinitPlatform_impl();
}

Core::View *Platform::tests_createView(Core::CreateViewOptions opts, Core::View *parent)
{
    auto parentItem = parent ? QtQuick::asQQuickItem(parent) : nullptr;
    auto newItem = new TestView(opts, parentItem);

    if (!parentItem && opts.createWindow) {
        auto view = new QQuickView(m_qmlEngine, nullptr);
        view->resize(QSize(800, 800));

        newItem->QQuickItem::setParentItem(view->contentItem());
        newItem->QQuickItem::setParent(view->contentItem());
        if (opts.isVisible)
            newItem->QtQuick::View::setVisible(true);

        QTest::qWait(100); // the root object gets sized delayed
    }

    return newItem;
}

Core::View *Platform::tests_createFocusableView(Core::CreateViewOptions opts, Core::View *parent)
{
    auto view = tests_createView(opts, parent);
    view->setFocusPolicy(Qt::StrongFocus);

    return view;
}

Core::View *Platform::tests_createNonClosableView(Core::View *parent)
{
    Core::CreateViewOptions opts;
    opts.isVisible = true;
    auto view = tests_createView(opts, parent);
    view->d->closeRequested.connect([](QCloseEvent *ev) { ev->ignore(); });

    return view;
}

Core::MainWindow *Platform::createMainWindow(const QString &uniqueName,
                                             Core::CreateViewOptions viewOpts,
                                             MainWindowOptions options, Core::View *parent,
                                             Qt::WindowFlags flags) const
{
    QQuickItem *parentItem = QtQuick::asQQuickItem(parent);

    if (!parentItem) {
        auto view = new QQuickView(m_qmlEngine, nullptr);
        view->resize(viewOpts.size);

        view->setResizeMode(QQuickView::SizeRootObjectToView);
        view->setSource(QUrl(QStringLiteral("qrc:/main.qml")));

        if (viewOpts.isVisible)
            view->show();

        parentItem = view->rootObject();
        Platform::instance()->tests_wait(100); // the root object gets sized delayed
    }

    auto view = new QtQuick::MainWindow(uniqueName, options, parentItem, flags);

    return view->mainWindow();
}

std::shared_ptr<Core::Window> Platform::tests_createWindow()
{
    Core::CreateViewOptions viewOpts;
    viewOpts.isVisible = true;
    static int id = 0;
    id++;
    auto mainWindow = createMainWindow(QStringLiteral("testWindow-%1").arg(id), viewOpts);
    return mainWindow->view()->window();
}


#endif
