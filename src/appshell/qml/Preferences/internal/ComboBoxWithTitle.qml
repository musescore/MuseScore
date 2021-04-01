import QtQuick 2.15
import QtQuick.Layouts 1.12

import MuseScore.UiComponents 1.0

Row {
    id: root

    property alias title: titleLabel.text
    property alias titleWidth: titleLabel.width

    property alias currentIndex: comboBox.currentIndex
    property alias currentValue: comboBox.currentValue
    property alias model: comboBox.model
    property alias control: comboBox

    signal valueEdited(var newValue)

    spacing: 0

    StyledTextLabel {
        id: titleLabel

        anchors.verticalCenter: parent.verticalCenter

        width: root.firstColumnWidth
        horizontalAlignment: Qt.AlignLeft
    }

    StyledComboBox {
        id: comboBox

        width: 210

        onActivated: {
            root.valueEdited(currentValue)
        }
    }
}
