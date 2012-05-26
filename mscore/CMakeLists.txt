#=============================================================================
#  MuseScore
#  Linux Music Score Editor
#  $Id:$
#
#  Copyright (C) 2002-2010 by Werner Schweer and others
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
      ${PROJECT_SOURCE_DIR}/scriptdebug
      ${QTSINGLEAPPLICATION_INCLUDE_DIRS}
      )

QT4_WRAP_UI (ui_headers
      insertmeasuresdialog.ui barline.ui chord.ui chordrest.ui editinstrument.ui editstyle.ui
      edittempo.ui element.ui hairpin.ui instrdialog.ui measure.ui measuresdialog.ui
      note.ui page.ui pagesettings.ui mixer.ui playpanel.ui prefsdialog.ui
      measureproperties.ui segment.ui text.ui textpalette.ui timedialog.ui
      symboldialog.ui dynamic.ui tuplet.ui shortcutcapturedialog.ui slurtie.ui
      slur.ui editdrumset.ui editstaff.ui line.ui linesegment.ui
      voltaproperties.ui timesigproperties.ui
      jumpproperties.ui markerproperties.ui boxproperties.ui
      instrwizard.ui timesigwizard.ui newwizard.ui aboutbox.ui aboutmusicxmlbox.ui
      chordedit.ui transposedialog.ui tempoproperties.ui
      lineproperties.ui excerptsdialog.ui lyrics.ui stafftext.ui
      tupletdialog.ui tupletproperties.ui
      glissandoprop.ui articulation.ui metaedit.ui palette.ui
      dynamicproperties.ui textline.ui slurproperties.ui debugger.ui
      select.ui synthcontrol.ui splitstaff.ui cellproperties.ui beam.ui keyedit.ui
      harmonyedit.ui selectinstr.ui tremolo.ui fretdprops.ui stafftype.ui
      sectionbreak.ui hairpinproperties.ui spanner.ui bend.ui tremolobar.ui
      textproperties.ui editpitch.ui editstringdata.ui slursegment.ui
      accidental.ui clef.ui editraster.ui mediadialog.ui textstyle.ui
      album.ui layer.ui articulationbase.ui inspector_element.ui inspector_vbox.ui
      inspector_articulation.ui inspector_spacer.ui inspector_hbox.ui keysig.ui
      inspector_segment.ui inspector_beam.ui inspector_note.ui
      rest.ui inspector_chord.ui omrpanel.ui startdialog.ui masterpalette.ui
      inspector_group_element.ui inspector_image.ui stem.ui
      )

if (OMR)
      set(OMR_MOCS omrpanel.h)
endif (OMR)

QT4_WRAP_CPP (mocs
      scoreview.h editinstrument.h editstyle.h edittempo.h instrdialog.h debugger.h
      musescore.h navigator.h pagesettings.h palette.h mixer.h playpanel.h
      measureproperties.h seq.h textpalette.h textstyle.h
      timedialog.h symboldialog.h shortcutcapturedialog.h simplebutton.h
      greendotbutton.h recordbutton.h editdrumset.h editstaff.h selinstrument.h
      voltaproperties.h timesigproperties.h
      jumpproperties.h markerproperties.h boxproperties.h newwizard.h
      transposedialog.h chordedit.h tempoproperties.h lineproperties.h
      excerptsdialog.h stafftextproperties.h glissandoproperties.h
      script.h articulationprop.h metaedit.h magbox.h voiceselector.h
      textprop.h dynamicprop.h textproperties.h tupletdialog.h tupletproperties.h
      slurproperties.h selectdialog.h synthcontrol.h drumroll.h pianoroll.h
      piano.h ruler.h file.h
      pianoview.h drumview.h splitstaff.h scoretab.h keyedit.h keycanvas.h harmonyedit.h
      harmonycanvas.h updatechecker.h fretproperties.h fretcanvas.h editstafftype.h
      prefsdialog.h sectionbreakprop.h hairpinprop.h bendproperties.h
      bendcanvas.h tremolobarprop.h tremolobarcanvas.h
      drumtools.h texteditor.h texttools.h edittools.h pianotools.h editpitch.h
      editstringdata.h editraster.h mediadialog.h chordeditor.h chordview.h album.h layer.h
      webpage.h inspector.h inspectorBase.h inspectorBeam.h masterpalette.h
      inspectorGroupElement.h inspectorImage.h waveview.h
      ${OMR_MOCS}
      )

