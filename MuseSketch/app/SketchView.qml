import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root
    
    property string sketchId: ""
    property var sketchData: null
    
    signal backRequested()
    signal createMotifRequested()
    signal editMotifRequested(string motifId)
    
    Component.onCompleted: {
        loadSketch()
    }
    
    function loadSketch() {
        sketchData = sketchManager.getSketch(sketchId)
    }
    
    header: ToolBar {
        background: Rectangle {
            color: "#2D2D2D"
        }
        
        RowLayout {
            anchors.fill: parent
            spacing: 8
            
            ToolButton {
                text: "←"
                font.pixelSize: 24
                onClicked: root.backRequested()
            }
            
            Text {
                Layout.fillWidth: true
                text: sketchData ? sketchData.name : "Sketch"
                color: "white"
                font.pixelSize: 20
                font.bold: true
            }
        }
    }
    
    background: Rectangle {
        color: "#1E1E1E"
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 20
        
        // Sketch Info
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 100
            color: "#2D2D2D"
            radius: 8
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 8
                
                Text {
                    text: "Sketch Settings"
                    color: "#AAAAAA"
                    font.pixelSize: 14
                }
                
                Row {
                    spacing: 20
                    
                    Text {
                        text: "Key: " + (sketchData ? sketchData.key : "C major")
                        color: "white"
                        font.pixelSize: 16
                    }
                    
                    Text {
                        text: "Tempo: " + (sketchData ? sketchData.tempo : 120) + " BPM"
                        color: "white"
                        font.pixelSize: 16
                    }
                    
                    Text {
                        text: "Time: " + (sketchData ? sketchData.timeSignature : "4/4")
                        color: "white"
                        font.pixelSize: 16
                    }
                }
            }
        }
        
        // Motifs Section
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12
            
            Row {
                Layout.fillWidth: true
                spacing: 12
                
                Text {
                    text: "Motifs"
                    color: "white"
                    font.pixelSize: 18
                    font.bold: true
                }
                
                Text {
                    text: "(" + (sketchData && sketchData.motifs ? sketchData.motifs.length : 0) + ")"
                    color: "#AAAAAA"
                    font.pixelSize: 18
                }
            }
            
            // Motifs List
            ListView {
                id: motifsListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                
                model: sketchData && sketchData.motifs ? sketchData.motifs : []
                spacing: 8
                clip: true
                
                delegate: Rectangle {
                    width: motifsListView.width
                    height: 80
                    color: motifMouseArea.pressed ? "#3D3D3D" : "#2D2D2D"
                    radius: 8
                    
                    MouseArea {
                        id: motifMouseArea
                        anchors.fill: parent
                        onClicked: {
                            root.editMotifRequested(modelData.id)
                        }
                    }
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 12
                        
                        Rectangle {
                            Layout.preferredWidth: 56
                            Layout.preferredHeight: 56
                            radius: 8
                            color: "#4A90E2"
                            
                            Text {
                                anchors.centerIn: parent
                                text: "♪"
                                color: "white"
                                font.pixelSize: 28
                            }
                        }
                        
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            
                            Text {
                                text: modelData.name
                                color: "white"
                                font.pixelSize: 16
                                font.bold: true
                            }
                            
                            Text {
                                text: modelData.pitchContour.length + " notes"
                                color: "#AAAAAA"
                                font.pixelSize: 14
                            }
                        }
                        
                        Text {
                            text: "›"
                            color: "#666666"
                            font.pixelSize: 24
                        }
                    }
                }
                
                // Empty state
                Rectangle {
                    anchors.fill: parent
                    visible: motifsListView.count === 0
                    color: "#2D2D2D"
                    radius: 8
                    
                    Text {
                        anchors.centerIn: parent
                        text: "No motifs yet\nTap + to create one"
                        color: "#666666"
                        font.pixelSize: 16
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }
        
        // Create Motif Button
        Button {
            Layout.alignment: Qt.AlignHCenter
            width: 200
            height: 50
            
            text: "+ Create Motif"
            font.pixelSize: 16
            font.bold: true
            
            onClicked: root.createMotifRequested()
            
            background: Rectangle {
                radius: 25
                color: parent.pressed ? "#3A7BC8" : "#4A90E2"
            }
            
            contentItem: Text {
                text: parent.text
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font: parent.font
            }
        }
    }
    
    // Reload sketch data when we return to this view
    onVisibleChanged: {
        if (visible) {
            loadSketch()
        }
    }
}
