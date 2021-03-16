import QtQuick 2.15
import QtQuick.Controls 2.15

import MuseScore.UiComponents 1.0

Row {
    property alias colors: view.model
    property alias currentColorIndex: view.currentIndex

    height: 36

    spacing: 142

    StyledTextLabel {
        anchors.verticalCenter: parent.verticalCenter

        text: qsTrc("preferences", "Accent colour:")
    }

    RadioButtonGroup {
        id: view

        spacing: 10

        delegate: RoundedRadioButton {
            width: 36
            height: width

            padding: 0
            spacing: 0

            checked: view.currentIndex === modelData.index

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
