/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "flutter_c.h"


#include <iostream>

#include <cassert>


namespace Dartagnan {

typedef int (*CleanupCallback)(void *thisPtr);
static CleanupCallback s_cleanupCallback = nullptr;

template<typename T>
struct ValueWrapper
{
    T value;
};

}
extern "C" {
// asView_flutter(KDDockWidgets::Core::Controller * controller)
void *c_static_KDDockWidgets__flutter__asView_flutter_Controller(void *controller_)
{
    auto controller = reinterpret_cast<KDDockWidgets::Core::Controller *>(controller_);
    const auto &result = flutter::asView_flutter(controller);
    return result;
}
// asView_flutter(KDDockWidgets::Core::View * view)
void *c_static_KDDockWidgets__flutter__asView_flutter_View(void *view_)
{
    auto view = reinterpret_cast<KDDockWidgets::Core::View *>(view_);
    const auto &result = flutter::asView_flutter(view);
    return result;
}
}
