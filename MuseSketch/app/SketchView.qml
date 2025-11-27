import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "components"

Page {
    id: root
    
    property string sketchId: ""
    property var sketchData: null
    
    property var sectionsData: []
    
    signal backRequested()
    signal createMotifRequested()
    signal editMotifRequested(string motifId)
    signal editSectionRequested(string sectionId)
    signal exportRequested()
    
    Component.onCompleted: {
        loadSketch()
    }
    
    function loadSketch() {
        sketchData = sketchManager.getSketch(sketchId)
        sectionsData = sketchManager.getSections(sketchId)
    }
    
    header: ToolBar {
        background: Rectangle {
            color: "#2D2D2D"
        }
        
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            spacing: 8
            
            ToolButton {
                text: "←"
                font.pixelSize: 24
                onClicked: root.backRequested()
                
                contentItem: Text {
                    text: parent.text
                    color: "#4A90E2"
                    font: parent.font
                    verticalAlignment: Text.AlignVCenter
                }
                
                background: Rectangle {
                    color: "transparent"
                }
            }
            
            Text {
                Layout.fillWidth: true
                text: sketchData ? sketchData.name : "Sketch"
                color: "white"
                font.pixelSize: 20
                font.bold: true
            }
            
            Button {
                text: "Export"
                font.pixelSize: 14
                
                onClicked: root.exportRequested()
                
                background: Rectangle {
                    radius: 6
                    color: parent.pressed ? "#3A7BC8" : "#4A90E2"
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
            
            Button {
                text: "+ New"
                font.pixelSize: 12
                
                onClicked: root.createMotifRequested()
                
                background: Rectangle {
                    radius: 4
                    color: parent.pressed ? "#3A7BC8" : "#4A90E2"
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
        
        // Sections Header
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 12
            spacing: 12
            
            Text {
                text: "Sections"
                color: "white"
                font.pixelSize: 18
            font.bold: true
            }
            
            Text {
                text: "(" + sectionsData.length + ")"
                color: "#AAAAAA"
                font.pixelSize: 18
            }
            
            Item { Layout.fillWidth: true }
            
            Button {
                text: "+ New"
                font.pixelSize: 12
                
                onClicked: newSectionDialog.open()
            
            background: Rectangle {
                    radius: 4
                color: parent.pressed ? "#3A7BC8" : "#4A90E2"
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
        
        // Sections List
        ListView {
            id: sectionsListView
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(sectionsData.length * 68, 200)
            
            model: sectionsData
            spacing: 8
            clip: true
            
            delegate: Rectangle {
                width: sectionsListView.width
                height: 60
                color: sectionMouseArea.pressed ? "#3D3D3D" : "#2D2D2D"
                radius: 8
                
                MouseArea {
                    id: sectionMouseArea
                    anchors.fill: parent
                    onClicked: root.editSectionRequested(modelData.id)
                    
                    onPressAndHold: {
                        sectionDeleteDialog.sectionId = modelData.id
                        sectionDeleteDialog.sectionName = modelData.name
                        sectionDeleteDialog.open()
                    }
                }
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12
                    
                    Rectangle {
                        Layout.preferredWidth: 36
                        Layout.preferredHeight: 36
                        radius: 4
                        color: "#9B59B6"
                        
                        Text {
                            anchors.centerIn: parent
                            text: "§"
                            color: "white"
                            font.pixelSize: 20
                            font.bold: true
                        }
                    }
                    
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        
                        Text {
                            text: modelData.name
                            color: "white"
                            font.pixelSize: 16
                            font.bold: true
                        }
                        
                        Text {
                            text: modelData.lengthBars + " bars • " + modelData.placementCount + " motifs"
                            color: "#AAAAAA"
                            font.pixelSize: 12
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
                visible: sectionsListView.count === 0
                color: "#2D2D2D"
                radius: 8
                
                Text {
                    anchors.centerIn: parent
                    text: "No sections yet\nCreate one to arrange motifs"
                    color: "#666666"
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                }
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
    
    // New Section Dialog Overlay
    Rectangle {
        id: newSectionDialogOverlay
        anchors.fill: parent
        color: "#80000000"
        visible: newSectionDialog.visible
        z: 100
        
        MouseArea {
            anchors.fill: parent
            onClicked: newSectionDialog.close()
        }
    }
    
    // New Section Dialog
    Rectangle {
        id: newSectionDialog
        anchors.centerIn: parent
        width: 300
        height: 220
        radius: 12
        color: "#2D2D2D"
        border.color: "#4A4A4A"
        border.width: 1
        visible: false
        z: 101
        
        function open() { 
            sectionNameField.text = "Section " + (sectionsData.length + 1)
            visible = true
            sectionNameField.forceActiveFocus()
            sectionNameField.selectAll()
        }
        function close() { visible = false }
        function accept() { 
            if (sectionNameField.text.trim() !== "") {
                var sectionId = sketchManager.createSection(root.sketchId, sectionNameField.text.trim(), sectionLengthSpinBox.value)
                loadSketch()
                close()
                root.editSectionRequested(sectionId)
            }
        }
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 16
            
            Text {
                text: "New Section"
                color: "white"
                font.pixelSize: 18
                font.bold: true
            }
            
            TextField {
                id: sectionNameField
                Layout.fillWidth: true
                Layout.preferredHeight: 44
                leftPadding: 12
                topPadding: 12
                bottomPadding: 12
                placeholderText: "Section name"
                placeholderTextColor: "#888888"
                color: "white"
                font.pixelSize: 16
                selectByMouse: true
                
                background: Rectangle {
                    color: "#1E1E1E"
                    radius: 8
                    border.color: sectionNameField.activeFocus ? "#4A90E2" : "#4A4A4A"
                    border.width: 1
                }
                
                onAccepted: newSectionDialog.accept()
            }
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                
                Text {
                    text: "Length:"
                    color: "#AAAAAA"
                    font.pixelSize: 14
                }
                
                Rectangle {
                    width: 80
                    height: 36
                    color: "#1E1E1E"
                    radius: 6
                    border.color: "#4A4A4A"
                    border.width: 1
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 4
                        spacing: 0
                        
                        Rectangle {
                            width: 24
                            height: 28
                            radius: 4
                            color: minusArea.pressed ? "#4A4A4A" : "transparent"
                            
                            Text {
                                anchors.centerIn: parent
                                text: "−"
                                color: "white"
                                font.pixelSize: 16
                            }
                            
                            MouseArea {
                                id: minusArea
                                anchors.fill: parent
                                onClicked: if (sectionLengthSpinBox.value > 1) sectionLengthSpinBox.value--
                            }
                        }
                        
                        Text {
                            Layout.fillWidth: true
                            text: sectionLengthSpinBox.value
                            color: "white"
                            font.pixelSize: 14
                            horizontalAlignment: Text.AlignHCenter
                        }
                        
                        Rectangle {
                            width: 24
                            height: 28
                            radius: 4
                            color: plusArea.pressed ? "#4A4A4A" : "transparent"
                            
                            Text {
                                anchors.centerIn: parent
                                text: "+"
                                color: "white"
                                font.pixelSize: 16
                            }
                            
                            MouseArea {
                                id: plusArea
                                anchors.fill: parent
                                onClicked: if (sectionLengthSpinBox.value < 16) sectionLengthSpinBox.value++
                            }
                        }
                    }
                    
                    // Hidden SpinBox for value storage
                    SpinBox {
                        id: sectionLengthSpinBox
                        visible: false
                        from: 1
                        to: 16
                        value: 8
                    }
                }
                
                Text {
                    text: "bars"
                    color: "#AAAAAA"
                    font.pixelSize: 14
                }
                
                Item { Layout.fillWidth: true }
            }
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                
                Button {
                    Layout.fillWidth: true
                    implicitHeight: 40
                    text: "Cancel"
                    
                    onClicked: newSectionDialog.close()
                    
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
                    
                    onClicked: newSectionDialog.accept()
                    
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
    
    // Section Delete Dialog Overlay
    Rectangle {
        id: sectionDeleteDialogOverlay
        anchors.fill: parent
        color: "#80000000"
        visible: sectionDeleteDialog.visible
        z: 100
        
        MouseArea {
            anchors.fill: parent
            onClicked: sectionDeleteDialog.close()
        }
    }
    
    // Section Delete Dialog
    Rectangle {
        id: sectionDeleteDialog
        anchors.centerIn: parent
        width: 300
        height: 180
        radius: 12
        color: "#2D2D2D"
        border.color: "#4A4A4A"
        border.width: 1
        visible: false
        z: 101
        
        property string sectionId: ""
        property string sectionName: ""
        
        function open() { visible = true }
        function close() { visible = false }
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 16
            
            Text {
                text: "Delete Section"
                color: "white"
                font.pixelSize: 18
                font.bold: true
            }
            
            Text {
                Layout.fillWidth: true
                text: "Are you sure you want to delete \"" + sectionDeleteDialog.sectionName + "\"?"
                color: "#AAAAAA"
                font.pixelSize: 14
                wrapMode: Text.WordWrap
            }
            
            Item { Layout.fillHeight: true }
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                
                Button {
                    Layout.fillWidth: true
                    implicitHeight: 40
                    text: "Cancel"
                    
                    onClicked: sectionDeleteDialog.close()
                    
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
                    text: "Delete"
                    
                    onClicked: {
                        sketchManager.deleteSection(root.sketchId, sectionDeleteDialog.sectionId)
                        loadSketch()
                        sectionDeleteDialog.close()
                    }
                    
                    background: Rectangle {
                        radius: 8
                        color: parent.pressed ? "#CC3333" : "#E74C3C"
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
    
    // Reload sketch data when we return to this view
    onVisibleChanged: {
        if (visible) {
            loadSketch()
        }
    }
}
