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
        Qt.callLater(pageModel.init)
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

            objectName: pageModel.playbackToolBarName()
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
            objectName: pageModel.undoRedoToolBarName()
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

            objectName: pageModel.noteInputBarName()
            title: qsTrc("appshell", "Note input")

            horizontalPreferredSize: Qt.size(720, root.toolBarHeight)
            verticalPreferredSize: Qt.size(root.toolBarHeight, 400)

            allowedAreas: { Qt.AllDockWidgetAreas }

            contentComponent: NoteInputBar {
                orientation: noteInputBar.orientation

                navigation.section: root.noteInputKeyNavSec
                navigation.order: 1

                floating: noteInputBar.floating
            }
        }
    ]

    panels: [
        DockPanelQml {
            id: palettesPanel

            objectName: pageModel.palettesPanelName()
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

        DockPanelQml {
            id: instrumentsPanel

            objectName: pageModel.instrumentsPanelName()
            title: qsTrc("appshell", "Instruments")

            navigationSection: root.navigationPanelSec(instrumentsPanel.location)

            width: root.defaultPanelWidth
            minimumWidth: root.defaultPanelWidth
            maximumWidth: root.defaultPanelWidth

            tabifyPanel: inspectorPanel

            InstrumentsPanel {
                navigationSection: instrumentsPanel.navigationSection

                Component.onCompleted: {
                    instrumentsPanel.contextMenuModel = contextMenuModel
                }
            }
        },

        DockPanelQml {
            id: inspectorPanel

            objectName: pageModel.inspectorPanelName()
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

        DockPanelQml {
            id: selectionFilterPanel

            objectName: pageModel.selectionFiltersPanelName()
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

        DockPanelQml {
            id: mixerPanel

            objectName: pageModel.mixerPanelName()
            title: qsTrc("appshell", "Mixer")

            allowedAreas: Qt.TopDockWidgetArea | Qt.BottomDockWidgetArea

            height: minimumHeight
            minimumHeight: 260
            maximumHeight: 520

            tabifyPanel: pianoRollPanel

            // TODO: Temporarily disabled on startup, but can be enabled via the app menu, see:
            // https://github.com/musescore/MuseScore/pull/8593
            visible: false

            MixerPanel {
                Component.onCompleted: {
                    mixerPanel.contextMenuModel = contextMenuModel
                }
            }
        },

        DockPanelQml {
            id: pianoRollPanel

            objectName: pageModel.pianoPanelName()
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

        DockPanelQml {
            id: timelinePanel

            objectName: pageModel.timelinePanelName()
            title: qsTrc("appshell", "Timeline")

            allowedAreas: Qt.TopDockWidgetArea | Qt.BottomDockWidgetArea

            height: 200
            minimumHeight: 100
            maximumHeight: 300

            // TODO: Temporarily disabled on startup, but can be enabled via the app menu, see:
            // https://github.com/musescore/MuseScore/pull/8593
            visible: false

            Timeline {}
        },

        DockPanelQml {
            id: drumsetPanel

            objectName: pageModel.drumsetPanelName()
            title: qsTrc("appshell", "Drumset Tools")

            allowedAreas: Qt.TopDockWidgetArea | Qt.BottomDockWidgetArea

            minimumHeight: 30
            maximumHeight: 30

            DrumsetPanel {}
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

        objectName: pageModel.statusBarName()

        NotationStatusBar {}
    }
}
