# FluidSynth - A Software Synthesizer
#
# Copyright (C) 2003-2010 Peter Hanappe and others.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation; either version 2.1 of
# the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307, USA

# CMake based build system. Pedro Lopez-Cabanillas <plcl@users.sf.net>

# ************ library ************

if ( PULSE_SUPPORT )
  set ( fluid_pulse_SOURCES drivers/fluid_pulse.c )
endif ( PULSE_SUPPORT )

if ( ALSA_SUPPORT )
  set ( fluid_alsa_SOURCES drivers/fluid_alsa.c )
endif ( ALSA_SUPPORT )

if ( COREAUDIO_SUPPORT )
  set ( fluid_coreaudio_SOURCES drivers/fluid_coreaudio.c )
endif ( COREAUDIO_SUPPORT )

if ( COREMIDI_SUPPORT )
  set ( fluid_coremidi_SOURCES drivers/fluid_coremidi.c )
endif ( COREMIDI_SUPPORT )

if ( DBUS_SUPPORT )
  set ( fluid_dbus_SOURCES bindings/fluid_rtkit.c bindings/fluid_rtkit.h )
endif ( DBUS_SUPPORT )

if ( JACK_SUPPORT )
  set ( fluid_jack_SOURCES drivers/fluid_jack.c )
endif ( JACK_SUPPORT )

if ( PIPEWIRE_SUPPORT )
  set ( fluid_pipewire_SOURCES drivers/fluid_pipewire.c )
endif ( PIPEWIRE_SUPPORT )

if ( PORTAUDIO_SUPPORT )
  set ( fluid_portaudio_SOURCES drivers/fluid_portaudio.c )
endif ( PORTAUDIO_SUPPORT )

if ( DSOUND_SUPPORT )
  set ( fluid_dsound_SOURCES drivers/fluid_dsound.c )
endif ( DSOUND_SUPPORT )

if ( WASAPI_SUPPORT )
  set ( fluid_wasapi_SOURCES drivers/fluid_wasapi.c )
endif ( WASAPI_SUPPORT )

if ( WAVEOUT_SUPPORT )
  set ( fluid_waveout_SOURCES drivers/fluid_waveout.c )
endif ( WAVEOUT_SUPPORT )

if ( WINMIDI_SUPPORT )
  set ( fluid_winmidi_SOURCES drivers/fluid_winmidi.c )
endif ( WINMIDI_SUPPORT )

if ( SDL2_SUPPORT )
  set ( fluid_sdl2_SOURCES drivers/fluid_sdl2.c )
endif ( SDL2_SUPPORT )

if ( OSS_SUPPORT )
  set ( fluid_oss_SOURCES drivers/fluid_oss.c )
endif ( OSS_SUPPORT )

if ( LASH_SUPPORT )
  set ( fluid_lash_SOURCES bindings/fluid_lash.c bindings/fluid_lash.h )
endif ( LASH_SUPPORT )

if ( DART_SUPPORT )
  set ( fluid_dart_SOURCES drivers/fluid_dart.c )
endif ( DART_SUPPORT )

if ( MIDISHARE_SUPPORT )
  set ( fluid_midishare_SOURCES drivers/fluid_midishare.c )
endif ( MIDISHARE_SUPPORT )

if ( AUFILE_SUPPORT )
  set ( fluid_aufile_SOURCES drivers/fluid_aufile.c )
endif ( AUFILE_SUPPORT )

if ( LIBINSTPATCH_SUPPORT )
  set ( fluid_libinstpatch_SOURCES sfloader/fluid_instpatch.c sfloader/fluid_instpatch.h )
endif ( LIBINSTPATCH_SUPPORT )

if ( OPENSLES_SUPPORT )
  set ( fluid_opensles_SOURCES drivers/fluid_opensles.c )
endif ( OPENSLES_SUPPORT )

if ( OBOE_SUPPORT )
  set ( fluid_oboe_SOURCES drivers/fluid_oboe.cpp )
endif ( OBOE_SUPPORT )

set ( config_SOURCES ${FluidSynth_BINARY_DIR}/config.h )

