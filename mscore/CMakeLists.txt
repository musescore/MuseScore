#=============================================================================
#  MuseScore
#  Linux Music Score Editor
#
#  Copyright (C) 2002-2016 by Werner Schweer and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

include (${PROJECT_SOURCE_DIR}/build/gch.cmake)

SET_SOURCE_FILES_PROPERTIES(revision.h PROPERTIES GENERATED TRUE)

include_directories(
      ${CMAKE_CURRENT_BINARY_DIR}
      ${PROJECT_SOURCE_DIR}
      ${PROJECT_SOURCE_DIR}/thirdparty
      ${QTSINGLEAPPLICATION_INCLUDE_DIRS}
      )

if (SCRIPT_INTERFACE)
      set (SCRIPT_FILES mscorePlugins pluginCreator.cpp qmledit.cpp pluginManager.cpp)
      set (SCRIPT_UI   pluginCreator.ui pluginManager.ui)
endif (SCRIPT_INTERFACE)

QT5_WRAP_UI (ui_headers
      insertmeasuresdialog.ui editinstrument.ui editstyle.ui instrdialog.ui instrwidget.ui
      measuresdialog.ui pagesettings.ui mixer.ui playpanel.ui prefsdialog.ui measureproperties.ui
      textpalette.ui  timedialog.ui symboldialog.ui  shortcutcapturedialog.ui  editdrumset.ui
      editstaff.ui timesigproperties.ui
      instrwizard.ui timesigwizard.ui newwizard.ui aboutbox.ui aboutmusicxmlbox.ui
      transposedialog.ui excerptsdialog.ui  stafftext.ui tupletdialog.ui
      articulation.ui metaedit.ui palette.ui selectdialog.ui selectnotedialog.ui
      synthcontrol.ui splitstaff.ui cellproperties.ui keyedit.ui selectinstr.ui
      fretdprops.ui editstafftype.ui sectionbreak.ui  bend.ui tremolobar.ui
      editpitch.ui editstringdata.ui editraster.ui mediadialog.ui albummanager.ui layer.ui
      omrpanel.ui masterpalette.ui harmonyedit.ui pathlistdialog.ui
      note_groups.ui resourceManager.ui stafftypetemplates.ui
      startcenter.ui scorePreview.ui scoreBrowser.ui
      logindialog.ui uploadscoredialog.ui breaksdialog.ui
      toolbarEditor.ui

      importmidi/importmidi_panel.ui

      debugger/barline.ui
      debugger/harmony.ui
      debugger/box.ui
      debugger/keysig.ui
      debugger/rest.ui
      debugger/articulationbase.ui
      debugger/accidental.ui
      debugger/clef.ui
      debugger/slursegment.ui
      debugger/tiesegment.ui
      debugger/spanner.ui
      debugger/tremolo.ui
      debugger/beam.ui
      debugger/lyrics.ui
      debugger/linesegment.ui
      debugger/textline.ui
      debugger/line.ui
      debugger/slurtie.ui
      debugger/tie.ui
      debugger/tuplet.ui
      debugger/dynamic.ui
      debugger/hairpin.ui
      debugger/chord.ui
      debugger/chordrest.ui
      debugger/segment.ui
      debugger/measure.ui
      debugger/element.ui
      debugger/debugger.ui
      debugger/note.ui
      debugger/page.ui
      debugger/text.ui
      debugger/stem.ui
      debugger/system.ui
      debugger/timesig.ui

      inspector/inspector_element.ui
      inspector/inspector_vbox.ui
      inspector/inspector_tbox.ui
      inspector/inspector_articulation.ui
      inspector/inspector_spacer.ui
      inspector/inspector_hbox.ui
      inspector/inspector_segment.ui
      inspector/inspector_beam.ui
      inspector/inspector_note.ui
      inspector/inspector_rest.ui
      inspector/inspector_chord.ui
      inspector/inspector_group_element.ui
      inspector/inspector_image.ui
      inspector/inspector_lasso.ui
      inspector/inspector_volta.ui
      inspector/inspector_ottava.ui
      inspector/inspector_trill.ui
      inspector/inspector_hairpin.ui
      inspector/inspector_jump.ui
      inspector/inspector_marker.ui
      inspector/inspector_glissando.ui
      inspector/inspector_clef.ui
      inspector/inspector_timesig.ui
      inspector/inspector_keysig.ui
      inspector/inspector_barline.ui
      inspector/inspector_tuplet.ui
      inspector/inspector_accidental.ui
      inspector/inspector_tempotext.ui
      inspector/inspector_dynamic.ui
      inspector/inspector_lyric.ui
      inspector/inspector_line.ui
      inspector/inspector_textline.ui
      inspector/inspector_slur.ui
      inspector/inspector_text.ui
      inspector/inspector_stafftext.ui
      inspector/inspector_ambitus.ui
      inspector/inspector_empty.ui
      inspector/inspector_fret.ui
      inspector/inspector_break.ui
      inspector/inspector_stafftypechange.ui
      inspector/inspector_bend.ui
      inspector/inspector_arpeggio.ui
      inspector/inspector_tremolo.ui
      inspector/inspector_caesura.ui
      inspector/inspector_bracket.ui
      inspector/inspector_iname.ui
      inspector/inspector_fingering.ui
      inspector/inspector_frametext.ui
      inspector/inspector_fermata.ui
      inspector/align_select.ui
      inspector/offset_select.ui
      inspector/scale_select.ui
      inspector/size_select.ui
      ${SCRIPT_UI}
      )

