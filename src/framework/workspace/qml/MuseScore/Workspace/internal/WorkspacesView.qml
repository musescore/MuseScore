import QtQuick 2.9
import QtQuick.Controls 2.2

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

RadioButtonGroup {
    id: root

    clip: true
    spacing: 0
    interactive: height < contentHeight
    boundsBehavior: Flickable.StopAtBounds
    orientation: Qt.Vertical

    property int leftPadding: 0

    delegate: RoundedRadioButton {
        anchors.left: parent.left
        anchors.right: parent.right
        leftPadding: root.leftPadding

        height: 46
        spacing: 12

        StyledTextLabel {
            text: model.name

            horizontalAlignment: Qt.AlignLeft
            font.bold: model.isSelected
        }

        checked: model.isSelected

        background: FlatRadioButton {
            anchors.fill: parent

            ButtonGroup.group: root.radioButtonGroup
            normalStateColor: "transparent"

            onClicked: {
                root.model.selectWorkspace(model.index)
            }
        }
    }
}
