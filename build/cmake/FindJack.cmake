
##
## Find JACK >= JACK_MIN_VERSION
##

IF(MINGW OR MSVC)
   set (USE_JACK 1)
   IF("$ENV{PROCESSOR_ARCHITEW6432}" STREQUAL "")
      IF("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
         # "pure" 32-bit environment
         set (progenv "PROGRAMFILES")
      ELSE("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
         # "pure" 64-bit environment
         set (progenv "PROGRAMFILES(x86)")
      ENDIF("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
   ELSE("$ENV{PROCESSOR_ARCHITEW6432}" STREQUAL "")
      IF("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
         # 32-bit program running with an underlying 64-bit environment
         set (progenv "PROGRAMFILES(x86)")
      ELSE("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
         # Theoretically impossible case...
         MESSAGE(SEND_ERROR "Error: Impossible program/environment bitness combination deduced: 64-bit program running in 32-bit environment. This is a programming error. PROCESSOR_ARCHITEW6432=$ENV{PROCESSOR_ARCHITEW6432}. PROCESSOR_ARCHITECTURE=$ENV{PROCESSOR_ARCHITECTURE}")
      ENDIF("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "x86")
   ENDIF("$ENV{PROCESSOR_ARCHITEW6432}" STREQUAL "")
   set (JACK_INCDIR "$ENV{${progenv}}/Jack/includes")
   set (JACK_LIB "$ENV{${progenv}}/Jack/lib/libjack.a")
   MESSAGE("JACK support enabled.")
ELSE(MINGW OR MSVC)
   include(UsePkgConfig1)
   PKGCONFIG1 (jack ${JACK_MIN_VERSION} JACK_INCDIR JACK_LIBDIR JACK_LIB JACK_CPP)
   IF(JACK_INCDIR)
         MESSAGE(STATUS "${JACK_LONGNAME} >= ${JACK_MIN_VERSION} found. jack support enabled.")
         SET(USE_JACK 1)
   ELSE(JACK_INCDIR)
         MESSAGE(STATUS "${JACK_LONGNAME} >= ${JACK_MIN_VERSION} not found")
         MESSAGE(SEND_ERROR "Error: JACK support requested, but JACK was not found")
   ENDIF(JACK_INCDIR)
ENDIF(MINGW OR MSVC)

