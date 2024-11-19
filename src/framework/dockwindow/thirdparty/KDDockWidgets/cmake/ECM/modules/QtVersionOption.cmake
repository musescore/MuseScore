# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

#[=======================================================================[.rst:
QtVersionOption
---------------

Adds a build option to select the major Qt version if necessary,
that is, if the major Qt version has not yet been determined otherwise
(e.g. by a corresponding ``find_package()`` call).
This module is typically included by other modules requiring knowledge
about the major Qt version.

If the ECM version passed to find_package was at least 5.240.0 Qt6 is picked by default.
Otherwise Qt5 is picked.

``QT_MAJOR_VERSION`` is defined to either be "5" or "6".

Since 5.82.0.
#]=======================================================================]

if (DEFINED QT_MAJOR_VERSION)
    return()
endif()

if (TARGET Qt5::Core)
    set(QT_MAJOR_VERSION 5)
elseif (TARGET Qt6::Core)
    set(QT_MAJOR_VERSION 6)
else()
    if (ECM_GLOBAL_FIND_VERSION VERSION_GREATER_EQUAL 5.240)
        option(BUILD_WITH_QT6 "Build against Qt 6" ON)
    else()
        option(BUILD_WITH_QT6 "Build against Qt 6" OFF)
    endif()

    if (BUILD_WITH_QT6)
        set(QT_MAJOR_VERSION 6)
    else()
        set(QT_MAJOR_VERSION 5)
    endif()
endif()
