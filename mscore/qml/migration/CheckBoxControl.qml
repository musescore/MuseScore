import QtQuick 2.9
import QtQuick.Controls 2.2

CheckBox {
    id: root

    indicator: Rectangle {
        anchors.verticalCenter: parent.verticalCenter

        implicitHeight: 22
        implicitWidth: 22

        color: "#FFFFFF"
        border.color: globalStyle.button
        border.width: 2

        Image {
            anchors.centerIn: parent

            visible: root.checked
            source: "Tick_Icon.png"
        }
    }
}
