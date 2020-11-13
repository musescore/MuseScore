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

    property int sideMargin: 0

    delegate: RoundedRadioButton {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: root.sideMargin

        height: 46

        StyledTextLabel {
            text: model.name

            horizontalAlignment: Qt.AlignLeft
            font.pixelSize: 12
            font.bold: model.isSelected
        }

        checked: model.isSelected

        background: FlatRadioButton {
            anchors.fill: parent
            anchors.leftMargin: -parent.anchors.leftMargin

            ButtonGroup.group: root.radioButtonGroup
            normalStateColor: "transparent"

            onClicked: {
                root.model.selectWorkspace(model.index)
            }
        }
    }
}
