/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import QtQuick 2.9
import QtQuick.Controls 2.9

Item {
    id: root

    readonly property bool hasCustomMouseEventRedirector: parent.hasCustomMouseEventRedirector

    /// This is our C++ Group.cpp view
    readonly property QtObject groupCpp: parent.groupCpp

    /// This is our C++ TabBar.cpp view
    readonly property QtObject tabBarCpp: groupCpp ? groupCpp.tabBar : null

    /// The number of tabs
    readonly property int count: tabBarCpp ? tabBarCpp.dockWidgetModel.count : 0

    property int currentTabIndex: -1

    /// Users adding their own mouse areas should use a Z bigger than this.
    readonly property alias mouseAreaZ: tabBarDragMouseArea.z

    /// Don't override in custom TabBar's.
    visible: tabBarCpp ? (tabBarCpp.tabBarAutoHide ? count > 1 : true) : count > 1

    /// Don't override in custom TabBar's. Change implicitHeight instead.
    height: visible ? implicitHeight : 0

    /// Feel free to customize
    width: parent ? parent.width : 0

    onCurrentTabIndexChanged: {
        // Tells the C++ backend that the current dock widget has changed
        if (groupCpp)
            groupCpp.tabBar.setCurrentIndex(currentTabIndex);
    }

    // If the currentIndex changes in the C++ backend then update it here
    Connections {
        target: root.groupCpp
        function onCurrentIndexChanged() {
            root.currentTabIndex =  groupCpp.currentIndex;
        }
    }

    MouseArea {
        id: tabBarDragMouseArea
        hoverEnabled: true
        anchors.fill: root
        enabled: root.visible
        z: 10
    }

    onTabBarCppChanged: {
        if (tabBarCpp) {
            if (!root.hasCustomMouseEventRedirector)
                tabBarCpp.redirectMouseEvents(tabBarDragMouseArea)

            // Setting just so the unit-tests can access the buttons
            tabBarCpp.tabBarQmlItem = this;
        }
    }

    /// Returns the QQuickItem* that implements the Tab
    /// This is called by C++ and needs to be implemented in the derived class.
    /// See TabBar.qml for an example.
    function getTabAtIndex(index) {
        console.warn("Override this function in the actual derived tab bar!");
    }

    /// Returns the index of the tab that's at localPoint
    /// Returns -1 if no tab there.
    /// This is called by C++ and needs to be implemented in the derived class.
    /// See TabBar.qml for an example.
    function getTabIndexAtPosition(localPoint) {
        console.warn("Override this function in the actual derived tab bar!");
    }
}
