import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

GridView {
    id: root
    
    property var currentSignature: null

    anchors.left: parent.left
    anchors.right: parent.right

    height: contentHeight

    clip: true
    
    cellWidth: 82
    cellHeight: 90
    
    interactive: height < contentHeight
    
    signal signatureSelected(var signature)

    delegate: Item {
        height: root.cellHeight
        width: root.cellWidth
        
        property bool isCurrent: modelData.title === currentSignature.title
        
        Rectangle {
            anchors.fill: parent
            
            color: isCurrent ? ui.theme.accentColor : ui.theme.backgroundPrimaryColor
            opacity: isCurrent ? 0.7 : 1
            radius: 3
        }
        
        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            spacing: 10
            
            StyledIconLabel {
                anchors.horizontalCenter: parent.horizontalCenter
                height: 50
                
                font.pixelSize: 65
                iconCode: modelData.icon
            }
            
            StyledTextLabel {
                anchors.horizontalCenter: parent.horizontalCenter

                font.pixelSize: 12
                text: modelData.title
            }
        }
        
        MouseArea {
            anchors.fill: parent
            
            onClicked: {
                signatureSelected(modelData)
            }
        }
    }
}
