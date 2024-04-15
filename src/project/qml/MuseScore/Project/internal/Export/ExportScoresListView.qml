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

import Muse.UiComponents 1.0
import Muse.Ui 1.0
import MuseScore.Project 1.0

Rectangle {
    id: root

    property alias scoresModel: listView.model
    property alias navigation: navPanel

    color: ui.theme.textFieldColor
    border.width: 1
    border.color: ui.theme.strokeColor

    function focusOnFirst() {
        listView.itemAtIndex(0).requestActiveFocus()
    }

    NavigationPanel {
        id: navPanel
        name: "ExportScoresListView"
        enabled: root.visible && root.enabled
        direction: NavigationPanel.Vertical
    }

    StyledListView {
        id: listView

        anchors.fill: parent
        anchors.margins: root.border.width

        delegate: ListItemBlank {
            mouseArea.hoverEnabled: false
            hoverHitColor: "transparent"

            function requestActiveFocus() {
                checkBox.navigation.requestActive()
            }

            CheckBox {
                id: checkBox
                anchors.margins: 4
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: parent.right

                text: model.isMain ? qsTrc("project/export", "Main score") : model.title
                font: model.isMain ? ui.theme.bodyBoldFont : ui.theme.bodyFont

                navigation.name: "ExportScoreCheckBox " + text
                navigation.panel: navPanel
                navigation.row: model.index

                checked: model.isSelected

                onClicked: {
                    scoresModel.setSelected(model.index, !checked)
                }
            }
        }
    }
}
