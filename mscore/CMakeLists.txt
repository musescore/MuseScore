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

function(prepend_to_list_items # prepend a string to each item in list
  LIST_INV # input list variable
  STRING # the string to prepend to each list item
  LIST_OUTV # input list variable
  )
  set(LIST_IN "${${LIST_INV}}")
  set(LIST_OUT "") # empty list
  foreach(ITEM IN LISTS LIST_IN)
    list(APPEND LIST_OUT "${STRING}${ITEM}")
  endforeach(ITEM)
  set("${LIST_OUTV}" "${LIST_OUT}" PARENT_SCOPE)
endfunction(prepend_to_list_items)

add_subdirectory(widgets)
prepend_to_list_items(WIDGETS_SOURCE_FILES "widgets/" WIDGETS_SOURCE_FILES)

# This is not needed for MSVC compilation
if (NOT MSVC)
   include (${PROJECT_SOURCE_DIR}/build/gch.cmake)
endif (NOT MSVC)
include (${PROJECT_SOURCE_DIR}/build/CopyFilesMacros.cmake)

SET_SOURCE_FILES_PROPERTIES(revision.h PROPERTIES GENERATED TRUE)

include_directories(
      ${CMAKE_CURRENT_BINARY_DIR}
      ${PROJECT_SOURCE_DIR}
      ${PROJECT_SOURCE_DIR}/thirdparty
      ${QTSINGLEAPPLICATION_INCLUDE_DIRS}
      )

if (SCRIPT_INTERFACE)
      set (SCRIPT_FILES
            plugin/pluginCreator.h plugin/pluginManager.h plugin/qmledit.h
            plugin/qmlplugin.h plugin/qmlpluginengine.h
            plugin/qmliconview.h
            plugin/api/qmlpluginapi.h plugin/api/cursor.h plugin/api/scoreelement.h plugin/api/elements.h
            plugin/api/part.h
            plugin/api/score.h
            plugin/api/fraction.h
            plugin/api/excerpt.h
            plugin/api/util.h
            plugin/api/selection.h
            plugin/api/tie.h
            plugin/api/playevent.h

            plugin/api/enums.cpp
            plugin/mscorePlugins.cpp plugin/pluginCreator.cpp plugin/pluginManager.cpp plugin/qmledit.cpp
            plugin/qmlplugin.cpp plugin/qmlpluginengine.cpp
            plugin/qmliconview.cpp
            plugin/api/qmlpluginapi.cpp plugin/api/cursor.cpp plugin/api/scoreelement.cpp plugin/api/elements.cpp
            plugin/api/score.cpp
            plugin/api/excerpt.cpp
            plugin/api/util.cpp
            plugin/api/selection.cpp
            plugin/api/tie.cpp
            plugin/api/playevent.cpp
            )

      set (SCRIPT_UI
            plugin/pluginCreator.ui plugin/pluginManager.ui
            )
endif (SCRIPT_INTERFACE)

QT5_WRAP_UI (ui_headers
      insertmeasuresdialog.ui editinstrument.ui editstyle.ui instrdialog.ui instrwidget.ui
      measuresdialog.ui pagesettings.ui mixer.ui mixertrackchannel.ui mixertrackpart.ui mixerdetails.ui parteditbase.ui
      playpanel.ui prefsdialog.ui measureproperties.ui
      textpalette.ui  timedialog.ui symboldialog.ui  shortcutcapturedialog.ui  editdrumset.ui
      editstaff.ui timesigproperties.ui
      instrwizard.ui timesigwizard.ui newwizard.ui aboutbox.ui aboutmusicxmlbox.ui
      transposedialog.ui excerptsdialog.ui  stafftext.ui tupletdialog.ui
      articulation.ui metaedit.ui paletteProperties.ui paletteCellProperties.ui selectdialog.ui selectnotedialog.ui
      synthcontrol.ui splitstaff.ui keyedit.ui selectinstr.ui
      fretdprops.ui editstafftype.ui sectionbreak.ui  bend.ui tremolobar.ui
      editpitch.ui editstringdata.ui editraster.ui mediadialog.ui albummanager.ui layer.ui
      omrpanel.ui masterpalette.ui harmonyedit.ui pathlistdialog.ui
      note_groups.ui resourceManager.ui stafftypetemplates.ui
      startcenter.ui scorePreview.ui scoreBrowser.ui templateBrowser.ui
      logindialog.ui uploadscoredialog.ui breaksdialog.ui
      toolbarEditor.ui workspacedialog.ui

      importmidi/importmidi_panel.ui

      scorecmp/scorecmp_tool.ui
      script/script_recorder.ui

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
      debugger/bracket.ui

      inspector/inspector_element.ui
      inspector/inspector_vbox.ui
      inspector/inspector_tbox.ui
      inspector/inspector_articulation.ui
      inspector/inspector_spacer.ui
      inspector/inspector_hbox.ui
      inspector/inspector_segment.ui
      inspector/inspector_beam.ui
      inspector/inspector_note.ui
      inspector/inspector_notedot.ui
      inspector/inspector_rest.ui
      inspector/inspector_chord.ui
      inspector/inspector_group_element.ui
      inspector/inspector_image.ui
      inspector/inspector_harmony.ui
      inspector/inspector_lasso.ui
      inspector/inspector_volta.ui
      inspector/inspector_ottava.ui
      inspector/inspector_trill.ui
      inspector/inspector_vibrato.ui
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
      inspector/inspector_instrchange.ui
      inspector/inspector_lyric.ui
      inspector/inspector_line.ui
      inspector/inspector_textline.ui
      inspector/inspector_letring.ui
      inspector/inspector_palmmute.ui
      inspector/inspector_pedal.ui
      inspector/inspector_textlinebase.ui
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
      inspector/inspector_tremolobar.ui
      inspector/inspector_caesura.ui
      inspector/inspector_bracket.ui
      inspector/inspector_iname.ui
      inspector/inspector_fingering.ui
      inspector/inspector_frametext.ui
      inspector/inspector_fermata.ui
      inspector/align_select.ui
      inspector/font_style_select.ui
      inspector/offset_select.ui
      inspector/scale_select.ui
      inspector/size_select.ui
      inspector/inspector_stem.ui
      ${SCRIPT_UI}
      )

