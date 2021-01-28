import QtQuick 2.7
import QtQuick.Controls 2.2

import MuseScore.Playback 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

Rectangle {
    id: root

    PlaybackToolBarModel {
        id: playbackModel
    }

    Component.onCompleted: {
        playbackModel.load()
    }

    Row {
        anchors.verticalCenter: parent.verticalCenter

        ListView {
            height: childrenRect.height
            width: childrenRect.width

            contentHeight: 32
            contentWidth: contentHeight

            model: playbackModel

            orientation: Qt.Horizontal
            interactive: false

            delegate: FlatButton {
                id: playbackButton

                icon: model.icon
                hint: model.hint
                enabled: model.enabled

                normalStateColor: model.checked ? ui.theme.accentColor : "transparent"

                onClicked: {
                    if (enabled) {
                        playbackModel.handleAction(model.code)
                    }
                }
            }
        }

        UndoRedoControls {}
    }
}
