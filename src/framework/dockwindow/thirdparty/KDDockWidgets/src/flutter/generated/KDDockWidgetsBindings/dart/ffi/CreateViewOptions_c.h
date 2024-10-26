/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <core/Platform.h>
#include <geometry_helpers_p.h>

namespace KDDockWidgetsBindings_wrappersNS {
class CreateViewOptions_wrapper : public ::KDDockWidgets::Core::CreateViewOptions
{
public:
    ~CreateViewOptions_wrapper();
    CreateViewOptions_wrapper();
    KDDockWidgets::Size getMaxSize() const;
    KDDockWidgets::Size getMinSize() const;
    KDDockWidgets::Size getSize() const;
};
}
extern "C" {
// KDDockWidgets::Core::CreateViewOptions::CreateViewOptions()
DOCKS_EXPORT void *c_KDDockWidgets__Core__CreateViewOptions__constructor();
// KDDockWidgets::Core::CreateViewOptions::getMaxSize() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__CreateViewOptions__getMaxSize(void *thisObj);
// KDDockWidgets::Core::CreateViewOptions::getMinSize() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__CreateViewOptions__getMinSize(void *thisObj);
// KDDockWidgets::Core::CreateViewOptions::getSize() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__CreateViewOptions__getSize(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__CreateViewOptions__destructor(void *thisObj);
DOCKS_EXPORT bool c_KDDockWidgets__Core__CreateViewOptions___get_isVisible(void *thisObj);
DOCKS_EXPORT bool c_KDDockWidgets__Core__CreateViewOptions___get_createWindow(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__CreateViewOptions___set_isVisible_bool(void *thisObj, bool isVisible_);
DOCKS_EXPORT void c_KDDockWidgets__Core__CreateViewOptions___set_createWindow_bool(void *thisObj, bool createWindow_);
DOCKS_EXPORT void c_KDDockWidgets__Core__CreateViewOptions_Finalizer(void *cppObj);
}
