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
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.InstrumentsScene 1.0

Item {
    id: root

    property alias genres: genreBox.model
    property alias groups: groupsView.model

    property int currentGenreIndex: -1
    property int currentGroupIndex: -1

    property alias navigation: groupsView.navigation

    signal genreSelected(int newIndex)
    signal groupSelected(int newIndex)

    function scrollToGroup(groupIndex) {
        groupsView.positionViewAtIndex(groupIndex, ListView.Beginning)
    }

    function focusOnFirst() {
        if (root.currentGroupIndex !== -1) {
            focusGroupNavigation(root.currentGroupIndex)
        } else {
            root.groupSelected(0)
        }
    }

    function focusGroupNavigation(groupIndex: int) {
        var item = groupsView.itemAtIndex(groupIndex)
        if (item && item.navigation) {
            item.navigation.requestActive()
        }
    }

    StyledTextLabel {
        id: titleLabel

        anchors.top: parent.top
        anchors.left: parent.left

        font: ui.theme.bodyBoldFont
        text: qsTrc("instruments", "Family")
    }

    StyledDropdown {
        id: genreBox

        anchors.top: titleLabel.bottom
        anchors.topMargin: 16
        anchors.left: parent.left
        anchors.right: parent.right

        navigation.name: "genreBox"
        navigation.panel: groupsView.navigation
        navigation.row: 1

        currentIndex: root.currentGenreIndex

        onActivated: function(index, value) {
            root.genreSelected(index)
        }
    }

    StyledListView {
        id: groupsView

        anchors.top: genreBox.bottom
        anchors.topMargin: 8
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        navigation.name: "FamilyView"
        accessible.name: titleLabel.text

        onModelChanged: {
            groupsView.currentIndex = Qt.binding(() => (root.currentGroupIndex))
        }

        delegate: ListItemBlank {
            id: item

            property string groupName: modelData

            isSelected: groupsView.currentIndex === model.index

            navigation.name: modelData
            navigation.panel: groupsView.navigation
            navigation.row: 2 + model.index
            navigation.accessible.name: itemTitleLabel.text
            navigation.accessible.row: model.index

            StyledTextLabel {
                id: itemTitleLabel
                anchors.fill: parent
                anchors.leftMargin: 12

                font: ui.theme.bodyBoldFont
                horizontalAlignment: Text.AlignLeft
                text: groupName
            }

            onClicked: {
                root.groupSelected(model.index)
            }
        }
    }
}
