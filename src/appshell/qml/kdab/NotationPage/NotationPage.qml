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
import MuseScore.AppShell 1.0
import MuseScore.NotationScene 1.0
import MuseScore.Palette 1.0
import MuseScore.Inspector 1.0
import MuseScore.Instruments 1.0

import "../docksystem"
import "../../NotationPage"

DockPage {
    id: root

    uniqueName: "Notation"

    property var color: ui.theme.backgroundPrimaryColor
    property var borderColor: ui.theme.strokeColor

    property bool isNotationToolBarVisible: false
    property bool isPlaybackToolBarVisible: false
    property bool isUndoRedoToolBarVisible: false
    property bool isNotationNavigatorVisible: false

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

    readonly property int defaultPanelWidth: 272
    readonly property int minimumPanelWidth: 200

    panels: [
        DockPanel {
            id: palettePanel

            uniqueName: "palettePanel"

            title: qsTrc("appshell", "Palette")

            width: defaultPanelWidth
            minimumWidth: minimumPanelWidth

            floatable: true
            closable: true

            onClosed: {
                root.pageModel.isPalettePanelVisible = false
            }

            PalettesWidget {}
        },

        DockPanel {
            id: instrumentsPanel

            uniqueName: "instrumentsPanel"

            title: qsTrc("appshell", "Instruments")

            width: defaultPanelWidth
            minimumWidth: minimumPanelWidth

            tabifyPanelName: "palettePanel"

            floatable: true
            closable: true

            onClosed: {
                root.pageModel.isInstrumentsPanelVisible = false
            }

            InstrumentsPanel {}
        },

        DockPanel {
            id: inspectorPanel

            uniqueName: "inspectorPanel"

            title: qsTrc("appshell", "Inspector")

            width: defaultPanelWidth
            minimumWidth: minimumPanelWidth

            tabifyPanelName: "instrumentsPanel"

            floatable: true
            closable: true

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

    statusBar: NotationStatusBar {
        color: root.color
        visible: root.pageModel.isStatusBarVisible
    }
}
