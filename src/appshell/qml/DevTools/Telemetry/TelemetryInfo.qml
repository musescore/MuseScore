import QtQuick 2.7

import MuseScore.UiComponents 1.0
import MuseScore.Telemetry 1.0

Rectangle {
    color: ui.theme.backgroundPrimaryColor

    TelemetryDevTools {
        id: tmDevTools
    }

    Column {
        anchors.fill: parent
        anchors.topMargin: 20

        spacing: 20

        readonly property int buttonWidth: 200

        FlatButton {
            text: "Crash"

            width: parent.buttonWidth

            onClicked: tmDevTools.doCrash()
        }

        FlatButton {
            text: "Open permission dialog"

            width: parent.buttonWidth

            onClicked: api.launcher.open("musescore://telemetry/permission")
        }
    }
}
