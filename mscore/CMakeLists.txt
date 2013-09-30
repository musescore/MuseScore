#=============================================================================
#  MuseScore
#  Linux Music Score Editor
#
#  Copyright (C) 2002-2013 by Werner Schweer and others
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
      set (SCRIPT_FILES plugins.cpp pluginCreator.cpp qmledit.cpp)
      set (SCRIPT_MOCS plugins.h pluginCreator.h qmledit.h)
      set (SCRIPT_UI   pluginCreator.ui)
endif (SCRIPT_INTERFACE)

QT4_WRAP_UI (ui_headers
      insertmeasuresdialog.ui editinstrument.ui editstyle.ui instrdialog.ui
      measuresdialog.ui pagesettings.ui mixer.ui playpanel.ui prefsdialog.ui measureproperties.ui
      textpalette.ui  timedialog.ui symboldialog.ui  shortcutcapturedialog.ui  editdrumset.ui
      editstaff.ui voltaproperties.ui timesigproperties.ui boxproperties.ui
      instrwizard.ui timesigwizard.ui newwizard.ui aboutbox.ui aboutmusicxmlbox.ui
      chordedit.ui transposedialog.ui lineproperties.ui excerptsdialog.ui  stafftext.ui tupletdialog.ui
      glissandoprop.ui articulation.ui metaedit.ui palette.ui select.ui
      synthcontrol.ui splitstaff.ui cellproperties.ui keyedit.ui selectinstr.ui
      fretdprops.ui stafftype.ui sectionbreak.ui  bend.ui tremolobar.ui textproperties.ui
      editpitch.ui editstringdata.ui editraster.ui mediadialog.ui textstyle.ui album.ui layer.ui
      omrpanel.ui startdialog.ui masterpalette.ui pluginManager.ui harmonyedit.ui pathlistdialog.ui
      note_groups.ui importmidi_panel.ui resourceManager.ui

      debugger/barline.ui
      debugger/harmony.ui
      debugger/box.ui
      debugger/keysig.ui
      debugger/rest.ui
      debugger/articulationbase.ui
      debugger/accidental.ui
      debugger/clef.ui
      debugger/slursegment.ui
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

      inspector/inspector_element.ui
      inspector/inspector_vbox.ui
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
      inspector/inspector_line.ui
      inspector/inspector_textline.ui
      inspector/inspector_slur.ui
      inspector/inspector_text.ui
      ${SCRIPT_UI}
      )

if (OMR)
      set(OMR_MOCS omrpanel.h)
endif (OMR)


QT4_WRAP_CPP (mocs
      scoreview.h editinstrument.h editstyle.h instrdialog.h
      musescore.h navigator.h pagesettings.h palette.h mixer.h playpanel.h
      measureproperties.h seq.h textpalette.h textstyle.h
      timedialog.h symboldialog.h shortcutcapturedialog.h simplebutton.h
      greendotbutton.h recordbutton.h editdrumset.h editstaff.h selinstrument.h
      voltaproperties.h timesigproperties.h boxproperties.h newwizard.h
      transposedialog.h chordedit.h lineproperties.h
      excerptsdialog.h stafftextproperties.h glissandoproperties.h
      articulationprop.h metaedit.h magbox.h
      textprop.h textproperties.h tupletdialog.h
      selectdialog.h synthcontrol.h drumroll.h pianoroll.h
      piano.h ruler.h file.h
      pianoview.h drumview.h splitstaff.h scoretab.h keyedit.h keycanvas.h harmonyedit.h
      harmonycanvas.h updatechecker.h fretproperties.h fretcanvas.h editstafftype.h
      prefsdialog.h sectionbreakprop.h bendproperties.h
      bendcanvas.h tremolobarprop.h tremolobarcanvas.h
      drumtools.h texteditor.h texttools.h pianotools.h editpitch.h
      editstringdata.h editraster.h mediadialog.h chordview.h album.h layer.h
      webpage.h
      inspector/inspector.h inspector/inspectorBase.h inspector/inspectorBeam.h masterpalette.h
      inspector/inspectorGroupElement.h inspector/inspectorImage.h waveview.h helpBrowser.h
      inspector/inspectorLasso.h inspector/inspectorVolta.h inspector/inspectorOttava.h
      inspector/inspectorTrill.h
      inspector/inspectorHairpin.h qmlplugin.h palettebox.h workspace.h pluginManager.h
      inspector/inspectorJump.h inspector/inspectorMarker.h inspector/inspectorGlissando.h
      inspector/inspectorNote.h
      paletteBoxButton.h pathlistdialog.h exampleview.h noteGroups.h inspector/inspectorTextLine.h
      importmidi_panel.h importmidi_operations.h
      importmidi_opmodel.h importmidi_trmodel.h importmidi_opdelegate.h
      importmidi_operation.h importmidi_quant.h importmidi_chord.h
      importmidi_inner.h importmidi_data.h importmidi_swing.h
      debugger/debugger.h importmidi_fraction.h importmidi_drum.h
      importmidi_lrhand.h importmidi_lyrics.h importmidi_trview.h
      resourceManager.h downloadUtils.h
      ${OMR_MOCS}
      ${SCRIPT_MOCS}
      )