if (APPLE)
      QT5_ADD_RESOURCES (qrc_files musescore.qrc
            qml.qrc # TODO: replace with qtquick_compiler_add_resources on Qt >= 5.11
            musescorefonts-Mac.qrc shortcut-Mac.qrc
            )
else (APPLE)
      QT5_ADD_RESOURCES (qrc_files musescore.qrc
            qml.qrc # TODO: replace with qtquick_compiler_add_resources on Qt >= 5.11
            musescorefonts-MScore.qrc
            musescorefonts-Gootville.qrc
            musescorefonts-Bravura.qrc
            musescorefonts-MuseJazz.qrc
            musescorefonts-Campania.qrc
            musescorefonts-FreeSerif.qrc
            musescorefonts-Free.qrc
            shortcut.qrc)
endif (APPLE)


set (AUDIO "")
if (MINGW)
      set (resource_file ${PROJECT_BINARY_DIR}/resfile.o)
endif (MINGW)
if (MSVC)
      # MSVC recognizes a *.rc file and will invoke the resource compiler to link it
      set (resource_file ${PROJECT_SOURCE_DIR}/mscore/data/mscore.rc)
endif ( MSVC )
if ( NOT MINGW AND NOT MSVC )
      if (USE_ALSA)
            set (AUDIO ${AUDIO} alsa.cpp)
      endif (USE_ALSA)
endif ( NOT MINGW AND NOT MSVC )

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
      if(CMAKE_BUILD_NUMBER)
        set (MACOSX_BUNDLE_SHORT_VERSION_STRING ${MUSESCORE_VERSION_FULL}.${CMAKE_BUILD_NUMBER})
        set (MACOSX_BUNDLE_BUNDLE_VERSION ${MUSESCORE_VERSION_FULL}.${CMAKE_BUILD_NUMBER})
      endif(CMAKE_BUILD_NUMBER)
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
      if (SPARKLE_FOUND)
            set(MAC_SPARKLE_FILES "macos/SparkleAutoUpdater.mm")
      endif(SPARKLE_FOUND)
else (APPLE)
      set(INCS "")
      set(COCOABRIDGE "")
      set(MAC_SPARKLE_FILES  "")
endif (APPLE)

if (NOT MSVC)
   set(_all_h_file "${PROJECT_BINARY_DIR}/all.h")
else (NOT MSVC)
   set(_all_h_file "${PROJECT_SOURCE_DIR}/all.h")
endif (NOT MSVC)

