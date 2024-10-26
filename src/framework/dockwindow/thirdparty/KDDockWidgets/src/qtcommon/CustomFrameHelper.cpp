/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "CustomFrameHelper_p.h"
#include "core/Logging_p.h"
#include "core/ScopedValueRollback_p.h"
#include "core/Utils_p.h"
#include "core/Window_p.h"
#include "qtcommon/Window_p.h"

#include <QGuiApplication>

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

#ifdef KDDW_FRONTEND_QT_WINDOWS
namespace KDDockWidgets {
Window::Ptr windowForHandle(WId id)
{
    const Window::List windows = Platform::instance()->windows();
    for (Core::Window::Ptr w : windows) {
        if (w->isVisible() && w->handle() == id) {
            return w;
        }
    }
    return nullptr;
}
}
#endif

CustomFrameHelper::CustomFrameHelper(ShouldUseCustomFrame func, QObject *parent)
    : QObject(parent)
    , m_shouldUseCustomFrameFunc(func)
{
#if defined(KDDW_FRONTEND_QT_WINDOWS)
    qGuiApp->installNativeEventFilter(this);
#endif
}

CustomFrameHelper::~CustomFrameHelper()
{
    m_inDtor = true;
}

void CustomFrameHelper::applyCustomFrame(Core::Window::Ptr window)
{
#ifdef KDDW_FRONTEND_QT_WINDOWS
    WidgetResizeHandler::setupWindow(window);
#else
    KDDW_UNUSED(window);
    KDDW_ERROR("Not implemented on this platform");
#endif
}

bool CustomFrameHelper::nativeEventFilter(const QByteArray &eventType, void *message,
                                          Qt5Qt6Compat::qintptr *result)
{
    if (m_shouldUseCustomFrameFunc == nullptr || m_recursionGuard)
        return false;

    ScopedValueRollback guard(m_recursionGuard, true);

#ifdef KDDW_FRONTEND_QT_WINDOWS
    if (m_inDtor || !KDDockWidgets::usesAeroSnapWithCustomDecos())
        return false;

    if (eventType != "windows_generic_MSG")
        return false;

    auto msg = static_cast<MSG *>(message);
    if (!WidgetResizeHandler::isInterestingNativeEvent(msg->message)) {
        // Save some CPU cycles
        return false;
    }

    Window::Ptr window = KDDockWidgets::windowForHandle(WId(msg->hwnd));
    if (!window)
        return false;

    const WidgetResizeHandler::NativeFeatures features = m_shouldUseCustomFrameFunc(window);
    if (!features.hasFeatures()) {
        // No native support for is desired for this window
        return false;
    }

    const char *propertyName = "kddw_customframe_setup_ran";
    auto windowQt = static_cast<QtCommon::Window *>(window.get());
    const bool setupRan = windowQt->property(propertyName).toBool();
    if (!setupRan) {
        // Add drop shadow
        WidgetResizeHandler::setupWindow(window);
        windowQt->setProperty(propertyName, true);
    }

    return WidgetResizeHandler::handleWindowsNativeEvent(window, msg, result, features);
#else
    KDDW_UNUSED(eventType);
    KDDW_UNUSED(message);
    KDDW_UNUSED(result);
    return false;
#endif
}
