import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"

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
        
        // Motif Library Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            
            Text {
                text: "Motif Library"
                color: "white"
                font.pixelSize: 18
                font.bold: true
            }
            
            Text {
                text: "(" + (sketchData && sketchData.motifs ? sketchData.motifs.length : 0) + ")"
                color: "#AAAAAA"
                font.pixelSize: 18
            }
            
            Item { Layout.fillWidth: true }
            
            Text {
                text: "Long-press for options"
                color: "#666666"
                font.pixelSize: 12
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
                id: motifDelegate
                width: motifsListView.width
                height: 100
                color: motifMouseArea.pressed ? "#3D3D3D" : "#2D2D2D"
                radius: 8
                
                property var motif: modelData
                property bool showActions: false
                
                MouseArea {
                    id: motifMouseArea
                    anchors.fill: parent
                    
                    onClicked: {
                        if (!motifDelegate.showActions) {
                            root.editMotifRequested(modelData.id)
                        }
                    }
                    
                    onPressAndHold: {
                        motifDelegate.showActions = !motifDelegate.showActions
                    }
                }
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12
                    
                    // Mini motif preview
                    Rectangle {
                        Layout.preferredWidth: 76
                        Layout.preferredHeight: 76
                        radius: 8
                        color: "#1E1E1E"
                        
                        Canvas {
                            id: miniPreview
                            anchors.fill: parent
                            anchors.margins: 4
                            
                            onPaint: {
                                var ctx = getContext("2d");
                                ctx.clearRect(0, 0, width, height);
                                
                                var pitches = modelData.pitchContour || [];
                                var cells = modelData.rhythmCells || [];
                                if (pitches.length === 0) return;
                                
                                var stepY = height / 8;
                                
                                // Build note positions accounting for rests
                                var notePositions = [];
                                var totalCells = cells.length > 0 ? cells.length : pitches.length;
                                var pitchIndex = 0;
                                
                                for (var c = 0; c < totalCells; c++) {
                                    var isRest = cells.length > 0 && cells[c] && cells[c].isRest;
                                    var x = totalCells > 1 ? (c / (totalCells - 1)) * width : width / 2;
                                    
                                    if (isRest) {
                                        notePositions.push({ x: x, isRest: true });
                                    } else if (pitchIndex < pitches.length) {
                                        var y = height - (pitches[pitchIndex] * stepY);
                                        notePositions.push({ x: x, y: y, isRest: false, pitch: pitches[pitchIndex] });
                                        pitchIndex++;
                                    }
                                }
                                
                                // Draw lines between notes (skip rests)
                                ctx.strokeStyle = "#4A90E2";
                                ctx.lineWidth = 2;
                                
                                var lastNotePos = null;
                                for (var i = 0; i < notePositions.length; i++) {
                                    var pos = notePositions[i];
                                    if (!pos.isRest) {
                                        if (lastNotePos !== null) {
                                            ctx.beginPath();
                                            ctx.moveTo(lastNotePos.x, lastNotePos.y);
                                            ctx.lineTo(pos.x, pos.y);
                                            ctx.stroke();
                                        }
                                        lastNotePos = pos;
                                    } else {
                                        // Rest breaks the line continuity
                                        lastNotePos = null;
                                    }
                                }
                                
                                // Draw dots for notes and rest indicators
                                for (var j = 0; j < notePositions.length; j++) {
                                    var np = notePositions[j];
                                    if (np.isRest) {
                                        // Draw small rest indicator
                                        ctx.fillStyle = "#666666";
                                        ctx.fillRect(np.x - 3, height / 2 - 1, 6, 2);
                                    } else {
                                        ctx.fillStyle = "#4A90E2";
                                        ctx.beginPath();
                                        ctx.arc(np.x, np.y, 3, 0, 2 * Math.PI);
                                        ctx.fill();
                                    }
                                }
                            }
                            
                            Component.onCompleted: requestPaint()
                        }
                    }
                    
                    // Motif info
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4
                        visible: !motifDelegate.showActions
                        
                        Text {
                            text: modelData.name
                            color: "white"
                            font.pixelSize: 16
                            font.bold: true
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        
                        Text {
                            text: modelData.pitchContour.length + " notes • " + modelData.lengthBars + (modelData.lengthBars === 1 ? " bar" : " bars")
                            color: "#AAAAAA"
                            font.pixelSize: 14
                        }
                        
                        // Rhythm display
                        Row {
                            spacing: 4
                            
                            Repeater {
                                model: Math.min(modelData.rhythmCells ? modelData.rhythmCells.length : 0, 8)
                                
                                Text {
                                    text: {
                                        var cell = modelData.rhythmCells[index];
                                        if (!cell) return "";
                                        if (cell.isRest) return "—";
                                        switch(cell.duration) {
                                            case "whole": return "○";
                                            case "half": return "◐";
                                            case "quarter": return "●";
                                            case "eighth": return "♪";
                                            case "sixteenth": return "♬";
                                            default: return "●";
                                        }
                                    }
                                    color: "#666666"
                                    font.pixelSize: 12
                                }
                            }
                            
                            Text {
                                visible: modelData.rhythmCells && modelData.rhythmCells.length > 8
                                text: "..."
                                color: "#666666"
                                font.pixelSize: 12
                            }
                        }
                    }
                    
                    // Action buttons (shown on long press)
                    RowLayout {
                        visible: motifDelegate.showActions
                        Layout.fillWidth: true
                        spacing: 8
                        
                        Button {
                            Layout.fillWidth: true
                            text: "Rename"
                            font.pixelSize: 12
                            
                            onClicked: {
                                renameDialog.motifId = modelData.id
                                renameDialog.currentName = modelData.name
                                renameDialog.open()
                                motifDelegate.showActions = false
                            }
                            
                            background: Rectangle {
                                radius: 6
                                color: parent.pressed ? "#5A5AE2" : "#4A4AE2"
                            }
                            
                            contentItem: Text {
                                text: parent.text
                                color: "white"
                                font: parent.font
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                        
                        Button {
                            Layout.fillWidth: true
                            text: "Duplicate"
                            font.pixelSize: 12
                            
                            onClicked: {
                                sketchManager.duplicateMotif(root.sketchId, modelData.id)
                                loadSketch()
                                motifDelegate.showActions = false
                            }
                            
                            background: Rectangle {
                                radius: 6
                                color: parent.pressed ? "#3A9A3A" : "#2A8A2A"
                            }
                            
                            contentItem: Text {
                                text: parent.text
                                color: "white"
                                font: parent.font
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                        
                        Button {
                            Layout.fillWidth: true
                            text: "Delete"
                            font.pixelSize: 12
                            
                            onClicked: {
                                deleteDialog.motifId = modelData.id
                                deleteDialog.motifName = modelData.name
                                deleteDialog.open()
                                motifDelegate.showActions = false
                            }
                            
                            background: Rectangle {
                                radius: 6
                                color: parent.pressed ? "#CC3333" : "#AA2222"
                            }
                            
                            contentItem: Text {
                                text: parent.text
                                color: "white"
                                font: parent.font
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }
                    
                    // Arrow indicator (when not showing actions)
                    Text {
                        visible: !motifDelegate.showActions
                        text: "›"
                        color: "#666666"
                        font.pixelSize: 24
                    }
                    
                    // Close actions button
                    Text {
                        visible: motifDelegate.showActions
                        text: "✕"
                        color: "#888888"
                        font.pixelSize: 18
                        
                        MouseArea {
                            anchors.fill: parent
                            anchors.margins: -8
                            onClicked: motifDelegate.showActions = false
                        }
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
    
    // Rename Dialog
    Dialog {
        id: renameDialog
        anchors.centerIn: parent
        width: 280
        title: "Rename Motif"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        
        property string motifId: ""
        property string currentName: ""
        
        TextField {
            id: renameField
            width: parent.width
            text: renameDialog.currentName
            placeholderText: "Motif name"
            selectByMouse: true
            
            onAccepted: renameDialog.accept()
        }
        
        onAccepted: {
            if (renameField.text.trim() !== "") {
                sketchManager.renameMotif(root.sketchId, motifId, renameField.text.trim())
                loadSketch()
            }
        }
        
        onOpened: {
            renameField.text = currentName
            renameField.forceActiveFocus()
            renameField.selectAll()
        }
    }
    
    // Delete Confirmation Dialog
    Dialog {
        id: deleteDialog
        anchors.centerIn: parent
        width: 280
        title: "Delete Motif"
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
        
        property string motifId: ""
        property string motifName: ""
        
        Text {
            width: parent.width
            text: "Are you sure you want to delete \"" + deleteDialog.motifName + "\"?\n\nThis action cannot be undone."
            color: "#CCCCCC"
            wrapMode: Text.WordWrap
        }
        
        onAccepted: {
            sketchManager.deleteMotif(root.sketchId, motifId)
            loadSketch()
        }
    }
    
    // Reload sketch data when we return to this view
    onVisibleChanged: {
        if (visible) {
            loadSketch()
        }
    }
}
