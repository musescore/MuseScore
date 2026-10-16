import QtQuick
import QtQuick.Controls

Slider {
    signal adjusting(bool active)
    onPressedChanged:adjusting(pressed)
    implicitWidth:200
}
