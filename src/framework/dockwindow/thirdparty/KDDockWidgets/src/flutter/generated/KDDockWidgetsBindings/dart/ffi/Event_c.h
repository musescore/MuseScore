/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <QtCompat_p.h>

namespace KDDockWidgetsBindings_wrappersNS {
class Event_wrapper : public ::KDDockWidgets::Event
{
public:
    ~Event_wrapper();
    Event_wrapper(KDDockWidgets::Event::Type type);
    void accept();
    void ignore();
    bool isAccepted() const;
    bool spontaneous() const;
    KDDockWidgets::Event::Type type() const;
};
}
extern "C" {
// KDDockWidgets::Event::Event(KDDockWidgets::Event::Type type)
DOCKS_EXPORT void *c_KDDockWidgets__Event__constructor_Type(int type);
// KDDockWidgets::Event::accept()
DOCKS_EXPORT void c_KDDockWidgets__Event__accept(void *thisObj);
// KDDockWidgets::Event::ignore()
DOCKS_EXPORT void c_KDDockWidgets__Event__ignore(void *thisObj);
// KDDockWidgets::Event::isAccepted() const
DOCKS_EXPORT bool c_KDDockWidgets__Event__isAccepted(void *thisObj);
// KDDockWidgets::Event::spontaneous() const
DOCKS_EXPORT bool c_KDDockWidgets__Event__spontaneous(void *thisObj);
// KDDockWidgets::Event::type() const
DOCKS_EXPORT int c_KDDockWidgets__Event__type(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Event__destructor(void *thisObj);
DOCKS_EXPORT bool c_KDDockWidgets__Event___get_m_accepted(void *thisObj);
DOCKS_EXPORT bool c_KDDockWidgets__Event___get_m_spontaneous(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Event___set_m_accepted_bool(void *thisObj, bool m_accepted_);
DOCKS_EXPORT void c_KDDockWidgets__Event___set_m_spontaneous_bool(void *thisObj, bool m_spontaneous_);
DOCKS_EXPORT void c_KDDockWidgets__Event_Finalizer(void *cppObj);
}
