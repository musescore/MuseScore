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
import MuseScore.Instruments 1.0

Item {
    id: root

    property alias instruments: instrumentsView.model
    property alias search: searchField.searchText

    property bool isInstrumentSelected: prv.currentInstrumentIndex != -1

    property alias navigation: navPanel

    signal selectInstrumentRequested(var instrumentId, var transposition)
    signal instrumentClicked()

    NavigationPanel {
        id: navPanel
        name: "InstrumentsView"
        direction: NavigationPanel.Vertical
        enabled: root.visible
    }

    QtObject {
        id: prv

        property int currentInstrumentIndex: -1
        property var currentInstrument: null
    }

    function clearSearch() {
        searchField.clear()
    }

    function currentInstrument() {
        if (!isInstrumentSelected) {
            return null
        }

        return prv.currentInstrument
    }

    function resetSelectedInstrument() {
        prv.currentInstrumentIndex = -1
    }

    function focusInstrument(instrumentId) {
        for (var i in root.instruments) {
            if (root.instruments[i].id === instrumentId) {
                prv.currentInstrumentIndex = i
                instrumentsView.positionViewAtIndex(prv.currentInstrumentIndex, ListView.Beginning)
                return
            }
        }
    }

    StyledTextLabel {
        id: instrumentsLabel

        anchors.top: parent.top
        anchors.left: parent.left

        font: ui.theme.bodyBoldFont
        text: qsTrc("instruments", "Instruments")
    }

    SearchField {
        id: searchField

        anchors.top: instrumentsLabel.bottom
        anchors.topMargin: 16
        anchors.left: parent.left
        anchors.right: parent.right

        navigation.name: "SearchInstruments"
        navigation.panel: navPanel
        navigation.row: 1

        onTextCleared: {
            root.resetSelectedInstrument()
        }
    }

    ListView {
        id: instrumentsView

        anchors.top: searchField.bottom
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

            navigation.name: modelData.name
            navigation.panel: navPanel
            navigation.row: 2 + model.index
            onNavigationActived: item.clicked()

            isSelected: prv.currentInstrumentIndex === model.index

            StyledTextLabel {
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.right: transpositionsBox.visible ? transpositionsBox.left : parent.right
                anchors.rightMargin: 4
                anchors.verticalCenter: parent.verticalCenter

                horizontalAlignment: Text.AlignLeft
                text: modelData.name
                font: ui.theme.bodyBoldFont
            }

            onClicked: {
                prv.currentInstrumentIndex = model.index
                root.instrumentClicked()
            }

            onDoubleClicked: {
                var currentSelection = root.currentInstrument()
                root.selectInstrumentRequested(currentSelection.instrument.id, currentSelection.transposition)
            }

            StyledComboBox {
                id: transpositionsBox

                anchors.right: parent.right
                anchors.rightMargin: 4
                anchors.verticalCenter: parent.verticalCenter

                width: 72
                implicitHeight: 24

                textRoleName: "text"
                valueRoleName: "value"

                visible: count > 1

                onFocusChanged: {
                    if (focus) {
                        prv.currentInstrumentIndex = index
                    }
                }

                model: {
                    var resultList = []

                    var _transpositions = modelData.transpositions

                    if (!_transpositions) {
                        return
                    }

                    for (var i = 0; i < _transpositions.length; ++i) {
                        resultList.push({"text" : _transpositions[i].name, "value" : _transpositions[i].id})
                    }

                    return resultList
                }

                onValueChanged: {
                    if (prv.currentInstrumentIndex === index) {
                        item.resetCurrentInstrument()
                    }
                }
            }

            function resetCurrentInstrument() {
                prv.currentInstrument = {
                    "instrument": modelData,
                    "transposition": transpositionsBox.value
                }

                root.instrumentClicked()
            }

            Connections {
                target: prv
                function onCurrentInstrumentIndexChanged() {
                    if (prv.currentInstrumentIndex === model.index) {
                        item.resetCurrentInstrument()
                    }
                }
            }
        }
    }
}
