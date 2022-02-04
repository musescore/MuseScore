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
import MuseScore.Plugins 1.0

FocusScope {
    id: root

    property alias color: background.color
    property string section: ""

    QtObject {
        id: prv

        readonly property int sideMargin: 46
    }

    NavigationSection {
        id: navSec
        name: "Add-ons"
        enabled: root.enabled && root.visible
        order: 3
        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }
    }

    onSectionChanged: {
        if (!Boolean(root.section)) {
            return
        }

        tabBar.selectPage(root.section)
    }

    Rectangle {
        id: background
        anchors.fill: parent
        color: ui.theme.backgroundSecondaryColor
    }

    RowLayout {
        id: topLayout

        anchors.top: parent.top
        anchors.topMargin: prv.sideMargin
        anchors.left: parent.left
        anchors.leftMargin: prv.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: prv.sideMargin

        spacing: 12

        NavigationPanel {
            id: navSearchPanel
            name: "AddonsSearch"
            enabled: topLayout.enabled && topLayout.visible
            section: navSec
            order: 1
            accessible.name: qsTrc("appshell", "Add-ons")
        }

        StyledTextLabel {
            id: addonsLabel
            Layout.fillWidth: true

            text: qsTrc("appshell", "Add-ons")
            font: ui.theme.titleBoldFont
            horizontalAlignment: Text.AlignLeft
        }

        SearchField {
            id: searchField

            Layout.preferredWidth: 220

            navigation.name: "AddonsSearch"
            navigation.panel: navSearchPanel
            navigation.order: 1
            accessible.name: qsTrc("appshell", "Add-ons search")

            onSearchTextChanged: {
                categoryComboBox.selectedCategory = ""
            }
        }

        Dropdown {
            id: categoryComboBox

            width: searchField.width

            navigation.name: "CategoryComboBox"
            navigation.panel: navSearchPanel
            navigation.order: 2

            readonly property string allCategoryValue: "ALL_CATEGORY"
            property string selectedCategory: (currentValue !== allCategoryValue) ? currentValue : ""

            displayText: qsTrc("appshell", "Category: ") + categoryComboBox.currentText
            currentIndex: indexOfValue(allCategoryValue)

            function initModel() {
                var categories = tabBar.categories()
                var result = []

                result.push({ "text": qsTrc("appshell", "All"), "value": allCategoryValue })

                for (var i = 0; i < categories.length; ++i) {
                    var category = categories[i]
                    result.push({ "text": category, "value": category })
                }

                model = result
            }

            Component.onCompleted: {
                initModel()
            }
        }
    }

    StyledTabBar {
        id: tabBar

        anchors.top: topLayout.bottom
        anchors.topMargin: prv.sideMargin
        anchors.left: parent.left
        anchors.leftMargin: prv.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: prv.sideMargin

        function categories() {
            var result = []

            if (tabBar.currentIndex === 0) {
                result = pluginsComp.categories()
            }

            return result
        }

        function pageIndex(pageName) {
            switch (pageName) {
            case "plugins": return 0
            }

            return 0
        }

        function selectPage(pageName) {
            currentIndex = pageIndex(pageName)
        }

        NavigationPanel {
            id: navTabPanel
            name: "AddonsTabs"
            enabled: tabBar.enabled && tabBar.visible
            section: navSec
            order: 2
            accessible.name: qsTrc("appshell", "Add-ons tabs")

            onNavigationEvent: function(event) {
                if (event.type === NavigationEvent.AboutActive) {
                    event.setData("controlName", tabBar.currentItem.navigation.name)
                }
            }
        }

        StyledTabButton {
            text: qsTrc("appshell", "Plugins")

            navigation.name: "Plugins"
            navigation.panel: navTabPanel
            navigation.order: 1
        }
    }

    StackLayout {
        anchors.top: tabBar.bottom
        anchors.topMargin: 36
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        currentIndex: tabBar.currentIndex

        PluginsPage {
            id: pluginsComp

            search: searchField.searchText
            selectedCategory: categoryComboBox.selectedCategory
            backgroundColor: root.color

            sideMargin: prv.sideMargin

            navigationSection: navSec
        }
    }
}
