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
        name: "Plugins"
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
        anchors.right: parent.right
        anchors.rightMargin: prv.sideMargin

        spacing: 12

        NavigationPanel {
            id: navSearchPanel
            name: "PluginsSearch"
            enabled: topLayout.enabled && topLayout.visible
            section: navSec
            order: 1
            accessible.name: qsTrc("appshell", "Plugins")
        }

        SearchField {
            id: searchField

            Layout.preferredWidth: 220

            navigation.name: "PluginsSearch"
            navigation.panel: navSearchPanel
            navigation.order: 1
            accessible.name: qsTrc("appshell", "Plugins search")

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
                var categories = pluginsComp.categories()
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

    PluginsPage {
        id: pluginsComp

        anchors.top: topLayout.bottom
        anchors.topMargin: 36
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        search: searchField.searchText
        selectedCategory: categoryComboBox.selectedCategory
        backgroundColor: root.color

        sideMargin: prv.sideMargin

        navigationSection: navSec
    }
}
