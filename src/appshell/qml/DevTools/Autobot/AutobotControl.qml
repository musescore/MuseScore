import QtQuick 2.15
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Rectangle {

    color: ui.theme.backgroundPrimaryColor

    FlatButton {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 24
        anchors.leftMargin: 24
        text: "Open Autobot"
        onClicked: api.launcher.open("musescore://autobot/main")
    }

    //! NOTE Will be some settings here
}
