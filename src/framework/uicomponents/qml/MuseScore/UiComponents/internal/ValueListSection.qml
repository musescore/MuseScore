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
import MuseScore.Ui 1.0

ListItemBlank {
    id: root

    property alias spacing: row.spacing
    property alias text: titleLabel.text
    property real sideMargin: 0
    property real valueItemWidth: 126

    property bool expanded: true

    height: 34

    normalColor:  ui.theme.backgroundPrimaryColor

    signal clickedUp()

    onClicked: {
        root.clickedUp()
        forceActiveFocus()
    }

    RowLayout {
        id: row

        anchors.fill: parent

        Row {
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.fillWidth: true
            Layout.leftMargin: root.sideMargin

            spacing: 18

            StyledIconLabel {
                iconCode: root.expanded ? IconCode.SMALL_ARROW_DOWN : IconCode.SMALL_ARROW_RIGHT
            }

            StyledTextLabel {
                id: titleLabel
                font: ui.theme.bodyBoldFont
                text: "section"
                horizontalAlignment: Text.AlignLeft
            }
        }
    }
}
