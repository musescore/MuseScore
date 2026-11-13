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
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.InstrumentsScene

import "internal"

Item {
    id: root

    property NavigationSection navigationSection: null
    property int navigationOrderStart: 1

    property alias contextMenuModel: contextMenuModel

    onVisibleChanged: {
        treeModel.setLayoutPanelVisible(root.visible)
    }

    Rectangle {
        id: background

        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor

        MouseArea {
            anchors.fill: parent

            // Only below the header: clicks on its (disabled) buttons and switches don't deselect
            onClicked: function(mouse) {
                if (mouse.y > headerDivider.mapToItem(background, 0, 0).y) {
                    treeModel.clearSelection()
                }
            }
        }
    }

    LayoutPanelTreeModel {
        id: treeModel
    }

    LayoutPanelContextMenuModel {
        id: contextMenuModel

        onExpandCollapseAllRequested: function(expand) {
            tree.expandCollapseAll(expand)
        }
    }

    Component.onCompleted: {
        contextMenuModel.load()
    }

    ColumnLayout {
        id: contentColumn

        anchors.fill: parent

        readonly property int sideMargin: 12
        spacing: sideMargin

        StyledTextLabel {
            Layout.fillWidth: true
            Layout.leftMargin: contentColumn.sideMargin
            Layout.rightMargin: contentColumn.sideMargin

            text: treeModel.isMasterScore ? qsTrc("layoutpanel", "Main score")
                                          //: %1 is the name of a part
                                          : qsTrc("layoutpanel", "Part: %1").arg(treeModel.scoreName)
            font: ui.theme.bodyBoldFont
            horizontalAlignment: Text.AlignLeft
        }

        LayoutControlPanel {
            id: controlPanel
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop

            Layout.leftMargin: contentColumn.sideMargin
            Layout.rightMargin: contentColumn.sideMargin

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart

            isMovingUpAvailable: treeModel.isMovingUpAvailable
            isMovingDownAvailable: treeModel.isMovingDownAvailable
            isAddingAvailable: treeModel.isAddingAvailable
            isAddingSystemMarkingsAvailable: treeModel.isAddingSystemMarkingsAvailable
            isRemovingAvailable: treeModel.isRemovingAvailable
            selectedItemsType: treeModel.selectedItemsType

            onAddInstrumentRequested: {
                treeModel.addInstruments()
            }

            onAddSystemMarkingsRequested: {
                treeModel.addSystemMarkings()
            }

            onMoveUpRequested: {
                treeModel.moveSelectedRowsUp()
            }

            onMoveDownRequested: {
                treeModel.moveSelectedRowsDown()
            }

            onRemovingRequested: {
                treeModel.removeSelectedRows()
            }
        }

        ToggleButton {
            Layout.leftMargin: contentColumn.sideMargin

            navigation.panel: controlPanel.navigation
            navigation.order: 5

            text: qsTrc("layoutpanel", "Automatically hide all empty staves")
            checked: treeModel.isHideEmptyStavesEnabled
            onToggled: treeModel.toggleHideEmptyStaves(!checked)
        }

        ToggleButton {
            Layout.leftMargin: contentColumn.sideMargin

            navigation.panel: controlPanel.navigation
            navigation.order: 6

            visible: treeModel.isMasterScore

            text: qsTrc("layoutpanel", "Combine instruments onto shared staves")
            checked: treeModel.isStaveSharingEnabled
            onToggled: treeModel.toggleStaveSharing(!checked)
        }

        SeparatorLine {
            id: headerDivider
        }

        StyledTextLabel {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: 12
            Layout.leftMargin: 20
            Layout.rightMargin: 20

            text: {
                if (treeModel.addInstrumentsKeyboardShortcut) {
                    //: Keep in sync with the text of the "Add" button at the top of the Layout panel (LayoutControlPanel.qml)
                    return qsTrc("layoutpanel", "There are no instruments in your score. To choose some, press <b>Add</b>, or use the keyboard shortcut %1.")
                    .arg("<b>" + treeModel.addInstrumentsKeyboardShortcut + "</b>")
                } else {
                    //: Keep in sync with the text of the "Add" button at the top of the Layout panel (LayoutControlPanel.qml)
                    return qsTrc("layoutpanel", "There are no instruments in your score. To choose some, press <b>Add</b>.")
                }
            }
            visible: treeModel.isEmpty && treeModel.isAddingAvailable

            verticalAlignment: Qt.AlignTop
            wrapMode: Text.WordWrap
        }

        LayoutPanelTree {
            id: tree

            Layout.fillWidth: true
            Layout.fillHeight: true

            visible: !treeModel.isEmpty

            model: treeModel

            navigationSection: root.navigationSection
            navigationOrder: controlPanel.navigation.order + 1

            sideMargin: contentColumn.sideMargin
        }
    }
}
