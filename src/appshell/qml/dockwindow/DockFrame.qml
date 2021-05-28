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
    readonly property int nonContentsHeight: titleBar.visible ? titleBar.heightWhenVisible + tabs.height : 0

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

    DockTitleBar {
        id: titleBar

        anchors.top: parent.top

        titleBarCpp: root.titleBarCpp

        visible: frameModel.titleBarVisible
    }

    MouseArea {
        id: dragMouseArea

        anchors.fill: tabs

        z: tabs.z + 1

        hoverEnabled: false
        propagateComposedEvents: true
        enabled: tabs.visible
    }

    Rectangle {
        id: tabs

        anchors.top: titleBar.visible ? titleBar.bottom : parent.top

        height: 36
        width: parent.width

        visible: tabsView.count > 1

        color: ui.theme.backgroundSecondaryColor

        readonly property QtObject tabBarCpp: Boolean(root.frameCpp) ? root.frameCpp.tabWidget.tabBar : null
        property int currentIndex: Boolean(root.frameCpp) ? root.frameCpp.currentIndex : 0

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
            id: tabsView

            anchors.fill: parent

            orientation: Qt.Horizontal
            interactive: false
            spacing: 0

            currentIndex: tabs.currentIndex
            model: Boolean(root.frameCpp) ? root.frameCpp.tabWidget.dockWidgetModel : 0

            delegate: DockPanelTab {
                text: title

                isCurrent: tabs && (tabs.currentIndex === model.index)
            }
        }
    }

    StackLayout {
        id: stackLayout

        anchors.top: tabs.visible ? tabs.bottom : (titleBar.visible ? titleBar.bottom : parent.top)
        anchors.topMargin: tabs.visible ? 12 : 0
        anchors.bottom: parent.bottom

        width: parent.width

        currentIndex: tabs.currentIndex
    }
}
