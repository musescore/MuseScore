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
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Item {
    id: root

    property alias notes: notesLabel.text

    QtObject {
        id: prv

        readonly property int sideMargin: 24
    }

    StyledTextLabel {
        id: titleLabel
        anchors.left: parent.left
        anchors.right: parent.right

        text: qsTrc("update", "Release notes")

        horizontalAlignment: Qt.AlignLeft
        font: ui.theme.bodyBoldFont
    }

    SeparatorLine {
        anchors.leftMargin: -prv.sideMargin
        anchors.rightMargin: -prv.sideMargin
        anchors.bottom: flickable.top
        anchors.bottomMargin: 16
    }

    StyledFlickable {
        id: flickable

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: titleLabel.bottom
        anchors.topMargin: 32
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16

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

        ScrollBar.vertical: scrollBar
    }

    StyledScrollBar {
        id: scrollBar
        anchors.top: flickable.top
        anchors.right: flickable.right
        anchors.rightMargin: -prv.sideMargin
        anchors.bottom: flickable.bottom
    }

    SeparatorLine {
        anchors.top: flickable.bottom
        anchors.topMargin: 16
        anchors.leftMargin: -prv.sideMargin
        anchors.rightMargin: -prv.sideMargin
    }
}
