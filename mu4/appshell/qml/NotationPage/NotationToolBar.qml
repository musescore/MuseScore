import QtQuick 2.7
import QtQuick.Controls 2.2
import MuseScore.NotationScene 1.0

Rectangle {
    id: root

    Row {
        anchors.fill: parent

        Repeater {
            anchors.fill: parent
            model: toolModel
            delegate: ToolButton {
                text: titleRole
                enabled: enabledRole
                down: checkedRole
                onClicked: toolModel.click(nameRole)
            }
        }
    }

    NotationToolBarModel {
        id: toolModel
    }

    Component.onCompleted: {
        toolModel.load()
    }
}
