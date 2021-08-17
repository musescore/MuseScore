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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

import "internal"

StyledDialogView {
    id: root

    contentWidth: 280
    contentHeight: 600
    margins: 12

    modal: true

    NoteInputBarCustomiseModel {
        id: customiseModel
    }

    Component.onCompleted: {
        customiseModel.load()
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 0

        StyledTextLabel {
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            Layout.topMargin: 8

            text: qsTrc("notation", "Customise toolbar")
            horizontalAlignment: Text.AlignLeft
            font: ui.theme.largeBodyBoldFont
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: 20

            height: childrenRect.height

            FlatButton {
                Layout.fillWidth: true

                text: qsTrc("notation", "Add separator line")

                enabled: customiseModel.isAddSeparatorAvailable

                onClicked: {
                    customiseModel.addSeparatorLine()
                }
            }

            FlatButton {
                Layout.alignment: Qt.AlignRight
                Layout.preferredWidth: 30

                icon: IconCode.DELETE_TANK
                enabled: customiseModel.isRemovingAvailable

                onClicked: {
                    customiseModel.removeSelection()
                }
            }

            FlatButton {
                Layout.alignment: Qt.AlignRight
                Layout.preferredWidth: 30

                icon: IconCode.ARROW_UP
                enabled: customiseModel.isMovingUpAvailable

                onClicked: {
                    customiseModel.moveSelectionUp()
                    Qt.callLater(view.positionViewAtSelectedItems)
                }
            }

            FlatButton {
                Layout.alignment: Qt.AlignRight
                Layout.preferredWidth: 30

                icon: IconCode.ARROW_DOWN
                enabled: customiseModel.isMovingDownAvailable

                onClicked: {
                    customiseModel.moveSelectionDown()
                    Qt.callLater(view.positionViewAtSelectedItems)
                }
            }
        }

        ListView {
            id: view

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: 12

            spacing: 0

            model: customiseModel

            boundsBehavior: Flickable.StopAtBounds
            clip: true

            function positionViewAtSelectedItems() {
                var selectedIndexes = customiseModel.selectionModel.selectedIndexes
                for (var _index in selectedIndexes) {
                    positionViewAtIndex(selectedIndexes[_index].row, ListView.Contain)
                }
            }

            ScrollBar.vertical: StyledScrollBar {

                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right

                visible: view.contentHeight > view.height
                z: 1
            }

            delegate: ListItemBlank {
                id: itemDelegate

                height: 38

                isSelected: model.isSelected

                onClicked: {
                    customiseModel.selectRow(index)
                }

                property var item: model.item

                Loader {
                    property var delegateType: Boolean(itemDelegate.item) ? itemDelegate.item.type : NoteInputBarCustomiseItem.UNDEFINED

                    anchors.fill: parent
                    sourceComponent: delegateType === NoteInputBarCustomiseItem.ACTION ? actionComponent : separatorLineComponent

                    Component {
                        id: actionComponent

                        NoteInputBarActionDelegate {
                            item: itemDelegate.item
                        }
                    }

                    Component {
                        id: separatorLineComponent

                        StyledTextLabel {
                            anchors.centerIn: parent
                            text: Boolean(itemDelegate.item) ? itemDelegate.item.title : ""
                        }
                    }
                }
            }
        }
    }
}
