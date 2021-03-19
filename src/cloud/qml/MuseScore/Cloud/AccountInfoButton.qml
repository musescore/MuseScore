import QtQuick 2.7

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Cloud 1.0

import "internal"

GradientTabButton {
    id: root

    property string userName: accountModel.accountInfo.userName
    property string avatarUrl: accountModel.accountInfo.avatarUrl

    signal userAuthorizedChanged()

    orientation: Qt.Horizontal

    spacing: Boolean(avatarUrl) ? 18 : 30
    leftPadding: spacing

    title: Boolean(userName) ? userName : qsTrc("cloud", "My Account")
    iconComponent: Boolean(avatarUrl) ? avatarComp : stubAvatarComp

    AccountModel {
        id: accountModel

        onUserAuthorizedChanged: {
            root.userAuthorizedChanged()
        }
    }

    Component.onCompleted: {
        accountModel.load()
    }

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
