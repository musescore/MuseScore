import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

ApplicationWindow {
    visible: true
    width: 400
    height: 600
    title: "MuseSketch"
    
    color: "#1E1E1E"
    
    StackView {
        id: stackView
        anchors.fill: parent
        
        initialItem: sketchListPage
        
        // Sketch List Page
        Component {
            id: sketchListPage
            
            Page {
                background: Rectangle {
                    color: "#1E1E1E"
                }
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 0
                    spacing: 0
                    
                    // Header
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 80
                        color: "#2D2D2D"
                        
                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 4
                            
                            Text {
                                Layout.alignment: Qt.AlignHCenter
                                text: "MuseSketch"
                                color: "white"
                                font.pixelSize: 28
                                font.bold: true
                            }
                            
                            Text {
                                Layout.alignment: Qt.AlignHCenter
                                text: "Your Sketches"
                                color: "#AAAAAA"
                                font.pixelSize: 14
                            }
                        }
                    }
                    
                    // Sketch List
                    ListView {
                        id: sketchListView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        
                        model: sketchManager.sketches
                        spacing: 1
                        
                        delegate: Rectangle {
                            width: sketchListView.width
                            height: 80
                            color: mouseArea.pressed ? "#3D3D3D" : "#2D2D2D"
                            
                            MouseArea {
                                id: mouseArea
                                anchors.fill: parent
                                onClicked: {
                                    stackView.push(sketchViewComponent, { sketchId: modelData.id })
                                }
                            }
                            
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 16
                                spacing: 12
                                
                                Rectangle {
                                    Layout.preferredWidth: 48
                                    Layout.preferredHeight: 48
                                    radius: 8
                                    color: "#4A90E2"
                                    
                                    Text {
                                        anchors.centerIn: parent
                                        text: modelData.name.substring(0, 1).toUpperCase()
                                        color: "white"
                                        font.pixelSize: 24
                                        font.bold: true
                                    }
                                }
                                
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4
                                    
                                    Text {
                                        text: modelData.name
                                        color: "white"
                                        font.pixelSize: 18
                                        font.bold: true
                                    }
                                    
                                    Text {
                                        text: "Modified: " + modelData.modifiedAt
                                        color: "#AAAAAA"
                                        font.pixelSize: 12
                                    }
                                }
                                
                                Button {
                                    text: "×"
                                    font.pixelSize: 24
                                    flat: true
                                    
                                    onClicked: {
                                        sketchManager.deleteSketch(modelData.id)
                                    }
                                    
                                    background: Rectangle {
                                        color: parent.pressed ? "#FF4444" : "transparent"
                                        radius: 4
                                    }
                                    
                                    contentItem: Text {
                                        text: parent.text
                                        color: parent.hovered ? "#FF6666" : "#888888"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                            }
                            
                            Rectangle {
                                anchors.bottom: parent.bottom
                                width: parent.width
                                height: 1
                                color: "#3D3D3D"
                            }
                        }
                        
                        // Empty state
                        Text {
                            anchors.centerIn: parent
                            visible: sketchListView.count === 0
                            text: "No sketches yet\nTap + to create one"
                            color: "#666666"
                            font.pixelSize: 16
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }
                    
                    // FAB Button
                    RoundButton {
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                        Layout.bottomMargin: 20
                        
                        width: 64
                        height: 64
                        text: "+"
                        font.pixelSize: 32
                        
                        background: Rectangle {
                            radius: 32
                            color: parent.pressed ? "#3A7BC8" : "#4A90E2"
                            
                            layer.enabled: true
                            layer.effect: DropShadow {
                                horizontalOffset: 0
                                verticalOffset: 4
                                radius: 8
                                samples: 17
                                color: "#80000000"
                            }
                        }
                        
                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font: parent.font
                        }
                        
                        onClicked: {
                            newSketchDialog.open()
                        }
                    }
                }
            }
        }
        
        // Sketch View Component
        Component {
            id: sketchViewComponent
            
            SketchView {
                onBackRequested: stackView.pop()
                onCreateMotifRequested: {
                    stackView.push(motifShapeViewComponent, { sketchId: sketchId })
                }
                onEditMotifRequested: function(motifId) {
                    stackView.push(motifEditViewComponent, { sketchId: sketchId, motifId: motifId })
                }
            }
        }
        
        // Motif Edit View Component
        Component {
            id: motifEditViewComponent
            
            MotifEditView {
                onBackRequested: stackView.pop()
                onMotifUpdated: {
                    sketchManager.refreshSketches()
                }
            }
        }
        
        // Motif Shape View Component
        Component {
            id: motifShapeViewComponent
            
            MotifShapeView {
                onBackRequested: stackView.pop()
                onMotifSaved: {
                    // Refresh sketch list when returning
                    sketchManager.refreshSketches()
                }
            }
        }
    }
    
    // New Sketch Dialog
    Dialog {
        id: newSketchDialog
        anchors.centerIn: parent
        width: 300
        title: "New Sketch"
        modal: true
        
        standardButtons: Dialog.Ok | Dialog.Cancel
        
        TextField {
            id: sketchNameField
            width: parent.width
            placeholderText: "Sketch name"
            selectByMouse: true
            
            onAccepted: newSketchDialog.accept()
        }
        
        onAccepted: {
            if (sketchNameField.text.trim() !== "") {
                sketchManager.createNewSketch(sketchNameField.text.trim())
                sketchNameField.text = ""
            }
        }
        
        onOpened: {
            sketchNameField.forceActiveFocus()
        }
    }
}
