/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Dock 1.0
import Muse.Extensions 1.0
import MuseScore.AppShell 1.0

import MuseScore.NotationScene 1.0
import MuseScore.Palette 1.0
import MuseScore.Inspector 1.0
import MuseScore.InstrumentsScene 1.0
import MuseScore.Playback 1.0

DockPage {
    id: root

    objectName: "Notation"
    uri: "musescore://notation"

    required property NavigationSection topToolbarKeyNavSec

    property NotationPageModel pageModel: NotationPageModel {}

    property NavigationSection noteInputKeyNavSec: NavigationSection {
        id: keynavSec
        name: "NoteInputSection"
        order: 2
    }

    property NavigationSection keynavTopPanelSec: NavigationSection {
        name: "NavigationTopPanel"
        enabled: root.visible
        order: 3
    }

    property NavigationSection keynavLeftPanelSec: NavigationSection {
        name: "NavigationLeftPanel"
        enabled: root.visible
        order: 4
    }

    property NavigationSection keynavRightPanelSec: NavigationSection {
        name: "NavigationRightPanel"
        enabled: root.visible
        order: 6
    }

    property NavigationSection keynavBottomPanelSec: NavigationSection {
        name: "NavigationBottomPanel"
        enabled: root.visible
        order: 7
    }

    function navigationPanelSec(location) {
        switch(location) {
        case Location.Top: return keynavTopPanelSec
        case Location.Left: return keynavLeftPanelSec
        case Location.Right: return keynavRightPanelSec
        case Location.Bottom: return keynavBottomPanelSec
        }

        return null
    }

    onInited: {
        Qt.callLater(pageModel.init)
    }

    readonly property int verticalPanelDefaultWidth: 300

    readonly property int horizontalPanelMinHeight: 100
    readonly property int horizontalPanelMaxHeight: 520

    readonly property string verticalPanelsGroup: "VERTICAL_PANELS"
    readonly property string horizontalPanelsGroup: "HORIZONTAL_PANELS"

    readonly property var verticalPanelDropDestinations: [
        { "dock": root.centralDock, "dropLocation": Location.Left, "dropDistance": root.verticalPanelDefaultWidth },
        { "dock": root.centralDock, "dropLocation": Location.Right, "dropDistance": root.verticalPanelDefaultWidth }
    ]

    readonly property var horizontalPanelDropDestinations: [
        root.panelTopDropDestination,
        root.panelBottomDropDestination
    ]

    property var notationView: null

    mainToolBars: [
        DockToolBar {
            id: notationToolBar

            objectName: "notationToolBar"
            title: qsTrc("appshell", "Notation toolbar")

            floatable: false
            closable: false
            resizable: false
            separatorsVisible: false

            alignment: DockToolBarAlignment.Center
            contentBottomPadding: 2

            NotationToolBar {
                navigationPanel.section: root.topToolbarKeyNavSec
                navigationPanel.order: 2
            }
        },

        DockToolBar {
            id: playbackToolBar

            objectName: root.pageModel.playbackToolBarName()
            title: qsTrc("appshell", "Playback controls")

            separatorsVisible: false
            alignment: DockToolBarAlignment.Right

            contentBottomPadding: floating ? 8 : 2
            contentTopPadding: floating ? 8 : 0

            dropDestinations: [
                { "dock": notationToolBar, "dropLocation": Location.Right }
            ]

            PlaybackToolBar {
                navigationPanelSection: root.topToolbarKeyNavSec
                navigationPanelOrder: 3

                floating: playbackToolBar.floating
            }
        },

        DockToolBar {
            id: extDockToolBar

            objectName: "extensionsToolBar"
            title: qsTrc("appshell", "Extensions toolbar")

            separatorsVisible: false
            orientation: Qt.Horizontal
            alignment: DockToolBarAlignment.Right

            contentBottomPadding: floating ? 8 : 2
            contentTopPadding: floating ? 8 : 0

            dropDestinations: [
                { "dock": notationToolBar, "dropLocation": Location.Right },
                { "dock": playbackToolBar, "dropLocation": Location.Right }
            ]

            ExtensionsToolBar {
                id: extToolBar


                function updateVisible() {
                    if (!extDockToolBar.inited) {
                        return;
                    }

                    if (!root.visible) {
                        return;
                    }

                    if (extToolBar.isEmpty) {
                        extDockToolBar.close()
                    } else {
                        extDockToolBar.open()
                    }
                }

                onIsEmptyChanged: extToolBar.updateVisible()

                Connections {
                    target: extDockToolBar
                    function onInitedChanged() { extToolBar.updateVisible() }
                }

                Connections {
                    target: root
                    function onVisibleChanged() { extToolBar.updateVisible() }
                }
            }
        },

        DockToolBar {
            objectName: root.pageModel.undoRedoToolBarName()
            title: qsTrc("appshell", "Undo/redo")

            floatable: false
            closable: false
            resizable: false
            separatorsVisible: false

            alignment: DockToolBarAlignment.Right
            contentBottomPadding: 2

            UndoRedoToolBar {
                navigationPanel.section: root.topToolbarKeyNavSec
                navigationPanel.order: 4
            }
        }
    ]

    toolBars: [

        DockToolBar {
            id: noteInputBar

            objectName: root.pageModel.noteInputBarName()
            title: qsTrc("appshell", "Note input")

            dropDestinations: [
                root.toolBarTopDropDestination,
                root.toolBarBottomDropDestination,
                root.toolBarLeftDropDestination,
                root.toolBarRightDropDestination
            ]

            thickness: orientation === Qt.Horizontal ? 40 : 76

            NoteInputBar {
                orientation: noteInputBar.orientation
                floating: noteInputBar.floating

                maximumWidth: noteInputBar.width
                maximumHeight: noteInputBar.height

                navigationPanel.section: root.noteInputKeyNavSec
                navigationPanel.order: 1
            }
        }
    ]

    panels: [
        DockPanel {
            id: palettesPanel

            objectName: root.pageModel.palettesPanelName()
            title: qsTrc("appshell", "Palettes")

            navigationSection: root.navigationPanelSec(palettesPanel.location)

            width: root.verticalPanelDefaultWidth
            minimumWidth: root.verticalPanelDefaultWidth
            maximumWidth: root.verticalPanelDefaultWidth

            groupName: root.verticalPanelsGroup

            dropDestinations: root.verticalPanelDropDestinations

            PalettesPanel {
                navigationSection: palettesPanel.navigationSection

                Component.onCompleted: {
                    palettesPanel.contextMenuModel = contextMenuModel
                }
            }
        },

        DockPanel {
            id: instrumentsPanel

            objectName: root.pageModel.instrumentsPanelName()
            title: qsTrc("appshell", "Instruments")

            navigationSection: root.navigationPanelSec(instrumentsPanel.location)

            width: root.verticalPanelDefaultWidth
            minimumWidth: root.verticalPanelDefaultWidth
            maximumWidth: root.verticalPanelDefaultWidth

            groupName: root.verticalPanelsGroup

            dropDestinations: root.verticalPanelDropDestinations

            InstrumentsPanel {
                navigationSection: instrumentsPanel.navigationSection

                Component.onCompleted: {
                    instrumentsPanel.contextMenuModel = contextMenuModel
                }
            }
        },

        DockPanel {
            id: inspectorPanel

            objectName: root.pageModel.inspectorPanelName()
            title: qsTrc("appshell", "Properties")

            navigationSection: root.navigationPanelSec(inspectorPanel.location)

            width: root.verticalPanelDefaultWidth
            minimumWidth: root.verticalPanelDefaultWidth
            maximumWidth: root.verticalPanelDefaultWidth

            groupName: root.verticalPanelsGroup

            dropDestinations: root.verticalPanelDropDestinations

            InspectorForm {
                navigationSection: inspectorPanel.navigationSection
                notationView: root.notationView
            }
        },

        DockPanel {
            id: selectionFilterPanel

            objectName: root.pageModel.selectionFiltersPanelName()
            title: qsTrc("appshell", "Selection filter")

            navigationSection: root.navigationPanelSec(selectionFilterPanel.location)

            width: root.verticalPanelDefaultWidth
            minimumWidth: root.verticalPanelDefaultWidth
            maximumWidth: root.verticalPanelDefaultWidth

            groupName: root.verticalPanelsGroup

            //! NOTE: hidden by default
            visible: false

            dropDestinations: root.verticalPanelDropDestinations

            SelectionFilterPanel {
                navigationSection: selectionFilterPanel.navigationSection
            }
        },
        
        // =============================================
        // Horizontal Panels
        // =============================================

        DockPanel {
            id: mixerPanel

            objectName: root.pageModel.mixerPanelName()
            title: qsTrc("appshell", "Mixer")

            height: 368
            minimumHeight: root.horizontalPanelMinHeight
            maximumHeight: root.horizontalPanelMaxHeight

            groupName: root.horizontalPanelsGroup

            //! NOTE: hidden by default
            visible: false

            location: Location.Bottom

            dropDestinations: root.horizontalPanelDropDestinations

            navigationSection: root.navigationPanelSec(mixerPanel.location)

            MixerPanel {
                navigationSection: mixerPanel.navigationSection

                Component.onCompleted: {
                    mixerPanel.contextMenuModel = contextMenuModel
                }

                onResizeRequested: function(newWidth, newHeight) {
                    mixerPanel.resize(newWidth, newHeight)
                }
            }
        },

        DockPanel {
            id: pianoKeyboardPanel

            objectName: root.pageModel.pianoKeyboardPanelName()
            title: qsTrc("appshell", "Piano keyboard")

            height: 200
            minimumHeight: root.horizontalPanelMinHeight
            maximumHeight: root.horizontalPanelMaxHeight

            groupName: root.horizontalPanelsGroup

            //! NOTE: hidden by default
            visible: false

            location: Location.Bottom

            dropDestinations: root.horizontalPanelDropDestinations

            navigationSection: root.navigationPanelSec(pianoKeyboardPanel.location)

            PianoKeyboardPanel {
                navigationSection: pianoKeyboardPanel.navigationSection

                Component.onCompleted: {
                    pianoKeyboardPanel.contextMenuModel = contextMenuModel
                }
            }
        },

        DockPanel {
            id: timelinePanel

            objectName: root.pageModel.timelinePanelName()
            title: qsTrc("appshell", "Timeline")

            height: 200
            minimumHeight: root.horizontalPanelMinHeight
            maximumHeight: root.horizontalPanelMaxHeight

            groupName: root.horizontalPanelsGroup

            //! NOTE: hidden by default
            visible: false

            location: Location.Bottom

            dropDestinations: root.horizontalPanelDropDestinations

            navigationSection: root.navigationPanelSec(timelinePanel.location)

            Timeline {
                navigationSection: timelinePanel.navigationSection
            }
        },

        DockPanel {
            id: drumsetPanel

            objectName: root.pageModel.drumsetPanelName()
            title: qsTrc("appshell", "Drumset tools")

            height: 64
            minimumHeight: 64
            maximumHeight: 64

            //! NOTE: hidden by default
            visible: false

            floatable: false
            closable: false

            location: Location.Bottom

            navigationSection: root.navigationPanelSec(drumsetPanel.location)

            DrumsetPanel {
                navigationSection: timelinePanel.navigationSection
            }
        },

        DockPanel {
            id: percussionPanel

            objectName: root.pageModel.percussionPanelName()
            title: qsTrc("appshell", "Percussion")

            height: 200
            minimumHeight: root.horizontalPanelMinHeight
            maximumHeight: root.horizontalPanelMaxHeight

            groupName: root.horizontalPanelsGroup

            //! NOTE: hidden by default
            visible: false

            location: Location.Bottom

            dropDestinations: root.horizontalPanelDropDestinations

            navigationSection: root.navigationPanelSec(percussionPanel.location)

            PercussionPanel {
                navigationSection: percussionPanel.navigationSection

                // TODO: #22050 needed for this
                /*
                // contentNavigationPanelOrderStart: percussionPanel.contentNavigationPanelOrderStart

                Component.onCompleted: {
                    percussionPanel.contextMenuModel = contextMenuModel
                    percussionPanel.toolbarComponent = toolbarComponent
                }

                Component.onDestruction: {
                    percussionPanel.contextMenuModel = null
                    percussionPanel.toolbarComponent = null
                }
                */
            }
        }
    ]

    central: NotationView {
        id: notationView
        name: "MainNotationView"

        isNavigatorVisible: root.pageModel.isNavigatorVisible
        isBraillePanelVisible: root.pageModel.isBraillePanelVisible
        isMainView: true

        Component.onCompleted: {
            root.notationView = notationView.paintView

            root.setDefaultNavigationControl(root.defaultNavigationControl)
        }

        Component.onDestruction: {
            root.setDefaultNavigationControl(null)
        }
    }

    statusBar: DockStatusBar {
        objectName: root.pageModel.statusBarName()

        contentNavigationPanel: content.navigationPanel

        NotationStatusBar {
            id: content
        }
    }
}
