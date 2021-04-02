import QtQuick 2.15
import QtQuick.Layouts 1.12

import MuseScore.UiComponents 1.0

Column {
    id: root

    property int firstColumnWidth: 0

    property alias currentAudioApiIndex: apiComboBox.currentIndex
    property alias audioApiList: apiComboBox.model

    signal currentAudioApiIndexChangeRequested(int newIndex)

    width: parent.width
    spacing: 18

    StyledTextLabel {
        text: qsTrc("appshell", "Audio")
        font: ui.theme.bodyBoldFont
    }

    Column {
        spacing: 12

        ComboBoxWithTitle {
            id: apiComboBox

            title: qsTrc("appshell", "Audio API:")
            titleWidth: root.firstColumnWidth

            onValueEdited: {
                root.currentAudioApiIndexChangeRequested(currentIndex)
            }
        }

        CommonAudioApiConfiguration {
            firstColumnWidth: root.firstColumnWidth
        }
    }
}
