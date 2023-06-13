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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

ColumnLayout {
    id: root

    property alias notes: notesLabel.text

    spacing: 16

    QtObject {
        id: prv

        readonly property int sideMargin: 36
    }

    StyledTextLabel {
        Layout.fillWidth: true

        text: qsTrc("update", "Release notes")

        horizontalAlignment: Qt.AlignLeft
        font: ui.theme.bodyBoldFont
    }

    SeparatorLine { Layout.leftMargin: -prv.sideMargin; Layout.rightMargin: -prv.sideMargin }

    StyledFlickable {
        Layout.fillWidth: true
        Layout.fillHeight: true

        contentHeight: notesLabel.implicitHeight

        StyledTextLabel {
            id: notesLabel

            anchors.left: parent.left
            anchors.right: parent.right

            horizontalAlignment: Text.AlignLeft
            font: ui.theme.largeBodyFont
            wrapMode: Text.WordWrap
            textFormat: Text.MarkdownText
        }
    }

    SeparatorLine { Layout.leftMargin: -prv.sideMargin; Layout.rightMargin: -prv.sideMargin }
}
