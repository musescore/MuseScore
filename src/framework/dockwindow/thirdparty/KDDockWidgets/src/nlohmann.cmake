# This file is part of KDDockWidgets.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Sergio Martins <sergio.martins@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

find_package(nlohmann_json QUIET)

# Function to link to nlohmann
function(link_to_nlohman target)
    if(nlohmann_json_FOUND)
        target_link_libraries(${target} PRIVATE nlohmann_json::nlohmann_json)
    else()
        message(STATUS "nlohmann_json not found in system. Using our own bundled one")
        target_include_directories(${target} SYSTEM PRIVATE ${KKDockWidgets_PROJECT_ROOT}/src/3rdparty/nlohmann)
    endif()
endfunction()
