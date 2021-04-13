import QtQuick 2.15
import MuseScore.Ui 1.0

QmlDialog {
    id: root

    title: "KeyNavDev"
    height: 900
    width: 600

    KeyNavDevPanel {
        anchors.fill: parent
        anchors.margins: 8
    }
}
