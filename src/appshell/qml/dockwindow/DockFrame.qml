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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Dock 1.0

Rectangle {
    id: root

    //! NOTE: please, don't rename those properties because they are used in c++
    property QtObject frameCpp
    readonly property QtObject titleBarCpp: Boolean(frameCpp) ? frameCpp.actualTitleBar : null
    readonly property int nonContentsHeight: titleBar.visible ? titleBar.heightWhenVisible + tabsPanel.height : 0

    anchors.fill: parent

    color: ui.theme.backgroundPrimaryColor

    onFrameCppChanged: {
        if (Boolean(frameCpp)) {
            frameCpp.setStackLayout(stackLayout)
        }
    }

    onNonContentsHeightChanged: {
        if (Boolean(frameCpp)) {
            frameCpp.geometryUpdated()
        }
    }

    Component.onDestruction: {
        tabs.model = 0
    }

    DockFrameModel {
        id: frameModel

        frame: root.frameCpp
    }

    NavigationPanel {
        id: navPanel
        name: root.objectName+"PanelTabs"
        section: frameModel.navigationSection
        order: 1

        onNavigationEvent: {
            if (event.type === NavigationEvent.AboutActive) {
                event.setData("controlName", tabs.currentItem.navigation.name)
            }
        }
    }

    DockTitleBar {
        id: titleBar

        anchors.top: parent.top

        titleBarCpp: root.titleBarCpp

        visible: frameModel.titleBarVisible
    }

    MouseArea {
        id: dragMouseArea

        anchors.top: tabsPanel.top
        width: tabs.contentWidth
        height: tabsPanel.height - bottomSeparator.height

        z: tabsPanel.z + 1

        hoverEnabled: false
        propagateComposedEvents: true
        enabled: tabsPanel.visible
    }

    Rectangle {
        id: tabsPanel

        anchors.top: titleBar.visible ? titleBar.bottom : parent.top

        height: 36
        width: parent.width

        visible: tabs.count > 1

        color: ui.theme.backgroundSecondaryColor

        readonly property QtObject tabBarCpp: Boolean(root.frameCpp) ? root.frameCpp.tabWidget.tabBar : null
        property int currentIndex: Boolean(root.frameCpp) && root.frameCpp.currentIndex >= 0 ? root.frameCpp.currentIndex : 0

        onTabBarCppChanged: {
            if (Boolean(tabBarCpp)) {
                tabBarCpp.redirectMouseEvents(dragMouseArea)
                tabBarCpp.tabBarQmlItem = this
            }
        }

        onCurrentIndexChanged: {
            if (Boolean(root) && Boolean(root.frameCpp)) {
                root.frameCpp.tabWidget.setCurrentDockWidget(currentIndex)
            }
        }

        ListView {
            id: tabs

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left

            width: contentWidth

            orientation: Qt.Horizontal
            interactive: false
            clip: true
            spacing: 0

            currentIndex: tabsPanel.currentIndex
            model: Boolean(root.frameCpp) ? root.frameCpp.tabWidget.dockWidgetModel : 0

            delegate: DockPanelTab {
                navigation.name: title
                navigation.panel: navPanel
                navigation.order: model.index * 2 // NOTE '...' button will have +1 order
                onNavigationTriggered: tabsPanel.currentIndex = model.index

                text: title
                isCurrent: tabsPanel && (tabsPanel.currentIndex === model.index)
            }
        }

        Item {
            anchors.left: tabs.right
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            height: bottomSeparator.height

            SeparatorLine { id: bottomSeparator }
        }
    }

    StackLayout {
        id: stackLayout

        anchors.top: tabsPanel.visible ? tabsPanel.bottom : (titleBar.visible ? titleBar.bottom : parent.top)
        anchors.topMargin: tabsPanel.visible ? 12 : 0
        anchors.bottom: parent.bottom

        width: parent.width

        currentIndex: {
            var currentDockUniqueName = frameModel.currentDockUniqueName

            for (var i in children) {
                if (children[i].uniqueName === currentDockUniqueName) {
                    return i
                }
            }

            return 0
        }
    }
}
