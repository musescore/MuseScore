import QtQuick 2.7
import MuseScore.Ui 1.0
import MuseScore.Dock 1.0

DockPage {
    id: publishPage

    objectName: "Publish"

    central: DockCentral {

        id: publishCentral
        objectName: "publishCentral"

        Rectangle {

            Text {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                text: "Publish"
            }
        }
    }
}