if (APPLE)
      QT4_ADD_RESOURCES (qrc_files musescore-apple.qrc)
else (APPLE)
      QT5_ADD_RESOURCES (qrc_files musescore.qrc)
endif (APPLE)


set (AUDIO "")
if (MINGW)
      set (resource_file ${PROJECT_BINARY_DIR}/resfile.o)
else (MINGW)
      if (USE_ALSA)
            set (AUDIO ${AUDIO} alsa.cpp mididriver.cpp)
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

if (USE_JACK)
      set (AUDIO ${AUDIO} jackaudio.cpp jackweakapi.cpp)
endif (USE_JACK)

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
else (APPLE)
        set(INCS "")
endif (APPLE)

add_executable ( ${ExecutableName}
#      mops.cpp                            # HACK! first file disappears during linking as CMake generates
#                                          # something like:    gcc ... -L mops.o ...
      ${qrc_files}
      ${ui_headers}
      ${mocs}
      ${PROJECT_BINARY_DIR}/all.h
      ${PCH}
      ${resource_file}
      ${INCS}

      actions.cpp scoreview.cpp editinstrument.cpp editstyle.cpp
      icons.cpp importbww.cpp instrdialog.cpp
      debugger/debugger.cpp menus.cpp importmidi.cpp
      musescore.cpp navigator.cpp pagesettings.cpp palette.cpp
      mixer.cpp playpanel.cpp preferences.cpp measureproperties.cpp
      seq.cpp boxproperties.cpp textpalette.cpp
      timedialog.cpp symboldialog.cpp shortcutcapturedialog.cpp
      simplebutton.cpp musedata.cpp
      # exportly.cpp
      editdrumset.cpp editstaff.cpp voltaproperties.cpp
      timesigproperties.cpp newwizard.cpp transposedialog.cpp
      chordedit.cpp excerptsdialog.cpp metaedit.cpp magbox.cpp
      capella.cpp capxml.cpp exportaudio.cpp palettebox.cpp
      textproperties.cpp synthcontrol.cpp drumroll.cpp pianoroll.cpp piano.cpp
      pianoview.cpp drumview.cpp scoretab.cpp keyedit.cpp harmonyedit.cpp
      updatechecker.cpp importove.cpp ove.cpp ruler.cpp
      importgtp.cpp fotomode.cpp drumtools.cpp
      selinstrument.cpp texteditor.cpp editstafftype.cpp texttools.cpp
      editpitch.cpp editstringdata.cpp editraster.cpp pianotools.cpp mediadialog.cpp
      workspace.cpp exportmp3.cpp chordview.cpp
      album.cpp webpage.cpp textstyle.cpp
      lineproperties.cpp stafftextproperties.cpp splitstaff.cpp
      tupletdialog.cpp glissandoproperties.cpp
      articulationprop.cpp textprop.cpp
      fretproperties.cpp sectionbreakprop.cpp
      bendproperties.cpp tremolobarprop.cpp file.cpp keyb.cpp osc.cpp
      layer.cpp selectdialog.cpp propertymenu.cpp shortcut.cpp bb.cpp
      inspector/inspector.cpp dragelement.cpp svggenerator.cpp
      inspector/inspectorBase.cpp inspector/inspectorBeam.cpp masterpalette.cpp
      inspector/inspectorGroupElement.cpp dragdrop.cpp inspector/inspectorImage.cpp
      waveview.cpp helpBrowser.cpp inspector/inspectorLasso.cpp
      editelement.cpp inspector/inspectorVolta.cpp inspector/inspectorOttava.cpp
      inspector/inspectorTrill.cpp
      inspector/inspectorHairpin.cpp qmlplugin.cpp editlyrics.cpp
      musicxmlsupport.cpp exportxml.cpp importxml.cpp importxmlfirstpass.cpp
      savePositions.cpp pluginManager.cpp inspector/inspectorJump.cpp inspector/inspectorMarker.cpp
      inspector/inspectorGlissando.cpp inspector/inspectorNote.cpp paletteBoxButton.cpp
      driver.cpp exportmidi.cpp noteGroups.cpp
      pathlistdialog.cpp exampleview.cpp inspector/inspectorTextLine.cpp
      importmidi_panel.cpp importmidi_operations.cpp miconengine.cpp
      importmidi_opmodel.cpp importmidi_trmodel.cpp importmidi_opdelegate.cpp
      importmidi_meter.cpp importmidi_quant.cpp importmidi_tuplet.cpp importmidi_chord.cpp
      importmidi_data.cpp importmidi_swing.cpp importmidi_fraction.cpp importmidi_drum.cpp
      importmidi_clef.cpp importmidi_lrhand.cpp importmidi_lyrics.cpp importmidi_trview.cpp
      importmidi_inner.cpp
      resourceManager.cpp downloadUtils.cpp

      ${OMR_FILES}
      ${AUDIO}
      ${SCRIPT_FILES}
      musescoreCore.h
      driver.h
      )

