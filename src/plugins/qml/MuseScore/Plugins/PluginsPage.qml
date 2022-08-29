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
import QtQuick 2.15
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Plugins 1.0

import "internal"

Item {
    id: root

    property string search: ""
    property string selectedCategory: ""
    property color backgroundColor: ui.theme.backgroundPrimaryColor

    property int sideMargin: 46

    property NavigationSection navigationSection: null

    function categories() {
        return pluginsModel.categories()
    }

    function reloadPlugins() {
        pluginsModel.reloadPlugins()
    }

    PluginsModel {
        id: pluginsModel

        onFinished: {
            prv.lastNavigatedExtension = null
            panel.close()
        }
    }

    onSearchChanged: {
        panel.close()
    }

    QtObject {
        id: prv

        property var selectedPlugin: undefined
        property var lastNavigatedExtension: undefined

        function resetSelectedPlugin() {
            selectedPlugin = undefined
        }
    }

    Component.onCompleted: {
        pluginsModel.load()
    }

    Rectangle {
        id: topGradient

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

    StyledFlickable {
        id: flickable

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: panel.visible ? panel.top : parent.bottom

        contentWidth: width
        contentHeight: column.implicitHeight

        topMargin: topGradient.height
        bottomMargin: 24

        Column {
            id: column

            anchors.fill: parent
            anchors.leftMargin: root.sideMargin
            anchors.rightMargin: root.sideMargin

            spacing: 24

            PluginsListView {
                id: enabledPluginsView

                width: parent.width
                title: qsTrc("plugins", "Enabled")
                visible: count > 0

                search: root.search
                pluginIsEnabled: true
                selectedCategory: root.selectedCategory
                selectedPluginCodeKey: prv.selectedPlugin ? prv.selectedPlugin.codeKey : ""

                model: pluginsModel

                flickableItem: column

                navigationPanel.section: root.navigationSection
                navigationPanel.name: "EnabledPlugins"
                navigationPanel.order: 4

                onPluginClicked: function(plugin, navigationControl) {
                    prv.selectedPlugin = plugin
                    panel.open()
                    prv.lastNavigatedExtension = navigationControl
                }

                onNavigationActivated: function(itemRect) {
                    Utils.ensureContentVisible(flickable, itemRect, enabledPluginsView.headerHeight + 16)
                }
            }

            PluginsListView {
                id: disabledPluginsView

                width: parent.width
                title: qsTrc("plugins", "Disabled")
                visible: count > 0

                search: root.search
                pluginIsEnabled: false
                selectedCategory: root.selectedCategory
                selectedPluginCodeKey: prv.selectedPlugin ? prv.selectedPlugin.codeKey : ""

                model: pluginsModel

                flickableItem: column

                navigationPanel.section: root.navigationSection
                navigationPanel.name: "DisabledPlugins"
                navigationPanel.order: 5

                onPluginClicked: function(plugin, navigationControl) {
                    prv.selectedPlugin = Object.assign({}, plugin)
                    panel.open()
                    prv.lastNavigatedExtension = navigationControl
                }

                onNavigationActivated: function(itemRect) {
                    Utils.ensureContentVisible(flickable, itemRect, disabledPluginsView.headerHeight + 16)
                }
            }
        }
    }

    EnablePanel {
        id: panel

        property alias selectedPlugin: prv.selectedPlugin

        title: Boolean(selectedPlugin) ? selectedPlugin.name : ""
        description: Boolean(selectedPlugin) ? selectedPlugin.description : ""
        background: flickable

        isEnabled: Boolean(selectedPlugin) ? selectedPlugin.enabled : false

        additionalInfoModel: [
            {"title": qsTrc("plugins", "Version:"), "value": Boolean(selectedPlugin) ? selectedPlugin.version : "" },
            //: Keyboard shortcut
            {"title": qsTrc("plugins", "Shortcut:"), "value": Boolean(selectedPlugin) ? selectedPlugin.shortcuts : ""}
        ]

        onEnabledChanged: {
            pluginsModel.setEnable(selectedPlugin.codeKey, enabled)
        }

        onEditShortcutRequested: {
            Qt.callLater(pluginsModel.editShortcut, selectedPlugin.codeKey)
            panel.close()
        }

        onClosed: {
            prv.resetSelectedPlugin()
            resetNavigationFocus()
        }

        function resetNavigationFocus() {
            if (prv.lastNavigatedExtension) {
                prv.lastNavigatedExtension.requestActive()
                return
            }

            if (enabledPluginsView.count > 0) {
                enabledPluginsView.focusOnFirst()
            } else {
                disabledPluginsView.focusOnFirst()
            }
        }
    }
}