set ( libfluidsynth_SOURCES
    utils/fluid_conv.c
    utils/fluid_conv.h
    utils/fluid_hash.c
    utils/fluid_hash.h
    utils/fluid_list.c
    utils/fluid_list.h
    utils/fluid_ringbuffer.c
    utils/fluid_ringbuffer.h
    utils/fluid_settings.c
    utils/fluid_settings.h
    utils/fluidsynth_priv.h
    utils/fluid_sys.c
    utils/fluid_sys.h
    sfloader/fluid_defsfont.c
    sfloader/fluid_defsfont.h
    sfloader/fluid_sfont.h
    sfloader/fluid_sfont.c
    sfloader/fluid_sffile.c
    sfloader/fluid_sffile.h
    sfloader/fluid_samplecache.c
    sfloader/fluid_samplecache.h
    rvoice/fluid_adsr_env.c
    rvoice/fluid_adsr_env.h
    rvoice/fluid_chorus.c
    rvoice/fluid_chorus.h
    rvoice/fluid_iir_filter.c
    rvoice/fluid_iir_filter.h
    rvoice/fluid_lfo.c
    rvoice/fluid_lfo.h
    rvoice/fluid_rvoice.h
    rvoice/fluid_rvoice.c
    rvoice/fluid_rvoice_dsp.c
    rvoice/fluid_rvoice_event.h
    rvoice/fluid_rvoice_event.c
    rvoice/fluid_rvoice_mixer.h
    rvoice/fluid_rvoice_mixer.c
    rvoice/fluid_phase.h
    rvoice/fluid_rev.c
    rvoice/fluid_rev.h
    synth/fluid_chan.c
    synth/fluid_chan.h
    synth/fluid_event.c
    synth/fluid_event.h
    synth/fluid_gen.c
    synth/fluid_gen.h
    synth/fluid_mod.c
    synth/fluid_mod.h
    synth/fluid_synth.c
    synth/fluid_synth.h
    synth/fluid_synth_monopoly.c
    synth/fluid_tuning.c
    synth/fluid_tuning.h
    synth/fluid_voice.c
    synth/fluid_voice.h
    midi/fluid_midi.c
    midi/fluid_midi.h
    midi/fluid_midi_router.c
    midi/fluid_midi_router.h
    midi/fluid_seqbind.c
    midi/fluid_seqbind_notes.cpp
    midi/fluid_seq.c
    midi/fluid_seq_queue.cpp
    drivers/fluid_adriver.c
    drivers/fluid_adriver.h
    drivers/fluid_mdriver.c
    drivers/fluid_mdriver.h
    bindings/fluid_cmd.c
    bindings/fluid_cmd.h
    bindings/fluid_filerenderer.c
    bindings/fluid_ladspa.c
    bindings/fluid_ladspa.h
)

set ( public_HEADERS
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/audio.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/event.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/gen.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/ladspa.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/log.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/midi.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/misc.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/mod.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/seq.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/seqbind.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/settings.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/sfont.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/shell.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/synth.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/types.h
    ${FluidSynth_SOURCE_DIR}/include/fluidsynth/voice.h
    ${FluidSynth_BINARY_DIR}/include/fluidsynth/version.h
)

set ( public_main_HEADER
    ${FluidSynth_BINARY_DIR}/include/fluidsynth.h
)

configure_file ( ${FluidSynth_SOURCE_DIR}/include/fluidsynth/version.h.in 
                 ${FluidSynth_BINARY_DIR}/include/fluidsynth/version.h )
configure_file ( ${FluidSynth_SOURCE_DIR}/include/fluidsynth.cmake
                 ${public_main_HEADER} )

if ( WIN32 )
include(generate_product_version)
generate_product_version(
    VersionFilesOutputVariable
    NAME "Fluidsynth"
    BUNDLE "Fluidsynth"
    VERSION_MAJOR ${FLUIDSYNTH_VERSION_MAJOR}
    VERSION_MINOR ${FLUIDSYNTH_VERSION_MINOR}
    VERSION_PATCH ${FLUIDSYNTH_VERSION_MICRO}
    VERSION_REVISION 0
    COMMENTS "Fluidsynth"
    COMPANY_NAME "Fluidsynth LGPL"
    ORIGINAL_FILENAME "libfluidsynth.dll"
    FILE_DESCRIPTION "Fluidsynth"
)
endif ( WIN32 )

