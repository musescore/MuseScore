# SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
# SPDX-FileCopyrightText: 2014 Alex Merry <alex.merry@kdemail.net>
# SPDX-FileCopyrightText: 2015 Patrick Spendrin <patrick.spendrin@kdab.com>
#
# SPDX-License-Identifier: BSD-3-Clause

#[=======================================================================[.rst:
ECMGenerateHeaders
------------------

Generate C/C++ CamelCase forwarding headers.

::

  ecm_generate_headers(<camelcase_forwarding_headers_var>
      HEADER_NAMES <CamelCaseName> [<CamelCaseName> [...]]
      [ORIGINAL <CAMELCASE|LOWERCASE>]
      [HEADER_EXTENSION <header_extension>]
      [OUTPUT_DIR <output_dir>]
      [PREFIX <prefix>]
      [REQUIRED_HEADERS <variable>]
      [COMMON_HEADER <HeaderName>]
      [RELATIVE <relative_path>])

For each CamelCase header name passed to ``HEADER_NAMES``, a file of that name
will be generated that will include a version with ``.h`` or, if set,
``.<header_extension>`` appended.
For example, the generated header ``ClassA`` will include ``classa.h`` (or
``ClassA.h``, see ``ORIGINAL``).
If a CamelCaseName consists of multiple comma-separated files, e.g.
``ClassA,ClassB,ClassC``, then multiple camelcase header files will be
generated which are redirects to the first header file.
The file locations of these generated headers will be stored in
<camelcase_forwarding_headers_var>.

``ORIGINAL`` specifies how the name of the original header is written: lowercased
or also camelcased.  The default is "LOWERCASE". Since 1.8.0.

``HEADER_EXTENSION`` specifies what file name extension is used for the header
files.  The default is "h". Since 5.48.0.

``PREFIX`` places the generated headers in subdirectories.  This should be a
CamelCase name like ``KParts``, which will cause the CamelCase forwarding
headers to be placed in the ``KParts`` directory (e.g. ``KParts/Part``).  It
will also, for the convenience of code in the source distribution, generate
forwarding headers based on the original names (e.g. ``kparts/part.h``).  This
allows includes like ``"#include <kparts/part.h>"`` to be used before
installation, as long as the include_directories are set appropriately.

``OUTPUT_DIR`` specifies where the files will be generated; this should be within
the build directory. By default, ``${CMAKE_CURRENT_BINARY_DIR}`` will be used.
This option can be used to avoid file conflicts.

``REQUIRED_HEADERS`` specifies an output variable name where all the required
headers will be appended so that they can be installed together with the
generated ones.  This is mostly intended as a convenience so that adding a new
header to a project only requires specifying the CamelCase variant in the
CMakeLists.txt file; the original variant will then be added to this
variable.

``COMMON_HEADER`` generates an additional convenience header which includes all
other header files.

The ``RELATIVE`` argument indicates where the original headers can be found
relative to ``CMAKE_CURRENT_SOURCE_DIR``.  It does not affect the generated
CamelCase forwarding files, but ``ecm_generate_headers()`` uses it when checking
that the original header exists, and to generate originally named forwarding
headers when ``PREFIX`` is set.

To allow other parts of the source distribution (eg: tests) to use the
generated headers before installation, it may be desirable to set the
``INCLUDE_DIRECTORIES`` property for the library target to output_dir.  For
example, if ``OUTPUT_DIR`` is ``CMAKE_CURRENT_BINARY_DIR`` (the default), you could do

