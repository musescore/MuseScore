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

import Muse.Ui
import Muse.UiComponents
import MuseScore.InstrumentsScene

Item {
    id: root

    required property InstrumentsOnScoreListModel instrumentsOnScoreModel

    property alias navigation: instrumentsView.navigation

    function scrollViewToEnd() {
        instrumentsView.positionViewAtEnd()
    }

    StyledTextLabel {
        id: instrumentsLabel

        anchors.top: parent.top
        anchors.left: parent.left

        font: ui.theme.bodyBoldFont
        text: qsTrc("instruments", "Your score")
    }

    RowLayout {
        id: operationsRow

        anchors.top: instrumentsLabel.bottom
        anchors.topMargin: 16
        anchors.left: parent.left
        anchors.right: parent.right

        StyledDropdown {
            id: ordersDropdown

            Layout.fillWidth: true

            navigation.name: "Orders"
            navigation.panel: instrumentsView.navigation
            navigation.row: 0
            navigation.column: 0

            model: root.instrumentsOnScoreModel.orders

            currentIndex: root.instrumentsOnScoreModel.currentOrderIndex

            displayText: qsTrc("instruments", "Order:") + " " + currentText

            onActivated: function(index, value) {
                root.instrumentsOnScoreModel.currentOrderIndex = index
            }
        }

        FlatButton {
            Layout.preferredWidth: width

            navigation.name: "Delete"
            navigation.panel: instrumentsView.navigation
            navigation.row: 0
            navigation.column: 1

            icon: IconCode.DELETE_TANK
            toolTipTitle: qsTrc("instruments", "Remove selected instruments from score")

            enabled: root.instrumentsOnScoreModel.isRemovingAvailable

            onClicked: {
                root.instrumentsOnScoreModel.removeSelection()
            }
        }
    }

    StyledListView {
        id: instrumentsView

        anchors.top: operationsRow.bottom
        anchors.topMargin: 8
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        model: root.instrumentsOnScoreModel

        accessible.name: instrumentsLabel.text

        property int draggedRow: -1
        property int dropRow: -1
        property point dragPoint: Qt.point(0, 0)

        function updateDrop(point) {
            dragPoint = point
            let bottom = Math.max(originY, originY + contentHeight - height)
            if (point.y <= 0 && contentY <= originY) {
                dropRow = 0
                return
            }
            if (point.y >= height && contentY >= bottom) {
                dropRow = count
                return
            }
            let hitY = Math.max(0, Math.min(Math.max(0, height - 1), point.y)) + contentY
            let row = indexAt(1, hitY)
            if (row < 0) {
                if (contentY >= bottom && hitY >= originY + contentHeight) {
                    dropRow = count
                }
                return
            }
            let delegateItem = itemAtIndex(row)
            if (delegateItem) {
                dropRow = row + (hitY > delegateItem.y + delegateItem.height / 2 ? 1 : 0)
            }
        }

        Timer {
            interval: 50
            repeat: true
            running: instrumentsView.draggedRow >= 0
            onTriggered: {
                let direction = instrumentsView.dragPoint.y < 24 ? -1
                              : instrumentsView.dragPoint.y > instrumentsView.height - 24 ? 1 : 0
                if (direction !== 0) {
                    let bottom = Math.max(instrumentsView.originY,
                                          instrumentsView.originY + instrumentsView.contentHeight - instrumentsView.height)
                    instrumentsView.contentY = Math.max(instrumentsView.originY,
                                                        Math.min(bottom, instrumentsView.contentY + direction * 10))
                    instrumentsView.updateDrop(instrumentsView.dragPoint)
                }
            }
        }

        delegate: ListItemBlank {
            id: item

            required property var model
            required isSelected
            required property string name
            required property string description
            required property bool isSoloist
            required property int index

            mouseArea.hoverEnabled: instrumentsView.draggedRow < 0

            HoverHandler {
                id: rowHover
                enabled: instrumentsView.draggedRow < 0
            }

            DragHandler {
                id: rowDrag
                target: null
                xAxis.enabled: false
                onActiveChanged: {
                    if (active) {
                        instrumentsView.draggedRow = item.index
                        instrumentsView.updateDrop(item.mapToItem(instrumentsView, centroid.position))
                    } else if (instrumentsView.draggedRow >= 0) {
                        let source = instrumentsView.draggedRow
                        let destination = instrumentsView.dropRow
                        instrumentsView.draggedRow = -1
                        instrumentsView.dropRow = -1
                        root.instrumentsOnScoreModel.moveInstrument(source, destination)
                    }
                }
                onCentroidChanged: {
                    if (active) {
                        instrumentsView.updateDrop(item.mapToItem(instrumentsView, centroid.position))
                    }
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                y: instrumentsView.dropRow === item.index ? 0 : parent.height - height
                height: 2
                color: ui.theme.accentColor
                visible: instrumentsView.draggedRow >= 0
                         && (instrumentsView.dropRow === item.index
                             || (item.index === instrumentsView.count - 1 && instrumentsView.dropRow === instrumentsView.count))
            }
            
            navigation.name: name
            navigation.panel: instrumentsView.navigation
            navigation.row: 1 + index
            navigation.column: 0
            navigation.accessible.name: item.isSoloist ? item.name + ", " + qsTrc("instruments", "Soloist") : item.name
            navigation.accessible.description: description
            navigation.accessible.row: index

            StyledTextLabel {
                id: itemTitleLabel
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.right: soloistControl.left
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter

                horizontalAlignment: Text.AlignLeft
                text: item.name
                font: ui.theme.bodyBoldFont
            }

            Item {
                id: soloistControl
                anchors.right: parent.right
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                width: soloistCheck.implicitWidth
                height: soloistCheck.implicitHeight

                property bool showCheck: instrumentsView.draggedRow < 0
                                         && (rowHover.hovered || item.navigation.active || soloistCheck.navigation.active)

                CheckBox {
                    id: soloistCheck
                    anchors.fill: parent
                    text: qsTrc("instruments", "Soloist")
                    checked: item.isSoloist
                    visible: soloistControl.showCheck

                    navigation.name: item.name + "Soloist"
                    navigation.panel: instrumentsView.navigation
                    navigation.row: 1 + item.index
                    navigation.column: 1

                    onClicked: item.model.isSoloist = !item.isSoloist
                }

                StyledTextLabel {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTrc("instruments", "Soloist")
                    visible: item.isSoloist && !soloistControl.showCheck
                }
            }

            onClicked: {
                root.instrumentsOnScoreModel.selectRow(index)
            }

            onDoubleClicked: {
                root.instrumentsOnScoreModel.removeSelection()
            }

            onRemoveSelectionRequested: {
                root.instrumentsOnScoreModel.removeSelection()
            }
        }
    }

    StyledTextLabel {
        anchors.top: operationsRow.bottom
        anchors.topMargin: 20
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        visible: instrumentsView.count === 0

        text: qsTrc("instruments", "Choose your instruments by adding them to this list")
        wrapMode: Text.WordWrap
        maximumLineCount: 2
    }
}