add_library ( libfluidsynth-OBJ OBJECT
    ${config_SOURCES}
    ${fluid_alsa_SOURCES}
    ${fluid_aufile_SOURCES}
    ${fluid_coreaudio_SOURCES}
    ${fluid_coremidi_SOURCES}
    ${fluid_dart_SOURCES}
    ${fluid_dbus_SOURCES}
    ${fluid_jack_SOURCES}
    ${fluid_pipewire_SOURCES}
    ${fluid_lash_SOURCES}
    ${fluid_midishare_SOURCES}
    ${fluid_opensles_SOURCES}
    ${fluid_oboe_SOURCES}
    ${fluid_oss_SOURCES}
    ${fluid_portaudio_SOURCES}
    ${fluid_pulse_SOURCES}
    ${fluid_dsound_SOURCES}
    ${fluid_wasapi_SOURCES}
    ${fluid_waveout_SOURCES}
    ${fluid_winmidi_SOURCES}
    ${fluid_sdl2_SOURCES}
    ${fluid_libinstpatch_SOURCES}
    ${libfluidsynth_SOURCES}
    ${public_HEADERS}
    ${public_main_HEADER}
    ${VersionFilesOutputVariable}
)

target_include_directories ( libfluidsynth-OBJ PRIVATE
    ${FluidSynth_BINARY_DIR}
    ${FluidSynth_BINARY_DIR}/include
    ${FluidSynth_SOURCE_DIR}/src
    ${FluidSynth_SOURCE_DIR}/src/drivers
    ${FluidSynth_SOURCE_DIR}/src/synth
    ${FluidSynth_SOURCE_DIR}/src/rvoice
    ${FluidSynth_SOURCE_DIR}/src/midi
    ${FluidSynth_SOURCE_DIR}/src/utils
    ${FluidSynth_SOURCE_DIR}/src/sfloader
    ${FluidSynth_SOURCE_DIR}/src/bindings
    ${FluidSynth_SOURCE_DIR}/include
)

if ( LIBFLUID_CPPFLAGS )
  set_target_properties ( libfluidsynth-OBJ
    PROPERTIES COMPILE_FLAGS ${LIBFLUID_CPPFLAGS} )
endif ( LIBFLUID_CPPFLAGS )

# Note: by default this target creates a shared object (or dll). To build a
# static library instead, set the option BUILD_SHARED_LIBS to FALSE.
# Further note: The headers must be explicitly added here to have CMake install
# them correctly in case of MACOSX_FRAMEWORK
add_library ( libfluidsynth
    $<TARGET_OBJECTS:libfluidsynth-OBJ>
    ${public_main_HEADER}
    ${public_HEADERS}
)

if ( MACOSX_FRAMEWORK )
    set_source_files_properties ( ${public_HEADERS}
         PROPERTIES MACOSX_PACKAGE_LOCATION Headers/fluidsynth
     )
    set_target_properties ( libfluidsynth
      PROPERTIES
        OUTPUT_NAME "FluidSynth"
        FRAMEWORK TRUE
        PUBLIC_HEADER "${public_main_HEADER}"
        FRAMEWORK_VERSION "${LIB_VERSION_CURRENT}"
        INSTALL_NAME_DIR ""
        VERSION ${LIB_VERSION_INFO}
        SOVERSION ${LIB_VERSION_CURRENT}
    )
elseif ( OS2 )
    set_target_properties ( libfluidsynth
      PROPERTIES
        PUBLIC_HEADER "${public_HEADERS}"
        OUTPUT_NAME "fluidsynth"
        VERSION ${LIB_VERSION_INFO}
        SOVERSION ${LIB_VERSION_CURRENT}
    )
elseif ( WIN32 )
  set_target_properties ( libfluidsynth
    PROPERTIES
      PUBLIC_HEADER "${public_HEADERS}"
      ARCHIVE_OUTPUT_NAME "fluidsynth"
      PREFIX "lib"
      OUTPUT_NAME "fluidsynth-${LIB_VERSION_CURRENT}"
      VERSION ${LIB_VERSION_INFO}
      SOVERSION ${LIB_VERSION_CURRENT}
    )
elseif ( CYGWIN )
  set_target_properties ( libfluidsynth
    PROPERTIES
      PUBLIC_HEADER "${public_HEADERS}"
      PREFIX "cyg"
      OUTPUT_NAME "fluidsynth"
      VERSION ${LIB_VERSION_INFO}
      SOVERSION ${LIB_VERSION_CURRENT}
    )
else ( MACOSX_FRAMEWORK )
  set_target_properties ( libfluidsynth
    PROPERTIES
      PUBLIC_HEADER "${public_HEADERS}"
      PREFIX "lib"
      OUTPUT_NAME "fluidsynth"
      VERSION ${LIB_VERSION_INFO}
      SOVERSION ${LIB_VERSION_CURRENT}
  )
