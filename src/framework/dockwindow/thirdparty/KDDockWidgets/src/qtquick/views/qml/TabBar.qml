/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import QtQuick 2.9
import QtQuick.Controls 2.9


TabBarBase {
    id: root

    // Helper, only applies if you're using the TabBar from QQControls.
    // Returns the internal ListView
    function getInternalListView() {
        for(var i = 0; i < tabBar.children.length; ++i) {
            if (tabBar.children[i].toString().startsWith("QQuickListView"))
                return tabBar.children[i];
        }

        console.warn("Couldn't find the internal ListView");
        return null;
    }

    function getTabAtIndex(index) {
        var listView = getInternalListView();
        var content = listView.children[0];

        var curr = 0;
        for (var i = 0; i < content.children.length; ++i) {
            var candidate = content.children[i];
            if (typeof candidate.tabIndex == "undefined") {
                // All tabs need to have "tabIndex" property.
                continue;
            }

            if (curr == index)
                return candidate;

            curr++;
        }

        if (index < listView.children.length)
            return listView.children[0].children[index];

        return null;
    }

    function getTabIndexAtPosition(globalPoint) {
        var listView = getInternalListView();
        var content = listView.children[0];

        for (var i = 0; i < content.children.length; ++i) {
            var candidate = content.children[i];
            if (typeof candidate.tabIndex == "undefined") {
                // All tabs need to have "tabIndex" property.
                continue;
            }

            var localPt = candidate.mapFromGlobal(globalPoint.x, globalPoint.y);
            if (candidate.contains(localPt)) {
                return i;
            }
        }

        return tabBar.currentIndex;
    }

    implicitHeight: tabBar.implicitHeight

    onCurrentTabIndexChanged: {
        // A change coming from C++
        tabBar.currentIndex = root.currentTabIndex
    }

    TabBar {
        id: tabBar

        width: parent.width

        onCurrentIndexChanged: {
            // Tells the C++ backend that the current dock widget has changed
            root.currentTabIndex = this.currentIndex
        }

        // If the currentIndex changes in the C++ backend then update it here
        Connections {
            target: root.groupCpp
            function onCurrentIndexChanged() {
                root.currentTabIndex = groupCpp.currentIndex;
            }
        }

        Repeater {
            /// The list of tabs is stored in a C++ model. This repeater populates our TabBar.
            model: root.groupCpp ? root.groupCpp.tabBar.dockWidgetModel : 0
            TabButton {
                readonly property int tabIndex: index
                text: title
            }
        }
    }
}
