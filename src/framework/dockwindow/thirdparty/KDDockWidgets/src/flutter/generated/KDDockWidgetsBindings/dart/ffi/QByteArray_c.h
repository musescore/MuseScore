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
class QByteArray_wrapper : public ::QByteArray
{
public:
    ~QByteArray_wrapper();
    QByteArray_wrapper();
    const char *constData() const;
    bool isEmpty() const;
};
}
extern "C" {
// QByteArray::QByteArray()
DOCKS_EXPORT void *c_QByteArray__constructor();
// QByteArray::constData() const
DOCKS_EXPORT const char *c_QByteArray__constData(void *thisObj);
// QByteArray::isEmpty() const
DOCKS_EXPORT bool c_QByteArray__isEmpty(void *thisObj);
DOCKS_EXPORT void c_QByteArray__destructor(void *thisObj);
DOCKS_EXPORT void c_QByteArray_Finalizer(void *cppObj);
}