endif ( MACOSX_FRAMEWORK )

# Since CMake 3.12, OBJECT libraries can be linked to with target_link_libraries().
# See https://cmake.org/cmake/help/latest/command/target_link_libraries.html#linking-object-libraries
# Object Libraries may "link" to other libraries to get usage requirements,
# but since they do not have a link step nothing is done with their object files.
target_link_libraries ( libfluidsynth-OBJ PUBLIC
    ${DART_LIBS}
    ${COREAUDIO_LIBS}
    ${COREMIDI_LIBS}
    ${WINDOWS_LIBS}
    ${LIBFLUID_LIBS}
)

if ( TARGET OpenMP::OpenMP_C AND HAVE_OPENMP )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC OpenMP::OpenMP_C )
endif()

target_link_libraries ( libfluidsynth-OBJ PUBLIC GLib2::glib-2 GLib2::gthread-2 )

if ( TARGET SndFile::sndfile AND LIBSNDFILE_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC SndFile::sndfile )
endif()

if ( PULSE_SUPPORT )
    target_include_directories( libfluidsynth-OBJ PRIVATE ${PULSEAUDIO_INCLUDE_DIRS} )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC ${PULSEAUDIO_LIBRARIES} )
endif()

if ( TARGET ALSA::ALSA AND ALSA_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC ALSA::ALSA )
endif()

if ( TARGET PortAudio::PortAudio AND PORTAUDIO_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC PortAudio::PortAudio )
endif()

if ( TARGET Jack::Jack AND JACK_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC Jack::Jack )
endif()

if ( TARGET PipeWire::PipeWire AND PIPEWIRE_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC PipeWire::PipeWire )
endif()

if ( TARGET LASH::LASH AND LASH_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC LASH::LASH )
endif()

if ( TARGET dbus-1 AND DBUS_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC dbus-1 )
endif()

if ( TARGET GLib2::gmodule-2 AND LADSPA_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC GLib2::gmodule-2 )
endif()

if ( TARGET InstPatch::libinstpatch AND LIBINSTPATCH_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC InstPatch::libinstpatch )
endif()

if ( SDL2_SUPPORT )
    target_include_directories ( libfluidsynth-OBJ PRIVATE ${SDL2_INCLUDE_DIRS} )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC ${SDL2_LIBRARIES} )
endif()

if ( TARGET oboe::oboe AND OBOE_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC oboe::oboe )
endif()

if ( TARGET Readline::Readline AND READLINE_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC Readline::Readline )
endif()

if ( DART_SUPPORT )
    target_include_directories ( libfluidsynth-OBJ PUBLIC ${DART_INCLUDE_DIRS} )
endif ( DART_SUPPORT )

if ( MIDISHARE_SUPPORT )
    target_link_libraries ( libfluidsynth-OBJ PUBLIC MidiShare::MidiShare )
endif ( MIDISHARE_SUPPORT )

if ( OPENSLES_SUPPORT )
    target_link_libraries( libfluidsynth-OBJ PUBLIC OpenSLES::OpenSLES )
endif ( OPENSLES_SUPPORT )

# This doesn't install any object file at all!
# it is only a trick for exporting targets
install( TARGETS libfluidsynth-OBJ
    EXPORT FluidSynthTargets
    OBJECTS )

# Here are applied/linked the OBJECT library dependencies
target_link_libraries ( libfluidsynth PRIVATE libfluidsynth-OBJ )

# ************ CLI program ************

set ( fluidsynth_SOURCES fluidsynth.c )

if ( WASAPI_SUPPORT )
  set ( fluidsynth_SOURCES ${fluidsynth_SOURCES} fluid_wasapi_device_enumerate.c )
endif ( WASAPI_SUPPORT )

add_executable ( fluidsynth
    ${fluidsynth_SOURCES}
)

set_target_properties ( fluidsynth
    PROPERTIES IMPORT_PREFIX "" )

if ( FLUID_CPPFLAGS )
  set_target_properties ( fluidsynth
    PROPERTIES COMPILE_FLAGS ${FLUID_CPPFLAGS} )
endif ( FLUID_CPPFLAGS )

