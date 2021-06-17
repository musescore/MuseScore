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
import MuseScore.Instruments 1.0
import MuseScore.Playback 1.0

import "../dockwindow"

DockPage {
    id: root

    objectName: "Notation"
    uri: "musescore://notation"

    property var color: ui.theme.backgroundPrimaryColor
    property var borderColor: ui.theme.strokeColor

    property bool isNotationToolBarVisible: false
    property bool isPlaybackToolBarVisible: false
    property bool isUndoRedoToolBarVisible: false
    property bool isNotationNavigatorVisible: false

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

    function updatePageState() {
        var states = [
                    {"Palette": palettePanel.visible},
                    {"Instruments": instrumentsPanel.visible},
                    {"Inspector": inspectorPanel.visible},
                    {"NotationToolBar": isNotationToolBarVisible},
                    {"PlaybackToolBar": isPlaybackToolBarVisible},
                    {"UndoRedoToolBar": isUndoRedoToolBarVisible}
                ]

        pageModel.setPanelsState(states)
    }

    Component.onCompleted: {
        updatePageState()

        palettePanel.visible = Qt.binding(function() { return pageModel.isPalettePanelVisible })
        instrumentsPanel.visible = Qt.binding(function() { return pageModel.isInstrumentsPanelVisible })
        inspectorPanel.visible = Qt.binding(function() { return pageModel.isInspectorPanelVisible })

        pageModel.init()
    }

    readonly property int defaultPanelWidth: 260
    readonly property int toolBarHeight: 48

    mainToolBars: [
        DockToolBar {
            id: notationToolBar

            objectName: "notationToolBar"
            title: qsTrc("appshell", "Notation Toolbar")

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
            title: qsTrc("appshell", "Playback Controls")

            width: root.width / 3
            minimumWidth: floating ? 526 : 476
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
            title: qsTrc("appshell", "Undo/Redo Toolbar")

            minimumWidth: 74
            maximumWidth: 74

            movable: false

            contentComponent: UndoRedoToolBar {}
        }
    ]

    toolBars: [
        DockToolBar {
            id: noteInputBar

            objectName: "noteInputBar"
            title: qsTrc("appshell", "Note Input")

            horizontalPreferredSize: Qt.size(720, root.toolBarHeight)
            verticalPreferredSize: Qt.size(root.toolBarHeight, 400)

            allowedAreas: { Qt.AllDockWidgetAreas }

            contentComponent: NoteInputBar {
                orientation: noteInputBar.orientation

                navigation.section: noteInputKeyNavSec
                navigation.order: 1
            }
        }
    ]

    panels: [
        DockPanel {
            id: palettePanel

            objectName: "palettePanel"
            title: qsTrc("appshell", "Palette")

            navigationSection: root.navigationPanelSec(palettePanel.location)

            width: root.defaultPanelWidth
            minimumWidth: root.defaultPanelWidth
            maximumWidth: root.defaultPanelWidth

            tabifyPanel: instrumentsPanel

            onClosed: {
                root.pageModel.isPalettePanelVisible = false
            }

            PalettesWidget {
                navigationSection: palettePanel.navigationSection
            }
        },

        DockPanel {
            id: instrumentsPanel

            objectName: "instrumentsPanel"
            title: qsTrc("appshell", "Instruments")

            navigationSection: root.navigationPanelSec(palettePanel.location)

            width: root.defaultPanelWidth
            minimumWidth: root.defaultPanelWidth
            maximumWidth: root.defaultPanelWidth

            tabifyPanel: inspectorPanel

            onClosed: {
                root.pageModel.isInstrumentsPanelVisible = false
            }

            InstrumentsPanel {
                navigationSection: instrumentsPanel.navigationSection
            }
        },

        DockPanel {
            id: inspectorPanel

            objectName: "inspectorPanel"
            title: qsTrc("appshell", "Inspector")

            navigationSection: root.navigationPanelSec(palettePanel.location)

            width: root.defaultPanelWidth
            minimumWidth: root.defaultPanelWidth
            maximumWidth: root.defaultPanelWidth

            onClosed: {
                root.pageModel.isInspectorPanelVisible = false
            }

            InspectorForm {
                navigationSection: inspectorPanel.navigationSection
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

            height: 200
            minimumHeight: 100
            maximumHeight: 300

            tabifyPanel: pianoRollPanel

            visible: false

            Rectangle {
                anchors.fill: parent
                color: ui.theme.backgroundPrimaryColor

                StyledTextLabel {
                    anchors.centerIn: parent
                    text: mixerPanel.title
                }
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

            visible: false

            Rectangle {
                anchors.fill: parent
                color: ui.theme.backgroundPrimaryColor

                StyledTextLabel {
                    anchors.centerIn: parent
                    text: pianoRollPanel.title
                }
            }
        }
    ]

    central: NotationView {
        id: notationView

        isNavigatorVisible: root.pageModel.isNotationNavigatorVisible

        onTextEdittingStarted: {
            notationView.forceActiveFocus()
        }
    }

    statusBar: DockStatusBar {
        id: notationStatusBar

        objectName: "notationStatusBar"

        NotationStatusBar {
            color: root.color
            visible: root.pageModel.isStatusBarVisible
        }
    }
}