if (APPLE)
      QT5_ADD_RESOURCES (qrc_files musescore.qrc musescorefonts-Mac.qrc shortcut-Mac.qrc)
else (APPLE)
      QT5_ADD_RESOURCES (qrc_files musescore.qrc
            musescorefonts-MScore.qrc
            musescorefonts-Gootville.qrc
            musescorefonts-Bravura.qrc
            musescorefonts-MuseJazz.qrc
            musescorefonts-FreeSerif.qrc
            musescorefonts-Free.qrc
            shortcut.qrc)
endif (APPLE)


set (AUDIO "")
if (MINGW)
      set (resource_file ${PROJECT_BINARY_DIR}/resfile.o)
else (MINGW)
      if (USE_ALSA)
            set (AUDIO ${AUDIO} alsa.cpp)
      endif (USE_ALSA)
endif (MINGW)

if (USE_PORTAUDIO)
      set (AUDIO ${AUDIO} pa.cpp)
endif (USE_PORTAUDIO)

if (USE_PULSEAUDIO)
      set (AUDIO ${AUDIO} pulseaudio.cpp)
endif (USE_PULSEAUDIO)

if (USE_PORTMIDI)
      set (AUDIO ${AUDIO} pm.cpp)
endif (USE_PORTMIDI)

if (USE_LAME)
      set (AUDIO ${AUDIO} exportmp3.cpp)
endif (USE_LAME)

if (USE_JACK)
      set (AUDIO ${AUDIO} jackaudio.cpp jackweakapi.cpp)
endif (USE_JACK)

if (USE_ALSA OR USE_PORTMIDI)
      set (AUDIO ${AUDIO} mididriver.cpp)
endif (USE_ALSA OR USE_PORTMIDI)

if (${CMAKE_SYSTEM} MATCHES "GNU-0.3")
      # Include some cmake support file
      INCLUDE (Platform/GNU)
      set ( USE_ALSA 0 )
      set ( USE_PORTMIDI 0 )
endif (${CMAKE_SYSTEM} MATCHES "GNU-0.3")

if (${CMAKE_SYSTEM} MATCHES "kFreeBSD")
      # Include some cmake support file
      INCLUDE (Platform/kFreeBSD)
      set ( USE_ALSA 0 )
      set ( USE_PORTMIDI 0 )
endif (${CMAKE_SYSTEM} MATCHES "kFreeBSD")

