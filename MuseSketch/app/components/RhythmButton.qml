import QtQuick
import QtQuick.Controls

Button {
    id: root
    
    property string rhythmType: "quarter"
    property string displayText: "♩"
    property bool isSelected: false
    
    width: 70
    height: 50
    
    text: displayText
    font.pixelSize: 28
    
    background: Rectangle {
        radius: 8
        color: root.pressed ? "#3D3D3D" : (root.isSelected ? "#4A90E2" : "#2D2D2D")
        border.color: root.isSelected ? "#4A90E2" : "#3D3D3D"
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
