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
      ${QTSINGLEAPPLICATION_INCLUDE_DIRS}
      )

if (SCRIPT_INTERFACE)
      set (SCRIPT_FILES plugins.cpp pluginCreator.cpp qmledit.cpp)
      set (SCRIPT_MOCS plugins.h pluginCreator.h qmledit.h)
      set (SCRIPT_UI   pluginCreator.ui)
endif (SCRIPT_INTERFACE)

QT4_WRAP_UI (ui_headers
      insertmeasuresdialog.ui barline.ui chord.ui chordrest.ui editinstrument.ui editstyle.ui
      edittempo.ui element.ui hairpin.ui instrdialog.ui measure.ui measuresdialog.ui
      note.ui page.ui pagesettings.ui mixer.ui playpanel.ui prefsdialog.ui
      measureproperties.ui segment.ui text.ui textpalette.ui timedialog.ui
      symboldialog.ui dynamic.ui tuplet.ui shortcutcapturedialog.ui slurtie.ui
      slur.ui editdrumset.ui editstaff.ui line.ui linesegment.ui
      voltaproperties.ui timesigproperties.ui boxproperties.ui
      instrwizard.ui timesigwizard.ui newwizard.ui aboutbox.ui aboutmusicxmlbox.ui
      chordedit.ui transposedialog.ui tempoproperties.ui
      lineproperties.ui excerptsdialog.ui lyrics.ui stafftext.ui tupletdialog.ui
      glissandoprop.ui articulation.ui metaedit.ui palette.ui
      dynamicproperties.ui textline.ui slurproperties.ui debugger.ui
      select.ui synthcontrol.ui splitstaff.ui cellproperties.ui beam.ui keyedit.ui
      harmonyedit.ui selectinstr.ui tremolo.ui fretdprops.ui stafftype.ui
      sectionbreak.ui hairpinproperties.ui spanner.ui bend.ui tremolobar.ui
      textproperties.ui editpitch.ui editstringdata.ui slursegment.ui
      accidental.ui clef.ui editraster.ui mediadialog.ui textstyle.ui
      album.ui layer.ui articulationbase.ui inspector_element.ui inspector_vbox.ui
      inspector_articulation.ui inspector_spacer.ui inspector_hbox.ui keysig.ui
      inspector_segment.ui inspector_beam.ui inspector_note.ui inspector_rest.ui
      rest.ui inspector_chord.ui omrpanel.ui startdialog.ui masterpalette.ui
      inspector_group_element.ui inspector_image.ui stem.ui inspector_lasso.ui
      inspector_volta.ui inspector_ottava.ui inspector_trill.ui inspector_hairpin.ui
      box.ui pluginManager.ui inspector_jump.ui inspector_marker.ui inspector_glissando.ui
      inspector_clef.ui inspector_timesig.ui inspector_keysig.ui inspector_barline.ui
      inspector_tuplet.ui inspector_accidental.ui
      ${SCRIPT_UI}
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
      voltaproperties.h timesigproperties.h boxproperties.h newwizard.h
      transposedialog.h chordedit.h tempoproperties.h lineproperties.h
      excerptsdialog.h stafftextproperties.h glissandoproperties.h
      articulationprop.h metaedit.h magbox.h voiceselector.h
      textprop.h dynamicprop.h textproperties.h tupletdialog.h
      slurproperties.h selectdialog.h synthcontrol.h drumroll.h pianoroll.h
      piano.h ruler.h file.h
      pianoview.h drumview.h splitstaff.h scoretab.h keyedit.h keycanvas.h harmonyedit.h
      harmonycanvas.h updatechecker.h fretproperties.h fretcanvas.h editstafftype.h
      prefsdialog.h sectionbreakprop.h hairpinprop.h bendproperties.h
      bendcanvas.h tremolobarprop.h tremolobarcanvas.h
      drumtools.h texteditor.h texttools.h edittools.h pianotools.h editpitch.h
      editstringdata.h editraster.h mediadialog.h chordview.h album.h layer.h
      webpage.h inspector.h inspectorBase.h inspectorBeam.h masterpalette.h
      inspectorGroupElement.h inspectorImage.h waveview.h helpBrowser.h
      inspectorLasso.h inspectorVolta.h inspectorOttava.h inspectorTrill.h
      inspectorHairpin.h qmlplugin.h palettebox.h workspace.h pluginManager.h
      inspectorJump.h inspectorMarker.h inspectorGlissando.h inspectorNote.h
      ${OMR_MOCS}
      ${SCRIPT_MOCS}
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
      ${qrc_files}
      ${ui_headers}
      ${mocs}
      ${PROJECT_BINARY_DIR}/all.h
      ${PCH}
      ${resource_file}
      ${INCS}

      actions.cpp scoreview.cpp editinstrument.cpp editstyle.cpp
      edittempo.cpp icons.cpp importbww.cpp
      instrdialog.cpp debugger.cpp menus.cpp importmidi.cpp
      musescore.cpp navigator.cpp pagesettings.cpp palette.cpp
      mixer.cpp playpanel.cpp preferences.cpp measureproperties.cpp
      seq.cpp boxproperties.cpp textpalette.cpp
      timedialog.cpp symboldialog.cpp shortcutcapturedialog.cpp
      simplebutton.cpp musedata.cpp exportly.cpp
      editdrumset.cpp editstaff.cpp voltaproperties.cpp
      timesigproperties.cpp newwizard.cpp transposedialog.cpp
      chordedit.cpp excerptsdialog.cpp metaedit.cpp magbox.cpp
      voiceselector.cpp capella.cpp exportaudio.cpp palettebox.cpp
      textproperties.cpp slurproperties.cpp
      synthcontrol.cpp drumroll.cpp pianoroll.cpp piano.cpp
      pianoview.cpp drumview.cpp scoretab.cpp keyedit.cpp harmonyedit.cpp
      updatechecker.cpp importove.cpp ove.cpp ruler.cpp
      importgtp.cpp fotomode.cpp drumtools.cpp
      selinstrument.cpp texteditor.cpp editstafftype.cpp texttools.cpp edittools.cpp
      editpitch.cpp editstringdata.cpp editraster.cpp pianotools.cpp mediadialog.cpp
      workspace.cpp exportmp3.cpp chordview.cpp
      album.cpp webpage.cpp textstyle.cpp tempoproperties.cpp
      lineproperties.cpp stafftextproperties.cpp splitstaff.cpp
      tupletdialog.cpp glissandoproperties.cpp
      articulationprop.cpp textprop.cpp dynamicprop.cpp
      fretproperties.cpp sectionbreakprop.cpp hairpinprop.cpp
      bendproperties.cpp tremolobarprop.cpp file.cpp keyb.cpp osc.cpp
      layer.cpp selectdialog.cpp propertymenu.cpp shortcut.cpp bb.cpp
      inspector.cpp dragelement.cpp svggenerator.cpp
      inspectorBase.cpp inspectorBeam.cpp masterpalette.cpp
      inspectorGroupElement.cpp dragdrop.cpp inspectorImage.cpp
      waveview.cpp helpBrowser.cpp inspectorLasso.cpp
      editelement.cpp inspectorVolta.cpp inspectorOttava.cpp inspectorTrill.cpp
      inspectorHairpin.cpp qmlplugin.cpp editlyrics.cpp
      musicxmlsupport.cpp exportxml.cpp importxml.cpp importxmlfirstpass.cpp
      savePositions.cpp pluginManager.cpp inspectorJump.cpp inspectorMarker.cpp
      inspectorGlissando.cpp inspectorNote.cpp
      ${OMR_FILES}
      ${AUDIO}
      ${SCRIPT_FILES}
      musescoreCore.h
      driver.h
      )

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
      libmscore
      awl
      mstyle
      diff_match_patch
      msynth
      bww
      rtf2html
      ${QTSINGLEAPPLICATION_LIBRARIES}
      fluid
      portaudio
      portmidi
      winmm
      vorbisfile
      )

   if (HAS_AUDIOFILE)
      target_link_libraries(mscore sndfile-1)
   endif (HAS_AUDIOFILE)

   if (SOUNDFONT3)
      target_link_libraries(mscore vorbis ogg)
   endif (SOUNDFONT3)

   if (AEOLUS)
      target_link_libraries(mscore aeolus)
   endif (AEOLUS)

   if (OMR)
      target_link_libraries(mscore omr fitz freetype jbig2dec jpeg openjpeg)
      if (OCR)
            target_link_libraries(mscore tesseract_api)
      endif (OCR)
   endif (OMR)

   target_link_libraries(mscore ${QT_mingw_LIBRARIES})

   install( TARGETS mscore RUNTIME DESTINATION bin )

   install_files ( /bin .dll
      ${CROSS}/bin/libgcc_s_sjlj-1.dll
      ${CROSS}/bin/libstdc++-6.dll
      ${CROSS}/bin/libwinpthread-1.dll
      ${CROSS}/lib/portaudio.dll
      ${CROSS}/lib/libsndfile-1.dll
      ${CROSS}/lib/libogg.dll
      ${CROSS}/lib/libvorbis.dll
      ${CROSS}/lib/libvorbisfile.dll
      ${CROSSQT}/bin/QtCore4.dll
      ${CROSSQT}/bin/QtGui4.dll
      ${CROSSQT}/bin/QtXml4.dll
      ${CROSSQT}/bin/QtSvg4.dll
      ${CROSSQT}/bin/QtNetwork4.dll
      ${CROSSQT}/bin/QtWebKit4.dll
      ${CROSSQT}/bin/phonon4.dll
      ${CROSSQT}/bin/QtXmlPatterns4.dll
      ${CROSSQT}/bin/QtDeclarative4.dll
      ${CROSSQT}/bin/QtScript4.dll
      ${CROSSQT}/bin/QtSql4.dll
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
      libmscore
      ${ALSA_LIB}
      ${QT_LIBRARIES}
      diff_match_patch
      awl
      mstyle
      msynth
      bww
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
         COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wno-overloaded-virtual -Winvalid-pch"
         LINK_FLAGS "-L${QT_LIBRARY_DIR}"
      )
   if (HAS_AUDIOFILE)
       target_link_libraries(mscore sndfile)
   endif (HAS_AUDIOFILE)

   if (SOUNDFONT3)
      target_link_libraries(mscore vorbis ogg)
   endif (SOUNDFONT3)

   if (AEOLUS)
      target_link_libraries(mscore aeolus)
   endif (AEOLUS)

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
      ../fonts/gonville-20.otf
      ../fonts/mscore-20.otf
      ../fonts/MuseJazz.ttf
      ../fonts/FreeSerifMscore.ttf
      ../fonts/FreeSans.ttf
      ../fonts/mscoreTab.ttf
      ../fonts/mscore-BC.ttf
     DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}fonts
     )
endif (APPLE)

ADD_DEPENDENCIES(${ExecutableName} mops1)
ADD_DEPENDENCIES(${ExecutableName} mops2)