add_executable ( ${ExecutableName}
      ${qrc_files}
      ${ui_headers}
      ${_all_h_file}
      ${PCH}
      ${resource_file}
      ${INCS}

      abstractdialog.h accessibletoolbutton.h albummanager.h
      alsa.h alsamidi.h analyse.h articulationprop.h bb.h
      bendcanvas.h bendproperties.h breaksdialog.h capella.h
      chordview.h click.h continuouspanel.h downloadUtils.h
      driver.h drumroll.h drumtools.h drumview.h editdrumset.h
      editinstrument.h editpitch.h editraster.h editstaff.h
      editstafftype.h editstringdata.h editstyle.h enableplayforwidget.h
      exampleview.h excerptsdialog.h exportmidi.h exportmp3.h extension.h
      file.h fotomode.h fretcanvas.h fretproperties.h globals.h greendotbutton.h
      harmonycanvas.h harmonyedit.h help.h helpBrowser.h icons.h importgtp.h importmxml.h
      importmxmllogger.h importmxmlnoteduration.h importmxmlnotepitch.h importmxmlpass1.h
      importmxmlpass2.h importptb.h importxmlfirstpass.h instrdialog.h instrwidget.h jackaudio.h
      keycanvas.h keyedit.h layer.h licence.h
      logindialog.h network/loginmanager.h network/loginmanager_p.h
      magbox.h masterpalette.h
      measureproperties.h mediadialog.h metaedit.h miconengine.h mididriver.h
      mixer.h mixertrack.h mixertrackchannel.h mixertrackgroup.h mixertrackitem.h mixertrackpart.h mixerdetails.h musedata.h
      musescore.h musicxml.h musicxmlfonthandler.h musicxmlsupport.h navigator.h newwizard.h noteGroups.h
      omrpanel.h ove.h pa.h pagesettings.h palette.h partedit.h parteditbase.h
      pathlistdialog.h piano.h pianolevels.h pianolevelschooser.h  pianolevelsfilter.h
      pianokeyboard.h pianoroll.h pianoruler.h pianotools.h pianoview.h
      playpanel.h pm.h preferences.h preferenceslistwidget.h prefsdialog.h
      recordbutton.h resourceManager.h revision.h ruler.h scoreaccessibility.h
      scoreBrowser.h scoreInfo.h scorePreview.h scoretab.h scoreview.h searchComboBox.h
      sectionbreakprop.h selectdialog.h selectionwindow.h selectnotedialog.h selinstrument.h
      seq.h shortcut.h shortcutcapturedialog.h simplebutton.h splitstaff.h stafftextproperties.h
      startcenter.h startupWizard.h stringutils.h svggenerator.h symboldialog.h synthcontrol.h
      templateBrowser.h textcursor.h textpalette.h texttools.h timedialog.h timeline.h timesigproperties.h
      toolbarEditor.h toolbuttonmenu.h transposedialog.h tremolobarcanvas.h tremolobarprop.h
      tupletdialog.h updatechecker.h uploadscoredialog.h waveview.h webpage.h workspace.h

      recordbutton.h greendotbutton prefsdialog.h prefsdialog.cpp
      stringutils.h stringutils.cpp
      scoreview.cpp editharmony.cpp editfiguredbass.cpp events.cpp
      editinstrument.cpp editstyle.cpp
      icons.cpp importbww.cpp
      importmxmllogger.cpp importmxmlnoteduration.cpp importmxmlnotepitch.cpp
      importmxml.cpp importmxmlpass1.cpp importmxmlpass2.cpp
      instrdialog.cpp instrwidget.cpp
      debugger/debugger.cpp menus.cpp
      musescore.cpp navigator.cpp pagesettings.cpp palette.cpp
      timeline.cpp
      mixer.cpp mixertrackchannel.cpp mixertrackitem.cpp mixertrackpart.cpp mixerdetails.cpp
      parteditbase.cpp playpanel.cpp selectionwindow.cpp
      preferences.cpp measureproperties.cpp
      seq.cpp textpalette.cpp
      timedialog.cpp symboldialog.cpp shortcutcapturedialog.cpp
      simplebutton.cpp musedata.cpp
      editdrumset.cpp editstaff.cpp
      timesigproperties.cpp newwizard.cpp transposedialog.cpp
      excerptsdialog.cpp metaedit.cpp magbox.cpp
      capella.cpp capxml.cpp exportaudio.cpp
      synthcontrol.cpp drumroll.cpp pianoroll.cpp piano.cpp
      pianokeyboard.cpp pianolevels.cpp pianolevelschooser.cpp pianolevelsfilter.cpp
      pianoruler.cpp pianoview.cpp drumview.cpp scoretab.cpp keyedit.cpp harmonyedit.cpp
      updatechecker.cpp
      importove.cpp
      ove.cpp
      ruler.cpp
      importgtp.cpp importgtp-gp4.cpp importgtp-gp5.cpp importgtp-gp6.cpp
      importptb.cpp
      fotomode.cpp drumtools.cpp
      selinstrument.cpp editstafftype.cpp texttools.cpp
      editpitch.cpp editstringdata.cpp editraster.cpp pianotools.cpp mediadialog.cpp
      workspace.cpp workspacedialog.cpp chordview.cpp
      workspacecombobox.cpp
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
      inspector/inspectorHarmony.cpp
      inspector/inspectorFret.cpp
      inspector/inspectorText.cpp
      inspector/inspectorMeasureNumber.cpp
      inspector/inspectorLasso.cpp
      inspector/inspectorBarline.cpp
      inspector/inspectorVolta.cpp
      inspector/inspectorOttava.cpp
      inspector/inspectorTrill.cpp
      inspector/inspectorVibrato.cpp
      inspector/inspectorHairpin.cpp
      inspector/inspectorJump.cpp
      inspector/inspectorMarker.cpp
      inspector/inspectorGlissando.cpp
      inspector/inspectorNote.cpp
      inspector/inspectorNoteDot.cpp
      inspector/inspectorAmbitus.cpp
      inspector/inspectorArpeggio.cpp
      inspector/inspectorFingering.cpp
      inspector/alignSelect.cpp
      inspector/fontStyleSelect.cpp
      inspector/offsetSelect.cpp
      inspector/scaleSelect.cpp
      inspector/sizeSelect.cpp
      inspector/inspectorTextBase.cpp
      inspector/inspectorTextLineBase.cpp
      inspector/inspectorTextLine.cpp
      inspector/inspectorLetRing.cpp
      inspector/inspectorPalmMute.cpp
      inspector/inspectorPedal.cpp
      inspector/inspectorDynamic.cpp
      inspector/inspectorInstrchange.cpp
      inspector/inspectorElementBase.cpp
      inspector/resetButton.cpp
      enableplayforwidget.cpp
      editelement.cpp
      editlyrics.cpp
      musicxmlfonthandler.cpp
      musicxmlsupport.cpp
      exportxml.cpp
      importxml.cpp
      importxmlfirstpass.cpp
      savePositions.cpp
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
      palette/palettedialog.cpp palette/palettecelldialog.cpp
      palette/palettemodel.cpp palette/palettetree.cpp palette/paletteworkspace.cpp palette/palettewidget.cpp
      scorecmp/scorecmp.cpp scorecmp/scorediffmodel.cpp scorecmp/scorelistmodel.cpp
      resourceManager.cpp downloadUtils.cpp
      textcursor.cpp continuouspanel.cpp accessibletoolbutton.cpp scoreaccessibility.cpp
      startcenter.cpp scoreBrowser.cpp scorePreview.cpp scoreInfo.cpp templateBrowser.cpp
      logindialog.cpp network/loginmanager.cpp uploadscoredialog.cpp
      breaksdialog.cpp searchComboBox.cpp
      help.cpp help.h
      toolbarEditor.cpp toolbarEditor.h
      abstractdialog.cpp abstractdialog.h
      toolbuttonmenu.cpp
      preferenceslistwidget.cpp preferenceslistwidget.h
      extension.cpp extension.h
      tourhandler.cpp
      script/script.cpp script/scriptentry.cpp script/testscript.cpp script/recorderwidget.cpp
      qml/msqmlengine.cpp qml/nativemenu.cpp qml/nativetooltip.cpp
      qmldockwidget.cpp

      ${WIDGETS_SOURCE_FILES}
      ${COCOABRIDGE}
      ${OMR_FILES}
      ${AUDIO}
      ${SCRIPT_FILES}
      ${MAC_SPARKLE_FILES}
      driver.h
      tremolobarcanvas.h bendcanvas.h fretcanvas.h keycanvas.h harmonycanvas.h

      sparkle/autoUpdater.h sparkle/sparkleAutoUpdater.h sparkle/winSparkleAutoUpdater.h
      )



