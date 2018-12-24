##
## create package target
##

include (InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MuseScore is a full featured WYSIWYG score editor")
SET(CPACK_PACKAGE_VENDOR "Werner Schweer and Others")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/LICENSE.GPL")
SET(CPACK_RESOURCE_FILE_LICENSE    "${PROJECT_SOURCE_DIR}/LICENSE.GPL")

SET(CPACK_PACKAGE_VERSION_MAJOR "${MUSESCORE_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${MUSESCORE_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${MUSESCORE_VERSION_PATCH}")
SET(CPACK_PACKAGE_VERSION_BUILD "${CMAKE_BUILD_NUMBER}")
SET(CPACK_PACKAGE_VERSION "${MUSESCORE_VERSION_MAJOR}.${MUSESCORE_VERSION_MINOR}.${MUSESCORE_VERSION_PATCH}.${CPACK_PACKAGE_VERSION_BUILD}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "MuseScore ${MUSESCORE_VERSION_MAJOR}.${MUSESCORE_VERSION_MINOR}")

set(git_date_string "")
if (MSCORE_UNSTABLE)
      find_program(GIT_EXECUTABLE git PATHS ENV PATH)
      if (GIT_EXECUTABLE)
            execute_process(
                  COMMAND "${GIT_EXECUTABLE}" log -1 --date=short --format=%cd
                  OUTPUT_VARIABLE git_date
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
      endif (GIT_EXECUTABLE)
      if (git_date)
            STRING(REGEX REPLACE "-" "" git_date "${git_date}")
            set(git_date_string "~git${git_date}")
      endif (git_date)
endif (MSCORE_UNSTABLE)

SET(CPACK_NSIS_COMPRESSOR "/FINAL /SOLID lzma")

IF(MINGW OR MSVC)
    SET(CPACK_PACKAGE_INSTALL_DIRECTORY ${MUSESCORE_NAME_VERSION})
    SET(CPACK_PACKAGE_NAME    ${MUSESCORE_NAME})
    SET(MSCORE_EXECUTABLE_NAME ${MUSESCORE_NAME}${MUSESCORE_VERSION_MAJOR})

    # There is a bug in NSI that does not handle full unix paths properly. Make
    # sure there is at least one set of four (4) backlasshes.
    SET(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/build/packaging\\\\installer_head_nsis.bmp")
    SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\${MSCORE_EXECUTABLE_NAME}.exe,0")
    SET(CPACK_NSIS_DISPLAY_NAME "${MUSESCORE_NAME} ${MUSESCORE_VERSION_FULL}")
    SET(CPACK_NSIS_HELP_LINK "http://www.musescore.org/")
    SET(CPACK_NSIS_URL_INFO_ABOUT "http://www.musescore.org/")
    SET(CPACK_NSIS_CONTACT "info@musescore.org")
    SET(CPACK_NSIS_MODIFY_PATH OFF)
    SET(CPACK_STRIP_FILES "${MSCORE_EXECUTABLE_NAME}.exe")

    # File types association:
    SET(CPACK_NSIS_DEFINES "!include ${PROJECT_SOURCE_DIR}/build/packaging\\\\FileAssociation.nsh")

    SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        \\\${registerExtension} \\\"MuseScore File\\\" \\\".mscx\\\" \\\"\\\$INSTDIR\\\\bin\\\\${MSCORE_EXECUTABLE_NAME}.exe\\\"
        \\\${registerExtension} \\\"Compressed MuseScore File\\\" \\\".mscz\\\" \\\"\\\$INSTDIR\\\\bin\\\\${MSCORE_EXECUTABLE_NAME}.exe\\\"
    ")
    SET(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
        \\\${unregisterExtension} \\\".mscx\\\" \\\"MuseScore File\\\"
        \\\${unregisterExtension} \\\".mscz\\\" \\\"Compressed MuseScore File\\\"
    ")

    file(TO_CMAKE_PATH $ENV{PROGRAMFILES} PROGRAMFILES)
    SET(CPACK_WIX_ROOT "${PROGRAMFILES}/WiX Toolset v3.11")
    SET(CPACK_WIX_PRODUCT_GUID "00000000-0000-0000-0000-000000000000")
    SET(CPACK_WIX_UPGRADE_GUID "429B1F11-5C0E-4641-BE19-7C2C7C24FF21")
    SET(CPACK_WIX_LICENSE_RTF   "${PROJECT_SOURCE_DIR}/LICENSE.rtf")
    SET(CPACK_WIX_PRODUCT_ICON "${PROJECT_SOURCE_DIR}/mscore/data/mscore.ico")
    SET(CPACK_WIX_UI_BANNER "${PROJECT_SOURCE_DIR}/build/packaging/installer_banner_wix.png")
    SET(CPACK_WIX_UI_DIALOG "${PROJECT_SOURCE_DIR}/build/packaging/installer_background_wix.png")
    SET(CPACK_WIX_PROGRAM_MENU_FOLDER "${MUSESCORE_NAME_VERSION}")
    SET(CPACK_CREATE_DESKTOP_LINKS "${MUSESCORE_NAME_VERSION}")
    SET(CPACK_WIX_EXTENSIONS "WixUtilExtension")

    SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build/packaging" ${CMAKE_MODULE_PATH}) # Use custom version of NSIS.InstallOptions.ini
    SET(CPACK_PACKAGE_EXECUTABLES   "${MSCORE_EXECUTABLE_NAME}" "${MUSESCORE_NAME_VERSION}") # exe name, label
    SET(CPACK_CREATE_DESKTOP_LINKS "${MSCORE_EXECUTABLE_NAME}" "${MUSESCORE_NAME_VERSION}") #exe name, label

    SET(CPACK_PACKAGE_FILE_NAME     "${MUSESCORE_NAME}-${MUSESCORE_VERSION_FULL}${git_date_string}")
ELSE(MINGW OR MSVC)
    SET(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/mscore/data/mscore.bmp")
    SET(CPACK_STRIP_FILES "${MSCORE_OUTPUT_NAME}")
    SET(CPACK_SOURCE_STRIP_FILES "")
    SET(CPACK_PACKAGE_EXECUTABLES   "mscore" "MuseScore")
    SET(CPACK_SOURCE_PACKAGE_FILE_NAME "mscore")
    SET(CPACK_PACKAGE_FILE_NAME     "${CPACK_SOURCE_PACKAGE_FILE_NAME}-${MUSESCORE_VERSION_FULL}${git_date_string}")
ENDIF(MINGW OR MSVC)

set(CPACK_DEBIAN_PACKAGE_NAME         "mscore")
set(CPACK_DEBIAN_PACKAGE_VERSION      "${MUSESCORE_VERSION_FULL}${git_date_string}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER   "tsmithe@ubuntu.com")
set(CPACK_DEBIAN_PACKAGE_SECTION      "devel")
set(CPACK_DEBIAN_PACKAGE_PRIORITY     "optional")
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS   "")
set(CPACK_DEBIAN_PACKAGE_SUGGESTS     "")

set(CPACK_PACKAGE_CONTACT       "info@musescore.org")

if (MINGW OR MSVC)
  set(CPACK_GENERATOR             "WIX")
else (MINGW OR MSVC)
   if (NOT APPLE)
     set(CPACK_GENERATOR             "DEB;TBZ2")
     set(CPACK_DEB "on")
   endif (NOT APPLE)
endif (MINGW OR MSVC)


if (CPACK_DEB)
      find_program(DPKG_EXECUTABLE dpkg PATHS ENV PATH)
      if (DPKG_EXECUTABLE)
            set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS	"ON")
            execute_process(
                  COMMAND "${DPKG_EXECUTABLE} --print-architecture"
                  OUTPUT_VARIABLE dpkg_architecture
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
            set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${dpkg_architecture}")
      endif (DPKG_EXECUTABLE)
endif (CPACK_DEB)

include (CPack)
