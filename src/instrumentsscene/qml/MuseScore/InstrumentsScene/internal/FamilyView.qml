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
import MuseScore.InstrumentsScene 1.0

Item {
    id: root

    property alias genres: genreBox.model
    property alias groups: groupsView.model

    property alias currentGenreIndex: genreBox.currentIndex
    property alias currentGroupIndex: groupsView.currentIndex

    property alias navigation: navPanel

    signal genreSelected(int newIndex)
    signal groupSelected(int newIndex)

    function focusGroup(groupIndex) {
        groupsView.positionViewAtIndex(groupIndex, ListView.Beginning)
    }

    NavigationPanel {
        id: navPanel
        name: "FamilyView"
        direction: NavigationPanel.Vertical
        enabled: root.visible
    }

    StyledTextLabel {
        id: titleLabel

        anchors.top: parent.top
        anchors.left: parent.left

        font: ui.theme.bodyBoldFont
        text: qsTrc("instruments", "Family")
    }

    Dropdown {
        id: genreBox

        anchors.top: titleLabel.bottom
        anchors.topMargin: 16
        anchors.left: parent.left
        anchors.right: parent.right

        navigation.name: "genreBox"
        navigation.panel: navPanel
        navigation.row: 1

        onActivated: {
            root.genreSelected(genreBox.currentIndex)
        }
    }

    ListView {
        id: groupsView

        anchors.top: genreBox.bottom
        anchors.topMargin: 8
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        boundsBehavior: ListView.StopAtBounds
        clip: true

        ScrollBar.vertical: StyledScrollBar {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
        }

        delegate: ListItemBlank {
            id: item

            isSelected: groupsView.currentIndex === model.index

            navigation.name: modelData
            navigation.panel: navPanel
            navigation.row: 2 + model.index

            onNavigationActived: item.clicked()

            StyledTextLabel {
                anchors.fill: parent
                anchors.leftMargin: 12

                font: ui.theme.bodyBoldFont
                horizontalAlignment: Text.AlignLeft
                text: modelData
            }

            onClicked: {
                root.groupSelected(model.index)
            }
        }
    }
}
