/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "core/Window_p.h"

#ifdef KDDockWidgets_XLIB

#include "DockRegistry.h"

#include <QtGui/qpa/qplatformnativeinterface.h>

#include <X11/Xlib.h>
#include <algorithm>

namespace KDDockWidgets {

static void travelTree(WId current, Display *disp, Window::List &remaining, Window::List &result)
{
    if (remaining.isEmpty())
        return;

    ::Window parent, root, *children;
    unsigned int nchildren;

    if (!XQueryTree(disp, current, &root, &parent, &children, &nchildren)) {
        return;
    }

    if (!children)
        return;

    for (int i = 0; i < int(nchildren); ++i) {
        /// XQueryTree returns a lot more stuff than our top-level stuff, let's search for it:
        auto it =
            std::find_if(remaining.begin(), remaining.end(), [i, children](Core::Window::Ptr window) {
                return window->handle() == children[i];
            });

        if (it != remaining.end()) {
            result.push_back(*it);
            remaining.erase(it);
        }

        // Recurs:
        travelTree(children[i], disp, remaining, result);
    }
}

static Display *x11Display()
{
    auto nativeInterface = qGuiApp->platformNativeInterface();
    void *disp = nativeInterface->nativeResourceForIntegration(QByteArrayLiteral("display"));
    return reinterpret_cast<Display *>(disp);
}

/// @brief returns the KDDW top-level windows (MainWindow and floating widgets) ordered by z-order
/// The front of the vector has stuff with lower Z
static Window::List orderedWindows(bool &ok)
{
    ok = true;
    Window::List windows = DockRegistry::self()->topLevels();
    if (windows.isEmpty())
        return {};

    Window::List orderedResult;
    Display *disp = reinterpret_cast<Display *>(x11Display());
    travelTree(DefaultRootWindow(disp), disp, /**by-ref*/ windows, /**by-ref*/ orderedResult);

    ok = windows.isEmpty();
    return orderedResult;
}
}

#else

namespace KDDockWidgets {
/// Dummy which is never called, just so code compiles on Windows without
/// adding more #ifdefery
static Core::Window::List orderedWindows(bool &ok)
{
    KDDW_UNUSED(ok);
    Q_UNREACHABLE();
    return {};
}
}

#endif
