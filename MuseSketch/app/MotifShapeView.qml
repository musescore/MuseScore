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
            // Sync tempo with sketch tempo
            var sketchData = sketchManager.getSketch(sketchId)
            if (sketchData && sketchData.tempo) {
                motifPlayer.tempo = sketchData.tempo
            }
        }
    }
    
    Component.onDestruction: {
        // Stop playback when leaving the screen
        motifPlayer.stop()
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
                
                Row {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 8
                    
                    Repeater {
                        model: 7
                        
                        ScaleDegreePad {
                            width: 44
                            height: 44
                            font.pixelSize: 18
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
                
                Item {
                    Layout.fillWidth: true
                    implicitHeight: rhythmButtonsRow.height
                    
                    Row {
                        id: rhythmButtonsRow
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 12
                        
                        RhythmButton {
                            rhythmType: "whole"
                            displayText: "1"
                            isSelected: selectedRhythm === "whole"
                            onClicked: {
                                selectedRhythm = "whole"
                                motifEditor.addRhythm("whole")
                            }
                        }
                        
                        RhythmButton {
                            rhythmType: "half"
                            displayText: "½"
                            isSelected: selectedRhythm === "half"
                            onClicked: {
                                selectedRhythm = "half"
                                motifEditor.addRhythm("half")
                            }
                        }
                        
                        RhythmButton {
                            rhythmType: "quarter"
                            displayText: "¼"
                            isSelected: selectedRhythm === "quarter"
                            onClicked: {
                                selectedRhythm = "quarter"
                                motifEditor.addRhythm("quarter")
                            }
                        }
                        
                        RhythmButton {
                            rhythmType: "eighth"
                            displayText: "⅛"
                            isSelected: selectedRhythm === "eighth"
                            onClicked: {
                                selectedRhythm = "eighth"
                                motifEditor.addRhythm("eighth")
                            }
                        }
                    }
                }
                
                // Rest and Tie toggles
                Item {
                    Layout.fillWidth: true
                    implicitHeight: restTieRow.height
                    
                    Row {
                        id: restTieRow
                        anchors.horizontalCenter: parent.horizontalCenter
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
                Item {
                    Layout.fillWidth: true
                    implicitHeight: playbackRow.height
                    
                    Row {
                        id: playbackRow
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 12
                        
                        // Play/Stop button (circular)
                        Button {
                        implicitWidth: 50
                        implicitHeight: 50
                        padding: 0
                        enabled: motifEditor.noteCount > 0
                        
                        onClicked: {
                            if (motifPlayer.isPlaying) {
                                motifPlayer.stop()
                            } else {
                                motifPlayer.voiceType = 0  // SketchPiano
                                motifPlayer.setMotif(
                                    motifEditor.pitchContour,
                                    motifEditor.rhythmCells,
                                    motifEditor.key
                                )
                                motifPlayer.playPrerendered()  // Use pre-rendered for smooth playback
                            }
                        }
                        
                        background: Rectangle {
                            implicitWidth: 50
                            implicitHeight: 50
                            radius: width / 2
                            color: parent.pressed ? "#2E7D32" : (motifPlayer.isPlaying ? "#4CAF50" : "#4A90E2")
                        }
                        
                        contentItem: Text {
                            text: motifPlayer.isPlaying ? "■" : "▶"
                            color: parent.enabled ? "white" : "#666666"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 20
                        }
                    }
                    
                    // Loop toggle (circular)
                    Button {
                        implicitWidth: 44
                        implicitHeight: 44
                        padding: 0
                        
                        onClicked: motifPlayer.isLooping = !motifPlayer.isLooping
                        
                        background: Rectangle {
                            implicitWidth: 44
                            implicitHeight: 44
                            radius: width / 2
                            color: parent.pressed ? "#3D3D3D" : (motifPlayer.isLooping ? "#FF9800" : "#2D2D2D")
                            border.color: motifPlayer.isLooping ? "#FF9800" : "#3D3D3D"
                            border.width: 2
                        }
                        
                        contentItem: Text {
                            text: "⟲"
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 18
                        }
                    }
                    
                    // Metronome toggle (circular)
                    Button {
                        implicitWidth: 44
                        implicitHeight: 44
                        padding: 0
                        
                        onClicked: motifPlayer.metronomeEnabled = !motifPlayer.metronomeEnabled
                        
                        background: Rectangle {
                            implicitWidth: 44
                            implicitHeight: 44
                            radius: width / 2
                            color: parent.pressed ? "#3D3D3D" : (motifPlayer.metronomeEnabled ? "#9C27B0" : "#2D2D2D")
                            border.color: motifPlayer.metronomeEnabled ? "#9C27B0" : "#3D3D3D"
                            border.width: 2
                        }
                        
                        contentItem: Text {
                            text: "♩"
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 16
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
    Rectangle {
        id: saveDialogOverlay
        anchors.fill: parent
        color: "#80000000"
        visible: saveDialog.visible
        
        MouseArea {
            anchors.fill: parent
            onClicked: saveDialog.close()
        }
    }
    
    Rectangle {
        id: saveDialog
        anchors.centerIn: parent
        width: 300
        height: 180
        radius: 12
        color: "#2D2D2D"
        border.color: "#4A4A4A"
        border.width: 1
        visible: false
        
        property bool isOpen: visible
        
        function open() { visible = true; motifNameField.forceActiveFocus() }
        function close() { visible = false }
        function accept() { 
            motifEditor.saveMotif(motifNameField.text.trim())
            motifNameField.text = ""
            root.motifSaved()
            root.backRequested()
            close()
        }
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 16
            
            Text {
                text: "Save Motif"
                color: "white"
                font.pixelSize: 18
                font.bold: true
            }
            
            TextField {
                id: motifNameField
                Layout.fillWidth: true
                Layout.preferredHeight: 44
                leftPadding: 12
                topPadding: 12
                bottomPadding: 12
                placeholderText: "Motif name (optional)"
                placeholderTextColor: "#888888"
                color: "white"
                font.pixelSize: 16
                selectByMouse: true
                
                background: Rectangle {
                    color: "#1E1E1E"
                    radius: 8
                    border.color: motifNameField.activeFocus ? "#4A90E2" : "#4A4A4A"
                    border.width: 1
                }
                
                onAccepted: saveDialog.accept()
            }
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                
                Button {
                    Layout.fillWidth: true
                    implicitHeight: 40
                    text: "Cancel"
                    
                    onClicked: saveDialog.close()
                    
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
                    text: "Save"
                    
                    onClicked: saveDialog.accept()
                    
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
    
    property string selectedRhythm: "quarter"
}
