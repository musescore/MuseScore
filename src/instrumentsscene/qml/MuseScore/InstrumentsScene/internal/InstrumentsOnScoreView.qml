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
import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.12

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.InstrumentsScene 1.0

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

        delegate: ListItemBlank {
            id: item

            isSelected: model.isSelected

            navigation.name: model.name
            navigation.panel: instrumentsView.navigation
            navigation.row: 1 + model.index
            navigation.column: 0
            navigation.accessible.name: itemTitleLabel.text
            navigation.accessible.description: model.description
            navigation.accessible.row: model.index

            StyledTextLabel {
                id: itemTitleLabel
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.right: parent.right
                anchors.rightMargin: 4
                anchors.verticalCenter: parent.verticalCenter

                horizontalAlignment: Text.AlignLeft
                text:  model.isSoloist ? qsTrc("instruments", "Soloist:") + " " + model.name : model.name
                font: ui.theme.bodyBoldFont
            }

            FlatButton {
                anchors.right: parent.right
                anchors.leftMargin: 4
                anchors.rightMargin: 4
                anchors.verticalCenter: parent.verticalCenter

                isNarrow: true

                text: model.isSoloist ? qsTrc("instruments", "Undo soloist") : qsTrc("instruments", "Make soloist")
                visible: model.isSelected

                navigation.name: model.name + "MakeSoloist"
                navigation.panel: instrumentsView.navigation
                navigation.row: 1 + model.index
                navigation.column: 1

                onClicked: {
                    model.isSoloist = !model.isSoloist
                }
            }

            onClicked: {
                root.instrumentsOnScoreModel.selectRow(model.index)
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
