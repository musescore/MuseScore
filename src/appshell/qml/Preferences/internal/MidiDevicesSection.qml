import QtQuick 2.15
import QtQuick.Layouts 1.12

import MuseScore.UiComponents 1.0

Column {
    id: root

    property alias currentInputDeviceIndex: inputDevicesComboBox.currentIndex
    property alias currentOutputDeviceIndex: ouputDevicesComboBox.currentIndex

    property alias inputDevices: inputDevicesComboBox.model
    property alias outputDevices: ouputDevicesComboBox.model

    property alias outputLatency: latencyControl.currentValue

    property int firstColumnWidth: 0

    signal currentInputDeviceIndexChangeRequested(int newIndex)
    signal currentOuputDeviceIndexChangeRequested(int newIndex)
    signal outputLatencyChangeRequested(int newLatencyMs)

    spacing: 18

    StyledTextLabel {
        text: qsTrc("appshell", "MIDI")
        font: ui.theme.bodyBoldFont
    }

    Column {
        width: parent.width
        spacing: 12

        Row {
            spacing: 0

            StyledTextLabel {
                anchors.verticalCenter: parent.verticalCenter

                width: root.firstColumnWidth
                horizontalAlignment: Qt.AlignLeft

                text: qsTrc("appshell", "MIDI input:")
            }

            StyledComboBox {
                id: inputDevicesComboBox

                width: 210

                onAccepted: {
                    root.currentInputDeviceIndexChangeRequested(currentIndex)
                }
            }
        }

        Row {
            spacing: 0

            StyledTextLabel {
                anchors.verticalCenter: parent.verticalCenter

                width: root.firstColumnWidth
                horizontalAlignment: Qt.AlignLeft

                text: qsTrc("appshell", "MIDI ouput:")
            }

            StyledComboBox {
                id: ouputDevicesComboBox

                width: inputDevicesComboBox.width

                onAccepted: {
                    root.currentOuputDeviceIndexChangeRequested(currentIndex)
                }
            }
        }

        Row {
            spacing: 0

            StyledTextLabel {
                anchors.verticalCenter: parent.verticalCenter

                width: root.firstColumnWidth
                horizontalAlignment: Qt.AlignLeft

                text: qsTrc("appshell", "MIDI ouput latency:")
            }

            IncrementalPropertyControl {
                id: latencyControl

                width: 75

                measureUnitsSymbol: qsTrc("global", "ms")

                decimals: 0
                step: 1

                onValueEdited: {
                    root.outputLatencyChangeRequested(newValue)
                }
            }
        }
    }
}
