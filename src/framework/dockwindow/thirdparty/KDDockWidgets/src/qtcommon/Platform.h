/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/core/Platform.h"

QT_BEGIN_NAMESPACE
class QCoreApplication;
QT_END_NAMESPACE

namespace KDDockWidgets {

namespace Core {
class Window;
class View;
}

namespace QtCommon {

/// @brief Baseclass platform for Qt based platform
/// Useful since QtWidgets and QtQuick share some similarities
class DOCKS_EXPORT Platform_qt : public Core::Platform
{
public:
    Platform_qt();
    ~Platform_qt() override;

    std::shared_ptr<Core::View> focusedView() const override;
    QVector<std::shared_ptr<Core::Window>> windows() const override;
    virtual std::shared_ptr<Core::Window> windowFromQWindow(QWindow *) const = 0;
    int screenNumberForWindow(std::shared_ptr<Core::Window>) const override;

    void sendEvent(Core::View *, QEvent *) const override;

    bool isProcessingAppQuitEvent() const override;

    QString applicationName() const override;

    void setMouseCursor(Qt::CursorShape, bool discardLast = false) override;
    void restoreMouseCursor() override;

    DisplayType displayType() const override;
    bool isLeftMouseButtonPressed() const override;

    QVector<std::shared_ptr<Core::Screen>> screens() const override;

    QPoint cursorPos() const override;
    void setCursorPos(QPoint) override;

    /// @brief Returns the specified QObject casted to View
    /// Nullptr if it's not a view.
    virtual std::shared_ptr<Core::View> qobjectAsView(QObject *) const = 0;
    std::shared_ptr<Core::Window> qobjectAsWindow(QObject *) const;

    static Platform_qt *instance();

    std::shared_ptr<Core::Screen> primaryScreen() const override;

    void runDelayed(int ms, Core::DelayedCall *) override;

    QByteArray readFile(const QString &fileName, bool &ok) const override;

    bool supportsAeroSnap() const override;

#ifdef DOCKS_DEVELOPER_MODE

    /// These methods are implemented in TestHelpers_qt.cpp

    static bool isGammaray();
    explicit Platform_qt(QCoreApplication *);
    bool tests_waitForWindowActive(std::shared_ptr<Core::Window>, int timeout = 5000) const override;
    bool tests_waitForEvent(Core::Object *w, QEvent::Type type, int timeout = 5000) const override;
    bool tests_waitForEvent(Core::View *, QEvent::Type type, int timeout = 5000) const override;
    bool tests_waitForEvent(std::shared_ptr<Core::Window>, QEvent::Type type,
                            int timeout = 5000) const override;
    void tests_initPlatform_impl() override;
    void tests_deinitPlatform_impl() override;
    bool tests_wait(int ms) const override;
    void tests_pressOn(QPoint globalPos, Core::View *receiver) override;
    void tests_pressOn(QPoint globalPos, std::shared_ptr<Core::Window> receiver) override;
    bool tests_releaseOn(QPoint globalPos, Core::View *receiver) override;
    void tests_doubleClickOn(QPoint globalPos, Core::View *receiver) override;
    void tests_doubleClickOn(QPoint globalPos, std::shared_ptr<Core::Window> receiver) override;
    bool tests_mouseMove(QPoint globalPos, Core::View *receiver) override;

    void installMessageHandler() override;
    void uninstallMessageHandler() override;

    /// @brief if "-platform" wasn't passed it picks the offscreen QPA to run the tests
    static void maybeSetOffscreenQPA(int argc, char **argv);

    bool tests_waitForResize(Core::View *, int timeout = 2000) const override;
    bool tests_waitForResize(Core::Controller *, int timeout = 2000) const override;
    bool tests_waitForDeleted(Core::View *, int timeout = 2000) const override;
    bool tests_waitForDeleted(Core::Controller *, int timeout = 2000) const override;

#endif

protected:
    int screenNumberForQWindow(QWindow *) const;

private:
    class GlobalEventFilter;
    GlobalEventFilter *const m_globalEventFilter;
    Q_DISABLE_COPY(Platform_qt)
};

}

}
