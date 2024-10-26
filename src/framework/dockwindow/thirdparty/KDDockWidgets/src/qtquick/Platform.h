/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/KDDockWidgets.h"
#include "kddockwidgets/qtcommon/Platform.h"

#include <QPointer>

QT_BEGIN_NAMESPACE
class QQmlEngine;
class QQuickItem;
QT_END_NAMESPACE

namespace KDDockWidgets {

class QtQuickHelpers;

namespace Core {
class DockWidget;
}

namespace QtQuick {

class ViewFactory;

/// @brief implements functions specific to a particular platform
/// A platform can be for example qtwidgets, qtquick, etc.
class DOCKS_EXPORT Platform : public QtCommon::Platform_qt
{
public:
    Platform();
    ~Platform() override;
    const char *name() const override;
    std::shared_ptr<Core::View> qobjectAsView(QObject *) const override;
    std::shared_ptr<Core::Window> windowFromQWindow(QWindow *) const override;
    Core::ViewFactory *createDefaultViewFactory() override;
    QtQuick::ViewFactory *viewFactory() const;
    std::shared_ptr<Core::Window> windowAt(QPoint globalPos) const override;
    int screenNumberForView(Core::View *) const override;
    QSize screenSizeFor(Core::View *) const override;
    void setQmlEngine(QQmlEngine *);
    QQmlEngine *qmlEngine() const;
    Core::View *createView(Core::Controller *controller, Core::View *parent = nullptr) const override;
    bool usesFallbackMouseGrabber() const override;
    bool inDisallowedDragView(QPoint globalPos) const override;
    void ungrabMouse() override;
    static Platform *instance();

    static Core::DockWidget *dockWidgetForItem(QQuickItem *);

#ifdef DOCKS_TESTING_METHODS
    explicit Platform(int &argc, char **argv, bool defaultToOffscreenQPA);
    void tests_initPlatform_impl() override;
    void tests_deinitPlatform_impl() override;
    Core::View *tests_createView(Core::CreateViewOptions, Core::View *parent = nullptr) override;
    std::shared_ptr<Core::Window> tests_createWindow() override;
    Core::View *tests_createFocusableView(Core::CreateViewOptions, Core::View *parent = nullptr) override;
    Core::View *tests_createNonClosableView(Core::View *parent = nullptr) override;
    Core::MainWindow *
    createMainWindow(const QString &uniqueName, Core::CreateViewOptions,
                     MainWindowOptions options = MainWindowOption_HasCentralFrame,
                     Core::View *parent = nullptr, Qt::WindowFlags = {}) const override;
#endif
private:
    void init();
    QPointer<QQmlEngine> m_qmlEngine;
    QtQuickHelpers *const m_qquickHelpers;
    Q_DISABLE_COPY(Platform)
};

}

inline QtQuick::Platform *plat()
{
    return static_cast<QtQuick::Platform *>(Core::Platform::instance());
}

}
