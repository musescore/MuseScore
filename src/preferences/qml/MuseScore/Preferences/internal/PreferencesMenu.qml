/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

import QtQuick

import Muse.Ui
import Muse.UiComponents

import MuseScore.Preferences

Item {
    id: root

    property PreferencesModel model: null

    property NavigationPanel navigation: NavigationPanel {
        name: "PreferencesMenuPanel"
        enabled: root.enabled && root.visible
        direction: NavigationPanel.Both
        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }

        onNavigationEvent: function(event) {
            if (event.type === NavigationEvent.AboutActive) {
                event.setData("controlIndex", prv.currentItemNavigationIndex)
            }
        }
    }

    QtObject {
        id: prv

        property var currentItemNavigationIndex: []

        // the dialog activates the first item on open, which must not switch the page
        property bool selectOnNavigation: false
    }

    function focusOnCurrentPage() {
        const item = prv.currentItemNavigationIndex.length > 0 ? listView.itemAtIndex(prv.currentItemNavigationIndex[0]) : null
        if (item) {
            item.navigation.requestActive()
        }

        prv.selectOnNavigation = true
    }

    Rectangle {
        id: background
        anchors.fill: parent
        color: ui.theme.backgroundPrimaryColor
    }

    StyledListView {
        id: listView

        anchors.fill: parent
        anchors.topMargin: 12

        model: root.model

        delegate: PageTabButton {
            id: pageButton

            required property PreferencePageItem item
            required property int index

            width: listView.width
            height: 36

            orientation: Qt.Horizontal

            spacing: 16
            leftPadding: pageButton.spacing

            normalStateFont: ui.theme.bodyFont
            selectedStateFont: ui.theme.bodyBoldFont

            title: pageButton.item ? pageButton.item.title : ""
            checked: Boolean(pageButton.item) && pageButton.item.id === root.model.currentPageId

            navigation.name: "PreferencesMenuItem"
            navigation.panel: root.navigation
            navigation.row: pageButton.index
            navigation.column: 0
            navigation.accessible.name: pageButton.title
            navigation.accessible.role: MUAccessible.ListItem
            navigation.onActiveChanged: {
                if (pageButton.navigation.active && prv.selectOnNavigation) {
                    root.model.selectRow(pageButton.index)
                }
            }

            iconComponent: StyledIconLabel {
                width: 24
                height: width
                iconCode: pageButton.item ? pageButton.item.icon : IconCode.NONE
            }

            function updateNavigationIndex() {
                if (pageButton.checked) {
                    prv.currentItemNavigationIndex = [pageButton.index, 0]
                }
            }

            Component.onCompleted: {
                pageButton.updateNavigationIndex()
            }

            onCheckedChanged: {
                pageButton.updateNavigationIndex()
            }

            onClicked: {
                root.model.selectRow(pageButton.index)
            }
        }
    }
}
