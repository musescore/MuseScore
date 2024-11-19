/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <string_p.h>

namespace KDDockWidgetsBindings_wrappersNS {
class QString_wrapper : public ::QString
{
public:
    ~QString_wrapper();
    QString_wrapper();
    QString_wrapper(const char *str);
    const char *data() const;
    static QString fromUtf8(const char *str);
    bool isEmpty() const;
};
}
extern "C" {
// QString::QString()
DOCKS_EXPORT void *c_QString__constructor();
// QString::QString(const char * str)
DOCKS_EXPORT void *c_QString__constructor_char(const char *str);
// QString::data() const
DOCKS_EXPORT const char *c_QString__data(void *thisObj);
// QString::fromUtf8(const char * str)
DOCKS_EXPORT void *c_static_QString__fromUtf8_char(const char *str);
// QString::isEmpty() const
DOCKS_EXPORT bool c_QString__isEmpty(void *thisObj);
DOCKS_EXPORT void c_QString__destructor(void *thisObj);
DOCKS_EXPORT void c_QString_Finalizer(void *cppObj);
}
