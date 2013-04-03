import QtQuick 1.1

Rectangle {
    id: screen
    width: 642
    height: 77
    border.width: 1
    border.color: "white"
    radius: 5
    color: "#3f3f3f"
    smooth: true

    signal valueChanged(string name, real val)

    function updateValues() {
        }
    Text {
        anchors.centerIn: parent
        text: "Freeverb (under construction)"
        color: "white"
        }
    }