#qt5_use_modules(${ExecutableName} Gui Widgets Network)

target_link_libraries(mscore
      libmscore
      awl
      mstyle
      diff_match_patch
      bww
      rtf2html
      ${QTSINGLEAPPLICATION_LIBRARIES}
      synthesizer
      midi
      fluid
      effects
      vorbisfile
      )
if (ZERBERUS)
      target_link_libraries(mscore zerberus synthesizer)
endif ()
if (AEOLUS)
      target_link_libraries(mscore aeolus)
endif ()
if (SOUNDFONT3)
      target_link_libraries(mscore ${VORBIS_LIB} ${OGG_LIB})
endif ()

if (HAS_AUDIOFILE)
      target_link_libraries(mscore audiofile ${SNDFILE_LIB})
endif (HAS_AUDIOFILE)

if (APPLE)
      set_target_properties (mscore
         PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/build/MacOSXBundleInfo.plist.in)
      #enable dSym generation
      #set_target_properties (mscore
      #   PROPERTIES
      #      XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")
endif(APPLE)

if (OSC)
    target_link_libraries(mscore ofqf)
endif (OSC)

if (MINGW)
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
   # Windows: add -mconsole to LINK_FLAGS to get a console window for debug output
   if(CMAKE_BUILD_TYPE MATCHES "DEBUG")
     set_target_properties( mscore
        PROPERTIES
           COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch ${QT_DEFINITIONS} -DQT_SVG_LIB -DQT_GUI_LIB -DQT_XML_LIB -DQT_CORE_LIB"
           LINK_FLAGS "${PROJECT_BINARY_DIR}/resfile.o -mwindows -mconsole -L ${CROSSQT}/lib"
        )
   else(CMAKE_BUILD_TYPE MATCHES "DEBUG")
     set_target_properties( mscore
          PROPERTIES
             COMPILE_FLAGS "${PCH_INCLUDE} -Wall -Wextra -Winvalid-pch ${QT_DEFINITIONS} -DQT_SVG_LIB -DQT_GUI_LIB -DQT_XML_LIB -DQT_CORE_LIB"
             LINK_FLAGS "-Wl,-S ${PROJECT_BINARY_DIR}/resfile.o -mwindows -L ${CROSSQT}/lib"
          )
   endif(CMAKE_BUILD_TYPE MATCHES "DEBUG")

   target_link_libraries(mscore
      portaudio
      portmidi
      winmm
      z
      )

   if (OMR)
      target_link_libraries(mscore omr fitz freetype jbig2dec jpeg openjpeg)
      if (OCR)
            target_link_libraries(mscore tesseract_api)
      endif (OCR)
   endif (OMR)

   target_link_libraries(mscore ${QT_mingw_LIBRARIES})

   install( TARGETS mscore RUNTIME DESTINATION bin )

   install( FILES
      ${CROSS}/bin/libgcc_s_dw2-1.dll
      ${CROSS}/bin/libstdc++-6.dll
      ${CROSS}/bin/libwinpthread-1.dll
      ${CROSS}/lib/portaudio.dll
      ${CROSS}/lib/libsndfile-1.dll
      ${CROSS}/lib/libogg.dll
      ${CROSS}/lib/libvorbis.dll
      ${CROSS}/lib/libvorbisfile.dll
      ${CROSSQT}/bin/Qt5Core.dll
      ${CROSSQT}/bin/Qt5Widgets.dll
      ${CROSSQT}/bin/Qt5Gui.dll
      ${CROSSQT}/bin/Qt5Xml.dll
      ${CROSSQT}/bin/Qt5Svg.dll
      ${CROSSQT}/bin/Qt5Network.dll
      ${CROSSQT}/bin/Qt5WebKit.dll
      ${CROSSQT}/bin/Qt5WebKitWidgets.dll
      ${CROSSQT}/bin/Qt5XmlPatterns.dll
      ${CROSSQT}/bin/Qt5Qml.dll
      ${CROSSQT}/bin/Qt5Quick.dll
      ${CROSSQT}/bin/Qt5Script.dll
      ${CROSSQT}/bin/Qt5Sql.dll
      ${CROSSQT}/bin/Qt5V8.dll
      ${CROSSQT}/bin/Qt5PrintSupport.dll
      ${CROSSQT}/bin/Qt5OpenGL.dll
      ${CROSSQT}/bin/Qt5Sensors.dll
      ${CROSSQT}/bin/Qt5Multimedia.dll
      ${CROSSQT}/bin/Qt5MultimediaWidgets.dll
      ${CROSSQT}/bin/icuin51.dll
      ${CROSSQT}/bin/icuuc51.dll
      ${CROSSQT}/bin/icudt51.dll
      DESTINATION bin)

    install(FILES
      ${CROSSQT}/plugins/iconengines/qsvgicon.dll
      DESTINATION bin/iconengines)

    install(FILES
      ${CROSSQT}/plugins/imageformats/qjpeg.dll
      ${CROSSQT}/plugins/imageformats/qmng.dll
      ${CROSSQT}/plugins/imageformats/qsvg.dll
      ${CROSSQT}/plugins/imageformats/qtiff.dll
      DESTINATION bin/imageformats)

    install(FILES
      ${CROSSQT}/plugins/platforms/qwindows.dll
      DESTINATION bin/platforms)

    install(FILES
      ${CROSSQT}/plugins/printsupport/windowsprintersupport.dll
      DESTINATION bin/printsupport)

    install(DIRECTORY
      ${CROSSQT}/qml
      DESTINATION .
      REGEX ".*QtWebkit.*" EXCLUDE
      REGEX ".*QtTest.*" EXCLUDE
      REGEX ".*QtSensors.*" EXCLUDE
      REGEX ".*QtGraphicalEffects.*" EXCLUDE
      REGEX ".*QtMultimedia.*" EXCLUDE
      REGEX ".*d\\.dll" EXCLUDE)