if (APPLE)
      set (ExecutableName mscore MACOSX_BUNDLE)
      set (MACOSX_BUNDLE_ICON_FILE      mscore.icns)
      set (MACOSX_BUNDLE_GUI_IDENTIFIER org.musescore.MuseScore)
      set (MACOSX_BUNDLE_LONG_VERSION_STRING ${MUSESCORE_VERSION_FULL})
      set (MACOSX_BUNDLE_BUNDLE_NAME MuseScore)
      set (MACOSX_BUNDLE_SHORT_VERSION_STRING ${MUSESCORE_VERSION})
      set (MACOSX_BUNDLE_BUNDLE_VERSION ${MUSESCORE_VERSION_FULL})
      set (MACOSX_BUNDLE_COPYRIGHT musescore.org)
else (APPLE)
      set (ExecutableName mscore)
endif (APPLE)

if (OMR)
      set(OMR_FILES omrpanel.cpp)
endif (OMR)

if (APPLE)
      file(GLOB_RECURSE INCS "*.h")
      set(COCOABRIDGE "macos/cocoabridge.mm")
else (APPLE)
      set(INCS "")
      set(COCOABRIDGE "")
endif (APPLE)

add_executable ( ${ExecutableName}
      ${qrc_files}
      ${ui_headers}
      ${PROJECT_BINARY_DIR}/all.h
      ${PCH}
      ${resource_file}
      ${INCS}

      recordbutton.h greendotbutton prefsdialog.h prefsdialog.cpp
      scoreview.cpp editharmony.cpp editfiguredbass.cpp events.cpp
      editinstrument.cpp editstyle.cpp
      icons.cpp importbww.cpp
      importmxmllogger.cpp importmxmlnoteduration.cpp importmxmlnotepitch.cpp
      importmxml.cpp importmxmlpass1.cpp importmxmlpass2.cpp
      instrdialog.cpp instrwidget.cpp
      debugger/debugger.cpp menus.cpp
      musescore.cpp navigator.cpp pagesettings.cpp palette.cpp
      timeline.cpp
      mixer.cpp playpanel.cpp selectionwindow.cpp preferences.cpp measureproperties.cpp
      seq.cpp textpalette.cpp
      timedialog.cpp symboldialog.cpp shortcutcapturedialog.cpp
      simplebutton.cpp musedata.cpp
      editdrumset.cpp editstaff.cpp
      timesigproperties.cpp newwizard.cpp transposedialog.cpp
      excerptsdialog.cpp metaedit.cpp magbox.cpp
      capella.cpp capxml.cpp exportaudio.cpp palettebox.cpp
      synthcontrol.cpp drumroll.cpp pianoroll.cpp piano.cpp
      pianoview.cpp drumview.cpp scoretab.cpp keyedit.cpp harmonyedit.cpp
      updatechecker.cpp
      importove.cpp
      ove.cpp
      ruler.cpp
      importgtp.cpp importgtp-gp4.cpp importgtp-gp5.cpp importgtp-gp6.cpp
      importptb.cpp
      fotomode.cpp drumtools.cpp
      selinstrument.cpp editstafftype.cpp texttools.cpp
      editpitch.cpp editstringdata.cpp editraster.cpp pianotools.cpp mediadialog.cpp
      workspace.cpp chordview.cpp
      albummanager.cpp
      stafftextproperties.cpp splitstaff.cpp
      tupletdialog.cpp
      articulationprop.cpp
      fretproperties.cpp sectionbreakprop.cpp
      bendproperties.cpp tremolobarprop.cpp file.cpp keyb.cpp osc.cpp
      layer.cpp selectdialog.cpp selectnotedialog.cpp propertymenu.cpp shortcut.cpp bb.cpp
      dragelement.cpp startupWizard.cpp
      svggenerator.cpp
      masterpalette.cpp
      dragdrop.cpp
      waveview.cpp helpBrowser.cpp
      inspector/inspector.cpp
      inspector/inspectorBase.cpp
      inspector/inspectorBeam.cpp
      inspector/inspectorGroupElement.cpp
      inspector/inspectorImage.cpp
      inspector/inspectorFret.cpp
      inspector/inspectorText.cpp
      inspector/inspectorLasso.cpp
      inspector/inspectorBarline.cpp
      inspector/inspectorVolta.cpp
      inspector/inspectorOttava.cpp
      inspector/inspectorTrill.cpp
      inspector/inspectorHairpin.cpp
      inspector/inspectorJump.cpp
      inspector/inspectorMarker.cpp
      inspector/inspectorGlissando.cpp
      inspector/inspectorNote.cpp
      inspector/inspectorAmbitus.cpp
      inspector/inspectorArpeggio.cpp
      inspector/inspectorFingering.cpp
      inspector/inspectorTextLine.cpp
      inspector/alignSelect.cpp
      inspector/offsetSelect.cpp
      inspector/scaleSelect.cpp
      inspector/sizeSelect.cpp
      inspector/inspectorTextBase.cpp
      inspector/inspectorDynamic.cpp
      inspector/inspectorElementBase.cpp
      enableplayforwidget.cpp
      editelement.cpp
      qmlplugin.cpp
      editlyrics.cpp
      musicxmlfonthandler.cpp
      musicxmlsupport.cpp
      exportxml.cpp
      importxml.cpp
      importxmlfirstpass.cpp
      savePositions.cpp
      paletteBoxButton.cpp
      driver.cpp
      exportmidi.cpp
      noteGroups.cpp
      pathlistdialog.cpp
      exampleview.cpp
      miconengine.cpp
      importmidi/importmidi.cpp
      importmidi/importmidi_panel.cpp importmidi/importmidi_operations.cpp
      importmidi/importmidi_model.cpp importmidi/importmidi_delegate.cpp
      importmidi/importmidi_meter.cpp importmidi/importmidi_quant.cpp
      importmidi/importmidi_tuplet.cpp importmidi/importmidi_chord.cpp
      importmidi/importmidi_swing.cpp importmidi/importmidi_fraction.cpp
      importmidi/importmidi_drum.cpp importmidi/importmidi_clef.cpp
      importmidi/importmidi_lrhand.cpp importmidi/importmidi_lyrics.cpp
      importmidi/importmidi_inner.cpp importmidi/importmidi_tie.cpp
      importmidi/importmidi_tuplet_voice.cpp importmidi/importmidi_beat.cpp
      importmidi/importmidi_tuplet_detect.cpp importmidi/importmidi_tuplet_filter.cpp
      importmidi/importmidi_tuplet_tonotes.cpp importmidi/importmidi_simplify.cpp
      importmidi/importmidi_voice.cpp importmidi/importmidi_view.cpp importmidi/importmidi_key.cpp
      importmidi/importmidi_tempo.cpp importmidi/importmidi_instrument.cpp
      importmidi/importmidi_chordname.cpp
      resourceManager.cpp downloadUtils.cpp
      textcursor.cpp continuouspanel.cpp accessibletoolbutton.cpp scoreaccessibility.cpp
      startcenter.cpp scoreBrowser.cpp scorePreview.cpp scoreInfo.cpp
      logindialog.cpp loginmanager.cpp uploadscoredialog.cpp breaksdialog.cpp searchComboBox.cpp
      help.cpp help.h
      toolbarEditor.cpp toolbarEditor.h
      abstractdialog.cpp abstractdialog.h
      toolbuttonmenu.cpp

      ${COCOABRIDGE}
      ${OMR_FILES}
      ${AUDIO}
      ${SCRIPT_FILES}
      driver.h
      tremolobarcanvas.h bendcanvas.h fretcanvas.h keycanvas.h harmonycanvas.h
      )

