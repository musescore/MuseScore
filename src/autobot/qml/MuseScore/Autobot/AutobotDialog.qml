import QtQuick 2.15
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

QmlDialog {
    id: root

    title: "Autobot"

    width: 360
    height: 600

    AutobotPanel {
        anchors.fill: parent
    }
}
