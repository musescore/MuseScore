import QtQuick 2.15

import MuseScore.UiComponents 1.0

GradientTabButton {
    id: root

    property string userName: ""
    property string avatarUrl: ""

    signal userAuthorizedChanged()

    orientation: Qt.Horizontal

    title: "Stub Account"
}
