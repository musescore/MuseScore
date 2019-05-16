import QtQuick 2.0
import QtQuick.Controls 2.5
import "../style.js" as Style

RoundButton {
    radius: Style.buttonRoundness
    property color backgroundColor: "red"

    background: Rectangle {
        radius: parent.radius
        color: parent.backgroundColor
    }

    contentItem: Text {
        text: parent.text
        color: Style.textColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