if (APPLE)
      QT4_ADD_RESOURCES (qrc_files musescore-apple.qrc)
else (APPLE)
      QT4_ADD_RESOURCES (qrc_files musescore.qrc)
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
	set (MACOSX_BUNDLE_INFO_STRING MuseScore prerelease for Mac OSX)
	set (MACOSX_BUNDLE_ICON_FILE      mscore.icns)
	set (MACOSX_BUNDLE_GUI_IDENTIFIER net.sourceforge.mscore)
	set (MACOSX_BUNDLE_LONG_VERSION_STRING ${Mscore_VERSION_FULL})
	set (MACOSX_BUNDLE_BUNDLE_NAME MuseScore)
	set (MACOSX_BUNDLE_SHORT_VERSION_STRING ${Mscore_VERSION})
	set (MACOSX_BUNDLE_BUNDLE_VERSION ${Mscore_VERSION_FULL})
	set (MACOSX_BUNDLE_COPYRIGHT musescore.org)
else (APPLE)
	set (ExecutableName mscore)
endif (APPLE)

if (OMR)
      set(OMR_FILES omrpanel.cpp)
endif (OMR)

add_executable ( ${ExecutableName}
      ${qrc_files}
      ${ui_headers}
      ${mocs}
      ${PROJECT_BINARY_DIR}/all.h
      ${PCH}
      ${resource_file}

      actions.cpp scoreview.cpp editinstrument.cpp editstyle.cpp
      edittempo.cpp exportxml.cpp icons.cpp importbww.cpp importxml.cpp
      instrdialog.cpp debugger.cpp menus.cpp importmidi.cpp
      musescore.cpp navigator.cpp pagesettings.cpp palette.cpp
      mixer.cpp playpanel.cpp preferences.cpp measureproperties.cpp
      seq.cpp boxproperties.cpp textpalette.cpp
      timedialog.cpp symboldialog.cpp shortcutcapturedialog.cpp
      simplebutton.cpp musedata.cpp exportly.cpp
      editdrumset.cpp editstaff.cpp voltaproperties.cpp
      timesigproperties.cpp newwizard.cpp transposedialog.cpp
      chordedit.cpp plugins.cpp excerptsdialog.cpp
      metaedit.cpp magbox.cpp voiceselector.cpp capella.cpp
      scscore.cpp sccursor.cpp scchord.cpp scnote.cpp scpart.cpp sctext.cpp
      scmeasure.cpp scpageformat.cpp exportaudio.cpp exportmidi.cpp
      textproperties.cpp screst.cpp scharmony.cpp slurproperties.cpp
      synthcontrol.cpp drumroll.cpp pianoroll.cpp piano.cpp
      pianoview.cpp drumview.cpp scoretab.cpp keyedit.cpp harmonyedit.cpp
      updatechecker.cpp importove.cpp ove.cpp ruler.cpp
      importgtp.cpp fotomode.cpp drumtools.cpp
      selinstrument.cpp texteditor.cpp editstafftype.cpp texttools.cpp edittools.cpp
      editpitch.cpp editstringdata.cpp editraster.cpp pianotools.cpp mediadialog.cpp
      profile.cpp exportmp3.cpp chordeditor.cpp chordview.cpp
      album.cpp webpage.cpp textstyle.cpp tempoproperties.cpp
      lineproperties.cpp stafftextproperties.cpp splitstaff.cpp
      tupletdialog.cpp tupletproperties.cpp glissandoproperties.cpp
      articulationprop.cpp textprop.cpp dynamicprop.cpp
      fretproperties.cpp sectionbreakprop.cpp hairpinprop.cpp
      bendproperties.cpp tremolobarprop.cpp file.cpp keyb.cpp osc.cpp
      layer.cpp jumpproperties.cpp selectdialog.cpp
      propertymenu.cpp shortcut.cpp bb.cpp
      midifile.cpp inspector.cpp dragelement.cpp svggenerator.cpp
      inspectorBase.cpp inspectorBeam.cpp masterpalette.cpp
      inspectorGroupElement.cpp dragdrop.cpp inspectorImage.cpp
      waveview.cpp musicxmlsupport.cpp
      ${OMR_FILES}
      ${AUDIO}
      )

