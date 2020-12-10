

include(GetPlatformInfo)

if (OS_IS_WIN)
    find_path(SNDFILE_INCDIR sndfile.h PATHS ${PROJECT_SOURCE_DIR}/dependencies/include;)
    if (MINGW)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")
    else (MINGW)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
    endif (MINGW)
    find_library(SNDFILE_LIB NAMES sndfile libsndfile-1 PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)

else()
    include(UsePkgConfig1)
    PKGCONFIG1 (sndfile 1.0.25 SNDFILE_INCDIR SNDFILE_LIBDIR SNDFILE_LIB SNDFILE_CPP)
endif()

if (SNDFILE_INCDIR AND SNDFILE_LIB)
    message(STATUS "Found sndfile: ${SNDFILE_LIB} ${SNDFILE_INCDIR}")
else ()
    message(FATAL_ERROR "Could not find: sndfile")
endif ()


