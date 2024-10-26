/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <Platform.h>
#include <View.h>
#include <core/View.h>
#include <core/Controller.h>

extern "C" {
// KDDockWidgets::flutter::asView_flutter(KDDockWidgets::Core::Controller * controller)
DOCKS_EXPORT void *c_static_KDDockWidgets__flutter__asView_flutter_Controller(void *controller_);
// KDDockWidgets::flutter::asView_flutter(KDDockWidgets::Core::View * view)
DOCKS_EXPORT void *c_static_KDDockWidgets__flutter__asView_flutter_View(void *view_);
DOCKS_EXPORT void c_KDDockWidgets__flutter_Finalizer(void *cppObj);
}
