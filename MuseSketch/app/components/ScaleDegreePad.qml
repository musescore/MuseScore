import QtQuick
import QtQuick.Controls

Button {
    id: root
    
    property int degree: 1
    property bool isActive: false
    
    width: 60
    height: 60
    
    text: degree.toString()
    font.pixelSize: 24
    font.bold: true
    
    background: Rectangle {
        radius: 30
        color: root.pressed ? "#3A7BC8" : (root.isActive ? "#4A90E2" : "#2D2D2D")
        border.color: "#4A90E2"
        border.width: 2
        
        Behavior on color {
            ColorAnimation { duration: 150 }
        }
    }
    
    contentItem: Text {
        text: root.text
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font: root.font
    }
}
