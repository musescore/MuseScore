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
import QtQuick 2.9
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

Item {
    id: root

    property alias families: familiesBox.model
    property var groups: null

    signal familySelected(string familyId)
    signal groupSelected(string groupId)

    QtObject {
        id: privateProperties

        property int currentGroupIndex: -1
    }

    function clearSelection() {
        privateProperties.currentGroupIndex = -1
    }

    function setFamily(familyId) {
        familiesBox.currentIndex = familiesBox.indexOfValue(familyId)
    }

    function focusGroup(groupId) {
        for (var i in root.groups) {
            if (root.groups[i].id === groupId) {
                privateProperties.currentGroupIndex = i
                groupsView.positionViewAtIndex(groupsView.currentGroupIndex, ListView.Beginning)
                return
            }
        }
    }

    StyledTextLabel {
        id: familyLabel

        anchors.top: parent.top
        anchors.left: parent.left

        font: ui.theme.bodyBoldFont
        text: qsTrc("instruments", "Family")
    }

    StyledComboBox {
        id: familiesBox

        anchors.top: familyLabel.bottom
        anchors.topMargin: 16
        anchors.left: parent.left
        anchors.right: parent.right

        textRoleName: "name"
        valueRoleName: "id"

        onActivated: {
            root.familySelected(value)
        }
    }

    ListView {
        id: groupsView

        anchors.top: familiesBox.bottom
        anchors.topMargin: 8
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        model: groups

        boundsBehavior: ListView.StopAtBounds
        clip: true

        ScrollBar.vertical: StyledScrollBar {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
        }

        delegate: ListItemBlank {
            isSelected: privateProperties.currentGroupIndex === index

            StyledTextLabel {
                anchors.fill: parent
                anchors.leftMargin: 12

                font: ui.theme.bodyBoldFont
                horizontalAlignment: Text.AlignLeft
                text: modelData.name
            }

            onClicked: {
                privateProperties.currentGroupIndex = index
                root.groupSelected(modelData.id)
            }
        }
    }
}
