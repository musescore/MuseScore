import QtQuick 2.0

Item {
//    property int borderWidth: 1
//    property color borderColor: "black"
    property color color: "white"

    property bool roundLeft: false
    property bool roundRight: false
    property int radius: 0

    Rectangle {
        id: leftCircle

        visible: parent.roundLeft
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left

        radius: parent.radius
        width: 2 * radius

//        border.width: parent.borderWidth
//        border.color: parent.borderColor
        color: parent.color
    }

    Rectangle {
        id: centerRectangle

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: roundLeft ? leftCircle.horizontalCenter : parent.left
        anchors.right: roundRight ? rightCircle.horizontalCenter : parent.right

//        border.width: parent.borderWidth
//        border.color: parent.borderColor
        color: parent.color
    }

    Rectangle {
        id: rightCircle

        visible: parent.roundRight
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        radius: parent.radius
        width: 2 * radius

//        border.width: parent.borderWidth
//        border.color: parent.borderColor
        color: parent.color
    }
}
