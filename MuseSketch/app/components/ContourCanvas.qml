import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    
    color: "#2D2D2D"
    radius: 8
    border.color: "#4A90E2"
    border.width: 2
    
    property var strokes: []         // Array of strokes, each stroke is an array of points
    property var currentStroke: []   // Current stroke being drawn
    property bool hasStrokes: strokes.length > 0
    
    signal contourDrawn(var points)
    
    // Flatten all strokes into a single array for the quantizer
    function getAllPoints() {
        var allPoints = [];
        for (var s = 0; s < strokes.length; s++) {
            for (var p = 0; p < strokes[s].length; p++) {
                allPoints.push(strokes[s][p]);
            }
        }
        return allPoints;
    }
    
    // Timer to auto-commit after user stops drawing
    Timer {
        id: commitTimer
        interval: 1500  // 1.5 seconds after last stroke
        onTriggered: {
            var allPoints = root.getAllPoints();
            if (allPoints.length > 0) {
                root.contourDrawn(allPoints)
            }
        }
    }
    
    // Grid lines for visual reference (scale degrees)
    Column {
        anchors.fill: parent
        Repeater {
            model: 8  // 7 degrees + top/bottom margins
            
            Rectangle {
                width: root.width
                height: root.height / 8
                color: "transparent"
                border.color: "#3D3D3D"
                border.width: index > 0 && index < 8 ? 1 : 0
                
                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    text: index > 0 && index < 8 ? (8 - index).toString() : ""
                    color: "#666666"
                    font.pixelSize: 12
                }
            }
        }
    }
    
    // Drawing canvas
    Canvas {
        id: canvas
        anchors.fill: parent
        anchors.margins: 4
        
        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            
            // Helper function to draw a single stroke
            function drawStroke(points, color, drawDots) {
                if (points.length < 1) return;
                
                ctx.strokeStyle = color;
                ctx.lineWidth = 3;
                ctx.lineCap = "round";
                ctx.lineJoin = "round";
                
                if (points.length >= 2) {
                    ctx.beginPath();
                    for (var i = 0; i < points.length; i++) {
                        var point = points[i];
                        var x = point.x * width;
                        var y = point.y * height;
                        
                        if (i === 0) {
                            ctx.moveTo(x, y);
                        } else {
                            ctx.lineTo(x, y);
                        }
                    }
                    ctx.stroke();
                }
                
                // Draw dots at each point
                if (drawDots) {
                    ctx.fillStyle = color;
                    for (var j = 0; j < points.length; j++) {
                        var pt = points[j];
                        var px = pt.x * width;
                        var py = pt.y * height;
                        ctx.beginPath();
                        ctx.arc(px, py, 4, 0, 2 * Math.PI);
                        ctx.fill();
                    }
                }
            }
            
            // Draw each committed stroke separately (no connecting lines between them)
            for (var s = 0; s < root.strokes.length; s++) {
                drawStroke(root.strokes[s], "#4A90E2", true);
            }
            
            // Draw current stroke being drawn (in progress)
            drawStroke(root.currentStroke, "#7AB8F5", false);
        }
    }
    
    // Touch/Mouse area for drawing
    MouseArea {
        id: drawArea
        anchors.fill: parent
        preventStealing: true  // Prevent ScrollView from stealing drag gestures
        
        property bool isDrawing: false
        property real firstStrokeTime: 0  // Time when first stroke started (for all strokes)
        
        onPressed: function(mouse) {
            // Stop the commit timer when starting a new stroke
            commitTimer.stop();
            
            isDrawing = true;
            
            // Track the first stroke time for the entire drawing session
            if (root.strokes.length === 0) {
                firstStrokeTime = Date.now();
            }
            
            root.currentStroke = [];
            addPoint(mouse.x, mouse.y);
        }
        
        onPositionChanged: function(mouse) {
            if (isDrawing) {
                addPoint(mouse.x, mouse.y);
            }
        }
        
        onReleased: function(mouse) {
            if (isDrawing) {
                isDrawing = false;
                
                // Add current stroke to strokes array (if it has points)
                if (root.currentStroke.length > 0) {
                    var strokesCopy = root.strokes.slice();
                    strokesCopy.push(root.currentStroke.slice());
                    root.strokes = strokesCopy;
                }
                root.currentStroke = [];
                
                // Start timer to auto-commit after a pause
                commitTimer.restart();
                
                canvas.requestPaint();
            }
        }
        
        function addPoint(x, y) {
            // Normalize coordinates to [0.0, 1.0]
            var normalizedX = x / canvas.width;
            var normalizedY = y / canvas.height;
            
            // Clamp to valid range
            normalizedX = Math.max(0.0, Math.min(1.0, normalizedX));
            normalizedY = Math.max(0.0, Math.min(1.0, normalizedY));
            
            // Calculate time since FIRST stroke start (continuous timeline)
            var timestamp = (Date.now() - firstStrokeTime) / 1000.0;
            
            root.currentStroke.push({
                x: normalizedX,
                y: normalizedY,
                t: timestamp
            });
            
            canvas.requestPaint();
        }
    }
    
    // Instruction text when empty
    Text {
        anchors.centerIn: parent
        visible: root.strokes.length === 0 && root.currentStroke.length === 0
        text: "Draw your pitch contour here\nLift finger to add rests\nAuto-commits after 1.5s pause"
        color: "#666666"
        font.pixelSize: 14
        horizontalAlignment: Text.AlignHCenter
    }
    
    // Button row
    Row {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 8
        spacing: 8
        visible: root.strokes.length > 0 || root.currentStroke.length > 0
        
        // Done button (commit now)
        Button {
            text: "Done"
            
            onClicked: {
                commitTimer.stop();
                var allPoints = root.getAllPoints();
                if (allPoints.length > 0) {
                    root.contourDrawn(allPoints);
                }
            }
            
            background: Rectangle {
                radius: 4
                color: parent.pressed ? "#3A7BC8" : "#4A90E2"
            }
            
            contentItem: Text {
                text: parent.text
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 12
                font.bold: true
            }
        }
        
        // Clear button
        Button {
            text: "Clear"
            
            onClicked: {
                commitTimer.stop();
                root.strokes = [];
                root.currentStroke = [];
                canvas.requestPaint();
            }
            
            background: Rectangle {
                radius: 4
                color: parent.pressed ? "#3D3D3D" : "#2D2D2D"
                border.color: "#666666"
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
    
    // Status indicator showing commit timer
    Text {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 8
        visible: commitTimer.running
        text: "Will commit in 1.5s... (tap Done to commit now)"
        color: "#888888"
        font.pixelSize: 11
    }
}
