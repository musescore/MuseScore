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
import MuseScore.Instruments 1.0

Item {
    id: root

    property var instruments: null
    property alias search: searchField.searchText

    property bool isInstrumentSelected: privateProperties.currentInstrumentIndex != -1

    signal selectInstrumentRequested(var instrumentId, var transposition)
    signal instrumentClicked()

    QtObject {
        id: privateProperties

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

        return privateProperties.currentInstrument
    }

    function resetSelectedInstrument() {
        privateProperties.currentInstrumentIndex = -1
    }

    function focusInstrument(instrumentId) {
        for (var i in root.instruments) {
            if (root.instruments[i].id === instrumentId) {
                privateProperties.currentInstrumentIndex = i
                instrumentsView.positionViewAtIndex(privateProperties.currentInstrumentIndex, ListView.Beginning)
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

        model: instruments

        boundsBehavior: ListView.StopAtBounds
        clip: true

        ScrollBar.vertical: StyledScrollBar {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
        }

        delegate: ListItemBlank {
            id: item

            isSelected: privateProperties.currentInstrumentIndex === index

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
                privateProperties.currentInstrumentIndex = index
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
                        privateProperties.currentInstrumentIndex = index
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
                    if (privateProperties.currentInstrumentIndex == index) {
                        item.resetCurrentInstrument()
                    }
                }
            }

            function resetCurrentInstrument() {
                privateProperties.currentInstrument = {
                    "instrument": modelData,
                    "transposition": transpositionsBox.value
                }

                root.instrumentClicked()
            }

            Connections {
                target: privateProperties
                function onCurrentInstrumentIndexChanged() {
                    if (privateProperties.currentInstrumentIndex == index) {
                        item.resetCurrentInstrument()
                    }
                }
            }
        }
    }
}