if (STATIC_SCRIPT_BINDINGS AND BUILD_SCRIPTGEN)
      target_link_libraries(mscore
            qtscript_core
            qtscript_gui
            qtscript_network
            qtscript_uitools
            qtscript_xml
            qtscript_webkit)
endif (STATIC_SCRIPT_BINDINGS AND BUILD_SCRIPTGEN)

if (APPLE)
      set_target_properties (mscore
         PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/build/MacOSXBundleInfo.plist.in)
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
           COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h -g -Wall -Wextra -Winvalid-pch ${QT_DEFINITIONS} -DQT_SVG_LIB -DQT_GUI_LIB -DQT_XML_LIB -DQT_CORE_LIB -DQT_SCRIPT_LIB"
           LINK_FLAGS "${PROJECT_BINARY_DIR}/resfile.o -mwindows -mconsole -L ${CROSSQT}/lib"
        )
   else(CMAKE_BUILD_TYPE MATCHES "DEBUG")
     set_target_properties( mscore
          PROPERTIES
             COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h -Wall -Wextra -Winvalid-pch ${QT_DEFINITIONS} -DQT_SVG_LIB -DQT_GUI_LIB -DQT_XML_LIB -DQT_CORE_LIB -DQT_SCRIPT_LIB"
             LINK_FLAGS "-Wl,-S ${PROJECT_BINARY_DIR}/resfile.o -mwindows -L ${CROSSQT}/lib"
          )
   endif(CMAKE_BUILD_TYPE MATCHES "DEBUG")

   target_link_libraries(mscore
      libmscore
      awl
      mstyle
      diff_match_patch
      msynth
      bww
      zarchive
      rtf2html
      ${QTSINGLEAPPLICATION_LIBRARIES}
      fluid
      portaudio
      portmidi
      winmm
      vorbisfile
      )

   if (HAS_AUDIOFILE)
      target_link_libraries(mscore libsndfile-1)
   endif (HAS_AUDIOFILE)

   if (OGGVORBIS)
      target_link_libraries(mscore vorbis ogg)
   endif (OGGVORBIS)

   if (AEOLUS)
      target_link_libraries(mscore aeolus)
   endif (AEOLUS)

	if (OMR)
      target_link_libraries(mscore omr fitz openjpeg jbig2dec jpeg)
      if (OCR)
            target_link_libraries(mscore tesseract_api)
      endif (OCR)
   endif (OMR)

   target_link_libraries(mscore ${QT_mingw_LIBRARIES})

   install( TARGETS mscore RUNTIME DESTINATION bin )

   install_files ( /bin .dll
      ${CROSS}/bin/mingwm10.dll
      ${CROSS}/bin/libgcc_s_dw2-1.dll
      ${CROSS}/lib/portaudio.dll
      ${CROSS}/lib/libsndfile-1.dll
      ${CROSS}/lib/libogg.dll
      ${CROSS}/lib/libvorbis.dll
      ${CROSS}/lib/libvorbisfile.dll
      ${CROSSQT}/bin/QtCore4.dll
      ${CROSSQT}/bin/QtGui4.dll
      ${CROSSQT}/bin/QtXml4.dll
      ${CROSSQT}/bin/QtSvg4.dll
      ${CROSSQT}/bin/QtScript4.dll
      ${CROSSQT}/bin/QtScriptTools4.dll
      ${CROSSQT}/bin/QtNetwork4.dll
      ${CROSSQT}/bin/QtWebKit4.dll
      ${CROSSQT}/bin/phonon4.dll
      ${CROSSQT}/bin/QtXmlPatterns4.dll
      )
   install_files ( /bin/iconengines .dll
      ${CROSSQT}/plugins/iconengines/qsvgicon4.dll
      )
   install_files ( /bin/imageformats .dll
      ${CROSSQT}/plugins/imageformats/qjpeg4.dll
      ${CROSSQT}/plugins/imageformats/qmng4.dll
      ${CROSSQT}/plugins/imageformats/qsvg4.dll
      ${CROSSQT}/plugins/imageformats/qtiff4.dll
      )