if (MSVC OR MINGW)
	target_link_libraries(mscore vorbisfiledll)
else (MSVC OR MINGW)
	target_link_libraries(mscore vorbisfile)
endif (MSVC OR MINGW)

target_link_libraries(mscore
      awl
      bww
      rtf2html
      ${QTSINGLEAPPLICATION_LIBRARIES}
      synthesizer
      midi
      fluid
      effects
      libmscore
      qzip
      beatroot
      kqoauth
      )

if (BUILD_CRASH_REPORTER)
      target_link_libraries(mscore crashreporter-handler)
endif (BUILD_CRASH_REPORTER)

add_dependencies(mscore workspaces)

if (USE_SYSTEM_FREETYPE)
      target_link_libraries(mscore ${FREETYPE_LIBRARIES})
endif (USE_SYSTEM_FREETYPE)

if (MINGW OR MSVC)
      set(MSCORE_OUTPUT_NAME ${MUSESCORE_NAME}${MUSESCORE_VERSION_MAJOR})
elseif (MSCORE_INSTALL_SUFFIX)
      set(MSCORE_OUTPUT_NAME "${ExecutableName}${MSCORE_INSTALL_SUFFIX}")
endif (MINGW OR MSVC)

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
      if (MSVC OR MINGW)
          target_link_libraries(mscore vorbisdll oggdll)
      else (MSVC OR MINGW)
          target_link_libraries(mscore ${VORBIS_LIB} ${OGG_LIB})
      endif (MSVC OR MINGW)
