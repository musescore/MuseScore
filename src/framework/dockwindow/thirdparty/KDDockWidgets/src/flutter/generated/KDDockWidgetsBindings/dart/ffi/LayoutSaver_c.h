/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <LayoutSaver.h>
#include <string_p.h>

namespace KDDockWidgetsBindings_wrappersNS {
class LayoutSaver_wrapper : public ::KDDockWidgets::LayoutSaver
{
public:
    ~LayoutSaver_wrapper();
    LayoutSaver_wrapper();
    bool restoreFromFile(const QString &jsonFilename);
    static bool restoreInProgress();
    bool saveToFile(const QString &jsonFilename);
};
}
extern "C" {
// KDDockWidgets::LayoutSaver::LayoutSaver()
DOCKS_EXPORT void *c_KDDockWidgets__LayoutSaver__constructor();
// KDDockWidgets::LayoutSaver::restoreFromFile(const QString & jsonFilename)
DOCKS_EXPORT bool c_KDDockWidgets__LayoutSaver__restoreFromFile_QString(void *thisObj, const char *jsonFilename_);
// KDDockWidgets::LayoutSaver::restoreInProgress()
DOCKS_EXPORT bool c_static_KDDockWidgets__LayoutSaver__restoreInProgress();
// KDDockWidgets::LayoutSaver::saveToFile(const QString & jsonFilename)
DOCKS_EXPORT bool c_KDDockWidgets__LayoutSaver__saveToFile_QString(void *thisObj, const char *jsonFilename_);
DOCKS_EXPORT void c_KDDockWidgets__LayoutSaver__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__LayoutSaver_Finalizer(void *cppObj);
}
