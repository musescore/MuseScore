import QtQuick 2.15
import QtQuick.Layouts 1.12

import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

Item {
    id: root

    property int firstColumnWidth: 0

    width: parent.width
    height: content.height

    CommonAudioApiConfigurationModel {
        id: apiModel
    }

    Column {
        id: content

        spacing: 12

        ComboBoxWithTitle {
            title: qsTrc("appshell", "Audio device:")
            titleWidth: root.firstColumnWidth

            currentIndex: apiModel.currentDeviceIndex
            model: apiModel.deviceList()

            onValueEdited: {
                apiModel.currentDeviceIndex = currentIndex
            }
        }

        ComboBoxWithTitle {
            title: qsTrc("appshell", "Sample rate:")
            titleWidth: root.firstColumnWidth

            currentIndex: apiModel.currentSampleRateIndex
            model: apiModel.sampleRateHzList()
            control.displayText: currentValue

            onValueEdited: {
                apiModel.currentSampleRateIndex = currentIndex
            }
        }
    }
}
