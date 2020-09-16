import QtQuick 2.9
import QtQuick.Controls 2.2

import MuseScore.UiComponents 1.0
import MuseScore.Plugins 1.0

Item {
    id: root

    property string search: ""
    property string selectedCategory: ""
    property string backgroundColor: ui.theme.backgroundPrimaryColor

    function categories() {
        return pluginsModel.categories()
    }

    PluginsModel {
        id: pluginsModel

        onFinished: {
            panel.close()
        }
    }

    onSearchChanged: {
        panel.close()
    }

    QtObject {
        id: privateProperties

        property var selectedPlugin: undefined
        property int sideMargin: 133

        function resetSelectedPlugin() {
            selectedPlugin = undefined

            notInstalledPluginsView.resetSelectedPlugin()
            installedPluginsView.resetSelectedPlugin()
        }
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
                color: root.backgroundColor
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
        anchors.leftMargin: privateProperties.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: privateProperties.sideMargin
        anchors.bottom: panel.visible ? panel.top : parent.bottom
        anchors.bottomMargin: panel.visible ? 0 : 21

        clip: true

        contentWidth: width
        contentHeight: notInstalledPluginsView.height + installedPluginsView.height
        interactive: height < contentHeight

        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: StyledScrollBar {
            parent: flickable.parent

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.rightMargin: 13

            z: 1
        }

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
                selectedCategory: root.selectedCategory

                model: pluginsModel

                onPluginClicked: {
                    privateProperties.selectedPlugin = plugin
                    panel.open()
                }
            }

            PluginsListView {
                id: installedPluginsView

                width: parent.width
                title: qsTrc("plugins", "Installed")
                visible: count > 0

                search: root.search
                installed: true
                selectedCategory: root.selectedCategory

                model: pluginsModel

                onPluginClicked: {
                    privateProperties.selectedPlugin = plugin
                    panel.open()
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
                color: root.backgroundColor
            }
        }
    }

    InstallationPanel {
        id: panel

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

        onClosed: {
            privateProperties.resetSelectedPlugin()
        }
    }
}