target_include_directories ( fluidsynth PRIVATE
    ${FluidSynth_BINARY_DIR}
    ${FluidSynth_BINARY_DIR}/include
    ${FluidSynth_SOURCE_DIR}/src/bindings
    ${FluidSynth_SOURCE_DIR}/src/midi
    ${FluidSynth_SOURCE_DIR}/src/rvoice
    ${FluidSynth_SOURCE_DIR}/src/sfloader
    ${FluidSynth_SOURCE_DIR}/src/synth
    ${FluidSynth_SOURCE_DIR}/src/utils
    ${FluidSynth_SOURCE_DIR}/src/
    ${FluidSynth_SOURCE_DIR}/include
)

target_link_libraries ( fluidsynth PRIVATE
    libfluidsynth
    ${FLUID_LIBS}
)

if ( TARGET OpenMP::OpenMP_C AND HAVE_OPENMP )
    target_link_libraries ( fluidsynth PRIVATE OpenMP::OpenMP_C )
endif()

if ( TARGET Systemd::libsystemd AND SYSTEMD_SUPPORT )
    target_link_libraries ( fluidsynth PRIVATE Systemd::libsystemd )
endif()

if ( TARGET GLib2::glib-2 ) # because g_file_test()
    target_link_libraries ( fluidsynth PRIVATE GLib2::glib-2 )
endif()

if ( SDL2_SUPPORT ) # because SDL_Init() etc.
    target_include_directories ( fluidsynth PRIVATE ${SDL2_INCLUDE_DIRS} )
    target_link_libraries ( fluidsynth PRIVATE ${SDL2_LIBRARIES} )
endif()

if ( TARGET PipeWire::PipeWire AND PIPEWIRE_SUPPORT ) # because pw_init() etc.
    target_link_libraries ( fluidsynth PRIVATE PipeWire::PipeWire )
endif()

if ( TARGET InstPatch::libinstpatch AND LIBINSTPATCH_SUPPORT )
    target_link_libraries ( fluidsynth PRIVATE InstPatch::libinstpatch )
endif()

if ( TARGET LASH::LASH AND LASH_SUPPORT )
    target_link_libraries ( fluidsynth PRIVATE LASH::LASH )
endif()

if ( MACOSX_FRAMEWORK )
  install ( TARGETS fluidsynth libfluidsynth
    EXPORT FluidSynthTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    FRAMEWORK DESTINATION "${FRAMEWORK_INSTALL_PREFIX}/${FRAMEWORK_INSTALL_DIR}"
    ARCHIVE DESTINATION "${FRAMEWORK_INSTALL_PREFIX}/${FRAMEWORK_INSTALL_DIR}"
)
else ( MACOSX_FRAMEWORK )
  install ( TARGETS fluidsynth libfluidsynth
    EXPORT FluidSynthTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fluidsynth
)
   install ( FILES ${public_main_HEADER} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} )
endif ( MACOSX_FRAMEWORK )

# Exported targets.

# build_interface: for the libfluidsynth target when imported from the build directory.
# install_interface: for the target when imported from the installed directory.
target_include_directories(libfluidsynth PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/;${PROJECT_BINARY_DIR}/include/>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# installation of the exported targets
install(EXPORT FluidSynthTargets
        FILE FluidSynthTargets.cmake
        NAMESPACE FluidSynth::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fluidsynth
)

# ******* Auto Generated Lookup Tables ******

include(ExternalProject)

set (GENTAB_SDIR ${CMAKE_CURRENT_SOURCE_DIR}/gentables)
set (GENTAB_BDIR ${CMAKE_CURRENT_BINARY_DIR}/gentables)

# Use external project to ensure that cmake uses the host compiler when building make_tables.exe
# To fix cross-compiling fluidsynth from Win32 to ARM (using vcpkg), we need to pass the current generator
# on to the external project, otherwise (for some unknown reason) the target compiler will be used rather
# than the host compiler.
ExternalProject_Add(gentables
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${GENTAB_SDIR}
    BINARY_DIR ${GENTAB_BDIR}
    CONFIGURE_COMMAND
        "${CMAKE_COMMAND}" -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE} -G "${CMAKE_GENERATOR}" -B "${GENTAB_BDIR}" "${GENTAB_SDIR}"
    BUILD_COMMAND
        "${CMAKE_COMMAND}" --build "${GENTAB_BDIR}"
    INSTALL_COMMAND ${GENTAB_BDIR}/make_tables.exe "${FluidSynth_BINARY_DIR}/"
)
add_dependencies(libfluidsynth-OBJ gentables)