target_link_libraries(mscore
      awl
      diff_match_patch
      bww
      rtf2html
      ${QTSINGLEAPPLICATION_LIBRARIES}
      synthesizer
      midi
      fluid
      effects
      vorbisfile
      libmscore
      qzip
      beatroot
      kqoauth
      )

if (USE_SYSTEM_FREETYPE)
      target_link_libraries(mscore ${FREETYPE_LIBRARIES})
endif (USE_SYSTEM_FREETYPE)

if (MINGW)
      set(MSCORE_OUTPUT_NAME ${MUSESCORE_NAME})
elseif (MSCORE_INSTALL_SUFFIX)
      set(MSCORE_OUTPUT_NAME "${ExecutableName}${MSCORE_INSTALL_SUFFIX}")
endif (MINGW)

# If MSCORE_OUTPUT_NAME is set (e.g, when cmake is called by the user), the output executable will be
# called MSCORE_OUTPUT_NAME instead of 'mscore'. This can be used to have MuseScore stable and unstable
# both installed in the same prefix on a Linux system.
if (MSCORE_OUTPUT_NAME)
  set_target_properties(
      ${ExecutableName}
      PROPERTIES
      OUTPUT_NAME ${MSCORE_OUTPUT_NAME}
      )
endif (MSCORE_OUTPUT_NAME)

