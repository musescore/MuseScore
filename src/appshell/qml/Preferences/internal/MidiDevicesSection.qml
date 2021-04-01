import QtQuick 2.15
import QtQuick.Layouts 1.12

import MuseScore.UiComponents 1.0

Column {
    id: root

    property alias currentInputDeviceIndex: inputDevicesBox.currentIndex
    property alias currentOutputDeviceIndex: outputDevicesBox.currentIndex

    property alias inputDevices: inputDevicesBox.model
    property alias outputDevices: outputDevicesBox.model

    property int firstColumnWidth: 0

    signal currentInputDeviceIndexChangeRequested(int newIndex)
    signal currentOuputDeviceIndexChangeRequested(int newIndex)

    spacing: 18

    StyledTextLabel {
        text: qsTrc("appshell", "MIDI")
        font: ui.theme.bodyBoldFont
    }

    Column {
        width: parent.width
        spacing: 12

        ComboBoxWithTitle {
            id: inputDevicesBox

            title: qsTrc("appshell", "MIDI input:")
            titleWidth: root.firstColumnWidth

            onValueEdited: {
                root.currentInputDeviceIndexChangeRequested(currentIndex)
            }
        }

        ComboBoxWithTitle {
            id: outputDevicesBox

            title: qsTrc("appshell", "MIDI ouput:")
            titleWidth: root.firstColumnWidth

            onValueEdited: {
                root.currentOuputDeviceIndexChangeRequested(currentIndex)
            }
        }
    }
}
