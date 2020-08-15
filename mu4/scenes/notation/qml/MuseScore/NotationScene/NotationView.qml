import QtQuick 2.7
import QtQuick.Layouts 1.3
import MuseScore.NotationScene 1.0

FocusScope {
    ColumnLayout {
        anchors.fill: parent

        NotationSwitchPanel {
            Layout.fillWidth: true
            visible: notationsCount > 1
        }

        NotationPaintView {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
