# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2025 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

muse_create_qml_module(notationscene_qml FOR notationscene)

if (MUE_BUILD_BRAILLE_QML)
    set(MAYBE_BRAILLE_QML
        TARGET braille_qml
    )
else()
    set(MAYBE_BRAILLE_QML)
endif()

qt_add_qml_module(notationscene_qml
    URI MuseScore.NotationScene
    VERSION 1.0
    SOURCES
        abstractelementpopupmodel.cpp
        abstractelementpopupmodel.h
        abstractnotationpaintview.cpp
        abstractnotationpaintview.h
        continuouspanel.cpp
        continuouspanel.h
        editgridsizedialogmodel.cpp
        editgridsizedialogmodel.h
        editpercussionshortcutmodel.cpp
        editpercussionshortcutmodel.h
        elementpopups/caposettingsmodel.cpp
        elementpopups/caposettingsmodel.h
        elementpopups/dynamicpopupmodel.cpp
        elementpopups/dynamicpopupmodel.h
        elementpopups/harppedalpopupmodel.cpp
        elementpopups/harppedalpopupmodel.h
        elementpopups/partialtiepopupmodel.cpp
        elementpopups/partialtiepopupmodel.h
        elementpopups/percussionnotepopupcontentmodel.cpp
        elementpopups/percussionnotepopupcontentmodel.h
        elementpopups/shadownotepopupmodel.cpp
        elementpopups/shadownotepopupmodel.h
        elementpopups/staffvisibilitypopupmodel.cpp
        elementpopups/staffvisibilitypopupmodel.h
        elementpopups/stringtuningssettingsmodel.cpp
        elementpopups/stringtuningssettingsmodel.h
        internal/undohistorymodel.cpp
        internal/undohistorymodel.h
        internal/undoredotoolbarmodel.cpp
        internal/undoredotoolbarmodel.h
        loopmarker.cpp
        loopmarker.h
        notationcontextmenumodel.cpp
        notationcontextmenumodel.h
        notationnavigator.cpp
        notationnavigator.h
        notationpaintview.cpp
        notationpaintview.h
        notationruler.cpp
        notationruler.h
        notationswitchlistmodel.cpp
        notationswitchlistmodel.h
        notationtoolbarmodel.cpp
        notationtoolbarmodel.h
        notationviewinputcontroller.cpp
        notationviewinputcontroller.h
        noteinputbarcustomiseitem.cpp
        noteinputbarcustomiseitem.h
        noteinputbarcustomisemodel.cpp
        noteinputbarcustomisemodel.h
        noteinputbarmodel.cpp
        noteinputbarmodel.h
        noteinputcursor.cpp
        noteinputcursor.h
        paintedengravingitem.cpp
        paintedengravingitem.h
        partlistmodel.cpp
        partlistmodel.h
        percussionpanel/percussionpanelmodel.cpp
        percussionpanel/percussionpanelmodel.h
        percussionpanel/percussionpanelpadlistmodel.cpp
        percussionpanel/percussionpanelpadlistmodel.h
        percussionpanel/percussionpanelpadmodel.cpp
        percussionpanel/percussionpanelpadmodel.h
        pianokeyboard/pianokeyboardcontroller.cpp
        pianokeyboard/pianokeyboardcontroller.h
        pianokeyboard/pianokeyboardpanelcontextmenumodel.cpp
        pianokeyboard/pianokeyboardpanelcontextmenumodel.h
        pianokeyboard/pianokeyboardtypes.h
        pianokeyboard/pianokeyboardview.cpp
        pianokeyboard/pianokeyboardview.h
        playbackcursor.cpp
        playbackcursor.h
        searchpopupmodel.cpp
        searchpopupmodel.h
        selectionfilter/abstractselectionfiltermodel.cpp
        selectionfilter/abstractselectionfiltermodel.h
        selectionfilter/elementsselectionfiltermodel.cpp
        selectionfilter/elementsselectionfiltermodel.h
        selectionfilter/notesinchordselectionfiltermodel.cpp
        selectionfilter/notesinchordselectionfiltermodel.h
        selectionfilter/voicesselectionfiltermodel.cpp
        selectionfilter/voicesselectionfiltermodel.h
        styledialog/abstractstyledialogmodel.cpp
        styledialog/abstractstyledialogmodel.h
        styledialog/accidentalspagemodel.cpp
        styledialog/accidentalspagemodel.h
        styledialog/beamspagemodel.cpp
        styledialog/beamspagemodel.h
        styledialog/bendspagemodel.cpp
        styledialog/bendspagemodel.h
        styledialog/chordsymbolspagemodel.cpp
        styledialog/chordsymbolspagemodel.h
        styledialog/clefkeytimesigpagemodel.cpp
        styledialog/clefkeytimesigpagemodel.h
        styledialog/fretboardspagemodel.cpp
        styledialog/fretboardspagemodel.h
        styledialog/glissandosectionmodel.cpp
        styledialog/glissandosectionmodel.h
        styledialog/hammeronpullofftappingpagemodel.cpp
        styledialog/hammeronpullofftappingpagemodel.h
        styledialog/measurenumberspagemodel.cpp
        styledialog/measurenumberspagemodel.h
        styledialog/measurerepeatmodel.cpp
        styledialog/measurerepeatmodel.h
        styledialog/notelinesectionmodel.cpp
        styledialog/notelinesectionmodel.h
        styledialog/notespagemodel.cpp
        styledialog/notespagemodel.h
        styledialog/repeatbarlinessectionmodel.cpp
        styledialog/repeatbarlinessectionmodel.h
        styledialog/repeatplaycounttextmodel.cpp
        styledialog/repeatplaycounttextmodel.h
        styledialog/restspagemodel.cpp
        styledialog/restspagemodel.h
        styledialog/slursandtiespagemodel.cpp
        styledialog/slursandtiespagemodel.h
        styledialog/styleitem.cpp
        styledialog/styleitem.h
        styledialog/tupletcenteringselectormodel.cpp
        styledialog/tupletcenteringselectormodel.h
        styledialog/voltaspagemodel.cpp
        styledialog/voltaspagemodel.h
        timelineview.cpp
        timelineview.h

    QML_FILES
        AbstractElementPopup.qml
        ConcertPitchControl.qml
        EditGridSizeDialog.qml
        EditPercussionShortcutDialog.qml
        elementpopups/CapoPopup.qml
        elementpopups/DynamicPopup.qml
        elementpopups/ElementPopupLoader.qml
        elementpopups/HarpPedalPopup.qml
        elementpopups/PartialTieMenuRowItem.qml
        elementpopups/PartialTiePopup.qml
        elementpopups/PercussionNotePopupContent.qml
        elementpopups/ShadowNotePopup.qml
        elementpopups/StaffVisibilityPopup.qml
        elementpopups/StringTuningsPopup.qml
        internal/CustomiseControlPanel.qml
        internal/CustomiseView.qml
        internal/NotationSwitchButton.qml
        internal/NotationSwitchPanel.qml
        internal/NotationSwitchScrollArrowButton.qml
        internal/NoteInputBarActionDelegate.qml
        internal/NoteInputBarCustomisePopup.qml
        internal/PartDelegate.qml
        internal/PartsTopPanel.qml
        internal/PartsView.qml
        internal/SearchPopup.qml
        internal/SelectionFilterSection.qml
        internal/VoicesPopup.qml
        NotationToolBar.qml
        NotationView.qml
        NoteInputBar.qml
        PartsDialog.qml
        PercussionPanel.qml
        percussionpanel/PercussionPanelPad.qml
        percussionpanel/PercussionPanelPadContent.qml
        percussionpanel/PercussionPanelToolBar.qml
        PercussionPanelPadSwapDialog.qml
        PianoKeyboardPanel.qml
        SelectionFilterPanel.qml
        SelectMeasuresCountDialog.qml
        styledialog/AccidentalsPage.qml
        styledialog/BeamsPage.qml
        styledialog/BendsPage.qml
        styledialog/ChordSymbolsPage.qml
        styledialog/ClefKeyTimeSigPage.qml
        styledialog/ComboBoxDropdown.qml
        styledialog/FretboardsPage.qml
        styledialog/GlissandoSection.qml
        styledialog/HammerOnPullOffTappingPage.qml
        styledialog/IconAndTextButtonSelector.qml
        styledialog/ItemWithTitle.qml
        styledialog/LaissezVibrSection.qml
        styledialog/LineStyleSection.qml
        styledialog/MeasureNumbersPage.qml
        styledialog/NoteFlagsTypeSelector.qml
        styledialog/NoteLineSection.qml
        styledialog/RadioButtonSelectorWithReset.qml
        styledialog/RepeatBarlinesSection.qml
        styledialog/RepeatPage.qml
        styledialog/RestOffsetSelector.qml
        styledialog/SlursAndTiesPage.qml
        styledialog/SlursSection.qml
        styledialog/StyleControlRowWithReset.qml
        styledialog/StyleDialogPage.qml
        styledialog/StyledImage.qml
        styledialog/StyledXYControllerWithReset.qml
        styledialog/StyleResetButton.qml
        styledialog/StyleSpinboxWithReset.qml
        styledialog/StyleToggle.qml
        styledialog/StyleToggleWithImage.qml
        styledialog/TextFieldWithReset.qml
        styledialog/TiesSection.qml
        styledialog/TupletCenteringSelector.qml
        styledialog/VoltasPage.qml
        TempoView.qml
        Timeline.qml
        UndoHistoryPanel.qml
        UndoRedoToolBar.qml
        ViewModeControl.qml
        ZoomControl.qml

    RESOURCES
        styledialog/accidentalImages/followOffset-OFF.png
        styledialog/accidentalImages/followOffset-ON.png
        styledialog/accidentalImages/octaveOffset-OFF.png
        styledialog/accidentalImages/octaveOffset-ON.png
        styledialog/accidentalImages/octaveSubchord-OFF.png
        styledialog/accidentalImages/octaveSubchord-ON.png
        styledialog/accidentalImages/secondsAdjacent-OFF.png
        styledialog/accidentalImages/secondsAdjacent-ON.png
        styledialog/beamImages/beam_style_french.svg
        styledialog/beamImages/beam_style_regular.svg
        styledialog/courtesyImages/cancel_other-off.png
        styledialog/courtesyImages/cancel_other-on-parens.png
        styledialog/courtesyImages/cancel_other-on.png
        styledialog/courtesyImages/cancel_repeat-off.png
        styledialog/courtesyImages/cancel_repeat-on-parens.png
        styledialog/courtesyImages/cancel_repeat-on.png
        styledialog/courtesyImages/clefs-after_repeat-allow_changes.png
        styledialog/courtesyImages/clefs-after_repeat-disallow_changes.png
        styledialog/courtesyImages/clefs-after_repeat.png
        styledialog/courtesyImages/clefs-before_repeat-allow_changes.png
        styledialog/courtesyImages/clefs-before_repeat-disallow_changes.png
        styledialog/courtesyImages/clefs-before_repeat.png
        styledialog/courtesyImages/other_jumps-off.png
        styledialog/courtesyImages/other_jumps-on-parens.png
        styledialog/courtesyImages/other_jumps-on.png
        styledialog/courtesyImages/repeats-off.png
        styledialog/courtesyImages/repeats-on-parens.png
        styledialog/courtesyImages/repeats-on.png
        styledialog/hammerOnPullOffImages/hopoNotShowAll.png
        styledialog/hammerOnPullOffImages/hopoShowAll.png
        styledialog/timeSigImages/timesig-above_staves.png
        styledialog/timeSigImages/timesig-across_staves.png
        styledialog/timeSigImages/timesig-courtesy-hang.png
        styledialog/timeSigImages/timesig-courtesy-inset.png
        styledialog/timeSigImages/timesig-courtesy-right_align.png
        styledialog/timeSigImages/timesig-on_all_staves.png
        styledialog/voltasImages/voltaAfterKeySig.png
        styledialog/voltasImages/voltaBeforeKeySig.png
        styledialog/voltasImages/voltaLeftOfBarline.png
        styledialog/voltasImages/voltaRightOfBarline.png

    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
        ${MAYBE_BRAILLE_QML}
)

fixup_qml_module_dependencies(notationscene_qml)

# Necessary for the auto-generated sources
target_include_directories(notationscene_qml PRIVATE
    elementpopups
    internal
    percussionpanel
    pianokeyboard
    selectionfilter
    styledialog
)

target_link_libraries(notationscene_qml PRIVATE
    muse::uicomponents_qml
)

if(MUE_BUILD_NOTATIONSCENE_TESTS)
    add_subdirectory(tests)
endif()