else (MINGW)
   target_link_libraries(mscore
      ${ALSA_LIB}
      ${QT_LIBRARIES}
      z
      dl
      pthread
      )
    if (USE_PORTAUDIO)
      target_link_libraries(mscore ${PORTAUDIO_LIB})
    endif (USE_PORTAUDIO)

    if (USE_PULSEAUDIO)
      target_link_libraries(mscore ${PULSEAUDIO_LIBRARY})
    endif (USE_PULSEAUDIO)

   set_target_properties (
      mscore
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wno-overloaded-virtual -Winvalid-pch"
      )

   if (OMR)
      target_link_libraries(mscore omr fitz fontconfig freetype jbig2dec jpeg openjpeg)
      if (OCR)
            target_link_libraries(mscore tesseract_api)
      endif (OCR)
   endif (OMR)

   if (APPLE)
     if (USE_PORTMIDI)
       target_link_libraries(mscore portmidi)
     endif (USE_PORTMIDI)
     target_link_libraries(mscore ${OsxFrameworks})
   else (APPLE)
       target_link_libraries(mscore rt)
   endif (APPLE)

   # gold does not use indirect shared libraries for symbol resolution, Linux only
   if (NOT APPLE)
      if(USE_JACK)
         target_link_libraries(mscore dl)
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
     install( TARGETS mscore RUNTIME DESTINATION bin )
     install( FILES   data/mscore.png DESTINATION share/pixmaps)
   endif (APPLE)
endif (MINGW)

if (APPLE)
     install (FILES
      ../fonts/gonville-20.ttf
      ../fonts/mscore-20.ttf
      ../fonts/MuseJazz.ttf
      ../fonts/FreeSerifMscore.ttf
      ../fonts/FreeSans.ttf
      ../fonts/mscoreTab.ttf
      ../fonts/mscore-BC.ttf
      ../fonts/bravura/Bravura.otf
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

