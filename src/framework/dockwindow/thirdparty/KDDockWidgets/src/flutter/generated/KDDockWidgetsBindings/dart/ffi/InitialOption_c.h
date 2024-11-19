/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <KDDockWidgets.h>
#include <geometry_helpers_p.h>

namespace KDDockWidgetsBindings_wrappersNS {
class InitialOption_wrapper : public ::KDDockWidgets::InitialOption
{
public:
    ~InitialOption_wrapper();
    InitialOption_wrapper();
    InitialOption_wrapper(KDDockWidgets::DefaultSizeMode mode);
    InitialOption_wrapper(KDDockWidgets::InitialVisibilityOption v);
    InitialOption_wrapper(KDDockWidgets::InitialVisibilityOption v, KDDockWidgets::Size size);
    InitialOption_wrapper(KDDockWidgets::Size size);
    bool preservesCurrentTab() const;
    bool startsHidden() const;
};
}
extern "C" {
// KDDockWidgets::InitialOption::InitialOption()
DOCKS_EXPORT void *c_KDDockWidgets__InitialOption__constructor();
// KDDockWidgets::InitialOption::InitialOption(KDDockWidgets::DefaultSizeMode mode)
DOCKS_EXPORT void *c_KDDockWidgets__InitialOption__constructor_DefaultSizeMode(int mode);
// KDDockWidgets::InitialOption::InitialOption(KDDockWidgets::InitialVisibilityOption v)
DOCKS_EXPORT void *c_KDDockWidgets__InitialOption__constructor_InitialVisibilityOption(int v);
// KDDockWidgets::InitialOption::InitialOption(KDDockWidgets::InitialVisibilityOption v, KDDockWidgets::Size size)
DOCKS_EXPORT void *c_KDDockWidgets__InitialOption__constructor_InitialVisibilityOption_Size(int v, void *size_);
// KDDockWidgets::InitialOption::InitialOption(KDDockWidgets::Size size)
DOCKS_EXPORT void *c_KDDockWidgets__InitialOption__constructor_Size(void *size_);
// KDDockWidgets::InitialOption::preservesCurrentTab() const
DOCKS_EXPORT bool c_KDDockWidgets__InitialOption__preservesCurrentTab(void *thisObj);
// KDDockWidgets::InitialOption::startsHidden() const
DOCKS_EXPORT bool c_KDDockWidgets__InitialOption__startsHidden(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__InitialOption__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__InitialOption_Finalizer(void *cppObj);
}
