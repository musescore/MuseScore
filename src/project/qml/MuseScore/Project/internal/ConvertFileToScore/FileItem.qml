/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

ListItemBlank {
    id: root

    property alias fileName: fileNameLabel.text
    property alias iconCode: iconlabel.iconCode
    property bool selectable: false

    implicitHeight: 32

    radius: 3
    normalColor: ui.theme.buttonColor

    mouseArea.enabled: root.visible && root.enabled && root.selectable

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 8

        StyledIconLabel {
            iconCode: IconCode.TOOLBAR_GRIP
            visible: root.selectable
        }

        StyledIconLabel {
            id: iconlabel
        }

        StyledTextLabel {
            id: fileNameLabel
            Layout.fillWidth: true

            horizontalAlignment: Text.AlignLeft
            elide: Text.ElideMiddle
        }

        FlatButton {
            icon: IconCode.DELETE_TANK
            transparent: true
            isNarrow: true

            onClicked: root.removeSelectionRequested()
        }
    }
}
