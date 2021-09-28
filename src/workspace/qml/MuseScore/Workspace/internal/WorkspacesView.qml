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
import QtQuick 2.12
import QtQuick.Controls 2.2

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

RadioButtonGroup {
    id: root

    clip: true
    spacing: 0
    interactive: height < contentHeight
    boundsBehavior: Flickable.StopAtBounds
    orientation: Qt.Vertical

    ScrollBar.vertical: StyledScrollBar {
        policy: ScrollBar.AsNeeded
    }

    property int leftPadding: 0

    Connections {
        target: root.model

        function onSelectedWorkspaceChanged(selectedWorkspace) {
            if (Boolean(selectedWorkspace)) {
                root.positionViewAtIndex(selectedWorkspace.index, ListView.Contain)
            }
        }
    }

    delegate: RoundedRadioButton {
        ButtonGroup.group: root.radioButtonGroup

        width: ListView.view.width
        height: 46

        leftPadding: root.leftPadding
        spacing: 12

        text: model.name
        font: model.isSelected ? ui.theme.bodyBoldFont : ui.theme.bodyFont

        checked: model.isSelected

        background: ListItemBlank {
            anchors.fill: parent

            isSelected: model.isSelected
            onClicked: {
                root.model.selectWorkspace(model.index)
            }

            SeparatorLine { anchors.bottom: parent.bottom }
        }
    }
}
