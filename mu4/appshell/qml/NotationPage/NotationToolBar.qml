import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import MuseScore.NotationScene 1.0

ToolBar {
    id: root

    RowLayout {
        anchors.fill: parent

        Repeater {
            anchors.fill: parent
            model: toolModel
            delegate: ToolButton {
                text: titleRole
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
