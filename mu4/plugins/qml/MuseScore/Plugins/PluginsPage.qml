import QtQuick 2.9

import MuseScore.UiComponents 1.0
import MuseScore.Plugins 1.0

Item {
    id: root

    property string search: ""

    PluginsModel {
        id: pluginsModel

        onFinished: {
            panel.hide()
        }
    }

    onSearchChanged: {
        panel.hide()
    }

    QtObject {
        id: privateProperties

        property var selectedPlugin: undefined
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

        anchors.top: parent.top
        anchors.topMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 120
        anchors.right: parent.right
        anchors.rightMargin: 120
        anchors.bottom: panel.visible ? panel.top : parent.bottom
        anchors.bottomMargin: panel.visible ? 0 : 21

        clip: true

        contentWidth: width
        contentHeight: notInstalledPluginsView.height + installedPluginsView.height
        interactive: height < contentHeight

        Column {
            id: column
            anchors.fill: parent

            spacing: 42

            PluginsListView {
                id: notInstalledPluginsView

                width: parent.width
                title: qsTrc("plugins", "Not installed")
                visible: count > 0

                search: root.search

                model: pluginsModel

                onPluginClicked: {
                    privateProperties.selectedPlugin = plugin
                    panel.show()
                }
            }

            PluginsListView {
                id: installedPluginsView

                width: parent.width
                title: qsTrc("plugins", "Installed")
                visible: count > 0

                search: root.search
                installed: true

                model: pluginsModel

                onPluginClicked: {
                    privateProperties.selectedPlugin = plugin
                    panel.show()
                }
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: flickable.bottom

        visible: !panel.visible

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

    InstallationPanel {
        id: panel

        anchors.bottom: parent.bottom

        property alias selectedPlugin: privateProperties.selectedPlugin

        title: Boolean(selectedPlugin) ? selectedPlugin.name : ""
        description: Boolean(selectedPlugin) ? selectedPlugin.description : ""
        installed: Boolean(selectedPlugin) ? selectedPlugin.installed : false
        hasUpdate: Boolean(selectedPlugin) ? selectedPlugin.hasUpdate : false
        background: flickable

        onInstallRequested: {
            pluginsModel.install(selectedPlugin.codeKey)
        }

        onUninstallRequested: {
            pluginsModel.uninstall(selectedPlugin.codeKey)
        }

        onUpdateRequested: {
            pluginsModel.update(selectedPlugin.codeKey)
        }

        onRestartRequested: {
            pluginsModel.restart(selectedPlugin.codeKey)
        }

        onOpenFullDescriptionRequested: {
            pluginsModel.openFullDescription(selectedPlugin.codeKey)
        }
    }
}
