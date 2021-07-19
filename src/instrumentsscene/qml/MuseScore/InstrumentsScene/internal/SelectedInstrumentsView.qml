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
import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.InstrumentsScene 1.0

Item {
    id: root

    property alias instruments: instrumentsView.model
    property var instrumentsModel: null

    property bool canLiftInstrument: currentInstrumentIndex > 0
    property bool canLowerInstrument: isInstrumentSelected && (currentInstrumentIndex < instrumentsView.count - 1)

    property bool isInstrumentSelected: currentInstrumentIndex != -1
    property int currentInstrumentIndex: -1

    property alias navigation: navPanel

    signal unselectInstrumentRequested(var index)
    signal orderChanged(string id)
    signal soloistChanged(string id)

    function selectedScoreOrder() {
        var orders = instrumentsModel.scoreOrders
        return orders[scoreOrderComboBox.currentIndex].config
    }

    function scrollViewToEnd() {
        instrumentsView.positionViewAtEnd()
    }

    function unselectCurrentInstrument() {
        unselectInstrumentRequested(currentInstrumentIndex)
        currentInstrumentIndex--
    }

    function soloistsButtonText(soloist) {
        return soloist ? qsTrc("instruments", "Undo soloist") : qsTrc("instruments", "Make soloist")
    }

    function instrumentName(data) {
        var name = data.name
        if (data.isSoloist) {
            name = qsTrc("instruments", "Soloist: ") + name
        }
        return name
    }

    NavigationPanel {
        id: navPanel
        name: "SelectedInstrumentsView"
        direction: NavigationPanel.Vertical
        enabled: root.visible
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

        Dropdown {
            id: scoreOrderComboBox

            Layout.fillWidth: true

            navigation.name: "Orders"
            navigation.panel: navPanel
            navigation.row: 1

            textRole: "name"
            valueRole: "id"

            model: instrumentsModel.scoreOrders

            currentIndex: instrumentsModel.selectedScoreOrderIndex

            onCurrentValueChanged: {
                root.orderChanged(scoreOrderComboBox.currentValue)
            }
        }

        FlatButton {
            Layout.preferredWidth: width

            navigation.name: "Delete"
            navigation.panel: navPanel
            navigation.row: 3

            enabled: root.isInstrumentSelected
            icon: IconCode.DELETE_TANK

            onClicked: {
                root.unselectCurrentInstrument()
            }
        }
    }

    ListView {
        id: instrumentsView

        anchors.top: operationsRow.bottom
        anchors.topMargin: 8
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        boundsBehavior: ListView.StopAtBounds
        clip: true

        ScrollBar.vertical: StyledScrollBar {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
        }

        delegate: ListItemBlank {
            id: item

            isSelected: root.currentInstrumentIndex === model.index

            navigation.name: modelData.name
            navigation.panel: navPanel
            navigation.row: 4 + model.index
            onNavigationActived: item.clicked()

            StyledTextLabel {
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.right: parent.right
                anchors.rightMargin: 4
                anchors.verticalCenter: parent.verticalCenter

                horizontalAlignment: Text.AlignLeft
                text: instrumentName(modelData)
                font: ui.theme.bodyBoldFont
            }

            FlatButton {
                anchors.right: parent.right
                anchors.leftMargin: 4
                anchors.rightMargin: 4
                anchors.verticalCenter: parent.verticalCenter

                narrowMargins: true

                visible: root.currentInstrumentIndex === index

                text: soloistsButtonText(modelData.isSoloist)

                onClicked: {
                    soloistChanged(modelData.id)
                }
            }

            onClicked: {
                root.currentInstrumentIndex = model.index
            }

            onDoubleClicked: {
                root.unselectCurrentInstrument()
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
