import QtQuick 2.7
import QtQuick.Controls 2.2

Rectangle {

    id: root

    property var model: null

    signal selected(string name)

    Column {
        anchors.fill: parent

        Repeater {
            model: root.model
            delegate: ItemDelegate {
                height: 56
                anchors.left: parent.left
                anchors.right: parent.right

                text: modelData.title
                onClicked: root.selected(modelData.name)
            }
        }
    }
}