else (MINGW)
   target_link_libraries(mscore
      ${ALSA_LIB}
      ${QT_LIBRARIES}
      ${QT_QTSCRIPT_LIBRARY_RELEASE}
      ${QT_QTSCRIPT_TOOLS_LIBRARY_RELEASE}
      diff_match_patch
      libmscore
      awl
      mstyle
      msynth
      bww
      zarchive
      rtf2html
      ${QTSINGLEAPPLICATION_LIBRARIES}
      fluid
      z
      dl
      pthread
      vorbisfile
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
         COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h -g -Wall -Wno-overloaded-virtual -Winvalid-pch"
      )
   if (HAS_AUDIOFILE)
       target_link_libraries(mscore sndfile)
   endif (HAS_AUDIOFILE)

   if (OGGVORBIS)
      target_link_libraries(mscore vorbis ogg)
   endif (OGGVORBIS)

   if (AEOLUS)
      target_link_libraries(mscore aeolus)
   endif (AEOLUS)

   if (OMR)
      target_link_libraries(mscore omr fitz jbig2dec jpeg openjpeg fontconfig freetype)
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
   if (NOT MINGW AND NOT APPLE)
      if(USE_JACK)
         target_link_libraries(mscore dl)
      endif(USE_JACK)
      target_link_libraries(mscore rt)
   endif (NOT MINGW AND NOT APPLE)

   if (QT45)
     target_link_libraries(mscore ${QT_QTSCRIPT_TOOLS_LIBRARY_RELEASE} )
   endif (QT45)

   if (APPLE)
     install (TARGETS mscore BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX})
     install (FILES data/mscore.icns DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME})
     install (FILES data/musescoreDocument.icns DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME})
   else (APPLE)
     install( TARGETS mscore RUNTIME DESTINATION bin )
     install( FILES   data/mscore.png DESTINATION share/pixmaps)
   endif (APPLE)
endif (MINGW)

#======================================
#     install icons if not embedded
#======================================

