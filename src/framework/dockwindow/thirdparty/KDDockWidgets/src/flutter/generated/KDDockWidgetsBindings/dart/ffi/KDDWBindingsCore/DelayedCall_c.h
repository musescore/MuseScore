/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <DelayedCall_p.h>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsCore {
class DelayedCall_wrapper : public ::KDDockWidgets::Core::DelayedCall
{
public:
    ~DelayedCall_wrapper();
    DelayedCall_wrapper();
    virtual void call();
    virtual void call_nocallback();
    typedef void (*Callback_call)(void *);
    Callback_call m_callCallback = nullptr;
};
}
}
extern "C" {
// KDDockWidgets::Core::DelayedCall::DelayedCall()
DOCKS_EXPORT void *c_KDDockWidgets__Core__DelayedCall__constructor();
// KDDockWidgets::Core::DelayedCall::call()
DOCKS_EXPORT void c_KDDockWidgets__Core__DelayedCall__call(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__DelayedCall__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__DelayedCall__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__Core__DelayedCall_Finalizer(void *cppObj);
}
