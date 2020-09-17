import QtQuick 2.7

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Cloud 1.0

GradientTabButton {
    id: root

    property string userName: ""
    property string avatarUrl: ""

    orientation: Qt.Horizontal

    title: Boolean(userName) ? userName : qsTrc("cloud", "My Account")
    iconComponent: Boolean(avatarUrl) ? avatarComp : stubAvatarComp

    Component {
        id: stubAvatarComp
        StyledIconLabel {
            anchors.verticalCenter: parent.verticalCenter
            height: root.height
            iconCode: IconCode.ACCOUNT
            visible: !Boolean(root.avatarUrl)
        }
    }

    Component {
        id: avatarComp
        AccountAvatar {
            url: root.avatarUrl
            side: 40
        }
    }
}
