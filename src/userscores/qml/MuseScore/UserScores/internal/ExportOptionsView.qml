import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.UserScores 1.0
import MuseScore.UiComponents 1.0

ColumnLayout {
    id: root
    required property ExportDialogModel exportModel

    spacing: 12

    QtObject {
        id: privateProperties

        readonly property int firstColumnWidth: 72
    }

    ExportOptionItem {
        firstColumnWidth: privateProperties.firstColumnWidth
        text: qsTrc("userscores", "Format:")

        StyledComboBox {
            id: fileTypeComboBox
            Layout.fillWidth: true

            model: [
                { textRole: qsTrc("userscores", "PDF File"), valueRole: "pdf" },
                { textRole: qsTrc("userscores", "PNG Images"), valueRole: "png" },
                { textRole: qsTrc("userscores", "SVG Images"), valueRole: "svg" },
                { textRole: qsTrc("userscores", "MP3 Audio"), valueRole: "mp3" },
                { textRole: qsTrc("userscores", "WAV Audio"), valueRole: "wav" },
                { textRole: qsTrc("userscores", "OGG Audio"), valueRole: "ogg" },
                { textRole: qsTrc("userscores", "FLAC Audio"), valueRole: "flac" },
                { textRole: qsTrc("userscores", "MIDI File"), valueRole: "mid" },
                { textRole: qsTrc("userscores", "MusicXML"), valueRole: "xml" }
            ]

            maxVisibleItemCount: count

            textRoleName: "textRole"
            valueRoleName: "valueRole"

            currentIndex: {
                if (exportModel.selectedExportSuffix === "musicxml" || exportModel.selectedExportSuffix === "mxl") {
                    return indexOfValue("xml")
                } else {
                    return indexOfValue(exportModel.selectedExportSuffix)
                }
            }

            onValueChanged: {
                if (value === "xml") {
                    exportModel.selectedExportSuffix = "mxl";
                } else {
                    exportModel.selectedExportSuffix = value
                }
            }
        }
    }

    ExportOptionItem {
        visible: exportModel.selectedExportSuffix === "pdf"
        text: qsTrc("userscores", "Resolution:")
        firstColumnWidth: privateProperties.firstColumnWidth

        IncrementalPropertyControl {
            Layout.preferredWidth: 80
            currentValue: exportModel.pdfResolution
            minValue: 72
            maxValue: 2400
            step: 1
            decimals: 0
            measureUnitsSymbol: qsTrc("userscores", "dpi")
            onValueEdited: {
                exportModel.pdfResolution = newValue
            }
        }
    }

    ColumnLayout {
        visible: exportModel.selectedExportSuffix === "png"
        spacing: 12

        ExportOptionItem {
            text: qsTrc("userscores", "Resolution:")
            firstColumnWidth: privateProperties.firstColumnWidth

            IncrementalPropertyControl {
                Layout.preferredWidth: 80
                currentValue: exportModel.pngResolution
                minValue: 32
                maxValue: 5000
                step: 1
                decimals: 0
                measureUnitsSymbol: qsTrc("userscores", "dpi")
                onValueEdited: {
                    exportModel.pngResolution = newValue
                }
            }
        }

        CheckBox {
            text: qsTrc("userscores", "Transparent background")
            checked: exportModel.pngTransparentBackground
            onClicked: {
                exportModel.pngTransparentBackground = !checked
            }
        }

        StyledTextLabel {
            Layout.fillWidth: true
            text: qsTrc("userscores", "Each page of the selected parts will be exported as a separate PNG file.")
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
        }
    }

    StyledTextLabel {
        visible: exportModel.selectedExportSuffix === "svg"
        Layout.fillWidth: true
        text: qsTrc("userscores", "Each page of the selected parts will be exported as a separate SVG file.")
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WordWrap
    }

    ColumnLayout {
        id: audioOptions
        spacing: 12

        visible: ["mp3", "wav", "ogg", "flac"].includes(exportModel.selectedExportSuffix)

        CheckBox {
            text: qsTrc("userscores", "Normalize")
            checked: exportModel.normalizeAudio
            onClicked: {
                exportModel.normalizeAudio = !checked
            }
        }

        ExportOptionItem {
            text: qsTrc("userscores", "Sample rate:")
            firstColumnWidth: privateProperties.firstColumnWidth

            StyledComboBox {
                Layout.preferredWidth: 126

                model: exportModel.availableSampleRates().map(
                           (sampleRate) => ({ textRole: qsTrc("userscores", "%1 Hz").arg(sampleRate), valueRole: sampleRate })
                           )

                textRoleName: "textRole"
                valueRoleName: "valueRole"

                currentIndex: indexOfValue(exportModel.sampleRate)
                onValueChanged: {
                    exportModel.sampleRate = value
                }
            }
        }

        ExportOptionItem {
            visible: exportModel.selectedExportSuffix === "mp3"
            text: qsTrc("userscores", "Bitrate:")
            firstColumnWidth: privateProperties.firstColumnWidth

            StyledComboBox {
                Layout.preferredWidth: 126

                model: exportModel.availableBitRates().map(
                           (bitRate) => ({ textRole: qsTrc("userscores", "%1 kBit/s").arg(bitRate), valueRole: bitRate })
                           )

                textRoleName: "textRole"
                valueRoleName: "valueRole"

                currentIndex: indexOfValue(exportModel.bitRate)
                onValueChanged: {
                    exportModel.bitRate = value
                }
            }
        }

        StyledTextLabel {
            Layout.fillWidth: true
            text: qsTrc("userscores", "Each selected part will be exported as a separate audio file.")
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
        }
    }

    ColumnLayout {
        visible: fileTypeComboBox.value === "mid"
        spacing: 12

        CheckBox {
            text: qsTrc("userscores", "Expand repeats")
            checked: exportModel.midiExpandRepeats
            onClicked: {
                exportModel.midiExpandRepeats = !checked
            }
        }

        CheckBox {
            text: qsTrc("userscores", "Export RPNs")
            checked: exportModel.midiExportRpns
            onClicked: {
                exportModel.midiExportRpns = !checked
            }
        }

        StyledTextLabel {
            Layout.fillWidth: true
            text: qsTrc("userscores", "Each selected part will be exported as a separate MIDI file.")
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        visible: fileTypeComboBox.value === "xml"
        spacing: 12

        ExportOptionItem {
            text: qsTrc("userscores", "File type:")
            firstColumnWidth: privateProperties.firstColumnWidth

            StyledComboBox {
                id: xmlTypeComboBox
                Layout.fillWidth: true

                model: [
                    { textRole: qsTrc("userscores", "Compressed") + " (*.mxl)", valueRole: "mxl" },
                    { textRole: qsTrc("userscores", "Uncompressed") + " (*.musicxml)", valueRole: "musicxml" },
                    { textRole: qsTrc("userscores", "Uncompressed (outdated)") + " (*.xml)", valueRole: "xml" }
                ]

                textRoleName: "textRole"
                valueRoleName: "valueRole"

                currentIndex: indexOfValue(exportModel.selectedExportSuffix)
                onValueChanged: {
                    exportModel.selectedExportSuffix = value
                }
            }
        }

        RadioButtonGroup {
            spacing: 12
            orientation: Qt.Vertical
            Layout.fillWidth: true
            model: exportModel.musicXmlLayoutTypes()

            delegate: RoundedRadioButton {
                text: modelData["text"]
                width: parent.width
                checked: exportModel.musicXmlLayoutType === modelData["value"]
                onToggled: {
                    exportModel.musicXmlLayoutType = modelData["value"]
                }
            }
        }
    }

    RadioButtonGroup {
        Layout.fillWidth: true
        visible: model.length > 1
        spacing: 12
        orientation: Qt.Vertical

        model: exportModel.availableUnitTypes

        delegate: RoundedRadioButton {
            text: modelData["text"]
            width: parent.width
            checked: exportModel.selectedUnitType === modelData["value"]
            onToggled: {
                exportModel.selectedUnitType = modelData["value"]
            }
        }
    }
}
