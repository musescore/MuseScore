import QtQuick 2.7
import QtQuick.Controls 2.2

import MuseScore.Playback 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    width: contentRow.width

    Row {
        id: contentRow

        anchors.verticalCenter: parent.verticalCenter

        Repeater {
            model: toolModel

            FlatButton {
                text: titleRole

                width: 60

                normalStateColor: checkedRole ? ui.theme.accentColor : "transparent"
                enabled: enabledRole

                onClicked: {
                    if (enabled) {
                        toolModel.click(codeRole)
                    }
                }
            }
        }
    }

    PlaybackToolBarModel {
        id: toolModel
    }

    Component.onCompleted: {
        toolModel.load()
    }
}
