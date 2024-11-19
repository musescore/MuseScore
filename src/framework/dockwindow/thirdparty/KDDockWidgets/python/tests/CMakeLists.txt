# This file is part of KDDockWidgets.
#
# SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Renato Araujo Oliveira Filho <renato.araujo@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.py.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.py @ONLY)

set(TEST_PYTHONPATH ${CMAKE_BINARY_DIR}/python ${CMAKE_CURRENT_BINARY_DIR})
set(TEST_LIBRARYPATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

if(WIN32)
    set(TEST_LIBRARY_VAR "PATH")
    string(REPLACE "\\" "/" TEST_PYTHONPATH "${TEST_PYTHONPATH}")
    string(REPLACE "\\" "/" TEST_LIBRARYPATH "${TEST_LIBRARYPATH}")
    list(JOIN TEST_PYTHONPATH "\\;" TEST_PYTHONPATH)
    list(JOIN TEST_LIBRARYPATH "\\;" TEST_LIBRARYPATH)
else()
    set(TEST_LIBRARY_VAR "LD_LIBRARY_PATH")
    list(JOIN TEST_PYTHONPATH ":" TEST_PYTHONPATH)
    list(JOIN TEST_LIBRARYPATH ":" TEST_LIBRARYPATH)
endif()

set(PYTHON_ENV_COMMON "PYTHONPATH=${TEST_PYTHONPATH};${TEST_LIBRARY_VAR}=${TEST_LIBRARYPATH}")

file(GLOB TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/tst_*.py)
foreach(test_file ${TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    add_test(${test_name} ${Python3_EXECUTABLE} ${test_file})
    set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "${PYTHON_ENV_COMMON}")
endforeach()
