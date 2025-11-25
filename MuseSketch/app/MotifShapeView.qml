import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"

Page {
    id: root
    
    property string sketchId: ""
    
    signal backRequested()
    signal motifSaved()
    
    Component.onCompleted: {
        if (sketchId) {
            motifEditor.startNewMotif(sketchId)
        }
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
                text: "Shape Motif"
                color: "white"
                font.pixelSize: 20
                font.bold: true
            }
        }
    }
    
    background: Rectangle {
        color: "#1E1E1E"
    }
    
    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true
        
        ColumnLayout {
            width: parent.width
            spacing: 20
            
            Item { height: 16 }  // Top margin
            
            // Motif Length Selector
            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                spacing: 8
                
                Text {
                    text: "Motif Length"
                    color: "#AAAAAA"
                    font.pixelSize: 14
                }
                
                Row {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 8
                    
                    Repeater {
                        model: [1, 2, 4]
                        
                        Button {
                            width: 70
                            height: 36
                            text: modelData + (modelData === 1 ? " bar" : " bars")
                            
                            property bool isSelected: motifEditor.motifBars === modelData
                            
                            onClicked: motifEditor.motifBars = modelData
                            
                            background: Rectangle {
                                radius: 6
                                color: parent.pressed ? "#3D3D3D" : (parent.isSelected ? "#4A90E2" : "#2D2D2D")
                                border.color: parent.isSelected ? "#4A90E2" : "#3D3D3D"
                                border.width: 1
                            }
                            
                            contentItem: Text {
                                text: parent.text
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                font.pixelSize: 12
                            }
                        }
                    }
                }
            }
            
            // Motif Display
            MotifDisplay {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                Layout.preferredHeight: 200
                pitchContour: motifEditor.pitchContour
                rhythmPattern: motifEditor.rhythmPattern
                rhythmCells: motifEditor.rhythmCells
                motifBars: motifEditor.motifBars
            }
            
            // Contour Canvas (Draw Mode)
            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                spacing: 12
                
                Text {
                    text: "Draw Contour (or use pads below)"
                    color: "#AAAAAA"
                    font.pixelSize: 14
                }
                
                ContourCanvas {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 150
                    motifBars: motifEditor.motifBars
                    
                    onContourDrawn: function(points) {
                        motifEditor.applyContourPath(points)
                    }
                }
            }
            
            // Scale Degree Pads
            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                spacing: 12
                
                Text {
                    text: "Scale Degrees (Tap Mode)"
                    color: "#AAAAAA"
                    font.pixelSize: 14
                }
                
                GridLayout {
                    Layout.alignment: Qt.AlignHCenter
                    columns: 4
                    rowSpacing: 12
                    columnSpacing: 12
                    
                    Repeater {
                        model: 7
                        
                        ScaleDegreePad {
                            degree: index + 1
                            onClicked: {
                                motifEditor.addNote(degree)
                                motifPlayer.playPreviewNote(degree, motifEditor.key)
                            }
                        }
                    }
                }
            }
            
            // Rhythm Selector
            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                spacing: 12
                
                Text {
                    text: "Rhythm (for last note)"
                    color: "#AAAAAA"
                    font.pixelSize: 14
                }
                
                Row {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 12
                    
                    RhythmButton {
                        rhythmType: "quarter"
                        displayText: "♩"
                        isSelected: selectedRhythm === "quarter"
                        onClicked: {
                            selectedRhythm = "quarter"
                            motifEditor.addRhythm("quarter")
                        }
                    }
                    
                    RhythmButton {
                        rhythmType: "eighth"
                        displayText: "♪"
                        isSelected: selectedRhythm === "eighth"
                        onClicked: {
                            selectedRhythm = "eighth"
                            motifEditor.addRhythm("eighth")
                        }
                    }
                    
                    RhythmButton {
                        rhythmType: "dotted-quarter"
                        displayText: "♩."
                        isSelected: selectedRhythm === "dotted-quarter"
                        onClicked: {
                            selectedRhythm = "dotted-quarter"
                            motifEditor.addRhythm("dotted-quarter")
                        }
                    }
                    
                    RhythmButton {
                        rhythmType: "half"
                        displayText: "𝅗𝅥"
                        isSelected: selectedRhythm === "half"
                        onClicked: {
                            selectedRhythm = "half"
                            motifEditor.addRhythm("half")
                        }
                    }
                }
                
                // Rest and Tie toggles
                Row {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 16
                    
                    Button {
                        width: 100
                        height: 40
                        enabled: motifEditor.noteCount > 0
                        
                        property bool isRest: motifEditor.noteCount > 0 ? motifEditor.isRest(motifEditor.noteCount - 1) : false
                        
                        onClicked: {
                            if (motifEditor.noteCount > 0) {
                                motifEditor.toggleRest(motifEditor.noteCount - 1)
                            }
                        }
                        
                        background: Rectangle {
                            radius: 8
                            color: parent.pressed ? "#3D3D3D" : (parent.isRest ? "#E67E22" : "#2D2D2D")
                            border.color: parent.isRest ? "#E67E22" : "#3D3D3D"
                            border.width: 2
                        }
                        
                        contentItem: Text {
                            text: "𝄽 Rest"
                            color: parent.enabled ? "white" : "#666666"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 14
                        }
                    }
                    
                    Button {
                        width: 100
                        height: 40
                        enabled: motifEditor.noteCount > 1
                        
                        property bool isTied: motifEditor.noteCount > 1 ? motifEditor.isTied(motifEditor.noteCount - 2) : false
                        
                        onClicked: {
                            if (motifEditor.noteCount > 1) {
                                motifEditor.toggleTie(motifEditor.noteCount - 2)
                            }
                        }
                        
                        background: Rectangle {
                            radius: 8
                            color: parent.pressed ? "#3D3D3D" : (parent.isTied ? "#9B59B6" : "#2D2D2D")
                            border.color: parent.isTied ? "#9B59B6" : "#3D3D3D"
                            border.width: 2
                        }
                        
                        contentItem: Text {
                            text: "⁀ Tie"
                            color: parent.enabled ? "white" : "#666666"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 14
                        }
                    }
                }
            }
            
            // Playback Controls
            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                Layout.topMargin: 12
                spacing: 12
                
                Text {
                    text: "Playback"
                    color: "#AAAAAA"
                    font.pixelSize: 14
                }
                
                // Play/Stop and Tempo
                Row {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 12
                    
                    // Play/Stop button
                    Button {
                        width: 60
                        height: 44
                        enabled: motifEditor.noteCount > 0
                        
                        onClicked: {
                            if (motifPlayer.isPlaying) {
                                motifPlayer.stop()
                            } else {
                                motifPlayer.setMotif(
                                    motifEditor.pitchContour,
                                    motifEditor.rhythmCells,
                                    motifEditor.key
                                )
                                motifPlayer.play()
                            }
                        }
                        
                        background: Rectangle {
                            radius: 8
                            color: parent.pressed ? "#2E7D32" : (motifPlayer.isPlaying ? "#4CAF50" : "#2D2D2D")
                            border.color: motifPlayer.isPlaying ? "#4CAF50" : "#3D3D3D"
                            border.width: 2
                        }
                        
                        contentItem: Text {
                            text: motifPlayer.isPlaying ? "■" : "▶"
                            color: parent.enabled ? "white" : "#666666"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 20
                        }
                    }
                    
                    // Loop toggle
                    Button {
                        width: 50
                        height: 44
                        
                        onClicked: motifPlayer.isLooping = !motifPlayer.isLooping
                        
                        background: Rectangle {
                            radius: 8
                            color: parent.pressed ? "#3D3D3D" : (motifPlayer.isLooping ? "#FF9800" : "#2D2D2D")
                            border.color: motifPlayer.isLooping ? "#FF9800" : "#3D3D3D"
                            border.width: 2
                        }
                        
                        contentItem: Text {
                            text: "⟲"
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 20
                        }
                    }
                    
                    // Metronome toggle
                    Button {
                        width: 50
                        height: 44
                        
                        onClicked: motifPlayer.metronomeEnabled = !motifPlayer.metronomeEnabled
                        
                        background: Rectangle {
                            radius: 8
                            color: parent.pressed ? "#3D3D3D" : (motifPlayer.metronomeEnabled ? "#9C27B0" : "#2D2D2D")
                            border.color: motifPlayer.metronomeEnabled ? "#9C27B0" : "#3D3D3D"
                            border.width: 2
                        }
                        
                        contentItem: Text {
                            text: "♩"
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 18
                        }
                    }
                    
                    // Tempo display/control
                    Row {
                        spacing: 4
                        
                        Button {
                            width: 30
                            height: 44
                            text: "-"
                            onClicked: motifPlayer.tempo = Math.max(40, motifPlayer.tempo - 10)
                            
                            background: Rectangle {
                                radius: 4
                                color: parent.pressed ? "#3D3D3D" : "#2D2D2D"
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                font.pixelSize: 18
                            }
                        }
                        
                        Rectangle {
                            width: 60
                            height: 44
                            color: "#2D2D2D"
                            radius: 4
                            
                            Text {
                                anchors.centerIn: parent
                                text: motifPlayer.tempo + "\nbpm"
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                font.pixelSize: 12
                                lineHeight: 0.9
                            }
                        }
                        
                        Button {
                            width: 30
                            height: 44
                            text: "+"
                            onClicked: motifPlayer.tempo = Math.min(240, motifPlayer.tempo + 10)
                            
                            background: Rectangle {
                                radius: 4
                                color: parent.pressed ? "#3D3D3D" : "#2D2D2D"
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                font.pixelSize: 18
                            }
                        }
                    }
                }
            }
            
            // Action Buttons
            Row {
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 20
                Layout.bottomMargin: 20
                spacing: 12
                
                Button {
                    text: "Clear"
                    width: 100
                    enabled: motifEditor.noteCount > 0
                    
                    onClicked: motifEditor.clear()
                    
                    background: Rectangle {
                        radius: 8
                        color: parent.pressed ? "#3D3D3D" : "#2D2D2D"
                        border.color: "#4A4A4A"
                        border.width: 1
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        color: parent.enabled ? "white" : "#666666"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                
                Button {
                    text: "Save Motif"
                    width: 150
                    enabled: motifEditor.noteCount > 0
                    
                    onClicked: saveDialog.open()
                    
                    background: Rectangle {
                        radius: 8
                        color: parent.pressed ? "#3A7BC8" : (parent.enabled ? "#4A90E2" : "#2D2D2D")
                        border.color: parent.enabled ? "#4A90E2" : "#4A4A4A"
                        border.width: 1
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.bold: true
                    }
                }
            }
            
            Item { height: 20 }  // Bottom margin
        }
    }
    
    // Save Dialog
    Dialog {
        id: saveDialog
        anchors.centerIn: parent
        width: 300
        title: "Save Motif"
        modal: true
        
        standardButtons: Dialog.Save | Dialog.Cancel
        
        TextField {
            id: motifNameField
            width: parent.width
            placeholderText: "Motif name (optional)"
            selectByMouse: true
            
            onAccepted: saveDialog.accept()
        }
        
        onAccepted: {
            motifEditor.saveMotif(motifNameField.text.trim())
            motifNameField.text = ""
            root.motifSaved()
            root.backRequested()
        }
        
        onOpened: {
            motifNameField.forceActiveFocus()
        }
    }
    
    property string selectedRhythm: "quarter"
}
