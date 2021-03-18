import QtQuick 2.15
import QtQuick.Controls 2.15

import MuseScore.UiComponents 1.0

Row {
    id: root

    property alias colors: view.model
    property alias currentColorIndex: view.currentIndex

    property int firstColumnWidth: 0

    signal accentColorChangeRequested(var newColorIndex)

    height: 36
    spacing: 0

    StyledTextLabel {
        width: root.firstColumnWidth

        anchors.verticalCenter: parent.verticalCenter
        horizontalAlignment: Qt.AlignLeft

        text: qsTrc("appshell", "Accent colour:")
    }

    RadioButtonGroup {
        id: view

        spacing: 10

        delegate: RoundedRadioButton {
            width: 36
            height: width

            padding: 0
            spacing: 0

            checked: view.currentIndex === model.index

            onClicked: {
                root.accentColorChangeRequested(model.index)
            }

            indicator: Rectangle {
                anchors.fill: parent

                radius: width / 2

                border.color: ui.theme.fontPrimaryColor
                border.width: parent.checked ? 1 : 0

                color: "transparent"

                Rectangle {
                    anchors.centerIn: parent

                    width: 30
                    height: width
                    radius: width / 2

                    border.color: ui.theme.strokeColor
                    border.width: 1

                    color: modelData
                }
            }

            background: Item {}
        }
    }
}
