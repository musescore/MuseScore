import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Page {
    id: root
    
    required property string sketchId
    required property string sketchName
    property var onBack: function() {}
    
    property int selectedFormat: 0 // 0=MIDI, 1=MusicXML, 2=MSCZ
    property bool isExporting: false
    property int exportProgress: 0
    property string exportStatus: ""
    
    // Data
    property var sketchData: null
    property var sectionsData: []
    
    // Selection state - maps id -> bool
    property var selectedSections: ({})
    property var selectedMotifs: ({})
    
    Component.onCompleted: {
        loadData()
    }
    
    function loadData() {
        sketchData = sketchManager.getSketch(sketchId)
        sectionsData = sketchManager.getSections(sketchId)
        
        // Default: all sections selected
        var secSel = {}
        for (var i = 0; i < sectionsData.length; i++) {
            secSel[sectionsData[i].id] = true
        }
        selectedSections = secSel
        
        // Default: no motifs selected (sections take priority)
        var motSel = {}
        if (sketchData && sketchData.motifs) {
            for (var j = 0; j < sketchData.motifs.length; j++) {
                motSel[sketchData.motifs[j].id] = false
            }
        }
        selectedMotifs = motSel
    }
    
    function getSelectedSectionIds() {
        var ids = []
        for (var id in selectedSections) {
            if (selectedSections[id]) {
                ids.push(id)
            }
        }
        return ids
    }
    
    function getSelectedMotifIds() {
        var ids = []
        for (var id in selectedMotifs) {
            if (selectedMotifs[id]) {
                ids.push(id)
            }
        }
        return ids
    }
    
    function hasSelection() {
        return getSelectedSectionIds().length > 0 || getSelectedMotifIds().length > 0
    }
    
    header: ToolBar {
        background: Rectangle {
            color: "#1E1E1E"
        }
        
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            
            ToolButton {
                text: "← Back"
                font.pixelSize: 16
                onClicked: root.onBack()
                
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
                text: "Export"
                color: "white"
                font.pixelSize: 18
                font.bold: true
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }
            
            Item { width: 60 } // Spacer
        }
    }
    
    background: Rectangle {
        color: "#1E1E1E"
    }
    
    Connections {
        target: exportEngine
        
        function onExportStarted(format) {
            root.isExporting = true
            root.exportStatus = "Exporting as " + format + "..."
            root.exportProgress = 0
        }
        
        function onExportProgress(percent) {
            root.exportProgress = percent
        }
        
        function onExportCompleted(filePath) {
            root.isExporting = false
            root.exportStatus = "Export complete!"
            exportSuccessDialog.filePath = filePath
            exportSuccessDialog.visible = true
        }
        
        function onExportFailed(error) {
            root.isExporting = false
            root.exportStatus = "Export failed: " + error
        }
    }
    
    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true
        
        ColumnLayout {
            width: parent.width
            spacing: 16
            
            Item { height: 4 } // Top padding
            
            // Content selection
            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                Layout.preferredHeight: contentColumn.height + 32
                color: "#2D2D2D"
                radius: 12
                
                ColumnLayout {
                    id: contentColumn
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 16
                    spacing: 12
                    
                    Text {
                        text: "Content to Export"
                        color: "white"
                        font.pixelSize: 16
                        font.bold: true
                    }
                    
                    // Sections header
                    RowLayout {
                        Layout.fillWidth: true
                        visible: sectionsData.length > 0
                        
                        Text {
                            text: "Sections"
                            color: "#AAAAAA"
                            font.pixelSize: 14
                        }
                        
                        Item { Layout.fillWidth: true }
                        
                        Text {
                            text: getSelectedSectionIds().length + " of " + sectionsData.length + " selected"
                            color: "#666666"
                            font.pixelSize: 12
                        }
                    }
                    
                    // Sections list
                    Repeater {
                        model: sectionsData
                        
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 48
                            color: selectedSections[modelData.id] ? "#3A5A8C" : "#252525"
                            radius: 8
                            border.color: selectedSections[modelData.id] ? "#4A90E2" : "#3D3D3D"
                            border.width: 1
                            
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    var newSel = Object.assign({}, selectedSections)
                                    newSel[modelData.id] = !newSel[modelData.id]
                                    selectedSections = newSel
                                }
                            }
                            
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 12
                                
                                Rectangle {
                                    width: 24
                                    height: 24
                                    radius: 4
                                    color: selectedSections[modelData.id] ? "#4A90E2" : "transparent"
                                    border.color: selectedSections[modelData.id] ? "#4A90E2" : "#666666"
                                    border.width: 2
                                    
                                    Text {
                                        visible: selectedSections[modelData.id]
                                        anchors.centerIn: parent
                                        text: "✓"
                                        color: "white"
                                        font.pixelSize: 14
                                        font.bold: true
                                    }
                                }
                                
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    
                                    Text {
                                        text: modelData.name
                                        color: "white"
                                        font.pixelSize: 14
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }
                                    
                                    Text {
                                        text: modelData.lengthBars + " bars • " + 
                                              (modelData.textureType === "SATBChorale" ? "SATB" : "Melody")
                                        color: "#888888"
                                        font.pixelSize: 11
                                    }
                                }
                            }
                        }
                    }
                    
                    // Divider if both sections and motifs exist
                    Rectangle {
                        visible: sectionsData.length > 0 && sketchData && sketchData.motifs && sketchData.motifs.length > 0
                        Layout.fillWidth: true
                        height: 1
                        color: "#3D3D3D"
                        Layout.topMargin: 8
                        Layout.bottomMargin: 8
                    }
                    
                    // Motifs header
                    RowLayout {
                        Layout.fillWidth: true
                        visible: sketchData && sketchData.motifs && sketchData.motifs.length > 0
                        
                        Text {
                            text: "Individual Motifs"
                            color: "#AAAAAA"
                            font.pixelSize: 14
                        }
                        
                        Item { Layout.fillWidth: true }
                        
                        Text {
                            text: getSelectedMotifIds().length + " of " + (sketchData && sketchData.motifs ? sketchData.motifs.length : 0) + " selected"
                            color: "#666666"
                            font.pixelSize: 12
                        }
                    }
                    
                    Text {
                        visible: sketchData && sketchData.motifs && sketchData.motifs.length > 0
                        text: "Export motifs not placed in sections"
                        color: "#666666"
                        font.pixelSize: 11
                        font.italic: true
                    }
                    
                    // Motifs list
                    Repeater {
                        model: sketchData ? sketchData.motifs : []
                        
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 48
                            color: selectedMotifs[modelData.id] ? "#3A5A8C" : "#252525"
                            radius: 8
                            border.color: selectedMotifs[modelData.id] ? "#4A90E2" : "#3D3D3D"
                            border.width: 1
                            
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    var newSel = Object.assign({}, selectedMotifs)
                                    newSel[modelData.id] = !newSel[modelData.id]
                                    selectedMotifs = newSel
                                }
                            }
                            
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 12
                                
                                Rectangle {
                                    width: 24
                                    height: 24
                                    radius: 4
                                    color: selectedMotifs[modelData.id] ? "#4A90E2" : "transparent"
                                    border.color: selectedMotifs[modelData.id] ? "#4A90E2" : "#666666"
                                    border.width: 2
                                    
                                    Text {
                                        visible: selectedMotifs[modelData.id]
                                        anchors.centerIn: parent
                                        text: "✓"
                                        color: "white"
                                        font.pixelSize: 14
                                        font.bold: true
                                    }
                                }
                                
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    
                                    Text {
                                        text: modelData.name
                                        color: "white"
                                        font.pixelSize: 14
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }
                                    
                                    Text {
                                        text: modelData.pitchContour.length + " notes • " + modelData.lengthBars + " bar" + (modelData.lengthBars === 1 ? "" : "s")
                                        color: "#888888"
                                        font.pixelSize: 11
                                    }
                                }
                            }
                        }
                    }
                    
                    // Empty state
                    Text {
                        visible: sectionsData.length === 0 && (!sketchData || !sketchData.motifs || sketchData.motifs.length === 0)
                        text: "No content to export.\nCreate sections or motifs first."
                        color: "#666666"
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.topMargin: 20
                    }
                }
            }
            
            // Format selection
            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                Layout.preferredHeight: formatColumn.height + 32
                color: "#2D2D2D"
                radius: 12
                
                ColumnLayout {
                    id: formatColumn
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 16
                    spacing: 12
                    
                    Text {
                        text: "Export Format"
                        color: "white"
                        font.pixelSize: 16
                        font.bold: true
                    }
                    
                    // MIDI format
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 56
                        color: selectedFormat === 0 ? "#3A5A8C" : "#252525"
                        radius: 8
                        border.color: selectedFormat === 0 ? "#4A90E2" : "#3D3D3D"
                        border.width: 1
                        
                        MouseArea {
                            anchors.fill: parent
                            onClicked: selectedFormat = 0
                        }
                        
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 12
                            
                            Rectangle {
                                width: 32
                                height: 32
                                radius: 6
                                color: "#4A9050"
                                
                                Text {
                                    anchors.centerIn: parent
                                    text: ".mid"
                                    color: "white"
                                    font.pixelSize: 9
                                    font.bold: true
                                }
                            }
                            
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2
                                
                                Text {
                                    text: "MIDI"
                                    color: "white"
                                    font.pixelSize: 14
                                    font.bold: true
                                }
                                
                                Text {
                                    text: "DAWs, synthesizers"
                                    color: "#888888"
                                    font.pixelSize: 11
                                }
                            }
                            
                            Rectangle {
                                width: 20
                                height: 20
                                radius: 10
                                color: selectedFormat === 0 ? "#4A90E2" : "transparent"
                                border.color: selectedFormat === 0 ? "#4A90E2" : "#666666"
                                border.width: 2
                                
                                Rectangle {
                                    visible: selectedFormat === 0
                                    anchors.centerIn: parent
                                    width: 8
                                    height: 8
                                    radius: 4
                                    color: "white"
                                }
                            }
                        }
                    }
                    
                    // MusicXML format
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 56
                        color: selectedFormat === 1 ? "#3A5A8C" : "#252525"
                        radius: 8
                        border.color: selectedFormat === 1 ? "#4A90E2" : "#3D3D3D"
                        border.width: 1
                        
                        MouseArea {
                            anchors.fill: parent
                            onClicked: selectedFormat = 1
                        }
                        
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 12
                            
                            Rectangle {
                                width: 32
                                height: 32
                                radius: 6
                                color: "#9050A0"
                                
                                Text {
                                    anchors.centerIn: parent
                                    text: ".xml"
                                    color: "white"
                                    font.pixelSize: 9
                                    font.bold: true
                                }
                            }
                            
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2
                                
                                Text {
                                    text: "MusicXML"
                                    color: "white"
                                    font.pixelSize: 14
                                    font.bold: true
                                }
                                
                                Text {
                                    text: "Finale, Sibelius, MuseScore"
                                    color: "#888888"
                                    font.pixelSize: 11
                                }
                            }
                            
                            Rectangle {
                                width: 20
                                height: 20
                                radius: 10
                                color: selectedFormat === 1 ? "#4A90E2" : "transparent"
                                border.color: selectedFormat === 1 ? "#4A90E2" : "#666666"
                                border.width: 2
                                
                                Rectangle {
                                    visible: selectedFormat === 1
                                    anchors.centerIn: parent
                                    width: 8
                                    height: 8
                                    radius: 4
                                    color: "white"
                                }
                            }
                        }
                    }
                }
            }
            
            // Export status
            Rectangle {
                visible: root.isExporting || root.exportStatus !== ""
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                Layout.preferredHeight: 50
                color: "#2D2D2D"
                radius: 8
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 6
                    
                    Text {
                        text: root.exportStatus
                        color: root.exportStatus.includes("failed") ? "#E74C3C" : 
                               root.exportStatus.includes("complete") ? "#27AE60" : "white"
                        font.pixelSize: 13
                    }
                    
                    ProgressBar {
                        visible: root.isExporting
                        Layout.fillWidth: true
                        value: root.exportProgress / 100
                        
                        background: Rectangle {
                            implicitHeight: 4
                            color: "#1E1E1E"
                            radius: 2
                        }
                        
                        contentItem: Item {
                            Rectangle {
                                width: parent.parent.visualPosition * parent.width
                                height: parent.height
                                radius: 2
                                color: "#4A90E2"
                            }
                        }
                    }
                }
            }
            
            // Export button
            Button {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                Layout.preferredHeight: 50
                enabled: !root.isExporting && hasSelection()
                
                onClicked: {
                    var filename = exportEngine.suggestedFilename(root.sketchName, selectedFormat)
                    var filePath = downloadPath + "/" + filename
                    
                    var sectionIds = getSelectedSectionIds()
                    var motifIds = getSelectedMotifIds()
                    
                    if (selectedFormat === 0) {
                        exportEngine.exportToMIDI(root.sketchId, filePath, sectionIds, motifIds)
                    } else if (selectedFormat === 1) {
                        exportEngine.exportToMusicXML(root.sketchId, filePath, sectionIds, motifIds)
                    }
                }
                
                background: Rectangle {
                    radius: 10
                    color: parent.enabled ? (parent.pressed ? "#3A7BC8" : "#4A90E2") : "#3D3D3D"
                }
                
                contentItem: Text {
                    text: root.isExporting ? "Exporting..." : 
                          !hasSelection() ? "Select content to export" : "Export to Downloads"
                    color: parent.enabled ? "white" : "#888888"
                    font.pixelSize: 15
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
            
            Item { height: 16 } // Bottom padding
        }
    }
    
    // Success dialog
    Rectangle {
        id: exportSuccessDialog
        visible: false
        anchors.fill: parent
        color: "#00000080"
        
        property string filePath: ""
        
        Rectangle {
            anchors.centerIn: parent
            width: 280
            height: 200
            radius: 12
            color: "#2D2D2D"
            border.color: "#4A4A4A"
            border.width: 1
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 12
                
                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 50
                    height: 50
                    radius: 25
                    color: "#27AE60"
                    
                    Text {
                        anchors.centerIn: parent
                        text: "✓"
                        color: "white"
                        font.pixelSize: 28
                        font.bold: true
                    }
                }
                
                Text {
                    Layout.fillWidth: true
                    text: "Export Successful!"
                    color: "white"
                    font.pixelSize: 17
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                }
                
                Text {
                    Layout.fillWidth: true
                    text: "File saved to Downloads"
                    color: "#AAAAAA"
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter
                }
                
                Button {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    text: "Done"
                    
                    onClicked: {
                        exportSuccessDialog.visible = false
                        root.exportStatus = ""
                    }
                    
                    background: Rectangle {
                        radius: 8
                        color: parent.pressed ? "#3A7BC8" : "#4A90E2"
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }
}
