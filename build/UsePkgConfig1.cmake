# - pkg-config module for CMake
#
# Defines the following macro:
#
# PKGCONFIG1(package includedir libdir linkflags cflags)
#
# Calling PKGCONFIG1 will fill the desired information into the 4 given arguments,
# e.g. PKGCONFIG1(libart-2.0 LIBART_INCLUDE_DIR LIBART_LINK_DIR LIBART_LINK_FLAGS LIBART_CFLAGS)
# if pkg-config was NOT found or the specified software package doesn't exist, the
# variable will be empty when the function returns, otherwise they will contain the respective information
#
IF (NOT MINGW AND NOT MSVC)
  FIND_PROGRAM(PKGCONFIG_EXECUTABLE NAMES pkg-config PATHS ${PATH} /usr/bin /usr/local/bin )

  MACRO(PKGCONFIG1 _package _minVersion _include_DIR _link_DIR _link_FLAGS _cflags)
  # Reset the variables at the beginning
    SET(${_include_DIR})
    SET(${_link_DIR})
    SET(${_link_FLAGS})
    SET(${_cflags})

    # If pkg-config has been found
    IF(PKGCONFIG_EXECUTABLE)

      EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --atleast-version=${_minVersion} RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )
      #EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --exists RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )

      # and if the package of interest also exists for pkg-config, then get the information
      IF(NOT _return_VALUE)

        EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --variable=includedir OUTPUT_VARIABLE ${_include_DIR} )

        EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --variable=libdir OUTPUT_VARIABLE ${_link_DIR} )

        EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --libs OUTPUT_VARIABLE ${_link_FLAGS} )

        EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --cflags OUTPUT_VARIABLE ${_cflags} )

      ENDIF(NOT _return_VALUE)

    ENDIF(PKGCONFIG_EXECUTABLE)

  ENDMACRO(PKGCONFIG1 _include_DIR _link_DIR _link_FLAGS _cflags)

  MARK_AS_ADVANCED(PKGCONFIG_EXECUTABLE)
ENDIF (NOT MINGW AND NOT MSVC)

