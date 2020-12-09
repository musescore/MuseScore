

include(GetPlatformInfo)

if (OS_IS_WIN)
    find_path(SNDFILE_INCDIR sndfile.h PATHS ${PROJECT_SOURCE_DIR}/dependencies/include;)
    if (MINGW)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")
    else (MINGW)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
    endif (MINGW)
    find_library(SNDFILE_LIB NAMES sndfile libsndfile-1 PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)

    if (SNDFILE_INCDIR AND SNDFILE_LIB)
        set(SNDFILE_FOUND TRUE)
        install(FILES ${SNDFILE_LIB} DESTINATION bin)
    endif (SNDFILE_INCDIR AND SNDFILE_LIB)

else()

    PKGCONFIG1 (sndfile 1.0.25 SNDFILE_INCDIR SNDFILE_LIBDIR SNDFILE_LIB SNDFILE_CPP)
    if (SNDFILE_INCDIR)
        set(SNDFILE_FOUND TRUE)
    endif()

endif()

if (SNDFILE_FOUND)
    message(STATUS "Found sndfile: ${SNDFILE_LIB} ${SNDFILE_INCDIR}")
else (SNDFILE_FOUND)
    message(FATAL_ERROR "Could not find: sndfile")
endif (SNDFILE_FOUND)


