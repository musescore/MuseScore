import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    
    property var pitchContour: []
    property var rhythmPattern: []
    property var rhythmCells: []  // New format: [{duration, tie, isRest}, ...]
    property int motifBars: 1     // Number of bars for beat grid
    property int beatsPerBar: 4   // Time signature (beats per bar)
    
    color: "#1E1E1E"
    radius: 8
    border.color: "#3D3D3D"
    border.width: 1
    
    implicitHeight: 200
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12
        
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: "Current Motif"
            color: "#AAAAAA"
            font.pixelSize: 14
        }
        
        // Pitch contour visualization (accounts for rests in rhythmCells)
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#2D2D2D"
            radius: 4
            
            Canvas {
                id: canvas
                anchors.fill: parent
                anchors.margins: 8
                
                onPaint: {
                    var ctx = getContext("2d");
                    ctx.clearRect(0, 0, width, height);
                    
                    // Use rhythmCells to determine total events (notes + rests)
                    var cells = root.rhythmCells.length > 0 ? root.rhythmCells : [];
                    var totalEvents = cells.length > 0 ? cells.length : root.pitchContour.length;
                    
                    if (totalEvents === 0) {
                        return;
                    }
                    
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
                    
                    // Calculate total beats for proportional spacing
                    var totalBeats = 0;
                    if (cells.length > 0) {
                        for (var c = 0; c < cells.length; c++) {
                            totalBeats += durationToBeats(cells[c].duration || "quarter");
                        }
                    } else {
                        totalBeats = root.pitchContour.length; // Fallback: 1 beat per note
                    }
                    
                    // Add horizontal padding so dots don't touch edges
                    var padding = 10;
                    var drawWidth = width - (padding * 2);
                    var pixelsPerBeat = totalBeats > 0 ? drawWidth / totalBeats : 0;
                    var stepY = height / 8; // 7 scale degrees + padding
                    
                    // Draw beat grid lines (vertical) based on motifBars setting
                    var gridTotalBeats = root.motifBars * root.beatsPerBar;
                    var gridPixelsPerBeat = drawWidth / gridTotalBeats;
                    
                    for (var b = 1; b < gridTotalBeats; b++) {
                        var beatX = padding + (b * gridPixelsPerBeat);
                        // Bar lines are brighter
                        if (b % root.beatsPerBar === 0) {
                            ctx.strokeStyle = "#5A5A5A";
                            ctx.lineWidth = 2;
                        } else {
                            ctx.strokeStyle = "#3A3A3A";
                            ctx.lineWidth = 1;
                        }
                        ctx.beginPath();
                        ctx.moveTo(beatX, 0);
                        ctx.lineTo(beatX, height);
                        ctx.stroke();
                    }
                    
                    ctx.strokeStyle = "#4A90E2";
                    ctx.fillStyle = "#4A90E2";
                    ctx.lineWidth = 2;
                    
                    // If we have rhythmCells, use them to determine what's a note vs rest
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
                                // Draw rest indicator (horizontal line spanning the rest duration)
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
                                
                                // Reset line continuity
                                lastNoteX = -1;
                                lastNoteY = -1;
                            } else {
                                // Draw note
                                if (pitchIndex < root.pitchContour.length) {
                                    var y = height - (root.pitchContour[pitchIndex] * stepY);
                                    
                                    // Connect to previous note if exists
                                    if (lastNoteX >= 0) {
                                        ctx.beginPath();
                                        ctx.moveTo(lastNoteX, lastNoteY);
                                        ctx.lineTo(x, y);
                                        ctx.stroke();
                                    }
                                    
                                    // Draw horizontal line showing note duration
                                    var noteEndX = padding + ((currentBeat + duration) * pixelsPerBeat);
                                    ctx.beginPath();
                                    ctx.moveTo(x, y);
                                    ctx.lineTo(noteEndX, y);
                                    ctx.stroke();
                                    
                                    // Draw dot at start of note
                                    ctx.fillRect(x - 3, y - 3, 6, 6);
                                    
                                    lastNoteX = noteEndX;
                                    lastNoteY = y;
                                    pitchIndex++;
                                }
                            }
                            
                            currentBeat += duration;
                        }
                    } else {
                        // Fallback: just draw pitchContour evenly spaced
                        var fallbackStepX = totalEvents > 1 ? drawWidth / (totalEvents - 1) : 0;
                        ctx.beginPath();
                        for (var j = 0; j < root.pitchContour.length; j++) {
                            var px = padding + (j * fallbackStepX);
                            var py = height - (root.pitchContour[j] * stepY);
                            
                            if (j === 0) {
                                ctx.moveTo(px, py);
                            } else {
                                ctx.lineTo(px, py);
                            }
                            ctx.fillRect(px - 3, py - 3, 6, 6);
                        }
                        ctx.stroke();
                    }
                }
            }
            
            Text {
                anchors.centerIn: parent
                visible: root.pitchContour.length === 0 && root.rhythmCells.length === 0
                text: "Tap scale degrees to add notes"
                color: "#666666"
                font.pixelSize: 14
            }
        }
        
        // Rhythm pattern display (scrollable if too many notes)
        ScrollView {
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            clip: true
            ScrollBar.horizontal.policy: ScrollBar.AsNeeded
            ScrollBar.vertical.policy: ScrollBar.AlwaysOff
            
            Row {
                anchors.centerIn: parent
                spacing: 8
                
                Repeater {
                    model: root.rhythmCells.length > 0 ? root.rhythmCells : root.rhythmPattern
                    
                    Column {
                        spacing: 2
                        
                        // Note/Rest symbol with pitch number
                        Row {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 2
                            
                            Text {
                                text: {
                                    var dur = modelData.duration || modelData;
                                    var isRest = modelData.isRest || false;
                                    
                                    if (isRest) {
                                        // Use simple dash for rests
                                        return "—";
                                    } else {
                                        // Use simple, universally-supported note symbols
                                        switch(dur) {
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
                                }
                                color: (modelData.isRest || false) ? "#888888" : "white"
                                font.pixelSize: 18
                            }
                            
                            // Tie indicator
                            Text {
                                visible: modelData.tie || false
                                text: "⌒"
                                color: "#4A90E2"
                                font.pixelSize: 14
                            }
                        }
                        
                        // Duration label below
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: {
                                var dur = modelData.duration || modelData;
                                var isRest = modelData.isRest || false;
                                var prefix = isRest ? "R:" : "";
                                
                                switch(dur) {
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
                            color: (modelData.isRest || false) ? "#666666" : "#AAAAAA"
                            font.pixelSize: 10
                        }
                    }
                }
            }
        }
        
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: {
                var noteCount = root.pitchContour.length;
                var restCount = 0;
                for (var i = 0; i < root.rhythmCells.length; i++) {
                    if (root.rhythmCells[i].isRest) restCount++;
                }
                if (restCount > 0) {
                    return noteCount + " notes, " + restCount + " rests";
                }
                return noteCount + " notes";
            }
            color: "#AAAAAA"
            font.pixelSize: 12
        }
    }
    
    // Redraw when data changes
    onPitchContourChanged: canvas.requestPaint()
    onRhythmPatternChanged: canvas.requestPaint()
    onRhythmCellsChanged: canvas.requestPaint()
}