# if (NOT EMBED_ICONS)
   install (FILES
      data/greendot.svg
      data/darkgreendot.svg
      data/recordOn.svg
      data/recordOff.svg
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}/icons
      )
   install (FILES
      data/icons/text_top.svg
      data/icons/text_bottom.svg
      data/icons/text_vcenter.svg
      data/icons/minus.svg
      data/icons/flip.svg
      data/icons/undo.svg
      data/icons/redo.svg
      data/icons/midiin.svg
      data/icons/speaker.svg
      data/icons/start.svg
      data/icons/play.svg
      data/icons/sbeam.svg
      data/icons/mbeam.svg
      data/icons/nbeam.svg
      data/icons/beam32.svg
      data/icons/beam64.svg
      data/icons/abeam.svg
      data/icons/fbeam1.svg
      data/icons/fbeam2.svg
      data/icons/fileopen.svg
      data/icons/filenew.svg
      data/icons/filesave.svg
      data/icons/filesaveas.svg
      data/icons/exit.svg
      data/icons/viewmag.svg
      data/icons/repeat.svg
      data/icons/pan.svg
      data/icons/noteentry.svg
      data/icons/grace4.svg
      data/icons/grace16.svg
      data/icons/grace32.svg
      data/icons/keyboard.svg
      data/icons/tie.svg
      data/icons/acciaccatura.svg
      data/icons/appoggiatura.svg
      data/icons/brevis.svg
      data/icons/clef.svg
      data/icons/dfermata.svg
      data/icons/dlongfermata.svg
      data/icons/dmarcato.svg
      data/icons/dotdot.svg
      data/icons/dot.svg
      data/icons/downbow.svg
      data/icons/downmordent.svg
      data/icons/downprall.svg
      data/icons/dportato.svg
      data/icons/dshortfermata.svg
      data/icons/dstaccatissimo.svg
      data/icons/dverylongfermata.svg
      data/icons/espressivo.svg
      data/icons/flatflat.svg
      data/icons/flat.svg
      data/icons/longaUp.svg
      data/icons/mordent.svg
      data/icons/natural.svg
      data/icons/note16.svg
      data/icons/note2.svg
      data/icons/note32.svg
      data/icons/note4.svg
      data/icons/note64.svg
      data/icons/note8.svg
      data/icons/note.svg
      data/icons/ouvert.svg
      data/icons/plus.svg
      data/icons/plusstop.svg
      data/icons/popping.svg
      data/icons/prallmordent.svg
      data/icons/prall.svg
      data/icons/prallprall.svg
      data/icons/quartrest.svg
      data/icons/reverseturn.svg
      data/icons/sforzatoaccent.svg
      data/icons/sforzato.svg
      data/icons/sharp.svg
      data/icons/sharpsharp.svg
      data/icons/slapping.svg
      data/icons/snappizzicato.svg
      data/icons/staccato.svg
      data/icons/tapping.svg
      data/icons/tenuto.svg
      data/icons/thumb.svg
      data/icons/trill.svg
      data/icons/turn.svg
      data/icons/ufermata.svg
      data/icons/ulongfermata.svg
      data/icons/umarcato.svg
      data/icons/upbow.svg
      data/icons/upmordent.svg
      data/icons/uportato.svg
      data/icons/upprall.svg
      data/icons/ushortfermata.svg
      data/icons/ustaccatissimo.svg
      data/icons/uverylongfermata.svg
      data/icons/editcut.svg
      data/icons/editcopy.svg
      data/icons/editpaste.svg
      data/icons/fileprint.svg
      data/icons/text_bold.svg
      data/icons/text_italic.svg
      data/icons/text_under.svg
      data/icons/text_left.svg
      data/icons/text_center.svg
      data/icons/text_right.svg
      data/icons/subscript.svg
      data/icons/superscript.svg
      data/icons/fotomode.svg
      data/icons/hraster.svg
      data/icons/vraster.svg
      data/icons/repitch.svg
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}/icons/icons
      )
   install (FILES
      data/icons-dark/text_top.svg
      data/icons-dark/text_bottom.svg
      data/icons-dark/text_vcenter.svg
      data/icons-dark/minus.svg
      data/icons-dark/flip.svg
      data/icons-dark/undo.svg
      data/icons-dark/redo.svg
      data/icons-dark/midiin.svg
      data/icons-dark/speaker.svg
      data/icons-dark/start.svg
      data/icons-dark/play.svg
      data/icons-dark/sbeam.svg
      data/icons-dark/mbeam.svg
      data/icons-dark/nbeam.svg
      data/icons-dark/beam32.svg
      data/icons-dark/beam64.svg
      data/icons-dark/abeam.svg
      data/icons-dark/fbeam1.svg
      data/icons-dark/fbeam2.svg
      data/icons-dark/fileopen.svg
      data/icons-dark/filenew.svg
      data/icons-dark/filesave.svg
      data/icons-dark/filesaveas.svg
      data/icons-dark/exit.svg
      data/icons-dark/viewmag.svg
      data/icons-dark/repeat.svg
      data/icons-dark/pan.svg
      data/icons-dark/noteentry.svg
      data/icons-dark/grace4.svg
      data/icons-dark/grace16.svg
      data/icons-dark/grace32.svg
      data/icons-dark/keyboard.svg
      data/icons-dark/tie.svg
      data/icons-dark/acciaccatura.svg
      data/icons-dark/appoggiatura.svg
      data/icons-dark/brevis.svg
      data/icons-dark/clef.svg
      data/icons-dark/dfermata.svg
      data/icons-dark/dlongfermata.svg
      data/icons-dark/dmarcato.svg
      data/icons-dark/dotdot.svg
      data/icons-dark/dot.svg
      data/icons-dark/downbow.svg
      data/icons-dark/downmordent.svg
      data/icons-dark/downprall.svg
      data/icons-dark/dportato.svg
      data/icons-dark/dshortfermata.svg
      data/icons-dark/dstaccatissimo.svg
      data/icons-dark/dverylongfermata.svg
      data/icons-dark/espressivo.svg
      data/icons-dark/flatflat.svg
      data/icons-dark/flat.svg
      data/icons-dark/longaUp.svg
      data/icons-dark/mordent.svg
      data/icons-dark/natural.svg
      data/icons-dark/note16.svg
      data/icons-dark/note2.svg
      data/icons-dark/note32.svg
      data/icons-dark/note4.svg
      data/icons-dark/note64.svg
      data/icons-dark/note8.svg
      data/icons-dark/note.svg
      data/icons-dark/ouvert.svg
      data/icons-dark/plus.svg
      data/icons-dark/plusstop.svg
      data/icons-dark/popping.svg
      data/icons-dark/prallmordent.svg
      data/icons-dark/prall.svg
      data/icons-dark/prallprall.svg
      data/icons-dark/quartrest.svg
      data/icons-dark/reverseturn.svg
      data/icons-dark/sforzatoaccent.svg
      data/icons-dark/sforzato.svg
      data/icons-dark/sharp.svg
      data/icons-dark/sharpsharp.svg
      data/icons-dark/slapping.svg
      data/icons-dark/snappizzicato.svg
      data/icons-dark/staccato.svg
      data/icons-dark/tapping.svg
      data/icons-dark/tenuto.svg
      data/icons-dark/thumb.svg
      data/icons-dark/trill.svg
      data/icons-dark/turn.svg
      data/icons-dark/ufermata.svg
      data/icons-dark/ulongfermata.svg
      data/icons-dark/umarcato.svg
      data/icons-dark/upbow.svg
      data/icons-dark/upmordent.svg
      data/icons-dark/uportato.svg
      data/icons-dark/upprall.svg
      data/icons-dark/ushortfermata.svg
      data/icons-dark/ustaccatissimo.svg
      data/icons-dark/uverylongfermata.svg
      data/icons-dark/editcut.svg
      data/icons-dark/editcopy.svg
      data/icons-dark/editpaste.svg
      data/icons-dark/fileprint.svg
      data/icons-dark/text_bold.svg
      data/icons-dark/text_italic.svg
      data/icons-dark/text_under.svg
      data/icons-dark/text_left.svg
      data/icons-dark/text_center.svg
      data/icons-dark/text_right.svg
      data/icons-dark/subscript.svg
      data/icons-dark/superscript.svg
      data/icons-dark/fotomode.svg
      data/icons-dark/hraster.svg
      data/icons-dark/vraster.svg
      data/icons-dark/repitch.svg
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}/icons/icons-dark
      )
# endif (NOT EMBED_ICONS)

if (APPLE)
     install (FILES
      ../fonts/gonville-20.otf
      ../fonts/mscore-20.otf
      ../fonts/mscore1-20.ttf
      ../fonts/MuseJazz.ttf
      ../fonts/FreeSerifMscore.ttf
      ../fonts/FreeSans.ttf
      ../fonts/mscore_tab_baroque.ttf
      ../fonts/mscore_tab_modern.ttf
      ../fonts/mscore_tab_renaiss.ttf
      ../fonts/mscore_tab_renaiss2.ttf
      ../fonts/mscore-BC.ttf
     DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}fonts
     )
endif (APPLE)

ADD_DEPENDENCIES(${ExecutableName} mops1)
ADD_DEPENDENCIES(${ExecutableName} mops2)

