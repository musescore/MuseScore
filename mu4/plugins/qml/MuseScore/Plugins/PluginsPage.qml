import QtQuick 2.9

import MuseScore.UiComponents 1.0
import MuseScore.Plugins 1.0

Item {
    id: root

    property string search: ""

    PluginsModel {
        id: pluginsModel
    }

    Component.onCompleted: {
        pluginsModel.load()
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: flickable.top

        height: 8
        z: 1

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: ui.theme.backgroundPrimaryColor
            }
            GradientStop {
                position: 1.0
                color: "transparent"
            }
        }
    }

    Flickable {
        id: flickable

        anchors.fill: parent
        anchors.topMargin: 5
        anchors.leftMargin: 120
        anchors.rightMargin: 120
        anchors.bottomMargin: 21

        clip: true

        contentWidth: width
        contentHeight: notInstalledPluginsView.height + installedPluginsView.height
        interactive: height < contentHeight

        Column {
            anchors.fill: parent

            spacing: 42

            PluginsListView {
                id: notInstalledPluginsView

                width: parent.width
                title: qsTrc("plugins", "Not installed")

                search: root.search

                model: pluginsModel

                onPluginClicked: {

                }
            }

            PluginsListView {
                id: installedPluginsView

                width: parent.width
                title: qsTrc("plugins", "Installed")

                search: root.search
                installed: true

                model: pluginsModel

                onPluginClicked: {

                }
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: flickable.bottom

        height: 8
        z: 1

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: "transparent"
            }
            GradientStop {
                position: 1.0
                color: ui.theme.backgroundPrimaryColor
            }
        }
    }
}
