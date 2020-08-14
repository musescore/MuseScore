import QtQuick 2.9
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

Item {
    id: root

    property var instruments: null
    property var instrumentOrderTypes: null

    property bool canLiftInstrument: currentInstrumentIndex > 0
    property bool canLowerInstrument: isInstrumentSelected && (currentInstrumentIndex < instrumentsView.count - 1)

    property bool isInstrumentSelected: currentInstrumentIndex != -1
    property int currentInstrumentIndex: -1

    signal unselectInstrumentRequested(string id)
    signal orderChanged(string id)

    StyledTextLabel {
        id: instrumentsLabel

        anchors.top: parent.top
        anchors.left: parent.left

        font.bold: true
        text: qsTrc("instruments", "Your score")
    }

    RowLayout {
        id: operationsRow

        anchors.top: instrumentsLabel.bottom
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.right: parent.right

        StyledComboBox {
            Layout.fillWidth: true

            textRoleName: "text"
            valueRoleName: "value"

            model: {
                var resultList = []
                var orders = instrumentOrderTypes

                for (var i = 0; i < orders.length; ++i) {
                    resultList.push({"text" : qsTrc("instruments", "Order: ") + orders[i].name, "value" : orders[i].id})
                }

                return resultList
            }

            onValueChanged: {
                orderChanged(value)
            }
        }

        FlatButton {
            Layout.alignment: Layout.Right
            width: 100

            enabled: isInstrumentSelected
            text: qsTrc("instruments", "Make soloist")
        }

        FlatButton {
            Layout.alignment: Layout.Right
            width: 30

            enabled: isInstrumentSelected
            icon: IconCode.TRASH

            onClicked: {
                unselectInstrumentRequested(instruments[currentInstrumentIndex].id)
                currentInstrumentIndex--
            }
        }
    }

    ListView {
        id: instrumentsView

        anchors.top: operationsRow.bottom
        anchors.topMargin: 20
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        model: instruments

        boundsBehavior: ListView.StopAtBounds
        clip: true

        delegate: Item {
            width: parent.width
            height: 40

            Rectangle {
                anchors.fill: parent

                color: root.currentInstrumentIndex === index ? ui.theme.accentColor : ui.theme.backgroundPrimaryColor
                opacity: root.currentInstrumentIndex === index ? 0.3 : 1
            }

            StyledTextLabel {
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.right: parent.right
                anchors.rightMargin: 4
                anchors.verticalCenter: parent.verticalCenter

                horizontalAlignment: Text.AlignLeft
                text: modelData.name
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    root.currentInstrumentIndex = index
                }
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
