#=============================================================================
# Copyright 2005-2011 Kitware, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the name of Kitware, Inc. nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

# The automoc_qt4 macro is superceded by CMAKE_AUTOMOC from CMake 2.8.6
# A Qt 5 version is not provided by CMake or Qt.

include(MacroAddFileDependencies)

MACRO (QT4_GET_MOC_FLAGS _moc_flags)
  SET(${_moc_flags})
  GET_DIRECTORY_PROPERTY(_inc_DIRS INCLUDE_DIRECTORIES)

  FOREACH(_current ${_inc_DIRS})
    IF("${_current}" MATCHES "\\.framework/?$")
      STRING(REGEX REPLACE "/[^/]+\\.framework" "" framework_path "${_current}")
      SET(${_moc_flags} ${${_moc_flags}} "-F${framework_path}")
    ELSE("${_current}" MATCHES "\\.framework/?$")
      SET(${_moc_flags} ${${_moc_flags}} "-I${_current}")
    ENDIF("${_current}" MATCHES "\\.framework/?$")
  ENDFOREACH(_current ${_inc_DIRS})

  GET_DIRECTORY_PROPERTY(_defines COMPILE_DEFINITIONS)
  FOREACH(_current ${_defines})
    SET(${_moc_flags} ${${_moc_flags}} "-D${_current}")
  ENDFOREACH(_current ${_defines})

  IF(Q_OS_WIN)
    SET(${_moc_flags} ${${_moc_flags}} -DWIN32)
  ENDIF(Q_OS_WIN)

ENDMACRO(QT4_GET_MOC_FLAGS)

# helper macro to set up a moc rule
MACRO (QT4_CREATE_MOC_COMMAND infile outfile moc_flags moc_options)
  # For Windows, create a parameters file to work around command line length limit
  IF (WIN32)
    # Pass the parameters in a file.  Set the working directory to
    # be that containing the parameters file and reference it by
    # just the file name.  This is necessary because the moc tool on
    # MinGW builds does not seem to handle spaces in the path to the
    # file given with the @ syntax.
    GET_FILENAME_COMPONENT(_moc_outfile_name "${outfile}" NAME)
    GET_FILENAME_COMPONENT(_moc_outfile_dir "${outfile}" PATH)
    IF(_moc_outfile_dir)
      SET(_moc_working_dir WORKING_DIRECTORY ${_moc_outfile_dir})
    ENDIF(_moc_outfile_dir)
    SET (_moc_parameters_file ${outfile}_parameters)
    SET (_moc_parameters ${moc_flags} ${moc_options} -o "${outfile}" "${infile}")
    STRING (REPLACE ";" "\n" _moc_parameters "${_moc_parameters}")
    FILE (WRITE ${_moc_parameters_file} "${_moc_parameters}")
    ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
                       COMMAND ${QT_MOC_EXECUTABLE} @${_moc_outfile_name}_parameters
                       DEPENDS ${infile}
                       ${_moc_working_dir}
                       VERBATIM)
  ELSE (WIN32)
    ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
                       COMMAND ${QT_MOC_EXECUTABLE}
                       ARGS ${moc_flags} ${moc_options} -o ${outfile} ${infile}
                       DEPENDS ${infile} VERBATIM)
  ENDIF (WIN32)
ENDMACRO (QT4_CREATE_MOC_COMMAND)


MACRO(QT4_AUTOMOC)
  QT4_GET_MOC_FLAGS(_moc_INCS)

  SET(_matching_FILES )
  FOREACH (_current_FILE ${ARGN})

    GET_FILENAME_COMPONENT(_abs_FILE ${_current_FILE} ABSOLUTE)
    # if "SKIP_AUTOMOC" is set to true, we will not handle this file here.
    # This is required to make uic work correctly:
    # we need to add generated .cpp files to the sources (to compile them),
    # but we cannot let automoc handle them, as the .cpp files don't exist yet when
    # cmake is run for the very first time on them -> however the .cpp files might
    # exist at a later run. at that time we need to skip them, so that we don't add two
    # different rules for the same moc file
    GET_SOURCE_FILE_PROPERTY(_skip ${_abs_FILE} SKIP_AUTOMOC)

    IF ( NOT _skip AND EXISTS ${_abs_FILE} )

      FILE(READ ${_abs_FILE} _contents)

      GET_FILENAME_COMPONENT(_abs_PATH ${_abs_FILE} PATH)

      STRING(REGEX MATCHALL "# *include +[^ ]+\\.moc[\">]" _match "${_contents}")
      IF(_match)
        FOREACH (_current_MOC_INC ${_match})
          STRING(REGEX MATCH "[^ <\"]+\\.moc" _current_MOC "${_current_MOC_INC}")

          GET_FILENAME_COMPONENT(_basename ${_current_MOC} NAME_WE)
          IF(EXISTS ${_abs_PATH}/${_basename}.hpp)
            SET(_header ${_abs_PATH}/${_basename}.hpp)
          ELSE(EXISTS ${_abs_PATH}/${_basename}.hpp)
            SET(_header ${_abs_PATH}/${_basename}.h)
          ENDIF(EXISTS ${_abs_PATH}/${_basename}.hpp)
          SET(_moc    ${CMAKE_CURRENT_BINARY_DIR}/${_current_MOC})
          QT4_CREATE_MOC_COMMAND(${_header} ${_moc} "${_moc_INCS}" "")
          MACRO_ADD_FILE_DEPENDENCIES(${_abs_FILE} ${_moc})
        ENDFOREACH (_current_MOC_INC)
      ENDIF(_match)
    ENDIF ( NOT _skip AND EXISTS ${_abs_FILE} )
  ENDFOREACH (_current_FILE)
