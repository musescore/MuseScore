import QtQuick 2.9
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

Item {
    id: root

    property var instruments: null
    property alias search: searchField.searchText

    property bool isInstrumentSelected: privateProperties.currentInstrumentIndex != -1

    QtObject {
        id: privateProperties

        property int currentInstrumentIndex: -1
        property var currentInstrument: null
    }

    function currentInstrument() {
        if (!isInstrumentSelected) {
            return null
        }

        return privateProperties.currentInstrument
    }

    function selectFirstInstrument() {
        if (instrumentsView.count == 0) {
            privateProperties.currentInstrumentIndex = -1
            return
        }

        privateProperties.currentInstrumentIndex = 0
    }

    StyledTextLabel {
        id: instrumentsLabel

        anchors.top: parent.top
        anchors.left: parent.left

        font.bold: true
        text: qsTrc("instruments", "Instruments")
    }

    SearchField {
        id: searchField

        anchors.top: instrumentsLabel.bottom
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.right: parent.right

        onSearchTextChanged: {
            selectFirstInstrument()
        }
    }

    ListView {
        id: instrumentsView

        anchors.top: searchField.bottom
        anchors.topMargin: 20
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        model: instruments

        boundsBehavior: ListView.StopAtBounds
        clip: true

        delegate: Item {
            id: item

            width: parent.width
            height: 40

            Rectangle {
                anchors.fill: parent

                color: privateProperties.currentInstrumentIndex === index ? ui.theme.accentColor : ui.theme.backgroundPrimaryColor
                opacity: privateProperties.currentInstrumentIndex === index ? 0.3 : 1
            }

            StyledTextLabel {
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.right: transpositionsBox.visible ? transpositionsBox.left : parent.right
                anchors.rightMargin: 4
                anchors.verticalCenter: parent.verticalCenter

                horizontalAlignment: Text.AlignLeft
                text: modelData.name
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    privateProperties.currentInstrumentIndex = index
                }
            }

            StyledComboBox {
                id: transpositionsBox

                anchors.right: parent.right
                anchors.rightMargin: 4
                anchors.verticalCenter: parent.verticalCenter

                height: 32
                width: 50

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
            }

            Connections {
                target: privateProperties
                onCurrentInstrumentIndexChanged: {
                    if (privateProperties.currentInstrumentIndex == index) {
                        item.resetCurrentInstrument()
                    }
                }
            }
        }
    }
}
