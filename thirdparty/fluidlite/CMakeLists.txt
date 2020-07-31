cmake_minimum_required(VERSION 3.1)

include(FindPkgConfig)
include(GNUInstallDirs)

project(fluidlite)

list(APPEND HEADERS
    include/fluidlite.h
    )

list(APPEND SCOPED_HEADERS
    include/fluidsynth/types.h
    include/fluidsynth/settings.h
    include/fluidsynth/synth.h
    include/fluidsynth/sfont.h
    include/fluidsynth/ramsfont.h
    include/fluidsynth/log.h
    include/fluidsynth/misc.h
    include/fluidsynth/mod.h
    include/fluidsynth/gen.h
    include/fluidsynth/voice.h
    include/fluidsynth/version.h
)

list(APPEND SOURCES
    src/fluid_chan.c
    src/fluid_chorus.c
    src/fluid_conv.c
    src/fluid_defsfont.c
    src/fluid_dsp_float.c
    src/fluid_gen.c
    src/fluid_hash.c
    src/fluid_list.c
    src/fluid_mod.c
    src/fluid_ramsfont.c
    src/fluid_rev.c
    src/fluid_settings.c
    src/fluid_synth.c
    src/fluid_sys.c
    src/fluid_tuning.c
    src/fluid_voice.c
)

# Dependencies:
pkg_check_modules(LIBVORBIS vorbis>=1.3.5)
pkg_check_modules(LIBVORBISFILE vorbisfile>=1.3.5)
if (NOT LIBVORBIS_FOUND OR NOT LIBVORBISFILE_FOUND)
    list(APPEND SOURCES
        libvorbis-1.3.5/lib/vorbisenc.c
        libvorbis-1.3.5/lib/info.c
        libvorbis-1.3.5/lib/analysis.c
        libvorbis-1.3.5/lib/bitrate.c
        libvorbis-1.3.5/lib/block.c
        libvorbis-1.3.5/lib/codebook.c
        libvorbis-1.3.5/lib/envelope.c
        libvorbis-1.3.5/lib/floor0.c
        libvorbis-1.3.5/lib/floor1.c
        libvorbis-1.3.5/lib/lookup.c
        libvorbis-1.3.5/lib/lpc.c
        libvorbis-1.3.5/lib/lsp.c
        libvorbis-1.3.5/lib/mapping0.c
        libvorbis-1.3.5/lib/mdct.c
        libvorbis-1.3.5/lib/psy.c
        libvorbis-1.3.5/lib/registry.c
        libvorbis-1.3.5/lib/res0.c
        libvorbis-1.3.5/lib/sharedbook.c
        libvorbis-1.3.5/lib/smallft.c
        libvorbis-1.3.5/lib/vorbisfile.c
        libvorbis-1.3.5/lib/window.c
        libvorbis-1.3.5/lib/synthesis.c
    )
    list(APPEND LIBVORBIS_INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/libvorbis-1.3.5/include
        ${CMAKE_SOURCE_DIR}/libvorbis-1.3.5/lib
    )
    message(WARNING "Using libvorbis shipped sources.")
else()
    message(STATUS "Using pkg-config provided libvorbis")
    set(ADDITIONAL_LIBS "${LIBVORBIS_LDFLAGS} ${LIBVORBISFILE_LDFLAGS}")
endif()

pkg_check_modules(LIBOGG ogg>=1.3.2)
if (NOT LIBOGG_FOUND)
    list(APPEND SOURCES
        libogg-1.3.2/src/bitwise.c
        libogg-1.3.2/src/framing.c
    )
    set(LIBOGG_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/libogg-1.3.2/include)
    message(WARNING "Using libogg shipped sources.")
else()
    message(STATUS "Using pkg-config provided libogg")
    string(CONCAT ADDITIONAL_LIBS "${ADDITIONAL_LIBS} ${LIBOGG_LDFLAGS}")
endif()

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${LIBOGG_INCLUDE_DIRS})
include_directories(${LIBVORBIS_INCLUDE_DIRS})

option(FLUIDLITE_BUILD_STATIC "Build static library" TRUE)
if(FLUIDLITE_BUILD_STATIC)
	add_library(${PROJECT_NAME}-static STATIC ${SOURCES})
	set_target_properties(${PROJECT_NAME}-static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
	set(FLUIDLITE_LIB_TARGET ${PROJECT_NAME}-static)
	set(FLUIDLITE_INSTALL_TARGETS ${FLUIDLITE_INSTALL_TARGETS} ";fluidlite-static")
endif()

option(FLUIDLITE_BUILD_SHARED "Build shared library" TRUE)
if(FLUIDLITE_BUILD_SHARED)
	add_library(${PROJECT_NAME} SHARED ${SOURCES})
	target_link_libraries(${PROJECT_NAME}
		${LIBVORBIS_LIBRARIES}
		${LIBVORBISFILE_LIBRARIES}
		${LIBOGG_LIBRARIES}
	)
	set(FLUIDLITE_LIB_TARGET ${PROJECT_NAME})
	set(FLUIDLITE_INSTALL_TARGETS ${FLUIDLITE_INSTALL_TARGETS} ";fluidlite")
endif()

if(FLUIDLITE_BUILD_SHARED AND FLUIDLITE_BUILD_STATIC)
	set_target_properties(${PROJECT_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
	set_target_properties(${PROJECT_NAME}-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
endif()

configure_file(fluidlite.pc.in ${CMAKE_BINARY_DIR}/fluidlite.pc @ONLY)

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 99)
install(TARGETS ${FLUIDLITE_INSTALL_TARGETS}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${SCOPED_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fluidsynth)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fluidlite.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