endif (SOUNDFONT3)

if (HAS_AUDIOFILE)
      if (MSVC OR MINGW)
         target_link_libraries(mscore audiofile sndfiledll)
      else (MSVC OR MINGW)
         target_link_libraries(mscore audiofile ${SNDFILE_LIB})
      endif (MSVC OR MINGW)
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
           LINK_FLAGS "-mwindows -mconsole -L ${QT_INSTALL_LIBS}"
        )
   else(CMAKE_BUILD_TYPE MATCHES "DEBUG")
     set_target_properties( mscore
          PROPERTIES
             COMPILE_FLAGS "${PCH_INCLUDE} -Wall -Wextra -Winvalid-pch ${QT_DEFINITIONS} -DQT_SVG_LIB -DQT_GUI_LIB -DQT_XML_LIB -DQT_CORE_LIB"
             LINK_FLAGS "-Wl,-S -mwindows -L ${QT_INSTALL_LIBS}"
          )
   endif(CMAKE_BUILD_TYPE MATCHES "DEBUG")

   target_link_libraries(mscore
      portaudiodll
      portmidi
      winmm
      mscore_freetype
      z
      )

   if (OMR)
      target_link_libraries(mscore omr poppler-qt5)
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

   if (BUILD_64)
      install( FILES
         ${MINGW_ROOT}/bin/libgcc_s_seh-1.dll
         DESTINATION bin)
      if (Qt5Widgets_VERSION VERSION_GREATER_EQUAL "5.12.4")
         install( FILES
            ${DEPENDENCIES_DIR}/libcrypto-1_1-x64.dll
            ${DEPENDENCIES_DIR}/libssl-1_1-x64.dll
            DESTINATION bin)
      endif (Qt5Widgets_VERSION VERSION_GREATER_EQUAL "5.12.4")
      install( FILES
         ${DEPENDENCIES_DIR}/portaudio.dll RENAME libportaudio-x86_64-w64-mingw32.static.dll
         DESTINATION bin)
   else (BUILD_64)
      install( FILES
         ${MINGW_ROOT}/bin/libgcc_s_dw2-1.dll
         ${DEPENDENCIES_DIR}/portaudio.dll
         DESTINATION bin)
      if (Qt5Widgets_VERSION VERSION_GREATER_EQUAL "5.12.4")
         install( FILES
            ${DEPENDENCIES_DIR}/libcrypto-1_1.dll
            ${DEPENDENCIES_DIR}/libssl-1_1.dll
            DESTINATION bin)
      endif (Qt5Widgets_VERSION VERSION_GREATER_EQUAL "5.12.4")
   endif (BUILD_64)

   install( FILES
      ${MINGW_ROOT}/bin/libstdc++-6.dll
      ${MINGW_ROOT}/bin/libwinpthread-1.dll
      ${DEPENDENCIES_DIR}/libogg.dll
      ${DEPENDENCIES_DIR}/libsndfile-1.dll
      ${DEPENDENCIES_DIR}/libvorbis.dll
      ${DEPENDENCIES_DIR}/libvorbisfile.dll
      ${QT_INSTALL_BINS}/libEGL.dll
      ${QT_INSTALL_BINS}/libGLESv2.dll
      ${QT_INSTALL_BINS}/opengl32sw.dll
      ${QtInstallLibraries}
      ${PROJECT_SOURCE_DIR}/build/qt.conf
      DESTINATION bin)
   if (Qt5Widgets_VERSION VERSION_LESS "5.12.4")
      install( FILES
         ${MINGW_ROOT}/opt/bin/libeay32.dll
         ${MINGW_ROOT}/opt/bin/ssleay32.dll
         DESTINATION bin)
   endif (Qt5Widgets_VERSION VERSION_LESS "5.12.4")

   install (FILES
      ${DEPENDENCIES_DIR}/lame_enc.dll
      DESTINATION bin
      OPTIONAL)

    install(FILES
      ${QT_INSTALL_PLUGINS}/iconengines/qsvgicon.dll
      DESTINATION bin/iconengines)

    install(FILES
      ${QT_INSTALL_PLUGINS}/imageformats/qjpeg.dll
      ${QT_INSTALL_PLUGINS}/imageformats/qsvg.dll
      ${QT_INSTALL_PLUGINS}/imageformats/qtiff.dll
      DESTINATION bin/imageformats)

    install(FILES
      ${QT_INSTALL_PLUGINS}/platforms/qwindows.dll
      DESTINATION bin/platforms)

    install(FILES
      ${QT_INSTALL_PLUGINS}/printsupport/windowsprintersupport.dll
      DESTINATION bin/printsupport)

    install(FILES
      ${QT_INSTALL_PLUGINS}/sqldrivers/qsqlite.dll
      DESTINATION bin/sqldrivers)

    install(DIRECTORY
      ${QT_INSTALL_QML}
      DESTINATION .
      REGEX ".*d\\.dll" EXCLUDE
      REGEX ".*QtMultimedia.*" EXCLUDE
      REGEX ".*QtSensors.*" EXCLUDE
      REGEX ".*QtTest.*" EXCLUDE
      REGEX ".*QtWebkit.*" EXCLUDE)