if (ZERBERUS)
      target_link_libraries(mscore zerberus synthesizer)
endif (ZERBERUS)
if (AEOLUS)
      target_link_libraries(mscore aeolus)
endif (AEOLUS)
if (SOUNDFONT3)
      target_link_libraries(mscore ${VORBIS_LIB} ${OGG_LIB})
endif (SOUNDFONT3)

if (HAS_AUDIOFILE)
      target_link_libraries(mscore audiofile ${SNDFILE_LIB})
endif (HAS_AUDIOFILE)

if (APPLE)
      set_target_properties (mscore
          PROPERTIES
          MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/build/MacOSXBundleInfo.plist.in)
      # Enable dSym generation
      #set_target_properties (mscore
      #    PROPERTIES
      #       XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")
endif (APPLE)

if (OSC)
      target_link_libraries(mscore ofqf)
endif (OSC)

if (MINGW)
   set (QT_WRC_EXECUTABLE        ${PROJECT_SOURCE_DIR}/build/wrc.bat)
   set (QT_WINE_EXECUTABLE       ${PROJECT_SOURCE_DIR}/build/wine.bat)
   add_custom_command(
      OUTPUT ${PROJECT_BINARY_DIR}/resfile.o
      COMMAND ${QT_WRC_EXECUTABLE} -i mscore.rc -o ${PROJECT_BINARY_DIR}/mscore.res
      COMMAND ${QT_WINE_EXECUTABLE} /home/ws/.wine/drive_c/MingW/bin/windres.exe ${PROJECT_BINARY_DIR}/mscore.res -o ${PROJECT_BINARY_DIR}/resfile.o
      DEPENDS ${PROJECT_SOURCE_DIR}/mscore/data/mscore.rc
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/mscore/data
      )
   set_source_files_properties(
      ${PROJECT_BINARY_DIR}/resfile.o
      PROPERTIES generated true
      )
   string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
   # Windows: Add -mconsole to LINK_FLAGS to get a console window for debug output
   if(CMAKE_BUILD_TYPE MATCHES "DEBUG")
     set_target_properties( mscore
        PROPERTIES
           COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch ${QT_DEFINITIONS} -DQT_SVG_LIB -DQT_GUI_LIB -DQT_XML_LIB -DQT_CORE_LIB"
           LINK_FLAGS "-mwindows -mconsole -L ${QT_INSTALL_PREFIX}/lib"
        )
   else(CMAKE_BUILD_TYPE MATCHES "DEBUG")
     set_target_properties( mscore
          PROPERTIES
             COMPILE_FLAGS "${PCH_INCLUDE} -Wall -Wextra -Winvalid-pch ${QT_DEFINITIONS} -DQT_SVG_LIB -DQT_GUI_LIB -DQT_XML_LIB -DQT_CORE_LIB"
             LINK_FLAGS "-Wl,-S -mwindows -L ${QT_INSTALL_PREFIX}/lib"
          )
   endif(CMAKE_BUILD_TYPE MATCHES "DEBUG")

   target_link_libraries(mscore
      portaudio
      portmidi
      winmm
      mscore_freetype
      z
      )

   if (OMR)
      target_link_libraries(mscore omr poppler)
      if (OCR)
            target_link_libraries(mscore tesseract_api)
      endif (OCR)
   endif (OMR)

   # Keep dependencies in alphabetical order. Changes made to this list
   # might need to be made in "build/Linux+BSD/portable/copy-libs" too.
   get_filename_component(COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
   get_filename_component (MINGW_ROOT ${COMPILER_DIR} DIRECTORY)
   foreach (QtLibrary ${QT_LIBRARIES})
      message("Library ${QtLibrary}")
      # always use release libs
      set_target_properties(${QtLibrary} PROPERTIES MAP_IMPORTED_CONFIG_DEBUG "RELEASE")
      get_target_property(QtSharedLibrary ${QtLibrary} LOCATION_RELEASE)
      if (EXISTS ${QtSharedLibrary})
          list (APPEND QtInstallLibraries ${QtSharedLibrary})
      endif (EXISTS ${QtSharedLibrary})
   endforeach (QtLibrary ${QT_LIBRARIES})
   list(REMOVE_DUPLICATES QtInstallLibraries)

   target_link_libraries(mscore ${QT_LIBRARIES})

   install( TARGETS mscore RUNTIME DESTINATION bin )

   install( FILES
      ${MINGW_ROOT}/bin/libgcc_s_dw2-1.dll
      ${MINGW_ROOT}/bin/libstdc++-6.dll
      ${MINGW_ROOT}/bin/libwinpthread-1.dll
      ${MINGW_ROOT}/lib/libogg.dll
      ${MINGW_ROOT}/lib/libsndfile-1.dll
      ${MINGW_ROOT}/lib/libvorbis.dll
      ${MINGW_ROOT}/lib/libvorbisfile.dll
      ${MINGW_ROOT}/lib/portaudio.dll
      ${MINGW_ROOT}/opt/bin/libeay32.dll
      ${MINGW_ROOT}/opt/bin/ssleay32.dll
      ${QtInstallLibraries}
      ${PROJECT_SOURCE_DIR}/build/qt.conf
      DESTINATION bin)

    install(FILES
      ${QT_INSTALL_PREFIX}/plugins/iconengines/qsvgicon.dll
      DESTINATION bin/iconengines)

    install(FILES
      ${QT_INSTALL_PREFIX}/plugins/imageformats/qjpeg.dll
      ${QT_INSTALL_PREFIX}/plugins/imageformats/qsvg.dll
      ${QT_INSTALL_PREFIX}/plugins/imageformats/qtiff.dll
      DESTINATION bin/imageformats)

    install(FILES
      ${QT_INSTALL_PREFIX}/plugins/platforms/qwindows.dll
      DESTINATION bin/platforms)

    install(FILES
      ${QT_INSTALL_PREFIX}/plugins/printsupport/windowsprintersupport.dll
      DESTINATION bin/printsupport)

    install(FILES
      ${QT_INSTALL_PREFIX}/plugins/sqldrivers/qsqlite.dll
      DESTINATION bin/sqldrivers)

    install(DIRECTORY
      ${QT_INSTALL_PREFIX}/qml
      DESTINATION .
      REGEX ".*d\\.dll" EXCLUDE
      REGEX ".*QtGraphicalEffects.*" EXCLUDE
      REGEX ".*QtMultimedia.*" EXCLUDE
      REGEX ".*QtSensors.*" EXCLUDE
      REGEX ".*QtTest.*" EXCLUDE
      REGEX ".*QtWebkit.*" EXCLUDE)

else (MINGW)
   target_link_libraries(mscore
      ${ALSA_LIB}
      ${QT_LIBRARIES}
      z
      ${CMAKE_DL_LIBS}
      pthread
      )

    if (USE_SYSTEM_FREETYPE)
         target_link_libraries(mscore freetype)
    else (USE_SYSTEM_FREETYPE)
         target_link_libraries(mscore mscore_freetype)
    endif (USE_SYSTEM_FREETYPE)

    if (USE_PORTAUDIO)
      target_link_libraries(mscore ${PORTAUDIO_LIB})
    endif (USE_PORTAUDIO)

    if (USE_PORTMIDI)
        if (APPLE)
            set(PORTMIDI_LIB portmidi)
        else (APPLE)
            set(PORTMIDI_LIB -lportmidi -lporttime) # Remove -lporttime on RPM-based systems where PortTime is part of PortMidi.
        endif (APPLE)
        target_link_libraries(mscore ${PORTMIDI_LIB})
    endif (USE_PORTMIDI)

    if (USE_PULSEAUDIO)
      target_link_libraries(mscore ${PULSEAUDIO_LIBRARY})
    endif (USE_PULSEAUDIO)

   set_target_properties (
      mscore
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wno-overloaded-virtual -Winvalid-pch"
      )

   if (OMR)
      target_link_libraries(mscore omr poppler)
      if (OCR)
            target_link_libraries(mscore tesseract_api)
      endif (OCR)
   endif (OMR)

   if (APPLE)
     target_link_libraries(mscore ${OsxFrameworks})
   else (APPLE)
       target_link_libraries(mscore rt)
   endif (APPLE)

   # 'gold' does not use indirect shared libraries for symbol resolution, Linux only
   if (NOT APPLE)
      if(USE_JACK)
         target_link_libraries(mscore ${CMAKE_DL_LIBS})
      endif(USE_JACK)
      target_link_libraries(mscore rt)
   endif (NOT APPLE)

   if (APPLE)
      set_target_properties(mscore
        PROPERTIES
           LINK_FLAGS "-stdlib=libc++"
        )
     xcode_pch(mscore all)
     install (TARGETS mscore BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX})
     install (FILES data/mscore.icns DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME})
     install (FILES data/musescoreDocument.icns DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME})
   else (APPLE)
     #### PACKAGING for Linux and BSD based systems (more in top-level CMakeLists.txt) ####
     # Install mscore executable (package maintainers may add "MuseScore" and/or "musescore" aliases that symlink to mscore)
     install( TARGETS mscore RUNTIME DESTINATION bin )
     if (LN_EXECUTABLE)
        add_custom_target(mscore_alias ALL
            COMMAND echo "Creating symlink alias for mscore executable."
            COMMAND ${LN_EXECUTABLE} -sf "mscore${MSCORE_INSTALL_SUFFIX}" "musescore${MSCORE_INSTALL_SUFFIX}"
            COMMAND echo 'Symlink alias: musescore${MSCORE_INSTALL_SUFFIX} -> mscore${MSCORE_INSTALL_SUFFIX}'
            )
        install( FILES ${PROJECT_BINARY_DIR}/mscore/musescore${MSCORE_INSTALL_SUFFIX} DESTINATION bin)
     else (LN_EXECUTABLE)
        add_custom_target(mscore_alias ALL
            COMMAND echo "No symlink aliases will be created."
            VERBATIM
            )
     endif (LN_EXECUTABLE)
     # Install MuseScore icons (use SVGs where possible, but install PNGs as backup for systems that don't support SVG)
     if (MSCORE_UNSTABLE)
         set(MSCORE_ICON_BASE ../assets/musescore-icon-square) # Square icons on development builds
     else (MSCORE_UNSTABLE)
         set(MSCORE_ICON_BASE ../assets/musescore-icon-round) # Round icons on stable releases
     endif (MSCORE_UNSTABLE)
     install(FILES ${MSCORE_ICON_BASE}.svg RENAME mscore${MSCORE_INSTALL_SUFFIX}.svg DESTINATION share/icons/hicolor/scalable/apps)
     install(FILES ${MSCORE_ICON_BASE}-16.png RENAME mscore${MSCORE_INSTALL_SUFFIX}.png DESTINATION share/icons/hicolor/16x16/apps)
     install(FILES ${MSCORE_ICON_BASE}-24.png RENAME mscore${MSCORE_INSTALL_SUFFIX}.png DESTINATION share/icons/hicolor/24x24/apps)
     install(FILES ${MSCORE_ICON_BASE}-32.png RENAME mscore${MSCORE_INSTALL_SUFFIX}.png DESTINATION share/icons/hicolor/32x32/apps)
     install(FILES ${MSCORE_ICON_BASE}-48.png RENAME mscore${MSCORE_INSTALL_SUFFIX}.png DESTINATION share/icons/hicolor/48x48/apps)
     install(FILES ${MSCORE_ICON_BASE}-64.png RENAME mscore${MSCORE_INSTALL_SUFFIX}.png DESTINATION share/icons/hicolor/64x64/apps)
     install(FILES ${MSCORE_ICON_BASE}-96.png RENAME mscore${MSCORE_INSTALL_SUFFIX}.png DESTINATION share/icons/hicolor/96x96/apps)
     install(FILES ${MSCORE_ICON_BASE}-128.png RENAME mscore${MSCORE_INSTALL_SUFFIX}.png DESTINATION share/icons/hicolor/128x128/apps)
     install(FILES ${MSCORE_ICON_BASE}-512.png RENAME mscore${MSCORE_INSTALL_SUFFIX}.png DESTINATION share/icons/hicolor/512x512/apps)
     # Install MIME (filetype) icons for each mimetype on Linux
     install( FILES   ../assets/mscz-icon.svg RENAME application-x-musescore${MSCORE_INSTALL_SUFFIX}.svg
        DESTINATION share/icons/hicolor/scalable/mimetypes) # SVG icon for .MSCZ files
     install( FILES   ../assets/mscz-icon-48.png RENAME application-x-musescore${MSCORE_INSTALL_SUFFIX}.png
        DESTINATION share/icons/hicolor/48x48/mimetypes) # PNG icon for .MSCZ files
     install( FILES   ../assets/mscx-icon.svg RENAME application-x-musescore${MSCORE_INSTALL_SUFFIX}+xml.svg
        DESTINATION share/icons/hicolor/scalable/mimetypes) # SVG icon for .MSCX files
     install( FILES   ../assets/mscx-icon-48.png RENAME application-x-musescore${MSCORE_INSTALL_SUFFIX}+xml.png
        DESTINATION share/icons/hicolor/48x48/mimetypes) # PNG icon for .MSCX files
     # use a custom icon for MusicXML files (there isn't a standard icon for MusicXML files)
     install( FILES   ../assets/mxl-icon.svg RENAME application-vnd.recordare.musicxml${MSCORE_INSTALL_SUFFIX}.svg
        DESTINATION share/icons/hicolor/scalable/mimetypes) # SVG icon for .MXL (compressed MusicXML) files
     install( FILES   ../assets/mxl-icon-48.png RENAME application-vnd.recordare.musicxml${MSCORE_INSTALL_SUFFIX}.png
        DESTINATION share/icons/hicolor/48x48/mimetypes) # PNG icon for .MXL (compressed MusicXML) files
     install( FILES   ../assets/xml-icon.svg RENAME application-vnd.recordare.musicxml${MSCORE_INSTALL_SUFFIX}+xml.svg
        DESTINATION share/icons/hicolor/scalable/mimetypes) # SVG icon for .XML (MusicXML) files
     install( FILES   ../assets/xml-icon-48.png RENAME application-vnd.recordare.musicxml${MSCORE_INSTALL_SUFFIX}+xml.png
        DESTINATION share/icons/hicolor/48x48/mimetypes) # PNG icon for .XML (MusicXML) files
     # Note: Must now run "gtk-update-icon-cache" to set the new icons. This is done in the Makefile.
   endif (APPLE)