.. code-block:: cmake

  target_include_directories(MyLib PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>")

Example usage (without ``PREFIX``):

.. code-block:: cmake

  ecm_generate_headers(
      MyLib_FORWARDING_HEADERS
      HEADERS
          MLFoo
          MLBar
          # etc
      REQUIRED_HEADERS MyLib_HEADERS
      COMMON_HEADER MLGeneral
  )
  install(FILES ${MyLib_FORWARDING_HEADERS} ${MyLib_HEADERS}
          DESTINATION ${CMAKE_INSTALL_PREFIX}/include
          COMPONENT Devel)

Example usage (with ``PREFIX``):

.. code-block:: cmake

  ecm_generate_headers(
      MyLib_FORWARDING_HEADERS
      HEADERS
          Foo
          # several classes are contained in bar.h, so generate
          # additional files
          Bar,BarList
          # etc
      PREFIX MyLib
      REQUIRED_HEADERS MyLib_HEADERS
  )
  install(FILES ${MyLib_FORWARDING_HEADERS}
          DESTINATION ${CMAKE_INSTALL_PREFIX}/include/MyLib
          COMPONENT Devel)
  install(FILES ${MyLib_HEADERS}
          DESTINATION ${CMAKE_INSTALL_PREFIX}/include/mylib
          COMPONENT Devel)

Since pre-1.0.0.
#]=======================================================================]

function(ECM_GENERATE_HEADERS camelcase_forwarding_headers_var)
    set(options)
    set(oneValueArgs ORIGINAL HEADER_EXTENSION OUTPUT_DIR PREFIX REQUIRED_HEADERS COMMON_HEADER RELATIVE)
    set(multiValueArgs HEADER_NAMES)
    cmake_parse_arguments(EGH "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (EGH_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unexpected arguments to ECM_GENERATE_HEADERS: ${EGH_UNPARSED_ARGUMENTS}")
    endif()

    if(NOT EGH_HEADER_NAMES)
       message(FATAL_ERROR "Missing header_names argument to ECM_GENERATE_HEADERS")
    endif()

    if(NOT EGH_ORIGINAL)
        # default
        set(EGH_ORIGINAL "LOWERCASE")
    endif()
    if(NOT EGH_ORIGINAL STREQUAL "LOWERCASE" AND NOT EGH_ORIGINAL STREQUAL "CAMELCASE")
        message(FATAL_ERROR "Unexpected value for original argument to ECM_GENERATE_HEADERS: ${EGH_ORIGINAL}")
    endif()

    if(NOT EGH_HEADER_EXTENSION)
        set(EGH_HEADER_EXTENSION "h")
    endif()

    if(NOT EGH_OUTPUT_DIR)
        set(EGH_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    endif()

    # Make sure EGH_RELATIVE is /-terminated when it's not empty
    if (EGH_RELATIVE AND NOT "${EGH_RELATIVE}" MATCHES "^.*/$")
        set(EGH_RELATIVE "${EGH_RELATIVE}/")
    endif()

    set(originalprefix)
    if (EGH_PREFIX)
        if (NOT "${EGH_PREFIX}" MATCHES "^.*/$")
            set(EGH_PREFIX "${EGH_PREFIX}/")
        endif()
        if (EGH_ORIGINAL STREQUAL "CAMELCASE")
            set(originalprefix "${EGH_PREFIX}")
        else()
            string(TOLOWER "${EGH_PREFIX}" originalprefix)
        endif()
    endif()

    foreach(_classnameentry ${EGH_HEADER_NAMES})
        string(REPLACE "," ";" _classnames ${_classnameentry})
        list(GET _classnames 0 _baseclass)

        if (EGH_ORIGINAL STREQUAL "CAMELCASE")
            set(originalbasename "${_baseclass}")
        else()
            string(TOLOWER "${_baseclass}" originalbasename)
        endif()

        set(_actualheader "${CMAKE_CURRENT_SOURCE_DIR}/${EGH_RELATIVE}${originalbasename}.${EGH_HEADER_EXTENSION}")
        get_source_file_property(_generated "${_actualheader}" GENERATED)
        if (NOT _generated AND NOT EXISTS ${_actualheader})
            message(FATAL_ERROR "Could not find \"${_actualheader}\"")
        endif()

        foreach(_CLASSNAME ${_classnames})
            set(FANCY_HEADER_FILE "${EGH_OUTPUT_DIR}/${EGH_PREFIX}${_CLASSNAME}")
            if (NOT EXISTS ${FANCY_HEADER_FILE})
                file(WRITE ${FANCY_HEADER_FILE} "#include \"${originalprefix}${originalbasename}.${EGH_HEADER_EXTENSION}\"\n")
            endif()
            list(APPEND ${camelcase_forwarding_headers_var} "${FANCY_HEADER_FILE}")
            if (EGH_PREFIX)
                # Local forwarding header, for namespaced headers, e.g. kparts/part.h
                if(EGH_ORIGINAL STREQUAL "CAMELCASE")
                    set(originalclassname "${_CLASSNAME}")
                else()
                    string(TOLOWER "${_CLASSNAME}" originalclassname)
                endif()
                set(REGULAR_HEADER_NAME ${EGH_OUTPUT_DIR}/${originalprefix}${originalclassname}.${EGH_HEADER_EXTENSION})
                if (NOT EXISTS ${REGULAR_HEADER_NAME})
                    file(WRITE ${REGULAR_HEADER_NAME} "#include \"${_actualheader}\"\n")
                endif()
            endif()
        endforeach()

        list(APPEND _REQUIRED_HEADERS "${_actualheader}")
    endforeach()

    if(EGH_COMMON_HEADER)
        #combine required headers into 1 big convenience header
        set(COMMON_HEADER ${EGH_OUTPUT_DIR}/${EGH_PREFIX}${EGH_COMMON_HEADER})
        file(WRITE ${COMMON_HEADER} "// convenience header\n")
        foreach(_header ${_REQUIRED_HEADERS})
            get_filename_component(_base ${_header} NAME)
            file(APPEND ${COMMON_HEADER} "#include \"${_base}\"\n")
        endforeach()
        list(APPEND ${camelcase_forwarding_headers_var} "${COMMON_HEADER}")
    endif()

    set(${camelcase_forwarding_headers_var} ${${camelcase_forwarding_headers_var}} PARENT_SCOPE)
    if (EGH_REQUIRED_HEADERS)
        set(${EGH_REQUIRED_HEADERS} ${${EGH_REQUIRED_HEADERS}} ${_REQUIRED_HEADERS} PARENT_SCOPE)
    endif ()
endfunction()
