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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Languages 1.0

ListItemBlank {
    id: root

    property string title: ""
    property string statusTitle: ""

    property real headerWidth: width / 2
    property real sideMargin: 0.0

    height: 48

    navigation.accessible.name: root.title + " " + root.statusTitle

    function ensureActiveFocus() {
        if (!root.activeFocus) {
            root.forceActiveFocus()
        }
    }

    Row {
        anchors.left: parent.left
        anchors.leftMargin: root.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: root.sideMargin

        anchors.verticalCenter: parent.verticalCenter

        Row {
            width: root.headerWidth

            spacing: 12

            StyledIconLabel {
                iconCode: IconCode.NEW_FILE
            }

            StyledTextLabel {
                text: root.title
                font: ui.theme.largeBodyFont
                horizontalAlignment: Text.AlignLeft
            }
        }

        StyledTextLabel {
            width: root.headerWidth

            text: root.statusTitle
            font: ui.theme.largeBodyFont
            horizontalAlignment: Text.AlignLeft
        }
    }
}
