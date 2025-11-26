import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"

Page {
    id: root
    
    property string sketchId: ""
    property string motifId: ""
    property var motifData: null
    
    // Editable copies of the motif data
    property var editablePitchContour: []
    property var editableRhythmCells: []
    
    signal backRequested()
    signal motifUpdated()
    
    Component.onCompleted: {
        loadMotif()
    }
    
    Component.onDestruction: {
        // Stop playback when leaving the screen
        motifPlayer.stop()
    }
    
    function loadMotif() {
        motifData = sketchManager.getMotif(sketchId, motifId)
        if (motifData) {
            // Create editable copies
            editablePitchContour = motifData.pitchContour.slice()
            editableRhythmCells = JSON.parse(JSON.stringify(motifData.rhythmCells || []))
            
            // If no rhythm cells but we have pitches, create default quarter notes
            if (editableRhythmCells.length === 0 && editablePitchContour.length > 0) {
                for (var i = 0; i < editablePitchContour.length; i++) {
                    editableRhythmCells.push({
                        duration: "quarter",
                        tie: false,
                        isRest: false
                    })
                }
            }
        }
    }
    
    function saveMotif() {
        if (motifData) {
            sketchManager.updateMotifPitches(sketchId, motifId, editablePitchContour)
            motifUpdated()
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
                text: motifData ? "Edit: " + motifData.name : "Edit Motif"
                color: "white"
                font.pixelSize: 20
                font.bold: true
            }
            
            ToolButton {
                text: "✓"
                font.pixelSize: 24
                onClicked: {
                    saveMotif()
                    root.backRequested()
                }
            }
        }
    }
    
    background: Rectangle {
        color: "#1E1E1E"
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16
        
        // Instructions
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: "Tap a note and drag up/down to change pitch"
            color: "#AAAAAA"
            font.pixelSize: 14
        }
        
        // Interactive Pitch Editor
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 300
            color: "#2D2D2D"
            radius: 8
            border.color: "#3D3D3D"
            border.width: 1
            
            // Scale degree labels on the left
            Column {
                id: pitchLabels
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.margins: 8
                width: 30
                
                Repeater {
                    model: 7
                    
                    Text {
                        width: pitchLabels.width
                        height: (parent.height - 16) / 7
                        text: (7 - index)
                        color: "#666666"
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
            
            // Interactive canvas area
            Item {
                id: canvasArea
                anchors.left: pitchLabels.right
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.margins: 8
                anchors.leftMargin: 4
                
                // Background canvas for grid and connections
                Canvas {
                    id: backgroundCanvas
                    anchors.fill: parent
                    
                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.clearRect(0, 0, width, height);
                        
                        var cells = root.editableRhythmCells;
                        var pitches = root.editablePitchContour;
                        
                        if (pitches.length === 0) return;
                        
                        // Helper function to convert duration string to beats
                        function durationToBeats(dur) {
                            switch(dur) {
                                case "whole": return 4.0;
                                case "dotted-half": return 3.0;
                                case "half": return 2.0;
                                case "dotted-quarter": return 1.5;
                                case "quarter": return 1.0;
                                case "dotted-eighth": return 0.75;
                                case "eighth": return 0.5;
                                case "sixteenth": return 0.25;
                                default: return 1.0;
                            }
                        }
                        
                        // Calculate total beats
                        var totalBeats = 0;
                        if (cells.length > 0) {
                            for (var c = 0; c < cells.length; c++) {
                                totalBeats += durationToBeats(cells[c].duration || "quarter");
                            }
                        } else {
                            totalBeats = pitches.length;
                        }
                        
                        var padding = 20;
                        var drawWidth = width - (padding * 2);
                        var pixelsPerBeat = totalBeats > 0 ? drawWidth / totalBeats : 0;
                        var stepY = height / 8;
                        
                        // Draw horizontal grid lines for each scale degree
                        ctx.strokeStyle = "#3A3A3A";
                        ctx.lineWidth = 1;
                        for (var d = 1; d <= 7; d++) {
                            var gridY = height - (d * stepY);
                            ctx.beginPath();
                            ctx.moveTo(0, gridY);
                            ctx.lineTo(width, gridY);
                            ctx.stroke();
                        }
                        
                        // Draw vertical measure lines
                        var motifBars = root.motifData ? (root.motifData.lengthBars || 1) : 1;
                        var beatsPerBar = 4; // Assuming 4/4 time signature
                        var gridTotalBeats = motifBars * beatsPerBar;
                        var gridPixelsPerBeat = drawWidth / gridTotalBeats;
                        
                        for (var b = 1; b <= gridTotalBeats; b++) {
                            var beatX = padding + (b * gridPixelsPerBeat);
                            
                            // Bar lines (every beatsPerBar beats) are brighter and thicker
                            if (b % beatsPerBar === 0) {
                                ctx.strokeStyle = "#5A5A5A";
                                ctx.lineWidth = 2;
                            } else {
                                // Beat lines are subtle
                                ctx.strokeStyle = "#3A3A3A";
                                ctx.lineWidth = 1;
                            }
                            
                            ctx.beginPath();
                            ctx.moveTo(beatX, 0);
                            ctx.lineTo(beatX, height);
                            ctx.stroke();
                        }
                        
                        // Draw connections and duration lines
                        ctx.strokeStyle = "#4A90E2";
                        ctx.lineWidth = 2;
                        
                        if (cells.length > 0) {
                            var pitchIndex = 0;
                            var lastNoteX = -1;
                            var lastNoteY = -1;
                            var currentBeat = 0;
                            
                            for (var i = 0; i < cells.length; i++) {
                                var duration = durationToBeats(cells[i].duration || "quarter");
                                var x = padding + (currentBeat * pixelsPerBeat);
                                var isRest = cells[i].isRest || false;
                                
                                if (isRest) {
                                    // Draw rest indicator
                                    ctx.strokeStyle = "#666666";
                                    ctx.setLineDash([4, 4]);
                                    ctx.beginPath();
                                    var restY = height / 2;
                                    var restEndX = padding + ((currentBeat + duration) * pixelsPerBeat);
                                    ctx.moveTo(x, restY);
                                    ctx.lineTo(restEndX, restY);
                                    ctx.stroke();
                                    ctx.setLineDash([]);
                                    ctx.strokeStyle = "#4A90E2";
                                    lastNoteX = -1;
                                    lastNoteY = -1;
                                } else {
                                    if (pitchIndex < pitches.length) {
                                        var y = height - (pitches[pitchIndex] * stepY);
                                        
                                        // Connect to previous note
                                        if (lastNoteX >= 0) {
                                            ctx.beginPath();
                                            ctx.moveTo(lastNoteX, lastNoteY);
                                            ctx.lineTo(x, y);
                                            ctx.stroke();
                                        }
                                        
                                        // Draw duration line
                                        var noteEndX = padding + ((currentBeat + duration) * pixelsPerBeat);
                                        ctx.beginPath();
                                        ctx.moveTo(x, y);
                                        ctx.lineTo(noteEndX, y);
                                        ctx.stroke();
                                        
                                        lastNoteX = noteEndX;
                                        lastNoteY = y;
                                        pitchIndex++;
                                    }
                                }
                                currentBeat += duration;
                            }
                        } else {
                            // Fallback: evenly spaced
                            var stepX = pitches.length > 1 ? drawWidth / (pitches.length - 1) : 0;
                            ctx.beginPath();
                            for (var j = 0; j < pitches.length; j++) {
                                var px = padding + (j * stepX);
                                var py = height - (pitches[j] * stepY);
                                if (j === 0) {
                                    ctx.moveTo(px, py);
                                } else {
                                    ctx.lineTo(px, py);
                                }
                            }
                            ctx.stroke();
                        }
                    }
                }
                
                // Interactive note handles
                Repeater {
                    id: noteHandles
                    model: root.editablePitchContour.length
                    
                    Rectangle {
                        id: noteHandle
                        
                        property int noteIndex: index
                        property real targetY: calculateY()
                        property real targetX: calculateX()
                        
                        function calculateY() {
                            var stepY = canvasArea.height / 8;
                            return canvasArea.height - (root.editablePitchContour[noteIndex] * stepY) - (height / 2);
                        }
                        
                        function calculateX() {
                            var cells = root.editableRhythmCells;
                            var padding = 20;
                            var drawWidth = canvasArea.width - (padding * 2);
                            
                            function durationToBeats(dur) {
                                switch(dur) {
                                    case "whole": return 4.0;
                                    case "dotted-half": return 3.0;
                                    case "half": return 2.0;
                                    case "dotted-quarter": return 1.5;
                                    case "quarter": return 1.0;
                                    case "dotted-eighth": return 0.75;
                                    case "eighth": return 0.5;
                                    case "sixteenth": return 0.25;
                                    default: return 1.0;
                                }
                            }
                            
                            if (cells.length > 0) {
                                var totalBeats = 0;
                                for (var c = 0; c < cells.length; c++) {
                                    totalBeats += durationToBeats(cells[c].duration || "quarter");
                                }
                                var pixelsPerBeat = totalBeats > 0 ? drawWidth / totalBeats : 0;
                                
                                // Find this note's position
                                var pitchIdx = 0;
                                var currentBeat = 0;
                                for (var i = 0; i < cells.length; i++) {
                                    if (cells[i].isRest) {
                                        currentBeat += durationToBeats(cells[i].duration || "quarter");
                                        continue;
                                    }
                                    if (pitchIdx === noteIndex) {
                                        return padding + (currentBeat * pixelsPerBeat) - (width / 2);
                                    }
                                    currentBeat += durationToBeats(cells[i].duration || "quarter");
                                    pitchIdx++;
                                }
                            }
                            
                            // Fallback: evenly spaced
                            var stepX = root.editablePitchContour.length > 1 
                                ? drawWidth / (root.editablePitchContour.length - 1) : 0;
                            return padding + (noteIndex * stepX) - (width / 2);
                        }
                        
                        x: targetX
                        y: targetY
                        width: 28
                        height: 28
                        radius: 14
                        color: dragArea.pressed ? "#6AB0FF" : "#4A90E2"
                        border.color: dragArea.pressed ? "white" : "#2D2D2D"
                        border.width: 2
                        
                        Behavior on color { ColorAnimation { duration: 100 } }
                        
                        Text {
                            anchors.centerIn: parent
                            text: root.editablePitchContour[noteHandle.noteIndex]
                            color: "white"
                            font.pixelSize: 12
                            font.bold: true
                        }
                        
                        MouseArea {
                            id: dragArea
                            anchors.fill: parent
                            anchors.margins: -10 // Larger touch target
                            
                            property real startY: 0
                            property int startPitch: 0
                            
                            onPressed: {
                                startY = mouse.y + noteHandle.y
                                startPitch = root.editablePitchContour[noteHandle.noteIndex]
                            }
                            
                            onPositionChanged: {
                                var currentY = mouse.y + noteHandle.y
                                var deltaY = startY - currentY
                                var stepY = canvasArea.height / 8
                                var deltaPitch = Math.round(deltaY / stepY)
                                
                                var newPitch = Math.max(1, Math.min(7, startPitch + deltaPitch))
                                
                                if (root.editablePitchContour[noteHandle.noteIndex] !== newPitch) {
                                    var newContour = root.editablePitchContour.slice()
                                    newContour[noteHandle.noteIndex] = newPitch
                                    root.editablePitchContour = newContour
                                    backgroundCanvas.requestPaint()
                                }
                            }
                        }
                    }
                }
            }
            
            // Empty state
            Text {
                anchors.centerIn: parent
                visible: root.editablePitchContour.length === 0
                text: "No notes to edit"
                color: "#666666"
                font.pixelSize: 16
            }
        }
        
        // Rhythm display (read-only for now)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: "#2D2D2D"
            radius: 8
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8
                
                Text {
                    text: "Rhythm Pattern"
                    color: "#AAAAAA"
                    font.pixelSize: 12
                }
                
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    ScrollBar.horizontal.policy: ScrollBar.AsNeeded
                    ScrollBar.vertical.policy: ScrollBar.AlwaysOff
                    
                    Row {
                        spacing: 12
                        
                        Repeater {
                            model: root.editableRhythmCells
                            
                            Column {
                                spacing: 2
                                
                                Text {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    text: {
                                        var isRest = modelData.isRest || false;
                                        if (isRest) return "—";
                                        switch(modelData.duration) {
                                            case "whole": return "○";
                                            case "half": return "◐";
                                            case "dotted-half": return "◐.";
                                            case "quarter": return "●";
                                            case "dotted-quarter": return "●.";
                                            case "eighth": return "♪";
                                            case "dotted-eighth": return "♪.";
                                            case "sixteenth": return "♬";
                                            default: return "●";
                                        }
                                    }
                                    color: (modelData.isRest || false) ? "#888888" : "white"
                                    font.pixelSize: 18
                                }
                                
                                Text {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    text: {
                                        var isRest = modelData.isRest || false;
                                        var prefix = isRest ? "R:" : "";
                                        switch(modelData.duration) {
                                            case "whole": return prefix + "1";
                                            case "dotted-half": return prefix + "2.";
                                            case "half": return prefix + "2";
                                            case "dotted-quarter": return prefix + "4.";
                                            case "quarter": return prefix + "4";
                                            case "dotted-eighth": return prefix + "8.";
                                            case "eighth": return prefix + "8";
                                            case "sixteenth": return prefix + "16";
                                            default: return prefix + "4";
                                        }
                                    }
                                    color: "#AAAAAA"
                                    font.pixelSize: 10
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // Action buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            
            Button {
                Layout.fillWidth: true
                height: 50
                text: motifPlayer.isPlaying ? "Stop" : "Play"
                font.pixelSize: 16
                
                onClicked: {
                    if (motifPlayer.isPlaying) {
                        motifPlayer.stop()
                    } else {
                        var sketchData = sketchManager.getSketch(sketchId)
                        // Set tempo from sketch before playing
                        if (sketchData && sketchData.tempo) {
                            motifPlayer.tempo = sketchData.tempo
                        }
                        motifPlayer.voiceType = 0  // SketchPiano
                        motifPlayer.setMotif(
                            root.editablePitchContour,
                            root.editableRhythmCells,
                            sketchData ? sketchData.key : "C major"
                        )
                        motifPlayer.playPrerendered()  // Use pre-rendered for smooth playback
                    }
                }
                
                background: Rectangle {
                    radius: 8
                    color: parent.pressed ? "#3A7BC8" : (motifPlayer.isPlaying ? "#4CAF50" : "#4A90E2")
                }
                
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font: parent.font
                }
            }
            
            Button {
                Layout.fillWidth: true
                height: 50
                text: "Reset"
                font.pixelSize: 16
                
                onClicked: {
                    loadMotif() // Reload original data
                    backgroundCanvas.requestPaint()
                }
                
                background: Rectangle {
                    radius: 8
                    color: parent.pressed ? "#555555" : "#3D3D3D"
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
    }
    
    // Repaint when data changes
    onEditablePitchContourChanged: backgroundCanvas.requestPaint()
    onEditableRhythmCellsChanged: backgroundCanvas.requestPaint()
}

