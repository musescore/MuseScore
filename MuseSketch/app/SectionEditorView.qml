import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root

    property string sketchId: ""
    property string sectionId: ""
    property var sectionData: null
    property var sketchData: null
    property string currentTexture: "MelodyOnly"
    property bool hasSATB: false
    
    // Playhead state
    property real playheadBeat: 0
    property bool isPlayheadVisible: false
    property real totalBeats: sectionData ? sectionData.lengthBars * 4 : 16
    
    signal backRequested()
    signal sectionUpdated()
    
    // Timer to animate the playhead during melody playback
    Timer {
        id: playheadTimer
        interval: 50  // Update 20 times per second
        repeat: true
        running: isPlayheadVisible && (motifPlayer.isPlaying || satbIsPlaying)
        
        property real startTime: 0
        property real startBeat: 0
        
        onTriggered: {
            var elapsedMs = Date.now() - startTime
            var msPerBeat = 60000 / motifPlayer.tempo
            var elapsedBeats = elapsedMs / msPerBeat
            playheadBeat = Math.min(startBeat + elapsedBeats, totalBeats)
            
            // Stop when reaching the end
            if (playheadBeat >= totalBeats) {
                isPlayheadVisible = false
            }
        }
    }

    Component.onCompleted: {
        loadData()
    }
    
    Component.onDestruction: {
        // Stop playback when leaving the screen
        motifPlayer.stop()
    }

    function loadData() {
        sketchData = sketchManager.getSketch(sketchId)
        sectionData = sketchManager.getSection(sketchId, sectionId)
        currentTexture = sketchManager.getSectionTexture(sketchId, sectionId)
        hasSATB = sketchManager.hasSATBVoices(sketchId, sectionId)
        // Update the detailed visualization
        if (sectionVisualization) {
            sectionVisualization.updateTimeline()
        }
        if (satbVisualization) {
            satbVisualization.requestPaint()
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
                text: sectionData ? sectionData.name : "Section"
                color: "white"
                font.pixelSize: 20
                font.bold: true
                
                MouseArea {
                    anchors.fill: parent
                    onClicked: renameDialog.open()
                }
            }

            Text {
                text: sectionData ? sectionData.lengthBars + " bars" : ""
                color: "#AAAAAA"
                font.pixelSize: 14
                
                MouseArea {
                    anchors.fill: parent
                    onClicked: lengthDialog.open()
                }
            }

            // Texture selector
            ComboBox {
                id: textureSelector
                Layout.preferredWidth: 140
                model: ["Melody Only", "SATB Chorale"]
                currentIndex: currentTexture === "SATBChorale" ? 1 : 0

                onActivated: function(index) {
                    if (index === 1) {
                        // SATB Chorale selected - just switch mode
                        sketchManager.setSectionTexture(sketchId, sectionId, "SATBChorale")
                        loadData()
                    } else {
                        // Melody Only
                        sketchManager.setSectionTexture(sketchId, sectionId, "MelodyOnly")
                        loadData()
                    }
                }

                background: Rectangle {
                    radius: 4
                    color: "#3D3D3D"
                    border.color: "#5D5D5D"
                }

                contentItem: Text {
                    text: textureSelector.displayText
                    color: "white"
                    font.pixelSize: 12
                    leftPadding: 8
                    verticalAlignment: Text.AlignVCenter
                }
            }

            // Generate SATB button (visible in SATB mode)
            Button {
                visible: currentTexture === "SATBChorale"
                Layout.rightMargin: 8
                text: "Generate"
                font.pixelSize: 12

                onClicked: {
                    var timeline = sketchManager.getSectionTimeline(sketchId, sectionId)
                    if (timeline.length === 0) {
                        noMelodyDialog.open()
                        return
                    }
                    sketchManager.generateSATBFromTimeline(sketchId, sectionId)
                    loadData()
                }

                background: Rectangle {
                    radius: 4
                    color: parent.pressed ? "#27AE60" : "#2ECC71"
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
    }

    background: Rectangle {
        color: "#1E1E1E"
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Timeline area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#252525"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                // Bar number header (aligned with timeline below)
                Row {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 24

                    // Spacer matching the "Melody" label width
                    Item {
                        width: 80
                        height: parent.height
                    }

                    // Bar numbers
                    Row {
                        id: barNumbersRow
                        width: parent.width - 80
                        height: parent.height

                        Repeater {
                            model: sectionData ? sectionData.lengthBars : 0

                            Text {
                                width: barNumbersRow.width / (sectionData ? sectionData.lengthBars : 1)
                                height: barNumbersRow.height
                                text: index + 1
                                color: "#666666"
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }
                }

                // Timeline track
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 80
                    color: "#1E1E1E"
                    radius: 4
                    border.color: "#3D3D3D"
                    border.width: 1

                    Row {
                        anchors.fill: parent

                        // Track label
                        Rectangle {
                            width: 80
                            height: parent.height
                            color: "#2D2D2D"
                            
                            Text {
                                anchors.centerIn: parent
                                text: "Melody"
                                color: "#AAAAAA"
                                font.pixelSize: 12
                            }
                        }

                        // Timeline content
                        Item {
                            id: timelineContent
                            width: parent.width - 80
                            height: parent.height

                            property real barWidth: width / (sectionData ? sectionData.lengthBars : 1)
                            property real pixelsPerBeat: width / totalBeats

                            // Grid lines
                            Canvas {
                                anchors.fill: parent

                                onPaint: {
                                    var ctx = getContext("2d");
                                    ctx.clearRect(0, 0, width, height);

                                    if (!sectionData) return;

                                    var bars = sectionData.lengthBars;
                                    var barWidth = width / bars;

                                    // Draw bar lines
                                    ctx.strokeStyle = "#3D3D3D";
                                    ctx.lineWidth = 1;

                                    for (var i = 0; i <= bars; i++) {
                                        var x = i * barWidth;
                                        ctx.beginPath();
                                        ctx.moveTo(x, 0);
                                        ctx.lineTo(x, height);
                                        ctx.stroke();
                                    }
                                }
                            }
                            
                            // Playhead on timeline track
                            Rectangle {
                                id: timelinePlayhead
                                visible: isPlayheadVisible
                                x: playheadBeat * timelineContent.pixelsPerBeat
                                width: 2
                                height: parent.height
                                color: "#FF5722"
                                z: 100
                                
                                // Playhead triangle indicator at top
                                Rectangle {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    anchors.top: parent.top
                                    width: 10
                                    height: 10
                                    rotation: 45
                                    color: "#FF5722"
                                    transformOrigin: Item.Center
                                    y: -3
                                }
                            }

                            // Placement blocks
                            Repeater {
                                model: sectionData ? sectionData.placements : []

                                Rectangle {
                                    id: placementBlock
                                    
                                    property int placementIndex: index
                                    property var placement: modelData
                                    
                                    x: placement.startBar * timelineContent.barWidth
                                    y: 8
                                    width: (placement.endBar - placement.startBar) * timelineContent.barWidth - 4
                                    height: parent.height - 16
                                    
                                    color: blockMouseArea.pressed ? "#5AA0F2" : "#4A90E2"
                                    radius: 4
                                    border.color: "#6AB0FF"
                                    border.width: 1

                                    // Motif name
                                    Text {
                                        anchors.centerIn: parent
                                        text: placement.motifName || "?"
                                        color: "white"
                                        font.pixelSize: 12
                                        font.bold: true
                                        elide: Text.ElideRight
                                        width: parent.width - 8
                                        horizontalAlignment: Text.AlignHCenter
                                    }

                                    // Repetition indicator
                                    Text {
                                        anchors.right: parent.right
                                        anchors.bottom: parent.bottom
                                        anchors.margins: 4
                                        visible: placement.repetitions > 1
                                        text: "×" + placement.repetitions
                                        color: "#CCCCCC"
                                        font.pixelSize: 10
                                    }

                                    MouseArea {
                                        id: blockMouseArea
                                        anchors.fill: parent
                                        
                                        onClicked: {
                                            placementMenu.placementIndex = placementBlock.placementIndex
                                            placementMenu.popup()
                                        }
                                        
                                        drag.target: placementBlock
                                        drag.axis: Drag.XAxis
                                        drag.minimumX: 0
                                        drag.maximumX: timelineContent.width - placementBlock.width
                                        
                                        onReleased: {
                                            // Snap to bar
                                            var newBar = Math.round(placementBlock.x / timelineContent.barWidth)
                                            newBar = Math.max(0, Math.min(newBar, sectionData.lengthBars - 1))
                                            sketchManager.movePlacement(sketchId, sectionId, placementBlock.placementIndex, newBar)
                                            loadData()
                                        }
                                    }
                                }
                            }

                            // Click to add placement
                            MouseArea {
                                anchors.fill: parent
                                z: -1
                                
                                onClicked: function(mouse) {
                                    var bar = Math.floor(mouse.x / timelineContent.barWidth)
                                    addPlacementDialog.targetBar = bar
                                    addPlacementDialog.open()
                                }
                            }
                        }
                    }
                }

                // Instructions
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Tap timeline to add motif • Drag blocks to move • Tap block for options"
                    color: "#666666"
                    font.pixelSize: 12
                }

                // Detailed Section Visualization (aligned with timeline above)
                // Shows either melody only or SATB based on texture type
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: currentTexture === "SATBChorale" && hasSATB ? 200 : 120
                    color: "#1E1E1E"
                    radius: 4
                    border.color: "#3D3D3D"
                    border.width: 1

                    // Melody-only visualization
                    Row {
                        anchors.fill: parent
                        visible: currentTexture !== "SATBChorale" || !hasSATB

                        // Scale degree labels (same width as "Melody" label above)
                        Rectangle {
                            width: 80
                            height: parent.height
                            color: "#2D2D2D"

                            Column {
                                anchors.fill: parent
                                anchors.margins: 4

                                Repeater {
                                    model: 7
                                    Text {
                                        width: parent.width
                                        height: (parent.height) / 7
                                        text: 7 - index
                                        color: "#666666"
                                        font.pixelSize: 10
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                            }
                        }

                        // Visualization canvas (aligned with timeline content)
                        Item {
                            width: parent.width - 80
                            height: parent.height

                            Canvas {
                                id: sectionVisualization
                                anchors.fill: parent

                                property var timeline: []

                                function updateTimeline() {
                                    timeline = sketchManager.getSectionTimeline(sketchId, sectionId)
                                    requestPaint()
                                }

                                onPaint: {
                                    var ctx = getContext("2d");
                                    ctx.clearRect(0, 0, width, height);

                                    if (!sectionData) return;

                                    var stepY = height / 8;
                                    var beatsPerBar = 4;
                                    var totalBars = sectionData.lengthBars;
                                    var totalBeats = totalBars * beatsPerBar;
                                    var pixelsPerBeat = width / totalBeats;

                                    // Draw bar and beat grid lines (aligned with timeline above)
                                    for (var b = 0; b <= totalBeats; b++) {
                                        var lineX = b * pixelsPerBeat;
                                        if (b % beatsPerBar === 0) {
                                            ctx.strokeStyle = "#5A5A5A";
                                            ctx.lineWidth = 1;
                                        } else {
                                            ctx.strokeStyle = "#3A3A3A";
                                            ctx.lineWidth = 1;
                                        }
                                        ctx.beginPath();
                                        ctx.moveTo(lineX, 0);
                                        ctx.lineTo(lineX, height);
                                        ctx.stroke();
                                    }

                                    // Draw horizontal pitch grid lines
                                    ctx.strokeStyle = "#2A2A2A";
                                    ctx.lineWidth = 1;
                                    for (var p = 1; p <= 7; p++) {
                                        var gridY = height - (p * stepY);
                                        ctx.beginPath();
                                        ctx.moveTo(0, gridY);
                                        ctx.lineTo(width, gridY);
                                        ctx.stroke();
                                    }

                                    if (timeline.length === 0) {
                                        // Draw empty state
                                        ctx.fillStyle = "#666666";
                                        ctx.font = "14px sans-serif";
                                        ctx.textAlign = "center";
                                        ctx.fillText("Add motifs to see the melody", width / 2, height / 2);
                                        return;
                                    }

                                    // Helper function
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

                                    // Draw notes and rests
                                    ctx.strokeStyle = "#4A90E2";
                                    ctx.fillStyle = "#4A90E2";
                                    ctx.lineWidth = 2;

                                    var lastNoteX = -1;
                                    var lastNoteY = -1;

                                    for (var i = 0; i < timeline.length; i++) {
                                        var event = timeline[i];
                                        var x = event.startBeat * pixelsPerBeat;
                                        var duration = durationToBeats(event.duration);
                                        var endX = (event.startBeat + duration) * pixelsPerBeat;

                                        if (event.isRest) {
                                            // Draw rest indicator
                                            ctx.strokeStyle = "#666666";
                                            ctx.setLineDash([3, 3]);
                                            ctx.beginPath();
                                            var restY = height / 2;
                                            ctx.moveTo(x, restY);
                                            ctx.lineTo(endX, restY);
                                            ctx.stroke();
                                            ctx.setLineDash([]);
                                            ctx.strokeStyle = "#4A90E2";

                                            // Reset line continuity
                                            lastNoteX = -1;
                                            lastNoteY = -1;
                                        } else {
                                            var y = height - (event.scaleDegree * stepY);

                                            // Connect to previous note
                                            if (lastNoteX >= 0) {
                                                ctx.beginPath();
                                                ctx.moveTo(lastNoteX, lastNoteY);
                                                ctx.lineTo(x, y);
                                                ctx.stroke();
                                            }

                                            // Draw horizontal duration line
                                            ctx.beginPath();
                                            ctx.moveTo(x, y);
                                            ctx.lineTo(endX, y);
                                            ctx.stroke();

                                            // Draw note dot
                                            ctx.beginPath();
                                            ctx.arc(x, y, 4, 0, 2 * Math.PI);
                                            ctx.fill();

                                            lastNoteX = endX;
                                            lastNoteY = y;
                                        }
                                    }
                                }

                                Component.onCompleted: updateTimeline()
                            }
                            
                            // Playhead on melody visualization
                            Rectangle {
                                visible: isPlayheadVisible
                                x: playheadBeat * (parent.width / totalBeats)
                                width: 2
                                height: parent.height
                                color: "#FF5722"
                                z: 100
                            }
                        }
                    }

                    // SATB visualization (4 staves)
                    Item {
                        anchors.fill: parent
                        visible: currentTexture === "SATBChorale" && hasSATB
                        
                        Column {
                            anchors.fill: parent

                            Repeater {
                                model: [
                                    { name: "S", color: "#E74C3C", voiceIndex: 0 },
                                    { name: "A", color: "#E67E22", voiceIndex: 1 },
                                    { name: "T", color: "#27AE60", voiceIndex: 2 },
                                    { name: "B", color: "#3498DB", voiceIndex: 3 }
                                ]

                                Rectangle {
                                    width: parent.width
                                    height: parent.height / 4
                                    color: "transparent"

                                    Row {
                                        anchors.fill: parent

                                        // Voice label
                                        Rectangle {
                                            width: 80
                                            height: parent.height
                                            color: "#2D2D2D"

                                            Text {
                                                anchors.centerIn: parent
                                                text: modelData.name
                                                color: modelData.color
                                                font.pixelSize: 14
                                                font.bold: true
                                            }
                                        }

                                        // Voice visualization
                                        Canvas {
                                            id: satbVisualization
                                            width: parent.width - 80
                                            height: parent.height

                                            property int voiceIndex: modelData.voiceIndex
                                            property string voiceColor: modelData.color

                                            onPaint: {
                                                var ctx = getContext("2d");
                                                ctx.clearRect(0, 0, width, height);

                                                if (!sectionData) return;

                                                var stepY = height / 8;
                                                var beatsPerBar = 4;
                                                var totalBars = sectionData.lengthBars;
                                                var totalBeats = totalBars * beatsPerBar;
                                                var pixelsPerBeat = width / totalBeats;

                                                // Draw bar lines
                                                ctx.strokeStyle = "#3D3D3D";
                                                ctx.lineWidth = 1;
                                            for (var b = 0; b <= totalBars; b++) {
                                                ctx.beginPath();
                                                ctx.moveTo(b * beatsPerBar * pixelsPerBeat, 0);
                                                ctx.lineTo(b * beatsPerBar * pixelsPerBeat, height);
                                                ctx.stroke();
                                            }

                                            // Get timeline for this voice
                                            var timeline = sketchManager.getSATBVoiceTimeline(sketchId, sectionId, voiceIndex);
                                            if (timeline.length === 0) return;

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

                                            // Draw notes
                                            ctx.strokeStyle = voiceColor;
                                            ctx.fillStyle = voiceColor;
                                            ctx.lineWidth = 2;

                                            var lastX = -1, lastY = -1;

                                            for (var i = 0; i < timeline.length; i++) {
                                                var event = timeline[i];
                                                var x = event.startBeat * pixelsPerBeat;
                                                var duration = durationToBeats(event.duration);
                                                var endX = (event.startBeat + duration) * pixelsPerBeat;

                                                if (!event.isRest && event.scaleDegree > 0) {
                                                    var y = height - (event.scaleDegree * stepY);

                                                    if (lastX >= 0) {
                                                        ctx.beginPath();
                                                        ctx.moveTo(lastX, lastY);
                                                        ctx.lineTo(x, y);
                                                        ctx.stroke();
                                                    }

                                                    ctx.beginPath();
                                                    ctx.moveTo(x, y);
                                                    ctx.lineTo(endX, y);
                                                    ctx.stroke();

                                                    ctx.beginPath();
                                                    ctx.arc(x, y, 3, 0, 2 * Math.PI);
                                                    ctx.fill();

                                                    lastX = endX;
                                                    lastY = y;
                                                } else {
                                                    lastX = -1;
                                                    lastY = -1;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        }
                        
                        // Playhead overlay for SATB visualization
                        Rectangle {
                            visible: isPlayheadVisible
                            x: 80 + playheadBeat * ((parent.width - 80) / totalBeats)
                            width: 2
                            height: parent.height
                            color: "#FF5722"
                            z: 100
                        }
                    }
                }
            }
        }

        // Motif library strip
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 140
            color: "#2D2D2D"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                Text {
                    text: "Motif Library"
                    color: "#AAAAAA"
                    font.pixelSize: 14
                }

                ListView {
                    id: motifStrip
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    orientation: ListView.Horizontal
                    spacing: 8
                    clip: true

                    model: sketchData && sketchData.motifs ? sketchData.motifs : []

                    delegate: Rectangle {
                        width: 100
                        height: motifStrip.height
                        color: motifTouchArea.pressed ? "#3D3D3D" : "#1E1E1E"
                        radius: 8
                        border.color: "#3D3D3D"
                        border.width: 1

                        property var motif: modelData

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 4

                            // Mini preview
                            Canvas {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 40

                                onPaint: {
                                    var ctx = getContext("2d");
                                    ctx.clearRect(0, 0, width, height);

                                    var pitches = motif.pitchContour || [];
                                    var cells = motif.rhythmCells || [];
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
                                            notePositions.push({ x: x, y: y, isRest: false });
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
                                            lastNotePos = null;
                                        }
                                    }

                                    // Draw dots for notes and rest indicators
                                    for (var j = 0; j < notePositions.length; j++) {
                                        var np = notePositions[j];
                                        if (np.isRest) {
                                            ctx.fillStyle = "#666666";
                                            ctx.fillRect(np.x - 2, height / 2 - 1, 4, 2);
                                        } else {
                                            ctx.fillStyle = "#4A90E2";
                                            ctx.beginPath();
                                            ctx.arc(np.x, np.y, 2, 0, 2 * Math.PI);
                                            ctx.fill();
                                        }
                                    }
                                }

                                Component.onCompleted: requestPaint()
                            }

                            Text {
                                Layout.fillWidth: true
                                text: motif.name
                                color: "white"
                                font.pixelSize: 12
                                font.bold: true
                                elide: Text.ElideRight
                                horizontalAlignment: Text.AlignHCenter
                            }

                            Text {
                                Layout.fillWidth: true
                                text: motif.lengthBars + (motif.lengthBars === 1 ? " bar" : " bars")
                                color: "#666666"
                                font.pixelSize: 10
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }

                        MouseArea {
                            id: motifTouchArea
                            anchors.fill: parent
                            
                            onClicked: {
                                // Quick add at first available bar
                                quickAddMotif(motif.id, motif.lengthBars)
                            }
                        }
                    }

                    // Empty state
                    Text {
                        anchors.centerIn: parent
                        visible: motifStrip.count === 0
                        text: "No motifs - create some first"
                        color: "#666666"
                        font.pixelSize: 14
                    }
                }
            }
        }

        // Playback controls
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#1E1E1E"

            RowLayout {
                anchors.centerIn: parent
                spacing: 20

                Button {
                    width: 50
                    height: 50
                    
                    property bool isAnyPlaying: motifPlayer.isPlaying || satbIsPlaying
                    
                    text: isAnyPlaying ? "■" : "▶"
                    font.pixelSize: 20

                    onClicked: {
                        if (isAnyPlaying) {
                            motifPlayer.stop()
                            stopSATBPlayback()
                        } else {
                            playSectionTimeline()
                        }
                    }

                    background: Rectangle {
                        radius: 25
                        color: parent.pressed ? "#3A7BC8" : (parent.isAnyPlaying ? "#4CAF50" : "#4A90E2")
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
    }

    // Check if a motif can fit at a given bar
    function canMotifFitAt(motifLengthBars, startBar) {
        if (!sectionData) return false
        
        // Check if motif would exceed section length
        if (startBar + motifLengthBars > sectionData.lengthBars) {
            return false
        }
        
        // Check for overlapping placements
        var placements = sectionData.placements || []
        for (var i = 0; i < placements.length; i++) {
            var pStart = placements[i].startBar
            var pEnd = placements[i].endBar
            var mEnd = startBar + motifLengthBars
            
            // Check for overlap
            if (startBar < pEnd && mEnd > pStart) {
                return false
            }
        }
        
        return true
    }

    function quickAddMotif(motifId, motifLengthBars) {
        // Find first available bar where the motif fits
        var placements = sectionData ? sectionData.placements : []
        var foundSpot = false
        var targetBar = 0
        
        for (var bar = 0; bar <= sectionData.lengthBars - motifLengthBars; bar++) {
            if (canMotifFitAt(motifLengthBars, bar)) {
                targetBar = bar
                foundSpot = true
                break
            }
        }
        
        if (!foundSpot) {
            // No space available - don't add
            console.log("No space available for motif of length", motifLengthBars, "bars")
            return
        }
        
        sketchManager.addPlacement(sketchId, sectionId, motifId, targetBar, 1)
        loadData()
    }

    function playSectionTimeline() {
        // Set tempo from sketch before playing
        if (sketchData && sketchData.tempo) {
            motifPlayer.tempo = sketchData.tempo
        }

        // If in SATB mode with voices generated, play SATB
        if (currentTexture === "SATBChorale" && hasSATB) {
            playSATBVoices()
            return
        }

        // Melody mode: use SketchPiano voice (0)
        motifPlayer.voiceType = 0
        
        // Otherwise play melody timeline
        var timeline = sketchManager.getSectionTimeline(sketchId, sectionId)
        if (timeline.length === 0) return
        
        // Build pitch contour and rhythm cells from timeline
        var pitches = []
        var rhythmCells = []
        
        for (var i = 0; i < timeline.length; i++) {
            var event = timeline[i]
            if (!event.isRest) {
                pitches.push(event.scaleDegree)
            }
            rhythmCells.push({
                duration: event.duration,
                isRest: event.isRest,
                tie: event.tie
            })
        }
        
        // Start playhead animation
        playheadBeat = 0
        playheadTimer.startTime = Date.now()
        playheadTimer.startBeat = 0
        isPlayheadVisible = true
        
        motifPlayer.voiceType = 0  // SketchPiano for melody-only playback
        motifPlayer.setMotif(pitches, rhythmCells, sketchData ? sketchData.key : "C major")
        motifPlayer.playPrerendered()  // Use pre-rendered for smooth playback
    }
    
    // Connect to motifPlayer to hide playhead when finished
    Connections {
        target: motifPlayer
        function onPlaybackFinished() {
            isPlayheadVisible = false
            satbIsPlaying = false
        }
        function onIsPlayingChanged() {
            if (!motifPlayer.isPlaying) {
                isPlayheadVisible = false
                satbIsPlaying = false
            }
        }
    }

    // SATB playback state
    property bool satbIsPlaying: false

    function playSATBVoices() {
        // Get all 4 voice timelines - each has independent rhythms
        var soprano = sketchManager.getSATBVoiceTimeline(sketchId, sectionId, 0)
        var alto = sketchManager.getSATBVoiceTimeline(sketchId, sectionId, 1)
        var tenor = sketchManager.getSATBVoiceTimeline(sketchId, sectionId, 2)
        var bass = sketchManager.getSATBVoiceTimeline(sketchId, sectionId, 3)

        if (soprano.length === 0 && alto.length === 0 && tenor.length === 0 && bass.length === 0) {
            console.log("No SATB voices to play")
            return
        }

        // Build unified event timeline from all voices
        // Each voice can have different rhythms - merge by startBeat
        var allEvents = []
        
        // Helper to add events from a voice timeline
        function addVoiceEvents(timeline, voiceType, octaveOffset) {
            for (var i = 0; i < timeline.length; i++) {
                var event = timeline[i]
                if (!event.isRest && event.scaleDegree > 0) {
                    allEvents.push({
                        startBeat: event.startBeat,
                        scaleDegree: event.scaleDegree,
                        voiceType: voiceType,
                        octaveOffset: octaveOffset,
                        duration: event.duration
                    })
                }
            }
        }
        
        // Add all voice events (voiceType: 1=S, 2=A, 3=T, 4=B)
        addVoiceEvents(soprano, 1, 1)   // Soprano: +1 octave
        addVoiceEvents(alto, 2, 0)      // Alto: middle
        addVoiceEvents(tenor, 3, 0)     // Tenor: middle
        addVoiceEvents(bass, 4, -1)     // Bass: -1 octave
        
        if (allEvents.length === 0) {
            console.log("No notes in SATB voices")
            return
        }
        
        // Sort by startBeat
        allEvents.sort(function(a, b) { return a.startBeat - b.startBeat })
        
        // Group events by startBeat (notes that play simultaneously)
        var groupedEvents = []
        var currentBeat = -1
        var currentGroup = null
        
        for (var j = 0; j < allEvents.length; j++) {
            var evt = allEvents[j]
            if (evt.startBeat !== currentBeat) {
                // New beat - create new group
                if (currentGroup) {
                    groupedEvents.push(currentGroup)
                }
                currentBeat = evt.startBeat
                currentGroup = {
                    startBeat: currentBeat,
                    duration: evt.duration,
                    notes: []
                }
            }
            currentGroup.notes.push({
                scaleDegree: evt.scaleDegree,
                voiceType: evt.voiceType,
                octaveOffset: evt.octaveOffset
            })
        }
        if (currentGroup) {
            groupedEvents.push(currentGroup)
        }
        
        // Start playhead animation
        playheadBeat = 0
        playheadTimer.startTime = Date.now()
        playheadTimer.startBeat = 0
        isPlayheadVisible = true
        satbIsPlaying = true
        
        // Use pre-rendered timeline playback (all audio mixed into one buffer)
        motifPlayer.playSATBTimeline(groupedEvents, sketchData ? sketchData.key : "C major")
    }

    function stopSATBPlayback() {
        satbIsPlaying = false
        motifPlayer.stop()
        isPlayheadVisible = false
    }

    function durationToMs(duration) {
        var msPerBeat = 60000 / motifPlayer.tempo
        switch(duration) {
            case "whole": return msPerBeat * 4
            case "dotted-half": return msPerBeat * 3
            case "half": return msPerBeat * 2
            case "dotted-quarter": return msPerBeat * 1.5
            case "quarter": return msPerBeat
            case "dotted-eighth": return msPerBeat * 0.75
            case "eighth": return msPerBeat / 2
            case "sixteenth": return msPerBeat / 4
            default: return msPerBeat
        }
    }
    
    function durationToBeats(duration) {
        switch(duration) {
            case "whole": return 4
            case "dotted-half": return 3
            case "half": return 2
            case "dotted-quarter": return 1.5
            case "quarter": return 1
            case "dotted-eighth": return 0.75
            case "eighth": return 0.5
            case "sixteenth": return 0.25
            default: return 1
        }
    }

    // Rename dialog
    Dialog {
        id: renameDialog
        anchors.centerIn: parent
        width: 280
        title: "Rename Section"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        TextField {
            id: renameField
            width: parent.width
            text: sectionData ? sectionData.name : ""
            placeholderText: "Section name"
            onAccepted: renameDialog.accept()
        }

        onAccepted: {
            if (renameField.text.trim() !== "") {
                sketchManager.renameSection(sketchId, sectionId, renameField.text.trim())
                loadData()
            }
        }

        onOpened: {
            renameField.text = sectionData ? sectionData.name : ""
            renameField.forceActiveFocus()
            renameField.selectAll()
        }
    }

    // Length dialog
    Dialog {
        id: lengthDialog
        anchors.centerIn: parent
        width: 280
        title: "Section Length"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        SpinBox {
            id: lengthSpinBox
            width: parent.width
            from: 1
            to: 64
            value: sectionData ? sectionData.lengthBars : 8
        }

        onAccepted: {
            sketchManager.setSectionLength(sketchId, sectionId, lengthSpinBox.value)
            loadData()
        }

        onOpened: {
            lengthSpinBox.value = sectionData ? sectionData.lengthBars : 8
        }
    }

    // Add placement dialog
    Dialog {
        id: addPlacementDialog
        anchors.centerIn: parent
        width: 300
        title: "Add Motif at Bar " + (targetBar + 1)
        modal: true

        property int targetBar: 0

        ColumnLayout {
            width: parent.width
            spacing: 8

            Text {
                visible: {
                    if (!sketchData || !sketchData.motifs) return true
                    for (var i = 0; i < sketchData.motifs.length; i++) {
                        if (canMotifFitAt(sketchData.motifs[i].lengthBars, addPlacementDialog.targetBar)) {
                            return false
                        }
                    }
                    return true
                }
                text: "No motifs fit at this position"
                color: "#888888"
                Layout.alignment: Qt.AlignHCenter
            }

            Repeater {
                model: sketchData && sketchData.motifs ? sketchData.motifs : []

                Button {
                    Layout.fillWidth: true
                    visible: canMotifFitAt(modelData.lengthBars, addPlacementDialog.targetBar)
                    text: modelData.name + " (" + modelData.lengthBars + " bars)"

                    onClicked: {
                        sketchManager.addPlacement(sketchId, sectionId, modelData.id, addPlacementDialog.targetBar, 1)
                        addPlacementDialog.close()
                        loadData()
                    }

                    background: Rectangle {
                        radius: 4
                        color: parent.pressed ? "#4A90E2" : "#3D3D3D"
                    }

                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }

    // Placement context menu
    Menu {
        id: placementMenu
        
        property int placementIndex: -1

        MenuItem {
            text: "Repeat ×2"
            onTriggered: {
                var current = sectionData.placements[placementMenu.placementIndex].repetitions
                sketchManager.setPlacementRepetitions(sketchId, sectionId, placementMenu.placementIndex, current + 1)
                loadData()
            }
        }

        MenuItem {
            text: "Remove"
            onTriggered: {
                sketchManager.removePlacement(sketchId, sectionId, placementMenu.placementIndex)
                loadData()
            }
        }
    }

    // No melody warning dialog
    Dialog {
        id: noMelodyDialog
        anchors.centerIn: parent
        width: 280
        title: "No Melody"
        modal: true
        standardButtons: Dialog.Ok

        Text {
            width: parent.width
            text: "Please add motifs to the Melody track first. The SATB voices will be generated from the melody you've arranged."
            color: "#CCCCCC"
            wrapMode: Text.WordWrap
        }
    }

    onVisibleChanged: {
        if (visible) loadData()
    }
}

