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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Extensions 1.0

import "internal"

Item {
    id: root

    property string search: ""
    property string selectedCategory: ""
    property color backgroundColor: ui.theme.backgroundPrimaryColor

    property int sideMargin: 46

    property NavigationSection navigationSection: null

    function categories() {
        return extensionsModel.categories()
    }

    function reloadPlugins() {
        extensionsModel.reloadPlugins()
    }

    ExtensionsListModel {
        id: extensionsModel

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
        extensionsModel.load()
    }

    GradientRectangle {
        id: topGradient

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: flickable.top

        startColor: root.backgroundColor
        endColor: "transparent"
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

            ExtensionsListView {
                id: enabledPluginsView

                width: parent.width
                title: qsTrc("extensions", "Enabled")
                visible: count > 0

                search: root.search
                pluginIsEnabled: true
                selectedCategory: root.selectedCategory
                selectedPluginUri: prv.selectedPlugin?.uri ?? ""

                model: extensionsModel

                flickableItem: column

                navigationPanel.section: root.navigationSection
                navigationPanel.name: "EnabledPlugins"
                navigationPanel.order: 4

                onPluginClicked: function(plugin, navigationControl) {
                    root.openInfoPanel(plugin, navigationControl)
                }

                onNavigationActivated: function(itemRect) {
                    Utils.ensureContentVisible(flickable, itemRect, enabledPluginsView.headerHeight + 16)
                }
            }

            ExtensionsListView {
                id: disabledPluginsView

                width: parent.width
                title: qsTrc("extensions", "Disabled")
                visible: count > 0

                search: root.search
                pluginIsEnabled: false
                selectedCategory: root.selectedCategory
                selectedPluginUri: prv.selectedPlugin?.uri ?? ""

                model: extensionsModel

                flickableItem: column

                navigationPanel.section: root.navigationSection
                navigationPanel.name: "DisabledPlugins"
                navigationPanel.order: 5

                onPluginClicked: function(plugin, navigationControl) {
                    root.openInfoPanel(plugin, navigationControl)
                }

                onNavigationActivated: function(itemRect) {
                    Utils.ensureContentVisible(flickable, itemRect, disabledPluginsView.headerHeight + 16)
                }
            }
        }
    }

    function openInfoPanel(plugin, navigationControl) {
        prv.selectedPlugin = Object.assign({}, plugin)
        panel.currentExecPointIndex = extensionsModel.currentExecPointIndex(prv.selectedPlugin.uri)
        panel.execPointsModel = extensionsModel.execPointsModel(prv.selectedPlugin.uri)
        panel.open()
        prv.lastNavigatedExtension = navigationControl
    }

    EnablePanel {
        id: panel

        property alias selectedPlugin: prv.selectedPlugin

        title: Boolean(selectedPlugin) ? selectedPlugin.name : ""
        description: Boolean(selectedPlugin) ? selectedPlugin.description : ""
        background: flickable

        isEnabled: Boolean(selectedPlugin) ? selectedPlugin.enabled : false

        additionalInfoModel: [
            {"title": qsTrc("extensions", "Version:"), "value": Boolean(selectedPlugin) ? selectedPlugin.version : "" },
            //: Keyboard shortcut
            {"title": qsTrc("extensions", "Shortcut:"), "value": Boolean(selectedPlugin) ? selectedPlugin.shortcuts : ""}
        ]

        onExecPointSelected: function(index) {
            extensionsModel.selectExecPoint(selectedPlugin.uri, index)
        }

        onEditShortcutRequested: {
            Qt.callLater(extensionsModel.editShortcut, selectedPlugin.uri)
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
