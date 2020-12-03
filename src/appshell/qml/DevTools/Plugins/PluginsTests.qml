import QtQuick 2.7
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Plugins 1.0

Rectangle {
    color: "#fea534"

    PluginsTestModel {
        id: pluginsModel
    }

    Component.onCompleted: {
        pluginsModel.load()
    }

    StyledTextLabel {
        id: pageTitle

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 16

        text: "Installed plugins"

        font.bold: true
        font.pixelSize: 18
    }

    Grid {
        anchors.top: pageTitle.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 16

        spacing: 16
        columns: 3

        Repeater {
            anchors.fill: parent

            model: pluginsModel.installedPluginsNames

            delegate: FlatButton {
                width: 200

                text: modelData

                onClicked: {
                    pluginsModel.run(index)
                }
            }
        }
    }
}
