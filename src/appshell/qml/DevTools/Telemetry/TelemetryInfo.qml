import QtQuick 2.7
import MuseScore.UiComponents 1.0
import MuseScore.Telemetry 1.0

Rectangle {

    color: ui.theme.backgroundPrimaryColor

    TelemetryDevTools {
        id: tmDevTools
    }

    FlatButton {
        text: "Crash"
        onClicked: tmDevTools.doCrash()
    }

}
