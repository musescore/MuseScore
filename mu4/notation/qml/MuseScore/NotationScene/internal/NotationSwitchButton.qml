import QtQuick 2.7
import QtQuick.Layouts 1.3

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

FlatRadioButton {
    id: root

    property string title: ""

    signal closeRequested()

    normalStateColor: ui.theme.backgroundSecondaryColor
    hoverStateColor: selectedStateColor
    pressedStateColor: selectedStateColor
    selectedStateColor: ui.theme.backgroundPrimaryColor

    width: 110
    radius: 0

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12

        StyledTextLabel {
            Layout.alignment: Qt.AlignLeft
            Layout.fillHeight: true
            Layout.fillWidth: true

            horizontalAlignment: Text.AlignLeft

            text: root.title
            font.pixelSize: 12
        }

        FlatButton {
            Layout.fillHeight: true
            Layout.topMargin: 1
            Layout.bottomMargin: 1
            Layout.preferredWidth: width
            Layout.alignment: Qt.AlignRight

            normalStateColor: "transparent"
            icon: IconCode.CLOSE_X_ROUNDED
            onClicked: root.closeRequested()
        }

        SeparatorLine { orientation: Qt.Vertical }
    }
}
