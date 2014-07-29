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

IF(MINGW)
    SET(CPACK_PACKAGE_INSTALL_DIRECTORY "MuseScore")
    # There is a bug in NSI that does not handle full unix paths properly. Make
    # sure there is at least one set of four (4) backlasshes.
    SET(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/mscore/data\\\\installerhead.bmp")
    SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\mscore.exe,0")
    SET(CPACK_NSIS_DISPLAY_NAME "MuseScore ${MUSESCORE_VERSION_FULL}")
    SET(CPACK_NSIS_HELP_LINK "http://www.musescore.org/")
    SET(CPACK_NSIS_URL_INFO_ABOUT "http://www.musescore.org/")
    SET(CPACK_NSIS_CONTACT "ws@wschweer.de")
    SET(CPACK_NSIS_MODIFY_PATH OFF)
    SET(CPACK_STRIP_FILES "mscore.exe")

    # File types association:
    SET(CPACK_NSIS_DEFINES "!include ${PROJECT_SOURCE_DIR}/build\\\\FileAssociation.nsh")

    SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        Push \\\"ATENDATA\\\"
        Push \\\"$INSTDIR\\\\share\\\\aten\\\"
        Call WriteEnvStr
    ")

    SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        \\\${registerExtension} \\\"MuseScore File\\\" \\\".mscx\\\" \\\"\\\$INSTDIR\\\\bin\\\\mscore.exe\\\"
        \\\${registerExtension} \\\"Compressed MuseScore File\\\" \\\".mscz\\\" \\\"\\\$INSTDIR\\\\bin\\\\mscore.exe\\\"
    ")
    SET(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
        \\\${unregisterExtension} \\\".mscx\\\" \\\"MuseScore File\\\"
        \\\${unregisterExtension} \\\".mscz\\\" \\\"Compressed MuseScore File\\\"
    ")
ELSE(MINGW)
    SET(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/mscore/data/mscore.bmp")
    SET(CPACK_STRIP_FILES "mscore")
    SET(CPACK_SOURCE_STRIP_FILES "")
ENDIF(MINGW)

SET(CPACK_SOURCE_PACKAGE_FILE_NAME "mscore")
SET(CPACK_PACKAGE_FILE_NAME     "${CPACK_SOURCE_PACKAGE_FILE_NAME}-${MUSESCORE_VERSION_FULL}${git_date_string}")
SET(CPACK_PACKAGE_EXECUTABLES   "mscore" "MuseScore")

set(CPACK_DEBIAN_PACKAGE_NAME         "mscore")
set(CPACK_DEBIAN_PACKAGE_VERSION      "${MUSESCORE_VERSION_FULL}${git_date_string}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER   "tsmithe@ubuntu.com")
set(CPACK_DEBIAN_PACKAGE_SECTION      "devel")
set(CPACK_DEBIAN_PACKAGE_PRIORITY     "optional")
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS   "")
set(CPACK_DEBIAN_PACKAGE_SUGGESTS     "")

set(CPACK_PACKAGE_CONTACT       "ws@schweer.de")

if (MINGW)
  set(CPACK_GENERATOR             "NSIS")
else (MINGW)
   if (NOT APPLE)
     set(CPACK_GENERATOR             "DEB;TBZ2")
     set(CPACK_DEB "on")
   endif (NOT APPLE)
endif (MINGW)


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
