import QtQuick 2.15
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

RadioButtonGroup {
    id: radioButtonList

    orientation: ListView.Vertical
    spacing: 0

    signal selected(string name)

    currentIndex: 0

    delegate: GradientTabButton {
        id: radioButtonDelegate

        width: parent.width

        leftPadding: 30

        ButtonGroup.group: radioButtonList.radioButtonGroup
        orientation: Qt.Horizontal
        checked: index === radioButtonList.currentIndex

        title: modelData["title"]

        onToggled: {
            radioButtonList.currentIndex = index
            radioButtonList.selected(modelData["name"])
        }
    }
}