else (MINGW)

   if ( NOT MSVC )
## install qwebengine core
      if (NOT APPLE AND USE_WEBENGINE)
         install(PROGRAMS
            ${QT_INSTALL_LIBEXECS}/QtWebEngineProcess
            DESTINATION bin
            )
         install(DIRECTORY
            ${QT_INSTALL_DATA}/resources
            DESTINATION lib/qt5
            )
         install(DIRECTORY
            ${QT_INSTALL_TRANSLATIONS}/qtwebengine_locales
            DESTINATION lib/qt5/translations
            )
      endif(NOT APPLE AND USE_WEBENGINE)

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
               find_library(PORTTIME_LIB porttime)
               if (PORTTIME_LIB)
                   set(PORTMIDI_LIB portmidi porttime)
               else (PORTTIME_LIB)
                   set(PORTMIDI_LIB portmidi)
               endif (PORTTIME_LIB)
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
         target_link_libraries(mscore omr poppler-qt5)
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
        set(MSCORE_ICON_BASE ../assets/musescore-icon-round)
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
   else ( NOT MSVC )
      # Microsoft Visual Studio-specific starts here!
      string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)

      # Create list of directories to search for libraries
      foreach (item ${CMAKE_LIBRARY_PATH})
         string( APPEND all_library_paths " /LIBPATH:${item}" )
      endforeach()

      # Windows: Add /SUBSYSTEM:WINDOWS to LINK_FLAGS to avoid a console window in release
      if(CMAKE_BUILD_TYPE MATCHES "REL*")
        set_target_properties( mscore
           PROPERTIES
              COMPILE_FLAGS "${PCH_INCLUDE} ${QT_DEFINITIONS} -DQT_SVG_LIB -DQT_GUI_LIB -DQT_XML_LIB -DQT_CORE_LIB"
              LINK_FLAGS    "/LIBPATH:${QT_INSTALL_LIBS} ${all_library_paths}"
              LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
           )
      else(CMAKE_BUILD_TYPE MATCHES "REL*")
        set_target_properties( mscore
           PROPERTIES
              COMPILE_FLAGS "${PCH_INCLUDE} ${QT_DEFINITIONS} /DQT_SVG_LIB /DQT_GUI_LIB /DQT_XML_LIB /DQT_CORE_LIB"
              LINK_FLAGS    "/LIBPATH:${QT_INSTALL_LIBS} ${all_library_paths}"
              LINK_FLAGS "/SUBSYSTEM:CONSOLE"
           )
      endif(CMAKE_BUILD_TYPE MATCHES "REL*")

      include(FindStaticLibrary)
      target_link_libraries(mscore
         portaudiodll
         portmidi
         winmm
         mscore_freetype
         zlibstat
         )

      if (OMR)
         target_link_libraries(mscore omr poppler-qt5)
         if (OCR)
               target_link_libraries(mscore tesseract_api)
         endif (OCR)
      endif (OMR)

      # Use pre-compiled headers
      vstudio_pch( mscore )

      # Copy DLL dependencies to .EXE DIRECTORY
      list(APPEND dlls_to_copy
                  "${QT_INSTALL_BINS}/Qt5Core.dll"  "${QT_INSTALL_BINS}/Qt5Gui.dll"  "${QT_INSTALL_BINS}/Qt5Help.dll"
                  "${QT_INSTALL_BINS}/Qt5Network.dll"  "${QT_INSTALL_BINS}/Qt5PrintSupport.dll"
                  "${QT_INSTALL_BINS}/Qt5Qml.dll"  "${QT_INSTALL_BINS}/Qt5Quick.dll"  "${QT_INSTALL_BINS}/Qt5Sql.dll"
                  "${QT_INSTALL_BINS}/Qt5QuickControls2.dll" "${QT_INSTALL_BINS}/Qt5QuickTemplates2.dll"
                  "${QT_INSTALL_BINS}/Qt5Svg.dll"  "${QT_INSTALL_BINS}/Qt5Widgets.dll"  "${QT_INSTALL_BINS}/Qt5Xml.dll"
                  "${QT_INSTALL_BINS}/Qt5XmlPatterns.dll"
                  "${QT_INSTALL_BINS}/Qt5WebChannel.dll"
                  "${QT_INSTALL_BINS}/Qt5QuickWidgets.dll" "${QT_INSTALL_BINS}/Qt5Positioning.dll"
                  "${QT_INSTALL_BINS}/libEGL.dll" "${QT_INSTALL_BINS}/libGLESv2.dll" "${QT_INSTALL_BINS}/opengl32sw.dll"
                  )
      if (USE_WEBENGINE)
         list(APPEND dlls_to_copy "${QT_INSTALL_BINS}/Qt5WebEngineWidgets.dll" "${QT_INSTALL_BINS}/Qt5WebEngineCore.dll")
      endif(USE_WEBENGINE)

      set(CMAKE_FIND_LIBRARY_PREFIX "")
      set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")

      if (WIN_SPARKLE_ENABLED)
            target_link_libraries(mscore winsparkledll)
            find_library( dll_winsparkle  NAMES "WinSparkle" PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
            list(APPEND dlls_to_copy ${dll_winsparkle})
      endif (WIN_SPARKLE_ENABLED)

      find_library( dll_lame        NAMES "lame_enc"       PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      find_library( dll_ogg         NAMES "libogg"         PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      find_library( dll_sndfile     NAMES "libsndfile-1"   PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      find_library( dll_vorbis      NAMES "libvorbis"      PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      find_library( dll_vorbisfile  NAMES "libvorbisfile"  PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      find_library( dll_portaudio   NAMES "portaudio"      PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      if (Qt5Widgets_VERSION VERSION_LESS "5.12.4")
         find_library( dll_ssl1        NAMES "libeay32"       PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
         find_library( dll_ssl2        NAMES "ssleay32"       PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      else (Qt5Widgets_VERSION VERSION_LESS "5.12.4")
         find_library( dll_ssl1        NAMES "libcrypto-1_1-x64" "libcrypto-1_1"  PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
         find_library( dll_ssl2        NAMES "libssl-1_1-x64" "libssl-1_1"        PATHS ${DEPENDENCIES_DIR} NO_DEFAULT_PATH)
      endif (Qt5Widgets_VERSION VERSION_LESS "5.12.4")

      list(APPEND dlls_to_copy ${dll_lame} ${dll_ogg} ${dll_sndfile} ${dll_vorbis} ${dll_vorbisfile} ${dll_portaudio} ${dll_ssl1} ${dll_ssl2} "$<TARGET_FILE_DIR:mscore>/${MSCORE_OUTPUT_NAME}.exe")
      set( output_dir_for_dlls "${PROJECT_BINARY_DIR}/../msvc.install${ARCH_TYPE}/bin")

      COPY_FILES_INTO_DIRECTORY_IF_CHANGED( "${dlls_to_copy}" ${output_dir_for_dlls}  mscore)

      add_dependencies(mscore pluginDocumentation)

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
      add_dependencies(mscore lrelease)

      install( FILES
            ${dll_ogg}
            ${dll_lame}
            ${dll_sndfile}
            ${dll_portaudio}
            ${dll_vorbis}
            ${dll_vorbisfile}
            ${dll_ssl1}
            ${dll_ssl2}
            ${dll_winsparkle}
            ${QtInstallLibraries}
            ${QT_INSTALL_BINS}/libEGL.dll
            ${QT_INSTALL_BINS}/libGLESv2.dll
            ${QT_INSTALL_BINS}/opengl32sw.dll
            ${QT_INSTALL_BINS}/Qt5Positioning.dll
            ${QT_INSTALL_BINS}/Qt5WebChannel.dll
            ${PROJECT_SOURCE_DIR}/build/qt.conf
            DESTINATION bin)

      install(FILES
         ${QT_INSTALL_PLUGINS}/iconengines/qsvgicon.dll
         DESTINATION bin/iconengines)

      install(FILES
         ${QT_INSTALL_PLUGINS}/imageformats/qjpeg.dll
         ${QT_INSTALL_PLUGINS}/imageformats/qsvg.dll
         ${QT_INSTALL_PLUGINS}/imageformats/qtiff.dll
         DESTINATION bin/imageformats)

      install(FILES
         ${QT_INSTALL_PLUGINS}/platforms/qwindows.dll
         DESTINATION bin/platforms)

      install(FILES
         ${QT_INSTALL_PLUGINS}/printsupport/windowsprintersupport.dll
         DESTINATION bin/printsupport)

      install(FILES
         ${QT_INSTALL_PLUGINS}/sqldrivers/qsqlite.dll
         DESTINATION bin/sqldrivers)

      install( TARGETS mscore RUNTIME DESTINATION bin ) # this duplicate due to the correctly package step

      if (USE_WEBENGINE)
         install(FILES
            ${QT_INSTALL_LIBEXECS}/QtWebEngineProcess.exe
            DESTINATION bin
            )
         install(DIRECTORY
            ${QT_INSTALL_DATA}/resources
            DESTINATION bin/webengineresources
            )
         install(DIRECTORY
            ${QT_INSTALL_TRANSLATIONS}/qtwebengine_locales
            DESTINATION bin/webengineresources/translations
            )
      endif (USE_WEBENGINE)

      install(DIRECTORY
         ${QT_INSTALL_QML}
         DESTINATION .
         REGEX ".*d\\.dll" EXCLUDE
         REGEX ".pdb" EXCLUDE
         REGEX ".*QtMultimedia.*" EXCLUDE
         REGEX ".*QtSensors.*" EXCLUDE
         REGEX ".*QtTest.*" EXCLUDE
         REGEX ".*QtWebkit.*" EXCLUDE
         )


   endif ( NOT MSVC )
endif (MINGW)

if (APPLE)
     install (FILES
      ../fonts/gootville/GootvilleText.otf
      ../fonts/mscore/MScoreText.ttf
      ../fonts/musejazz/MuseJazzText.otf
      ../fonts/campania/Campania.otf
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
      ${QT_INSTALL_QML}
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}
      REGEX ".*QtWebkit.*" EXCLUDE
      REGEX ".*QtTest.*" EXCLUDE
      REGEX ".*QtSensors.*" EXCLUDE
      REGEX ".*QtMultimedia.*" EXCLUDE
      REGEX ".*QtAudioEngine.*" EXCLUDE
      REGEX ".*_debug\\.dylib" EXCLUDE)
endif (APPLE)

# MSVC does not depend on mops1 & mops2 for PCH
if (NOT MSVC)
   ADD_DEPENDENCIES(${ExecutableName} mops1)
   ADD_DEPENDENCIES(${ExecutableName} mops2)
endif (NOT MSVC)

add_library(
      inspectorplugin SHARED
      inspector/inspectorplugin.cpp
      inspector/inspectorplugin.h
#      inspector/alignSelect.cpp
      inspector/fontStyleSelect.cpp
#      inspector/offsetSelect.cpp
#      inspector/scaleSelect.cpp
#      inspector/sizeSelect.cpp
      inspector/resetButton.cpp
      icons.cpp
      miconengine.cpp
      ${qrc_files}
      )

if (APPLE)
      set_property(SOURCE inspector/inspectorplugin.cpp inspector/inspectorplugin.h PROPERTY SKIP_AUTOGEN ON)
endif (APPLE)

if (NOT MSVC)
   set_target_properties(inspectorplugin
      PROPERTIES
      COMPILE_FLAGS
         "-fPIC -D_GNU_SOURCE -D_REENTRANT -DHAVE_CONFIG_H -DQT_PLUGIN -DQT_SVG_LIB -DQT_SHARED -DQT_NO_DEBUG -include ${PROJECT_SOURCE_DIR}/all.h"
      )
else (NOT MSVC)
   # x86 archictecture does not support position-independent code, the -fPIC option is not relevant for this processor.
   set_target_properties (
      inspectorplugin
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE} -D_GNU_SOURCE -D_REENTRANT -DHAVE_CONFIG_H -DQT_PLUGIN -DQT_SVG_LIB -DQT_SHARED -DQT_NO_DEBUG"
      )
endif (NOT MSVC)

# Don't install awlplugin on default
#install( TARGETS inspectorplugin DESTINATION ${designerPlugins}/plugins/designer/ )

# add_executable(itest inspector/itest.cpp)
# target_link_libraries (itest inspectorplugin ${QT_LIBRARIES})
# set_target_properties(itest PROPERTIES EXCLUDE_FROM_ALL 1)

target_link_libraries (inspectorplugin ${QT_LIBRARIES})
set_target_properties(inspectorplugin PROPERTIES EXCLUDE_FROM_ALL 1)

if (MSCORE_OUTPUT_NAME)
      set (MSCORE_EXECUTABLE_NAME ${MSCORE_OUTPUT_NAME})
else (MSCORE_OUTPUT_NAME)
      set (MSCORE_EXECUTABLE_NAME ${ExecutableName})
endif (MSCORE_OUTPUT_NAME)
# Export the executable name to the global context
set (MSCORE_EXECUTABLE_NAME "${MSCORE_EXECUTABLE_NAME}" PARENT_SCOPE)
