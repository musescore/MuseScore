import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

GridView {
    id: root
    
    property var currentSignature: null
    signal signatureSelected(var signature)

    height: contentHeight

    clip: true
    
    cellWidth: 82
    cellHeight: 90
    
    interactive: height < contentHeight
    
    delegate: ListItemBlank {
        height: root.cellHeight
        width: root.cellWidth

        radius: 3
        isSelected: modelData.title === currentSignature.title

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

                text: modelData.title
            }
        }
        
        onClicked: {
            root.signatureSelected(modelData)
        }
    }
}
