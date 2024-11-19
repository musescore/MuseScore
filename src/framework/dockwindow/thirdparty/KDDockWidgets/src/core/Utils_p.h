/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/Config.h"
#include "kddockwidgets/core/View.h"
#include "kddockwidgets/core/Platform.h"
#include "core/layouting/Item_p.h"
#include "core/Group_p.h"

#include <cstdlib>
#include <string>

namespace KDDockWidgets {

inline bool isWayland()
{
    return Core::Platform::instance()->displayType() == Core::Platform::DisplayType::Wayland;
}

inline bool isOffscreen()
{
    return Core::Platform::instance()->displayType() == Core::Platform::DisplayType::QtOffscreen;
}

inline bool isXCB()
{
    return Core::Platform::instance()->displayType() == Core::Platform::DisplayType::X11;
}

inline bool isEGLFS()
{
    return Core::Platform::instance()->displayType() == Core::Platform::DisplayType::QtEGLFS;
}

inline bool isWindows()
{
    return Core::Platform::instance()->displayType() == Core::Platform::DisplayType::Windows;
}

inline bool usesNativeTitleBar()
{
    return Config::self().flags() & Config::Flag_NativeTitleBar;
}

inline bool usesClientTitleBar()
{
    if (isWayland()) {
        // Wayland has both client and native title bars, due to limitations.
        return true;
    }

    // All other platforms have either the OS native title bar or a Qt title bar (aka client title
    // bar).
    return !usesNativeTitleBar();
}

inline bool usesAeroSnapWithCustomDecos()
{
    return Config::self().flags() & Config::Flag_AeroSnapWithClientDecos;
}

inline bool usesNativeDraggingAndResizing()
{
    // a native title bar implies native resizing and dragging
    // Windows Aero-Snap also implies native dragging, and implies no native-title bar
    assert(!(usesNativeTitleBar() && usesAeroSnapWithCustomDecos()));
    return usesNativeTitleBar() || usesAeroSnapWithCustomDecos();
}

inline bool linksToXLib()
{
#ifdef KDDockWidgets_XLIB
    return true;
#else
    return false;
#endif
}

inline bool usesQTBUG83030Workaround()
{
    const bool useWorkaround =

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
        // Bug is fixed in 6.7
        false;
#else
        // Workaround by default, unless explicitly told not to
        !(Config::self().internalFlags() & Config::InternalFlag_NoDeleteLaterWorkaround);
#endif

    return useWorkaround;
}

inline bool isNormalWindowState(WindowStates states)
{
    return !(states & WindowState::Maximized) && !(states & WindowState::FullScreen);
}

inline MouseEvent *mouseEvent(Event *e)
{
    switch (e->type()) {
    case Event::MouseButtonPress:
    case Event::MouseButtonDblClick:
    case Event::MouseButtonRelease:
    case Event::MouseMove:
    case Event::NonClientAreaMouseButtonPress:
    case Event::NonClientAreaMouseButtonRelease:
    case Event::NonClientAreaMouseMove:
    case Event::NonClientAreaMouseButtonDblClick:
        return static_cast<MouseEvent *>(e);
    default:
        break;
    }

    return nullptr;
}

inline HoverEvent *hoverEvent(Event *e)
{
    switch (e->type()) {
    case Event::HoverEnter:
    case Event::HoverLeave:
    case Event::HoverMove:
        return static_cast<HoverEvent *>(e);
    default:
        break;
    }

    return nullptr;
}

inline bool isNonClientMouseEvent(const Event *e)
{
    switch (e->type()) {
    case Event::NonClientAreaMouseButtonPress:
    case Event::NonClientAreaMouseButtonRelease:
    case Event::NonClientAreaMouseMove:
        return true;
    default:
        break;
    }

    return false;
}

inline bool isDnDEvent(const Event *e)
{
    switch (e->type()) {
    case Event::DragEnter:
    case Event::DragLeave:
    case Event::DragMove:
    case Event::Drop:
        return true;
    default:
        break;
    }

    return false;
}

/// @brief Returns whether we support the specified scalling factor
/// This is a workaround against a bug in older Qt (QTBUG-86170).
/// Mostly affects Linux. Unless you're using Qt::HighDpiScaleFactorRoundingPolicy::PassThrough, in
/// which case it will affect other OSes too.
inline bool scalingFactorIsSupported(double factor)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 2)
    // We don't support fractional factors in older Qt.
    const bool isInteger = int(factor) == factor;
    return isInteger;
#else
    KDDW_UNUSED(factor);
    return true;
#endif
}

template<typename T>
void deleteAll(const T &vec)
{
    for (const auto &e : vec)
        delete e;
}

}

#ifdef DOCKS_DEVELOPER_MODE

inline bool stringContains(const std::string_view haystack, std::string_view needle)
{
    return haystack.find(needle) != std::string::npos;
}

inline int envVarIntValue(const char *variableName, bool &ok)
{
    ok = true;

#ifdef KDDW_FRONTEND_QT
    return qEnvironmentVariableIntValue(variableName, &ok);

#else
    // Flutter:

#ifdef _MSC_VER
    size_t requiredSize;
    char value[3]; // just used for the platform Id
    auto bufSize = sizeof(value);

    const errno_t err = getenv_s(&requiredSize, value, bufSize, variableName);
    if (err != 0 || requiredSize == 0 || requiredSize > bufSize) {
        ok = false;
        return -1;
    }
#else
    auto value = std::getenv(variableName);
#endif // msvc

    if (value) {
        try {
            return std::stoi(value);
        } catch (const std::exception &) {
        }
    }

    ok = false;
    return -1;

#endif // KDDW_FRONTEND_QT
}

#endif // dev mode
