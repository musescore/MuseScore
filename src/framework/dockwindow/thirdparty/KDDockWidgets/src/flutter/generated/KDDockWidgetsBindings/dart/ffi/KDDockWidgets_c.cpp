/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "KDDockWidgets_c.h"


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
// fuzzyCompare(double a, double b, double epsilon)
bool c_static_KDDockWidgets__fuzzyCompare_double_double_double(double a, double b, double epsilon)
{
    const auto &result = KDDockWidgets::fuzzyCompare(a, b, epsilon);
    return result;
}
// initFrontend(KDDockWidgets::FrontendType arg__1)
void c_static_KDDockWidgets__initFrontend_FrontendType(int arg__1)
{
    KDDockWidgets::initFrontend(static_cast<KDDockWidgets::FrontendType>(arg__1));
}
// spdlogLoggerName()
const char *c_static_KDDockWidgets__spdlogLoggerName()
{
    const auto &result = KDDockWidgets::spdlogLoggerName();
    return result;
}
}
