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

    implicitHeight: contentColumn.height

    ColumnLayout {
        id: contentColumn

        height: implicitHeight
        width: parent.width

        spacing: 0

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

                onClicked: model.deleteSelectedMeasures()
            }
        }

        StyledTextLabel {
            Layout.fillWidth: true
            Layout.topMargin: 12
            visible: root.model ? model.scoreIsInPageView : false
            horizontalAlignment: Qt.AlignLeft
            text: qsTrc("inspector", "Move to system")
        }

        RowLayout {
            id: moveSystemLayout

            Layout.topMargin: 8
            visible: model.scoreIsInPageView

            width: parent.width
            spacing: 4

            FlatButton {
                id: upSystem

                Layout.preferredWidth: (moveSystemLayout.width - moveSystemLayout.spacing) / 2

                navigation.panel: root.navigationPanel
                navigation.name: "SystemUp"
                navigation.row: deleteButton.navigation.row + 1

                orientation: Qt.Horizontal
                icon: IconCode.ARROW_UP
                text: qsTrc("inspector", "Previous")

                toolTipTitle: qsTrc("inspector", "Move measure(s) to previous system")
                toolTipShortcut: model.shortcutMoveMeasureUp

                onClicked: model.moveMeasureUp()
            }

            FlatButton {
                id: downSystem

                Layout.preferredWidth: (moveSystemLayout.width - moveSystemLayout.spacing) / 2

                navigation.panel: root.navigationPanel
                navigation.name: "SystemDown"
                navigation.row: upSystem.navigation.row + 1

                orientation: Qt.Horizontal
                icon: IconCode.ARROW_DOWN
                text: qsTrc("inspector", "Next")

                toolTipTitle: qsTrc("inspector", "Move measure(s) to next system")
                toolTipShortcut: model.shortcutMoveMeasureDown

                onClicked: model.moveMeasureDown()
            }
        }


        FlatButton {
            Layout.topMargin: 12
            Layout.fillWidth: true
            visible: model.scoreIsInPageView

            id: toggleSystemLock

            width: parent.width

            navigation.panel: root.navigationPanel
            navigation.name: "SystemLock"
            navigation.row: downSystem.navigation.row + 1

            orientation: Qt.Horizontal
            icon: Boolean(model.allSystemsAreLocked) ? IconCode.LOCK_CLOSED : IconCode.LOCK_OPEN
            text: Boolean(model.allSystemsAreLocked) ? model.systemCount > 1 ? qsTrc("inspector", "Unlock selected systems")
                                                                             : qsTrc("inspector", "Unlock selected system")
                                                     : model.systemCount > 1 ? qsTrc("inspector", "Lock selected systems")
                                                                             : qsTrc("inspector", "Lock selected system")

            toolTipTitle: qsTrc("inspector", "Lock/unlock selected system(s)")
            toolTipDescription: qsTrc("inspector", "Keep measures on the selected system(s) together and prevent them from reflowing to the next system")
            toolTipShortcut: model.shortcutToggleSystemLock

            accentButton: Boolean(model.allSystemsAreLocked)

            onClicked: model.toggleSystemLock()
        }

        FlatButton {
            id: makeIntoOneSystem
            enabled: root.model ? model.isMakeIntoSystemAvailable : false

            Layout.topMargin: 4
            Layout.fillWidth: true

            navigation.panel: root.navigationPanel
            navigation.name: "MakeSystem"
            navigation.row: toggleSystemLock.navigation.row + 1

            orientation: Qt.Horizontal
            //icon: TODO maybe
            text: qsTrc("inspector", "Create system from selection")

            toolTipTitle: qsTrc("inspector", "Create system from selection")
            toolTipDescription: qsTrc("inspector", "Create a system containing only the selected measure(s)")
            toolTipShortcut: model.shortcutMakeIntoSystem

            onClicked: model.makeIntoSystem()
        }
    }
}
