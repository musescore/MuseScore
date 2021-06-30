/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.9
import QtQuick.Controls 2.2

import MuseScore.UiComponents 1.0
import MuseScore.Plugins 1.0

import "internal"

Item {
    id: root

    property string search: ""
    property string selectedCategory: ""
    property string backgroundColor: ui.theme.backgroundPrimaryColor

    property int sideMargin: 46

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
        id: prv

        property var selectedPlugin: undefined

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
        anchors.leftMargin: root.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: root.sideMargin
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
            anchors.bottom: panel.visible ? panel.top : parent.bottom
            anchors.right: parent.right
            anchors.rightMargin: 16

            visible: flickable.contentHeight > flickable.height
            z: 1
        }

        Column {
            id: column
            anchors.fill: parent

            spacing: 42

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
                    prv.selectedPlugin = plugin
                    panel.open()
                }
            }

            PluginsListView {
                id: notInstalledPluginsView

                width: parent.width
                title: qsTrc("plugins", "Not Installed")
                visible: count > 0

                search: root.search
                selectedCategory: root.selectedCategory

                model: pluginsModel

                onPluginClicked: {
                    prv.selectedPlugin = Object.assign({}, plugin)
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

        property alias selectedPlugin: prv.selectedPlugin

        title: Boolean(selectedPlugin) ? selectedPlugin.name : ""
        description: Boolean(selectedPlugin) ? selectedPlugin.description : ""
        installed: Boolean(selectedPlugin) ? selectedPlugin.installed : false
        hasUpdate: Boolean(selectedPlugin) ? selectedPlugin.hasUpdate : false
        neutralButtonTitle: qsTrc("plugins", "View full description")
        background: flickable

        additionalInfoModel: [
            {"title": qsTrc("plugins", "Author:"), "value": qsTrc("plugins", "MuseScore")},
            {"title": qsTrc("plugins", "Maintained by:"), "value": qsTrc("plugins", "MuseScore")}
        ]

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

        onNeutralButtonClicked: {
            pluginsModel.openFullDescription(selectedPlugin.codeKey)
        }

        onClosed: {
            prv.resetSelectedPlugin()
        }
    }
}
