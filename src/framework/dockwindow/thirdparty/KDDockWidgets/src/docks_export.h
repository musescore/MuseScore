/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#ifdef KDDW_FRONTEND_QT

#include <QtCore/QtGlobal>

#if defined(KDDOCKWIDGETS_STATICLIB)
#define DOCKS_EXPORT
#define DOCKS_EXPORT_FOR_UNIT_TESTS
#else
#if defined(BUILDING_DOCKS_LIBRARY)
#define DOCKS_EXPORT Q_DECL_EXPORT
#if defined(DOCKS_DEVELOPER_MODE)
#define DOCKS_EXPORT_FOR_UNIT_TESTS Q_DECL_EXPORT
#else
#define DOCKS_EXPORT_FOR_UNIT_TESTS
#endif
#else
#define DOCKS_EXPORT Q_DECL_IMPORT
#if defined(DOCKS_DEVELOPER_MODE)
#define DOCKS_EXPORT_FOR_UNIT_TESTS Q_DECL_IMPORT
#else
#define DOCKS_EXPORT_FOR_UNIT_TESTS Q_DECL_IMPORT
#endif
#endif
#endif

#else // non-Qt


#ifdef DARTAGNAN_BINDINGS_RUN

// no-op if bindings are being generated
#define DOCKS_EXPORT
#define DOCKS_EXPORT_FOR_UNIT_TESTS

#else

// Auto-generated in build directory, defines DOCKS_EXPORT
#include "kddockwidgets_export.h"

// DOCKS_EXPORT_FOR_UNIT_TESTS
#if defined(DOCKS_DEVELOPER_MODE)
#define DOCKS_EXPORT_FOR_UNIT_TESTS DOCKS_EXPORT
#else
#define DOCKS_EXPORT_FOR_UNIT_TESTS
#endif

#endif

#endif
