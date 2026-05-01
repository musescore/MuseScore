/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Muse.UiComponents
import Muse.Ui
import MuseScore.PropertiesPanel

import "../common"

PropertiesPanelSection {
    id: root

    required property MeasuresSettingsModel model

    implicitHeight: contentColumn.implicitHeight

    Column {
        id: contentColumn

        width: parent.width
        spacing: 12

        RowLayout {
            width: parent.width
            spacing: 4

            PropertiesPanelPopupButton {
                id: insertMeasuresPopupButton

                anchorItem: root.anchorItem

                navigation.panel: root.navigationPanel
                navigation.name: "InsertMeasures"
                navigation.row: root.navigationRowStart + 1

                text: qsTrc("propertiespanel", "Insert measures")

                popupContent: InsertMeasuresPopup {
                    model: root.model

                    navigationPanel: insertMeasuresPopupButton.popupNavigationPanel
                }

                onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                    root.ensureContentVisibleRequested(invisibleContentHeight)
                }
            }

            FlatButton {
                id: deleteButton

                navigation.panel: root.navigationPanel
                navigation.name: "DeleteMeasures"
                navigation.row: insertMeasuresPopupButton.navigation.row + 1

                toolTipTitle: qsTrc("propertiespanel", "Delete selected measures")

                icon: IconCode.DELETE_TANK

                onClicked: {
                    root.model?.deleteSelectedMeasures?.()
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
                text: qsTrc("propertiespanel", "Move to system")
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
                    text: qsTrc("propertiespanel", "Previous")

                    toolTipTitle: qsTrc("propertiespanel", "Move measure(s) to previous system")
                    toolTipShortcut: root.model?.shortcutMoveMeasureUpSystem ?? ""

                    onClicked: {
                        root.model?.moveMeasureUpSystem?.()
                    }
                }

                FlatButton {
                    id: downSystem

                    width: (moveSystemLayout.width - moveSystemLayout.spacing) / 2

                    navigation.panel: root.navigationPanel
                    navigation.name: "SystemDown"
                    navigation.row: upPage.navigation.row + 1

                    orientation: Qt.Horizontal
                    icon: IconCode.ARROW_DOWN
                    text: qsTrc("propertiespanel", "Next")

                    toolTipTitle: qsTrc("propertiespanel", "Move measure(s) to next system")
                    toolTipShortcut: root.model ? root.model.shortcutMoveMeasureDownSystem : ""

                    onClicked: {
                        root.model?.moveMeasureDownSystem?.()
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
            text: root.model ? (root.model.allSystemsAreLocked ? root.model.systemCount > 1 ? qsTrc("propertiespanel", "Unlock selected systems")
                                                                                            : qsTrc("propertiespanel", "Unlock selected system")
                                                               : root.model.systemCount > 1 ? qsTrc("propertiespanel", "Lock selected systems")
                                                                                            : qsTrc("propertiespanel", "Lock selected system"))
                             : ""

            toolTipTitle: qsTrc("propertiespanel", "Lock/unlock selected system(s)")
            toolTipDescription: qsTrc("propertiespanel", "Keep measures on the selected system(s) together and prevent them from reflowing to the next system")
            toolTipShortcut: root.model ? root.model.shortcutToggleSystemLock : ""

            accentButton: root.model ? root.model.allSystemsAreLocked : false

            onClicked: {
                root.model?.toggleSystemLock?.()
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
            text: qsTrc("propertiespanel", "Create system from selection")

            toolTipTitle: qsTrc("propertiespanel", "Create system from selection")
            toolTipDescription: qsTrc("propertiespanel", "Create a system containing only the selected measure(s)")
            toolTipShortcut: root.model ? root.model.shortcutMakeIntoSystem : ""

            onClicked: {
                root.model?.makeIntoSystem?.()
            }
        }

            Column {
            width: parent.width
            spacing: 8

            StyledTextLabel {
                width: parent.width
                visible: root.model ? root.model.scoreIsInPageView : false
                horizontalAlignment: Qt.AlignLeft
                text: qsTrc("inspector", "Move to page")
            }

            Row {
                id: movePageLayout

                visible: root.model ? root.model.scoreIsInPageView : false

                width: parent.width
                spacing: 4

                FlatButton {
                    id: upPage

                    width: (movePageLayout.width - movePageLayout.spacing) / 2

                    navigation.panel: root.navigationPanel
                    navigation.name: "PageUp"
                    navigation.row: deleteButton.navigation.row + 1

                    orientation: Qt.Horizontal
                    icon: IconCode.ARROW_UP
                    text: qsTrc("inspector", "Previous")

                    toolTipTitle: qsTrc("inspector", "Move measure(s) to previous page")
                    toolTipShortcut: root.model?.shortcutMoveMeasureUpPage ?? ""

                    onClicked: {
                        root.model?.moveMeasureUpPage?.()
                    }
                }

                FlatButton {
                    id: downPage

                    width: (movePageLayout.width - movePageLayout.spacing) / 2

                    navigation.panel: root.navigationPanel
                    navigation.name: "PageDown"
                    navigation.row: upPage.navigation.row + 1

                    orientation: Qt.Horizontal
                    icon: IconCode.ARROW_DOWN
                    text: qsTrc("inspector", "Next")

                    toolTipTitle: qsTrc("inspector", "Move measure(s) to next page")
                    toolTipShortcut: root.model ? root.model.shortcutMoveMeasureDownPage : ""

                    onClicked: {
                        root.model?.moveMeasureDownPage?.()
                    }
                }
            }
        }

        FlatButton {
            id: togglePageLock
            visible: root.model ? root.model.scoreIsInPageView : false
            
            width: parent.width

            navigation.panel: root.navigationPanel
            navigation.name: "PageLock"
            navigation.row: downPage.navigation.row + 1

            orientation: Qt.Horizontal
            icon: root.model && root.model.allPagesAreLocked ? IconCode.LOCK_CLOSED : IconCode.LOCK_OPEN
            text: root.model ? (root.model.allPagesAreLocked ? root.model.pageCount > 1 ? qsTrc("inspector", "Unlock selected pages")
                                                                                            : qsTrc("inspector", "Unlock selected page")
                                                               : root.model.pageCount > 1 ? qsTrc("inspector", "Lock selected pages")
                                                                                            : qsTrc("inspector", "Lock selected page"))
                             : ""

            toolTipTitle: qsTrc("inspector", "Lock/unlock selected page(s)")
            toolTipDescription: qsTrc("inspector", "Keep measures on the selected page(s) together and prevent them from reflowing to the next page")
            toolTipShortcut: root.model ? root.model.shortcutToggleSystemLock : ""

            accentButton: root.model ? root.model.allPagesAreLocked : false

            onClicked: {
                root.model?.togglePageLock?.()
            }
        }

        FlatButton {
            id: makeIntoOnePage
            visible: root.model ? root.model.scoreIsInPageView : false
            enabled: root.model ? root.model.isMakeIntoPageAvailable : false

            width: parent.width

            navigation.panel: root.navigationPanel
            navigation.name: "MakePage"
            navigation.row: togglePageLock.navigation.row + 1

            orientation: Qt.Horizontal
            //icon: TODO maybe
            text: qsTrc("inspector", "Create page from selection")

            toolTipTitle: qsTrc("inspector", "Create page from selection")
            toolTipDescription: qsTrc("inspector", "Create a page containing only the selected measure(s)")
            toolTipShortcut: root.model ? root.model.shortcutMakeIntoPage : ""

            onClicked: {
                root.model?.makeIntoPage?.()
            }
        }
    }
}
