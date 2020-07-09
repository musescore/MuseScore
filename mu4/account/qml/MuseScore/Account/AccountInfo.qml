import QtQuick 2.7
import QtGraphicalEffects 1.0

import MuseScore.UiComponents 1.0
import MuseScore.Account 1.0

Item {
    id: root

    implicitHeight: 40

    AccountModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }

    Row {
        anchors.fill: parent
        spacing: 16

        Image {
            id: avatar

            width: 40
            height: width

            fillMode: Image.PreserveAspectCrop

            layer.enabled: true
            layer.effect: OpacityMask {
                maskSource: Rectangle {
                    width: avatar.width
                    height: avatar.height
                    radius: width / 2
                    visible: false
                }
            }

            source: model.accountInfo.avatarUrl
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter

            text: model.accountInfo.userName
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: model.logIn()
    }
}
