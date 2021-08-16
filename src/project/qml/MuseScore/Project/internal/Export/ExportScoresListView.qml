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

import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0

Rectangle {
    id: background

    property alias scoresModel: listView.model

    color: ui.theme.textFieldColor
    border.width: 1
    border.color: ui.theme.strokeColor

    ListView {
        id: listView
        anchors.fill: parent
        anchors.margins: background.border.width

        boundsBehavior: Flickable.StopAtBounds
        clip: true

        ScrollBar.vertical: StyledScrollBar {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
        }

        delegate: ListItemBlank {
            hoveredStateColor: "transparent"
            pressedStateColor: "transparent"

            CheckBox {
                anchors.margins: 4
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: parent.right

                text: model.isMain ? qsTrc("project", "Main Score") : model.title
                font: model.isMain ? ui.theme.bodyBoldFont : ui.theme.bodyFont

                checked: model.isSelected

                onClicked: {
                    scoresModel.setSelected(model.index, !model.isSelected)
                }
            }
        }
    }
}
