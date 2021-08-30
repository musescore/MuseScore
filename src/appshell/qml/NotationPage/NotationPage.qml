/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.15
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Dock 1.0
import MuseScore.AppShell 1.0

import MuseScore.NotationScene 1.0
import MuseScore.Palette 1.0
import MuseScore.Inspector 1.0
import MuseScore.InstrumentsScene 1.0
import MuseScore.Playback 1.0

import "../dockwindow"

DockPage {
    id: root

    objectName: "Notation"
    uri: "musescore://notation"

    property DockWindow dockWindow: null

    property var topToolKeyNavSec

    property NotationPageModel pageModel: NotationPageModel {}

    property NavigationSection noteInputKeyNavSec: NavigationSection {
        id: keynavSec
        name: "NoteInputSection"
        order: 2
    }

    property NavigationSection keynavLeftPanelSec: NavigationSection {
        name: "NavigationLeftPanel"
        enabled: root.visible
        order: 3
    }

    property NavigationSection keynavRightPanelSec: NavigationSection {
        name: "NavigationRightPanel"
        enabled: root.visible
        order: 5
    }

    function navigationPanelSec(location) {
        if (location === DockBase.Right) {
            return keynavRightPanelSec
        }
        return keynavLeftPanelSec
    }

    onInited: {
        pageModel.setNotationToolBarDockName(notationToolBar.objectName)
        pageModel.setPlaybackToolBarDockName(playbackToolBar.objectName)
        pageModel.setUndoRedoToolBarDockName(undoRedoToolBar.objectName)
        pageModel.setNoteInputBarDockName(noteInputBar.objectName)
        pageModel.setPalettesPanelDockName(palettesPanel.objectName)
        pageModel.setInspectorPanelDockName(inspectorPanel.objectName)
        pageModel.setInstrumentsPanelDockName(instrumentsPanel.objectName)
        pageModel.setSelectionFilterPanelDockName(selectionFilterPanel.objectName)
        pageModel.setPianoRollDockName(pianoRollPanel.objectName)
        pageModel.setMixerDockName(mixerPanel.objectName)
        pageModel.setTimelineDockName(timelinePanel.objectName)
        pageModel.setDrumsetPanelDockName(drumsetPanel.objectName)
        pageModel.setStatusBarDockName(notationStatusBar.objectName)

        Qt.callLater(pageModel.init, root.dockWindow)
    }

    readonly property int defaultPanelWidth: 300
    readonly property int toolBarHeight: 48

    mainToolBars: [
        DockToolBar {
            id: notationToolBar

            objectName: "notationToolBar"
            title: qsTrc("appshell", "Notation toolbar")

            minimumWidth: 198

            contentComponent: NotationToolBar {
                navigation.section: root.topToolKeyNavSec
                navigation.order: 2

                onActiveFocusRequested: {
                    if (navigation.active) {
                        notationToolBar.forceActiveFocus()
                    }
                }
            }
        },

        DockToolBar {
            id: playbackToolBar

            objectName: "playbackToolBar"
            title: qsTrc("appshell", "Playback controls")

            width: root.width / 3
            minimumWidth: floating ? 526 : 452
            minimumHeight: floating ? 56 : root.toolBarHeight

            contentComponent: PlaybackToolBar {
                navigation.section: root.topToolKeyNavSec
                navigation.order: 3

                floating: playbackToolBar.floating
            }
        },

        DockToolBar {
            id: undoRedoToolBar

            objectName: "undoRedoToolBar"
            title: qsTrc("appshell", "Undo/redo toolbar")

            minimumWidth: 74
            maximumWidth: 74

            movable: false

            contentComponent: UndoRedoToolBar {
                navigation.section: root.topToolKeyNavSec
                navigation.order: 4
            }
        }
    ]

    toolBars: [
        DockToolBar {
            id: noteInputBar

            objectName: "noteInputBar"
            title: qsTrc("appshell", "Note input")

            horizontalPreferredSize: Qt.size(720, root.toolBarHeight)
            verticalPreferredSize: Qt.size(root.toolBarHeight, 400)

            allowedAreas: { Qt.AllDockWidgetAreas }

            contentComponent: NoteInputBar {
                orientation: noteInputBar.orientation

                navigation.section: root.noteInputKeyNavSec
                navigation.order: 1
            }
        }
    ]

    panels: [
        DockPanel {
            id: palettesPanel

            objectName: "palettesPanel"
            title: qsTrc("appshell", "Palettes")

            navigationSection: root.navigationPanelSec(palettesPanel.location)

            width: root.defaultPanelWidth
            minimumWidth: root.defaultPanelWidth
            maximumWidth: root.defaultPanelWidth

            tabifyPanel: instrumentsPanel

            PalettesPanel {
                navigationSection: palettesPanel.navigationSection
            }
        },

        DockPanel {
            id: instrumentsPanel

            objectName: "instrumentsPanel"
            title: qsTrc("appshell", "Instruments")

            navigationSection: root.navigationPanelSec(instrumentsPanel.location)

            width: root.defaultPanelWidth
            minimumWidth: root.defaultPanelWidth
            maximumWidth: root.defaultPanelWidth

            tabifyPanel: inspectorPanel
            contextMenuModel: instrumentsPanelContent.contextMenuModel

            InstrumentsPanel {
                navigationSection: instrumentsPanel.navigationSection

                onContextMenuModelChanged: {
                    instrumentsPanel.contextMenuModel = newModel
                }
            }
        },

        DockPanel {
            id: inspectorPanel

            objectName: "inspectorPanel"
            title: qsTrc("appshell", "Properties")

            navigationSection: root.navigationPanelSec(inspectorPanel.location)

            width: root.defaultPanelWidth
            minimumWidth: root.defaultPanelWidth
            maximumWidth: root.defaultPanelWidth
            
            tabifyPanel: selectionFilterPanel

            InspectorForm {
                navigationSection: inspectorPanel.navigationSection
            }
        },

        DockPanel {
            id: selectionFilterPanel

            objectName: "selectionFilterPanel"
            title: qsTrc("appshell", "Selection Filter")

            navigationSection: root.navigationPanelSec(selectionFilterPanel.location)

            width: root.defaultPanelWidth
            minimumWidth: root.defaultPanelWidth
            maximumWidth: root.defaultPanelWidth

            // TODO: Temporarily disabled on startup, but can be enabled via the app menu, see:
            // https://github.com/musescore/MuseScore/pull/8593
            visible: false

            SelectionFilterPanel {
                navigationSection: selectionFilterPanel.navigationSection
            }
        },
        
        // =============================================
        // Horizontal Panels
        // =============================================

        DockPanel {
            id: mixerPanel

            objectName: "mixerPanel"
            title: qsTrc("appshell", "Mixer")

            allowedAreas: Qt.TopDockWidgetArea | Qt.BottomDockWidgetArea

            height: minimumHeight
            minimumHeight: 180
            maximumHeight: 520

            tabifyPanel: pianoRollPanel

            // TODO: Temporarily disabled on startup, but can be enabled via the app menu, see:
            // https://github.com/musescore/MuseScore/pull/8593
            visible: false

            Loader {
                asynchronous: true
                sourceComponent: MixerPanel {}
            }
        },

        DockPanel {
            id: pianoRollPanel

            objectName: "pianoRollPanel"
            title: qsTrc("appshell", "Piano Roll")

            allowedAreas: Qt.TopDockWidgetArea | Qt.BottomDockWidgetArea

            height: 200
            minimumHeight: 100
            maximumHeight: 300

            tabifyPanel: timelinePanel

            // TODO: Temporarily disabled on startup, but can be enabled via the app menu, see:
            // https://github.com/musescore/MuseScore/pull/8593
            visible: false

            Rectangle {
                anchors.fill: parent
                color: ui.theme.backgroundPrimaryColor

                StyledTextLabel {
                    anchors.centerIn: parent
                    text: pianoRollPanel.title
                }
            }
        },

        DockPanel {
            id: timelinePanel

            objectName: "timelinePanel"
            title: qsTrc("appshell", "Timeline")

            allowedAreas: Qt.TopDockWidgetArea | Qt.BottomDockWidgetArea

            height: 200
            minimumHeight: 100
            maximumHeight: 300

            // TODO: Temporarily disabled on startup, but can be enabled via the app menu, see:
            // https://github.com/musescore/MuseScore/pull/8593
            visible: false

            Timeline {
                anchors.fill: parent
            }
        },

        DockPanel {
            id: drumsetPanel

            objectName: "drumsetPanel"
            title: qsTrc("appshell", "Drumset Tools")

            allowedAreas: Qt.TopDockWidgetArea | Qt.BottomDockWidgetArea

            minimumHeight: 30
            maximumHeight: 30

            DrumsetPanel {
                anchors.fill: parent
            }
        }
    ]

    central: NotationView {
        id: notationView

        isNavigatorVisible: pageModel.isNavigatorVisible

        onTextEdittingStarted: {
            notationView.forceActiveFocus()
        }
    }

    statusBar: DockStatusBar {
        id: notationStatusBar

        objectName: "notationStatusBar"

        NotationStatusBar {}
    }
}
