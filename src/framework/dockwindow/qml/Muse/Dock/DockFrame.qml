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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Dock 1.0

Rectangle {
    id: root

    //! NOTE: please, don't rename those properties because they are used in c++
    property QtObject frameCpp
    readonly property QtObject titleBarCpp: Boolean(frameCpp) ? frameCpp.actualTitleBar : null
    readonly property int nonContentsHeight: titleBar.height + tabBar.height + stackLayout.anchors.topMargin
    property int titleBarNavigationPanelOrder: 1
    //! ---

    readonly property bool hasTitleBar: frameModel.titleBarAllowed && !(frameModel.tabs.length > 1 || frameModel.isHorizontalPanel)
    readonly property bool hasSingleTab: frameModel.titleBarAllowed && frameModel.tabs.length === 1 && frameModel.isHorizontalPanel
    readonly property bool hasTabBar: frameModel.titleBarAllowed && (frameModel.tabs.length > 1 || frameModel.isHorizontalPanel)

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

    DockFrameModel {
        id: frameModel

        frame: root.frameCpp
    }

    NavigationPanel {
        id: navPanel
        name: root.objectName+"PanelTabs"
        enabled: root.enabled && root.visible
        section: frameModel.navigationSection
        order: root.titleBarNavigationPanelOrder

        onNavigationEvent: function(event) {
            if (event.type === NavigationEvent.AboutActive && tabBar.visible) {
                event.setData("controlName", tabBar.currentItemNavigationName)
            }
        }
    }

    DockTitleBar {
        id: titleBar

        anchors.top: parent.top
        width: parent.width
        height: visible ? implicitHeight : 0

        visible: root.hasTitleBar

        property alias titleBar: frameModel.titleBar

        titleBarCpp: root.titleBarCpp

        contextMenuModel: frameModel.currentDockContextMenuModel

        navigationPanel: navPanel
        navigationOrder: 1

        onHandleContextMenuItemRequested: function(itemId) {
            frameModel.handleMenuItem(itemId)
        }

        titleBarItem: frameModel.titleBar
    }

    DockTabBar {
        id: tabBar

        anchors.top: parent.top
        width: parent.width
        height: visible ? 35 : 0

        visible: root.hasTabBar
        draggingTabsAllowed: root.hasTabBar && !root.hasSingleTab

        tabBarCpp: Boolean(root.frameCpp) ? root.frameCpp.tabWidget.tabBar : null
        tabsModel: frameModel.tabs
        currentIndex: Boolean(root.frameCpp) && root.frameCpp.currentIndex >= 0 ? root.frameCpp.currentIndex : 0

        currentToolbarComponent: frameModel.currentDockToolbarComponent

        navigationPanel: navPanel

        function setCurrentDockWidget(index: int) {
            if (root.frameCpp) {
                root.frameCpp.tabWidget.setCurrentDockWidget(index)
            }
        }

        onCurrentIndexChanged: {
            setCurrentDockWidget(currentIndex)
        }

        onTabClicked: function(index) {
            setCurrentDockWidget(index)
        }

        onHandleContextMenuItemRequested: function(itemId) {
            frameModel.handleMenuItem(itemId)
        }
    }

    DockTitleBarMouseArea {
        id: titleBarMouseArea

        readonly property var titleBarComp: tabBar.visible ? tabBar
                                                           : titleBar.visible ? titleBar : null
        anchors.fill: titleBarComp

        enabled: root.hasTitleBar || root.hasSingleTab
        propagateComposedEvents: true

        titleBarCpp: root.titleBarCpp

        onDoubleClicked: function(mouse) {
            if (titleBarComp) {
                let pos = titleBarMouseArea.mapToItem(titleBarComp, mouse.x, mouse.y)
                titleBarComp.doubleClicked(pos)
            }
        }
    }

    StackLayout {
        id: stackLayout

        anchors.top: tabBar.visible ? tabBar.bottom
                                    : titleBar.visible ? titleBar.bottom : parent.top
        anchors.topMargin: tabBar.visible ? 12 : 0
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

    Rectangle {
        visible: frameModel.highlightingVisible

        x: frameModel.highlightingRect.x
        y: frameModel.highlightingRect.y
        width: frameModel.highlightingRect.width
        height: frameModel.highlightingRect.height

        color: "transparent"

        border.width: 1
        border.color: ui.theme.accentColor

        Rectangle {
            anchors.fill: parent

            color: ui.theme.accentColor
            opacity: 0.3
        }
    }
}
