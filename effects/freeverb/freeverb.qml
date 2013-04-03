import QtQuick 1.1

Rectangle {
    id: screen
    width: 640
    height: 75
    border.width: 2
    border.color: "white"
    radius: 5
    smooth: true

    signal valueChanged(string name, real val)

    function updateValues() {
        }
    }

