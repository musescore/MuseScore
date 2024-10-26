/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "kddockwidgets/KDDockWidgets.h"
#include "Platform.h"
#include "views/ViewWrapper_p.h"
#include "views/View.h"
#include "kddockwidgets/core/MainWindow.h"
#include "qtwidgets/views/MainWindow.h"
#include "Window_p.h"

#include <QStyleFactory>
#include <QApplication>
#include <QLineEdit>
#include <QWidget>

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtWidgets;

#ifdef DOCKS_TESTING_METHODS

namespace KDDockWidgets::QtWidgets {
class TestView : public QtWidgets::View<QWidget>
{
    Q_OBJECT
public:
    explicit TestView(Core::CreateViewOptions opts, QWidget *parent)
        : QtWidgets::View<QWidget>(nullptr, Core::ViewType::None, parent)
        , m_opts(opts)
    {
        create();
        setMinimumSize(opts.minSize.boundedTo(opts.maxSize));
    }

    QSize sizeHint() const override
    {
        return m_opts.sizeHint;
    }

    QSize maxSizeHint() const override
    {
        return m_opts.maxSize.boundedTo(QtWidgets::View<QWidget>::maximumSize());
    }

private:
    Core::CreateViewOptions m_opts;
};

class FocusableTestView : public QtWidgets::View<QLineEdit>
{
    Q_OBJECT
public:
    explicit FocusableTestView(Core::CreateViewOptions opts, QWidget *parent)
        : QtWidgets::View<QLineEdit>(nullptr, Core::ViewType::None, parent)
        , m_opts(opts)
    {
        create();
        setMinimumSize(opts.minSize.boundedTo(opts.maxSize));
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    }

    QSize sizeHint() const override
    {
        return m_opts.sizeHint;
    }

    QSize maxSizeHint() const override
    {
        return m_opts.maxSize.boundedTo(QtWidgets::View<QLineEdit>::maximumSize());
    }

private:
    Core::CreateViewOptions m_opts;
};

class NonClosableTestView : public QtWidgets::View<QWidget>
{
    Q_OBJECT
public:
    explicit NonClosableTestView(QWidget *parent)
        : QtWidgets::View<QWidget>(nullptr, Core::ViewType::None, parent)
    {
        create();
    }

    void closeEvent(QCloseEvent *ev) override
    {
        ev->ignore(); // don't allow to close
    }
};

}

void Platform::tests_initPlatform_impl()
{
    Platform_qt::tests_initPlatform_impl();
}

void Platform::tests_deinitPlatform_impl()
{
    tests_wait(500); // Some windows are currently being destroyed

    qDeleteAll(qApp->topLevelWidgets());

    Platform_qt::tests_deinitPlatform_impl();
}

Core::View *Platform::tests_createView(Core::CreateViewOptions opts, Core::View *parent)
{
    QWidget *parentWidget = QtCommon::View_qt::asQWidget(parent);

    auto newWidget = new TestView(opts, parentWidget);
    if (opts.isVisible)
        newWidget->show();

    return newWidget;
}

Core::View *Platform::tests_createFocusableView(Core::CreateViewOptions opts, Core::View *parent)
{
    QWidget *parentWidget = QtCommon::View_qt::asQWidget(parent);

    auto newWidget = new FocusableTestView(opts, parentWidget);
    if (opts.isVisible)
        newWidget->show();

    return newWidget;
}

Core::View *Platform::tests_createNonClosableView(Core::View *parent)
{
    QWidget *parentWidget = QtCommon::View_qt::asQWidget(parent);
    auto newWidget = new NonClosableTestView(parentWidget);

    return newWidget;
}

Core::MainWindow *Platform::createMainWindow(const QString &uniqueName,
                                             Core::CreateViewOptions opts,
                                             MainWindowOptions options,
                                             Core::View *parent,
                                             Qt::WindowFlags flags) const
{
    auto view = new QtWidgets::MainWindow(
        uniqueName, options,
        parent ? static_cast<QtWidgets::View<QMainWindow> *>(parent) : nullptr, flags);

    if (opts.isVisible)
        view->show();
    view->resize(opts.size);

    return view->mainWindow();
}

std::shared_ptr<Core::Window> Platform::tests_createWindow()
{
    auto window = new Window(new QWidget());
    window->setVisible(true);
    return std::shared_ptr<Core::Window>(window);
}

#endif

#include "TestHelpers.moc"
