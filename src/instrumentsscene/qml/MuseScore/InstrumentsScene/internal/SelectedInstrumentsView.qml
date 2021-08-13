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

    property bool hasInstruments: instrumentsOnScore.count > 0
    property alias isMovingUpAvailable: instrumentsOnScore.isMovingUpAvailable
    property alias isMovingDownAvailable: instrumentsOnScore.isMovingDownAvailable

    property alias navigation: navPanel

    function scoreContent() {
        return instrumentsOnScore.scoreContent()
    }

    function addInstruments(instruments) {
        instrumentsOnScore.addInstruments(instruments)
    }

    function moveInstrumentsUp() {
        instrumentsOnScore.moveSelectionUp()
    }

    function moveInstrumentsDown() {
        instrumentsOnScore.moveSelectionDown()
    }

    function scrollViewToEnd() {
        instrumentsView.positionViewAtEnd()
    }

    InstrumentsOnScoreModel {
        id: instrumentsOnScore
    }

    Component.onCompleted: {
        instrumentsOnScore.load()
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
            Layout.fillWidth: true

            navigation.name: "Orders"
            navigation.panel: navPanel
            navigation.row: 1

            model: instrumentsOnScore.orders
            currentIndex: instrumentsOnScore.currentOrderIndex

            onCurrentValueChanged: {
                instrumentsOnScore.currentOrderIndex = currentIndex
            }
        }

        FlatButton {
            Layout.preferredWidth: width

            navigation.name: "Delete"
            navigation.panel: navPanel
            navigation.row: 3

            icon: IconCode.DELETE_TANK

            enabled: instrumentsOnScore.isRemovingAvailable

            onClicked: {
                instrumentsOnScore.removeSelection()
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

        model: instrumentsOnScore

        delegate: ListItemBlank {
            id: item

            isSelected: model.isSelected

            navigation.name: model.name
            navigation.panel: navPanel
            navigation.row: 4 + model.index

            onNavigationActived: {
                item.clicked()
            }

            StyledTextLabel {
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.right: parent.right
                anchors.rightMargin: 4
                anchors.verticalCenter: parent.verticalCenter

                horizontalAlignment: Text.AlignLeft
                text:  model.isSoloist ? qsTrc("instruments", "Soloist: ") + model.name : model.name
                font: ui.theme.bodyBoldFont
            }

            FlatButton {
                anchors.right: parent.right
                anchors.leftMargin: 4
                anchors.rightMargin: 4
                anchors.verticalCenter: parent.verticalCenter

                narrowMargins: true

                text: model.isSoloist ? qsTrc("instruments", "Undo soloist") : qsTrc("instruments", "Make soloist")
                visible: model.isSelected

                onClicked: {
                    instrumentsOnScore.toggleSoloist(model.index)
                }
            }

            onClicked: {
                instrumentsOnScore.selectInstrument(model.index)
            }

            onDoubleClicked: {
                instrumentsOnScore.removeSelection()
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
