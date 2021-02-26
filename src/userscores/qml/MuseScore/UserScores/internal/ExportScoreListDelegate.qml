import QtQuick 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

ListItemBlank {
    id: root

    property string title: ""
    property bool isMain: false

    signal scoreClicked()

    height: 30

    onClicked: {
        root.scoreClicked();
    }

    CheckBox {
        id: scoreCheckBox

        checked: isSelected

        width: parent.width * 1 / 6
        height: checkBoxHeight

        onClicked: {
            root.scoreClicked();
        }

        anchors.left: parent.left
        anchors.leftMargin: (width - checkBoxWidth) / 2
        anchors.verticalCenter: parent.verticalCenter
    }

    StyledTextLabel {
        text: root.title

        horizontalAlignment: Qt.AlignLeft
        font: ui.theme.bodyFont

        width: parent.width * 5 / 6
        height: parent.height

        anchors.left: scoreCheckBox.right
    }
}
