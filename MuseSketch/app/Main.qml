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
                    Rectangle {
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                        Layout.bottomMargin: 20
                        width: 64
                        height: 64
                        radius: 32
                        color: fabMouseArea.pressed ? "#3A7BC8" : "#4A90E2"
                        
                        layer.enabled: true
                        layer.effect: DropShadow {
                            horizontalOffset: 0
                            verticalOffset: 4
                            radius: 8
                            samples: 17
                            color: "#80000000"
                        }
                        
                        Text {
                            anchors.centerIn: parent
                            anchors.verticalCenterOffset: -2  // Adjust for font baseline
                            text: "+"
                            color: "white"
                            font.pixelSize: 32
                            font.weight: Font.Light
                        }
                        
                        MouseArea {
                            id: fabMouseArea
                            anchors.fill: parent
                            onClicked: newSketchDialog.open()
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
                onEditSectionRequested: function(sectionId) {
                    stackView.push(sectionEditorViewComponent, { sketchId: sketchId, sectionId: sectionId })
                }
                onExportRequested: {
                    stackView.push(exportViewComponent, { 
                        sketchId: sketchId, 
                        sketchName: sketchData ? sketchData.name : "Sketch" 
                    })
                }
            }
        }
        
        // Export View Component
        Component {
            id: exportViewComponent
            
            ExportView {
                onBack: stackView.pop()
            }
        }
        
        // Section Editor View Component
        Component {
            id: sectionEditorViewComponent
            
            SectionEditorView {
                onBackRequested: stackView.pop()
                onSectionUpdated: {
                    sketchManager.refreshSketches()
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
    
    // New Sketch Dialog Overlay
    Rectangle {
        id: newSketchDialogOverlay
        anchors.fill: parent
        color: "#80000000"
        visible: newSketchDialog.visible
        z: 100
        
        MouseArea {
            anchors.fill: parent
            onClicked: newSketchDialog.close()
        }
    }
    
    // New Sketch Dialog
    Rectangle {
        id: newSketchDialog
        anchors.centerIn: parent
        width: 300
        height: 180
        radius: 12
        color: "#2D2D2D"
        border.color: "#4A4A4A"
        border.width: 1
        visible: false
        z: 101
        
        function open() { 
            sketchNameField.text = ""
            visible = true
            sketchNameField.forceActiveFocus()
        }
        function close() { visible = false }
        function accept() { 
            if (sketchNameField.text.trim() !== "") {
                sketchManager.createNewSketch(sketchNameField.text.trim())
                sketchNameField.text = ""
                close()
            }
        }
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 16
            
            Text {
                text: "New Sketch"
                color: "white"
                font.pixelSize: 18
                font.bold: true
            }
            
            TextField {
                id: sketchNameField
                Layout.fillWidth: true
                Layout.preferredHeight: 44
                leftPadding: 12
                topPadding: 12
                bottomPadding: 12
                placeholderText: "Sketch name"
                placeholderTextColor: "#888888"
                color: "white"
                font.pixelSize: 16
                selectByMouse: true
                
                background: Rectangle {
                    color: "#1E1E1E"
                    radius: 8
                    border.color: sketchNameField.activeFocus ? "#4A90E2" : "#4A4A4A"
                    border.width: 1
                }
                
                onAccepted: newSketchDialog.accept()
            }
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                
                Button {
                    Layout.fillWidth: true
                    implicitHeight: 40
                    text: "Cancel"
                    
                    onClicked: newSketchDialog.close()
                    
                    background: Rectangle {
                        radius: 8
                        color: parent.pressed ? "#3D3D3D" : "#2D2D2D"
                        border.color: "#4A4A4A"
                        border.width: 1
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 14
                    }
                }
                
                Button {
                    Layout.fillWidth: true
                    implicitHeight: 40
                    text: "Create"
                    
                    onClicked: newSketchDialog.accept()
                    
                    background: Rectangle {
                        radius: 8
                        color: parent.pressed ? "#3A7BC8" : "#4A90E2"
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 14
                        font.bold: true
                    }
                }
            }
        }
    }
}
