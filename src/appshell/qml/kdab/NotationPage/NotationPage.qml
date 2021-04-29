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
import MuseScore.Dock 1.0
import MuseScore.AppShell 1.0

import MuseScore.NotationScene 1.0
import MuseScore.Palette 1.0
import MuseScore.Inspector 1.0
import MuseScore.Instruments 1.0
import MuseScore.Playback 1.0

import "../docksystem"
import "../../NotationPage"

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

    readonly property int defaultPanelWidth: 160
    readonly property int minimumPanelWidth: 50
    readonly property int maximumPanelWidth: 260
    readonly property int toolBarHeight: 48

    mainToolBars: [
        DockToolBar {
            id: notationToolBar

            objectName: "notationToolBar"
            title: qsTrc("appshell", "Notation Toolbar")

            width: 198
            height: root.toolBarHeight
            minimumWidth: 198
            minimumHeight: height
            maximumHeight: height

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
            height: root.toolbarHeight
            minimumWidth: floating ? 526 : 476
            minimumHeight: floating ? 76 : root.toolBarHeight
            maximumHeight: height

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

            width: 74
            height: root.toolBarHeight
            minimumWidth: width
            minimumHeight: height
            maximumHeight: height

            movable: false

            contentComponent: UndoRedoToolBar {}
        }
    ]

    toolBars: [
        DockToolBar {
            id: notationNoteInputBar

            objectName: "notationNoteInputBar"
            title: qsTrc("appshell", "Note Input")

            width: root.width
            height: root.toolBarHeight

            minimumWidth: orientation == Qt.Horizontal ? 900 : root.toolBarHeight
            maximumWidth: orientation == Qt.Horizontal ? root.width : 96
            minimumHeight: orientation == Qt.Horizontal ? root.toolBarHeight : 900
            maximumHeight: orientation == Qt.Horizontal ? root.toolBarHeight : root.height

            allowedAreas: { Qt.AllDockWidgetAreas }

            contentComponent: NoteInputBar {
                orientation: notationNoteInputBar.orientation

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

            width: root.defaultPanelWidth
            minimumWidth: root.minimumPanelWidth
            maximumWidth: root.maximumPanelWidth

            tabifyPanel: instrumentsPanel

            onClosed: {
                root.pageModel.isPalettePanelVisible = false
            }

            PalettesWidget {}
        },

        DockPanel {
            id: instrumentsPanel

            objectName: "instrumentsPanel"
            title: qsTrc("appshell", "Instruments")

            width: root.defaultPanelWidth
            minimumWidth: root.minimumPanelWidth
            maximumWidth: root.maximumPanelWidth

            tabifyPanel: inspectorPanel

            onClosed: {
                root.pageModel.isInstrumentsPanelVisible = false
            }

            InstrumentsPanel {}
        },

        DockPanel {
            id: inspectorPanel

            objectName: "inspectorPanel"
            title: qsTrc("appshell", "Inspector")

            width: root.defaultPanelWidth
            minimumWidth: root.minimumPanelWidth
            maximumWidth: root.maximumPanelWidth

            onClosed: {
                root.pageModel.isInspectorPanelVisible = false
            }

            InspectorForm {}
        }
    ]

    central: NotationView {
        id: notationView

        isNavigatorVisible: root.pageModel.isNotationNavigatorVisible

        onTextEdittingStarted: {
            notationView.forceActiveFocus()
        }
    }

    statusBars: [
        DockStatusBar {
            id: notationStatusBar

            objectName: "notationStatusBar"

            height: root.toolBarHeight
            minimumHeight: root.toolBarHeight
            maximumHeight: root.toolBarHeight

            NotationStatusBar {
                color: root.color
                visible: root.pageModel.isStatusBarVisible
            }
        }
    ]
}