endif (MINGW)

if (APPLE)
     install (FILES
      ../fonts/gootville/GootvilleText.otf
      ../fonts/mscore/MScoreText.ttf
      ../fonts/musejazz/MuseJazzText.otf
      ../fonts/FreeSerif.ttf
      ../fonts/FreeSerifBold.ttf
      ../fonts/FreeSerifItalic.ttf
      ../fonts/FreeSerifBoldItalic.ttf
      ../fonts/FreeSans.ttf
      ../fonts/mscoreTab.ttf
      ../fonts/mscore-BC.ttf
      ../fonts/bravura/BravuraText.otf
     DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}fonts
     )
     install(DIRECTORY
      ${QT_INSTALL_PREFIX}/qml
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}
      REGEX ".*QtWebkit.*" EXCLUDE
      REGEX ".*QtTest.*" EXCLUDE
      REGEX ".*QtSensors.*" EXCLUDE
      REGEX ".*QtGraphicalEffects.*" EXCLUDE
      REGEX ".*QtMultimedia.*" EXCLUDE
      REGEX ".*QtAudioEngine.*" EXCLUDE
      REGEX ".*_debug\\.dylib" EXCLUDE)
endif (APPLE)

ADD_DEPENDENCIES(${ExecutableName} mops1)
ADD_DEPENDENCIES(${ExecutableName} mops2)

