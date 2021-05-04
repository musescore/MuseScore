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
import MuseScore.Extensions 1.0
import MuseScore.Languages 1.0
import MuseScore.Plugins 1.0

FocusScope {
    id: root

    property var color: ui.theme.backgroundSecondaryColor
    property string item: ""

    signal requestActiveFocus()

    NavigationSection {
        id: navSec
        name: "Addons"
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
        anchors.topMargin: 66
        anchors.left: parent.left
        anchors.right: parent.right

        spacing: 12

        NavigationPanel {
            id: navSearchPanel
            name: "AddonsSearch"
            section: navSec
            order: 1
        }

        StyledTextLabel {
            id: addonsLabel

            Layout.leftMargin: 133
            Layout.alignment: Qt.AlignLeft

            font: ui.theme.titleBoldFont

            text: qsTrc("appshell", "Add-ons")
        }

        Row {
            Layout.alignment: Qt.AlignHCenter

            spacing: 12

            SearchField {
                id: searchField

                navigation.name: "AddonsSearch"
                navigation.panel: navSearchPanel
                navigation.order: 1

                onSearchTextChanged: {
                    categoryComboBox.selectedCategory = ""
                }
            }

            StyledComboBox {
                id: categoryComboBox

                width: searchField.width

                navigation.name: "CategoryComboBox"
                navigation.panel: navSearchPanel
                navigation.order: 2

                textRoleName: "text"
                valueRoleName: "value"

                visible: bar.canFilterByCategories

                property string selectedCategory: Boolean(value) ? value : ""

                displayText: qsTrc("appshell", "Category: ") + currentText

                function initModel() {
                    var categories = bar.categories()
                    var result = []

                    result.push({ "text": qsTrc("appshell", "All"), "value": "" })

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

        Item {
            Layout.preferredWidth: addonsLabel.width
            Layout.rightMargin: 133
        }
    }

    TabBar {
        id: bar

        anchors.top: topLayout.bottom
        anchors.topMargin: 54
        anchors.horizontalCenter: parent.horizontalCenter

        contentHeight: 32
        spacing: 0

        property bool canFilterByCategories: bar.currentIndex === 0 || bar.currentIndex === 1

        function categories() {
            var result = []

            if (bar.currentIndex === 0) {
                result = pluginsComp.categories()
            }

            return result
        }

        function pageIndex(pageName) {
            switch (pageName) {
            case "plugins": return 0
            case "extensions": return 1
            case "languages": return 2
            }

            return 0
        }

        function selectPage(pageName) {
            currentIndex = pageIndex(pageName)
        }

        NavigationPanel {
            id: navTabPanel
            name: "AddonsTabs"
            section: navSec
            order: 2

            onNavigationEvent: {
                if (event.type === NavigationEvent.AboutActive) {
                    event.setData("controlName", bar.currentItem.navigation.name)
                }
            }
        }

        StyledTabButton {
            text: qsTrc("appshell", "Plugins")
            sideMargin: 22
            isCurrent: bar.currentIndex === 0
            backgroundColor: root.color

            navigation.name: "Plugins"
            navigation.panel: navTabPanel
            navigation.order: 1
            onNavigationTriggered: bar.currentIndex = 0
        }
        StyledTabButton {
            text: qsTrc("appshell", "Extensions")
            sideMargin: 22
            isCurrent: bar.currentIndex === 1
            backgroundColor: root.color

            navigation.name: "Extensions"
            navigation.panel: navTabPanel
            navigation.order: 2
            onNavigationTriggered: bar.currentIndex = 1
        }
        StyledTabButton {
            text: qsTrc("appshell", "Languages")
            sideMargin: 22
            isCurrent: bar.currentIndex === 2
            backgroundColor: root.color

            navigation.name: "Languages"
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

        PluginsPage {
            id: pluginsComp

            search: searchField.searchText
            selectedCategory: categoryComboBox.selectedCategory
            backgroundColor: root.color
        }

        ExtensionsPage {
            id: extensionsComp

            search: searchField.searchText
            backgroundColor: root.color
        }

        LanguagesPage {
            id: languagesComp
            navigation.section: navSec
            navigation.order: 3
            search: searchField.searchText
            backgroundColor: root.color
        }
    }
}

