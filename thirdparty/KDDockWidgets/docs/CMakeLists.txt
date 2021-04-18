#
# This file is part of KDDockWidgets.
#
# SPDX-FileCopyrightText: 2020-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Allen Winter <allen.winter@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

# qhelpgenerator
find_program(QHELPGEN_EXECUTABLE qhelpgenerator
  HINTS ${QT_INSTALL_BINS}
)
if(QHELPGEN_EXECUTABLE)
  set(HAVE_QHELPGEN "YES")
else()
  set(HAVE_QHELPGEN "NO")
  message(STATUS "Unable to generate the API documentation in qch format. To fix, install the qthelpgenerator program which comes with Qt5.")
endif()

# Doxygen
find_package(Doxygen)
set_package_properties(Doxygen PROPERTIES
  TYPE OPTIONAL
  DESCRIPTION "API Documentation system"
  URL "https://www.doxygen.org"
  PURPOSE "Needed to build the API documentation."
)
if(DOXYGEN_DOT_EXECUTABLE)
  set(HAVE_DOT "YES")
else()
  set(HAVE_DOT "NO")
  message(STATUS "Unable to provide inheritance diagrams for the API documentation. To fix, install the graphviz project from https://www.graphviz.org")
endif()

if(DOXYGEN_FOUND)
  add_subdirectory(api)
endif()
