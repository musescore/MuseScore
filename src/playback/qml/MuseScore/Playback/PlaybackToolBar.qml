import QtQuick 2.7
import QtQuick.Controls 2.2

import MuseScore.Playback 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

Rectangle {
    id: root

    property bool floating: false

    PlaybackToolBarModel {
        id: playbackModel
    }

    Component.onCompleted: {
        playbackModel.load()
    }

    Column {
        spacing: 14

        anchors.verticalCenter: parent.verticalCenter

        height: childrenRect.height
        width: parent.width

        Row {
            id: playbackActions

            width: childrenRect.width
            height: childrenRect.height

            spacing: 2

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

            SeparatorLine { orientation: Qt.Vertical }

            Rectangle {
                height: parent.height
                width: 200
                opacity: 0.5
            }

            SeparatorLine { orientation: Qt.Vertical }
        }

        StyledSlider {
            width: playbackActions.width
            visible: root.floating
            value: playbackModel.playPosition
        }
    }
}
