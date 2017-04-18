import QtQuick 2.0

Item {
    id: rotary
    width:  23
    height: 23
    property string pid
    property real value: myEffect.value(pid)

    function updateValues() {
        value = myEffect.value(pid)
        }

    onValueChanged: {
        knob.rotation = value * 270 - 45
        screen.valueChanged(pid, value)
        }

    Rectangle {
        id:     knob
        smooth: true

        x:      0
        y:      11.5 - 2
        color: "#3f3f3f"

        width:  11.5
        height: 4

        transformOrigin: Item.Right
        }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        property double xOrg
        property double yOrg

        onPressed: {
            xOrg = mouse.x
            yOrg = mouse.y
            }

        onPositionChanged: {
            var dx = mouse.x - xOrg
            var dy = mouse.y - yOrg
            if (dy > 0 && dy > dx)
                dx = dy;
            else if (dy < 0 && dy < dx)
                dx = dy;

            var v = rotary.value
            v = v - dx * .01;
            if (v < 0)
                v = 0
            else if (v > 1.0)
                v = 1.0
            rotary.value = v
            xOrg = mouse.x
            yOrg = mouse.y
            }
        onWheel: {
            var v = rotary.value
            v = v - wheel.angleDelta.y * .0005;
            if (v < 0)
                v = 0
            else if (v > 1.0)
                v = 1.0
            rotary.value = v
            }
        }
    }

