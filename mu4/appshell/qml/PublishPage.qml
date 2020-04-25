import QtQuick 2.9
import MuseScore.Ui 1.0
import MuseScore.Dock 1.0

DockPage {
    id: publishPage

    objectName: "publish"

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
