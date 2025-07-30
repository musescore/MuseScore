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
import QtQuick.Layouts 1.15

import Muse.UiComponents 1.0
import Muse.Ui 1.0
import MuseScore.Inspector 1.0

import "../common"

InspectorSectionView {
    id: root

    implicitHeight: contentColumn.implicitHeight

    Column {
        id: contentColumn

        width: parent.width
        spacing: 12

        RowLayout {
            width: parent.width
            spacing: 4

            PopupViewButton {
                id: insertMeasuresPopupButton

                anchorItem: root.anchorItem

                navigation.panel: root.navigationPanel
                navigation.name: "InsertMeasures"
                navigation.row: root.navigationRowStart + 1

                text: qsTrc("inspector", "Insert measures")

                popupContent: InsertMeasuresPopup {
                    model: root.model

                    navigationPanel: insertMeasuresPopupButton.popupNavigationPanel
                }

                onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                    root.ensureContentVisibleRequested(invisibleContentHeight)
                }

                onPopupOpened: function(popup, control) {
                    root.popupOpened(popup, control)
                }
            }

            FlatButton {
                id: deleteButton

                navigation.panel: root.navigationPanel
                navigation.name: "DeleteMeasures"
                navigation.row: insertMeasuresPopupButton.navigation.row + 1

                toolTipTitle: qsTrc("inspector", "Delete selected measures")

                icon: IconCode.DELETE_TANK

                onClicked: {
                    if (root.model) {
                        root.model.deleteSelectedMeasures()
                    }
                }
            }
        }

        Column {
            width: parent.width
            spacing: 8

            StyledTextLabel {
                width: parent.width
                visible: root.model ? root.model.scoreIsInPageView : false
                horizontalAlignment: Qt.AlignLeft
                text: qsTrc("inspector", "Move to system")
            }

            Row {
                id: moveSystemLayout

                visible: root.model ? root.model.scoreIsInPageView : false

                width: parent.width
                spacing: 4

                FlatButton {
                    id: upSystem

                    width: (moveSystemLayout.width - moveSystemLayout.spacing) / 2

                    navigation.panel: root.navigationPanel
                    navigation.name: "SystemUp"
                    navigation.row: deleteButton.navigation.row + 1

                    orientation: Qt.Horizontal
                    icon: IconCode.ARROW_UP
                    text: qsTrc("inspector", "Previous")

                    toolTipTitle: qsTrc("inspector", "Move measure(s) to previous system")
                    toolTipShortcut: root.model ? model.shortcutMoveMeasureUp : ""

                    onClicked: {
                        if (root.model) {
                            root.model.moveMeasureUp()
                        }
                    }
                }

                FlatButton {
                    id: downSystem

                    width: (moveSystemLayout.width - moveSystemLayout.spacing) / 2

                    navigation.panel: root.navigationPanel
                    navigation.name: "SystemDown"
                    navigation.row: upSystem.navigation.row + 1

                    orientation: Qt.Horizontal
                    icon: IconCode.ARROW_DOWN
                    text: qsTrc("inspector", "Next")

                    toolTipTitle: qsTrc("inspector", "Move measure(s) to next system")
                    toolTipShortcut: root.model ? root.model.shortcutMoveMeasureDown : ""

                    onClicked: {
                        if (root.model) {
                            model.moveMeasureDown()
                        }
                    }
                }
            }
        }

        FlatButton {
            id: toggleSystemLock
            visible: root.model ? root.model.scoreIsInPageView : false
            
            width: parent.width

            navigation.panel: root.navigationPanel
            navigation.name: "SystemLock"
            navigation.row: downSystem.navigation.row + 1

            orientation: Qt.Horizontal
            icon: root.model && root.model.allSystemsAreLocked ? IconCode.LOCK_CLOSED : IconCode.LOCK_OPEN
            text: root.model ? (root.model.allSystemsAreLocked ? root.model.systemCount > 1 ? qsTrc("inspector", "Unlock selected systems")
                                                                                            : qsTrc("inspector", "Unlock selected system")
                                                               : root.model.systemCount > 1 ? qsTrc("inspector", "Lock selected systems")
                                                                                            : qsTrc("inspector", "Lock selected system"))
                             : ""

            toolTipTitle: qsTrc("inspector", "Lock/unlock selected system(s)")
            toolTipDescription: qsTrc("inspector", "Keep measures on the selected system(s) together and prevent them from reflowing to the next system")
            toolTipShortcut: root.model ? root.model.shortcutToggleSystemLock : ""

            accentButton: root.model ? root.model.allSystemsAreLocked : false

            onClicked: {
                if (root.model) {
                    root.model.toggleSystemLock()
                }
            }
        }

        FlatButton {
            id: makeIntoOneSystem
            visible: root.model ? root.model.scoreIsInPageView : false
            enabled: root.model ? root.model.isMakeIntoSystemAvailable : false

            width: parent.width

            navigation.panel: root.navigationPanel
            navigation.name: "MakeSystem"
            navigation.row: toggleSystemLock.navigation.row + 1

            orientation: Qt.Horizontal
            //icon: TODO maybe
            text: qsTrc("inspector", "Create system from selection")

            toolTipTitle: qsTrc("inspector", "Create system from selection")
            toolTipDescription: qsTrc("inspector", "Create a system containing only the selected measure(s)")
            toolTipShortcut: root.model ? root.model.shortcutMakeIntoSystem : ""

            onClicked: {
                if (root.model) {
                    root.model.makeIntoSystem()
                }
            }
        }
    }
}
