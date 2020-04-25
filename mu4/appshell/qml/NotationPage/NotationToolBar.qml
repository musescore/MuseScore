import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

ToolBar {
    id: root

    signal clicked(string cmd)

    RowLayout {
        anchors.fill: parent

        ToolButton {
            text: "Open"
            onClicked: root.clicked("open")
        }
    }
}
