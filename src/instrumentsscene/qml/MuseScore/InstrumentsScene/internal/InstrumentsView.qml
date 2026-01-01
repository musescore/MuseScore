/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

pragma ComponentBehavior: Bound

import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.InstrumentsScene

Item {
    id: root

    property InstrumentListModel instrumentsModel
    property alias navigation: instrumentsView.navigation

    property alias searching: searchField.hasText

    signal addSelectedInstrumentsToScoreRequested()

    function clearSearch() {
        searchField.clear()
    }

    function focusInstrument(instrumentIndex) {
        instrumentsView.positionViewAtIndex(instrumentIndex, ListView.Beginning)
    }

    StyledTextLabel {
        id: instrumentsLabel

        anchors.top: parent.top
        anchors.left: parent.left

        font: ui.theme.bodyBoldFont
        text: qsTrc("instruments", "Instruments")
    }

    SearchField {
        id: searchField

        anchors.top: instrumentsLabel.bottom
        anchors.topMargin: 16
        anchors.left: parent.left
        anchors.right: parent.right

        navigation.name: "SearchInstruments"
        navigation.panel: instrumentsView.navigation
        navigation.row: 1
        navigation.column: 0

        onSearchTextChanged: {
            root.instrumentsModel.setSearchText(searchText)
        }
    }

    StyledListView {
        id: instrumentsView

        anchors.top: searchField.bottom
        anchors.topMargin: 8
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        model: root.instrumentsModel

        navigation.name: "InstrumentsView"
        accessible.name: instrumentsLabel.text

        delegate: ListItemBlank {
            id: item

            required property var model
            required property string name
            required property string description
            required isSelected
            required property var traits
            required property int currentTraitIndex
            required property int index

            navigation.name: name
            navigation.panel: instrumentsView.navigation
            navigation.row: 2 + index
            navigation.column: 0
            navigation.accessible.name: itemTitleLabel.text
            navigation.accessible.description: description
            navigation.accessible.row: index

            onNavigationTriggered: {
                root.addSelectedInstrumentsToScoreRequested()
            }

            StyledTextLabel {
                id: itemTitleLabel
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.right: traitsBox.visible ? traitsBox.left : parent.right
                anchors.rightMargin: 4
                anchors.verticalCenter: parent.verticalCenter

                horizontalAlignment: Text.AlignLeft
                text: item.name
                font: ui.theme.bodyBoldFont
            }

            onClicked: {
                root.instrumentsModel.selectInstrument(index)
            }

            onDoubleClicked: {
                root.addSelectedInstrumentsToScoreRequested()
            }

            StyledDropdown {
                id: traitsBox

                navigation.name: "TraitsBox"
                navigation.panel: instrumentsView.navigation
                navigation.row: item.navigation.row
                navigation.column: 1
                navigation.accessible.name: itemTitleLabel.text + " " + qsTrc("instruments", "traits")
                navigation.accessible.row: item.index

                anchors.right: parent.right
                anchors.rightMargin: 4
                anchors.verticalCenter: parent.verticalCenter

                width: 86
                height: 24

                label.anchors.leftMargin: 8
                dropIcon.anchors.rightMargin: 4

                visible: traitsBox.count > 1

                model: item.traits
                currentIndex: item.currentTraitIndex

                onActivated: function(index, value) {
                    item.model.currentTraitIndex = index
                }
            }
        }
    }
}