ENDMACRO(QT4_AUTOMOC)


# Portability helpers.

set(QT_QTGUI_LIBRARIES
  ${Qt5Gui_LIBRARIES}
  ${Qt5Widgets_LIBRARIES}
  ${Qt5PrintSupport_LIBRARIES}
  ${Qt5Svg_LIBRARIES}
)

set(QT_INCLUDES
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5PrintSupport_INCLUDE_DIRS}
    ${Qt5Svg_INCLUDE_DIRS}
)
set(QT_QTGUI_LIBRARY ${QT_QTGUI_LIBRARIES})

set(_qt_modules
  Core
  Widgets
  Network
  Test
  Designer
  Concurrent
  Xml
  XmlPatterns
  UiTools
  Qml
  Quick
  QuickWidgets
  WebKit
  WebKitWidgets
  Sql
  OpenGL
  Help
)

foreach(_module ${_qt_modules})
    string(TOUPPER ${_module} _module_upper)
    set(QT_QT${_module_upper}_LIBRARIES ${Qt5${_module}_LIBRARIES})
    set(QT_QT${_module_upper}_LIBRARY ${QT_QT${_module_upper}_LIBRARIES})
    list(APPEND QT_INCLUDES ${Qt5${_module}_INCLUDE_DIRS})
    set(QT_QT${_module_upper}_FOUND ${Qt5${_module}_FOUND})
endforeach()

list(APPEND QT_QTCORE_LIBRARIES ${Qt5Concurrent_LIBRARIES})
list(APPEND QT_QTCORE_LIBRARY ${Qt5Concurrent_LIBRARIES})

list(APPEND QT_QTWEBKIT_LIBRARIES ${Qt5WebKitWidgets_LIBRARIES})
list(APPEND QT_QTWEBKIT_LIBRARY ${Qt5WebKitWidgets_LIBRARIES})

set(QT_QTDECLARATIVE_LIBRARIES ${Qt5Quick1_LIBRARIES})
set(QT_QTDECLARATIVE_LIBRARY ${Qt5Quick1_LIBRARIES})

set(QT_LRELEASE_EXECUTABLE Qt5::lrelease)
set(QT_LUPDATE_EXECUTABLE Qt5::lupdate)

set(QT_INSTALL_PREFIX ${_qt5Core_install_prefix})

get_target_property(QT_QMAKE_EXECUTABLE Qt5::qmake LOCATION)
set(QT_RCC_EXECUTABLE Qt5::rcc LOCATION)
if (TARGET Qt5::uic)
    get_target_property(QT_UIC_EXECUTABLE Qt5::uic LOCATION)
endif()


if (TARGET Qt5::qdbuscpp2xml)
    get_target_property(QT_QDBUSCPP2XML_EXECUTABLE Qt5::qdbuscpp2xml LOCATION)
endif()

if (TARGET Qt5::qdbusxml2cpp)
    get_target_property(QT_QDBUSXML2CPP_EXECUTABLE Qt5::qdbusxml2cpp LOCATION)
endif()

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

macro(qt4_wrap_ui)
  qt5_wrap_ui(${ARGN})
endmacro()

macro(qt4_wrap_cpp)
  qt5_wrap_cpp(${ARGN})
endmacro()

macro(qt4_generate_moc)
  qt5_generate_moc(${ARGN})
endmacro()

macro(qt4_add_dbus_adaptor)
  qt5_add_dbus_adaptor(${ARGN})
endmacro()

macro(qt4_add_dbus_interfaces)
  qt5_add_dbus_interfaces(${ARGN})
endmacro()

macro(qt4_add_dbus_interface)
  qt5_add_dbus_interface(${ARGN})
endmacro()

macro(qt4_generate_dbus_interface)
  qt5_generate_dbus_interface(${ARGN})
endmacro()

macro(qt4_add_resources)
  qt5_add_resources(${ARGN})
endmacro()

