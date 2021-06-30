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
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

FocusScope {
    id: root

    property var color: ui.theme.backgroundSecondaryColor
    property string item: ""

    signal requestActiveFocus()

    NavigationSection {
        id: navSec
        name: "Learn"
        enabled: root.visible
        order: 3
        onActiveChanged: {
            if (active) {
                root.requestActiveFocus()
            }
        }
    }

    onItemChanged: {
        if (!Boolean(root.item)) {
            return
        }

        bar.selectPage(root.item)
    }

    Rectangle {
        anchors.fill: parent
        color: root.color
    }

    RowLayout {
        id: topLayout
        anchors.top: parent.top
        anchors.topMargin: 46
        anchors.left: parent.left
        anchors.leftMargin: 46
        anchors.right: parent.right
        anchors.rightMargin: 46

        spacing: 12

        NavigationPanel {
            id: navSearchPanel
            name: "LearnSearch"
            section: navSec
            order: 1
            accessible.name: qsTrc("learn", "Learn")
        }

        StyledTextLabel {
            id: learnLabel

            font: ui.theme.titleBoldFont
            text: qsTrc("learn", "Learn")
        }

        Item {
            Layout.preferredWidth: topLayout.width - learnLabel.width - 24 - searchField.width
            Layout.fillHeight: true
        }

        SearchField {
            id: searchField

            Layout.preferredWidth: 220

            navigation.name: "LearnSearch"
            navigation.panel: navSearchPanel
            navigation.order: 1
            accessible.name: qsTrc("learn", "Learn search")
        }
    }

    TabBar {
        id: bar

        anchors.top: topLayout.bottom
        anchors.topMargin: 36
        anchors.left: parent.left
        anchors.leftMargin: 24

        contentHeight: 32
        spacing: 0

        function pageIndex(pageName) {
            switch (pageName) {
            case "get-started": return 0
            case "advanced": return 1
            case "classes": return 2
            }

            return 0
        }

        function selectPage(pageName) {
            currentIndex = pageIndex(pageName)
        }

        NavigationPanel {
            id: navTabPanel
            name: "LearnTabs"
            section: navSec
            order: 2
            accessible.name: qsTrc("learn", "Learn tabs")

            onNavigationEvent: {
                if (event.type === NavigationEvent.AboutActive) {
                    event.setData("controlName", bar.currentItem.navigation.name)
                }
            }
        }

        StyledTabButton {
            text: qsTrc("learn", "Get Started")
            sideMargin: 22
            isCurrent: bar.currentIndex === 0
            backgroundColor: root.color

            navigation.name: "Get Started"
            navigation.panel: navTabPanel
            navigation.order: 1
            onNavigationTriggered: bar.currentIndex = 0
        }

        StyledTabButton {
            text: qsTrc("learn", "Advanced")
            sideMargin: 22
            isCurrent: bar.currentIndex === 1
            backgroundColor: root.color

            navigation.name: "Advanced"
            navigation.panel: navTabPanel
            navigation.order: 2
            onNavigationTriggered: bar.currentIndex = 1
        }

        StyledTabButton {
            text: qsTrc("learn", "Classes")
            sideMargin: 22
            isCurrent: bar.currentIndex === 2
            backgroundColor: root.color

            navigation.name: "Classes"
            navigation.panel: navTabPanel
            navigation.order: 3
            onNavigationTriggered: bar.currentIndex = 2
        }
    }

    StackLayout {
        anchors.top: bar.bottom
        anchors.topMargin: 24
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        currentIndex: bar.currentIndex

        Rectangle {
            id: getStartedComp

            color: root.color

            //            search: searchField.searchText
            //            backgroundColor: root.color
        }

        Rectangle {
            id: advancedComp

            color: root.color

            //            search: searchField.searchText
            //            backgroundColor: root.color
        }

        Rectangle {
            id: classesComp

            color: root.color

            //            navigation.section: navSec
            //            navigation.order: 3
            //            search: searchField.searchText
            //            backgroundColor: root.color
        }
    }
}

