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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

Item {
    id: root

    property alias notes: notesLabel.text
    property alias previousReleasesNotes: previousReleasesNotesRepeater.model

    QtObject {
        id: prv

        readonly property int sideMargin: 24
    }

    StyledFlickable {
        id: flickable

        anchors.fill: parent

        property int notesSpacing: 12

        contentHeight: notesLabel.implicitHeight + notesSpacing + previousReleasesNotesColumn.childrenRect.height

        StyledTextLabel {
            id: notesLabel

            anchors.left: parent.left
            anchors.right: parent.right

            horizontalAlignment: Text.AlignLeft
            font: ui.theme.largeBodyFont
            wrapMode: Text.WordWrap
            textFormat: Text.MarkdownText
            lineHeight: 1.2
        }

        Column {
            id: previousReleasesNotesColumn

            anchors.top: notesLabel.bottom
            anchors.topMargin: flickable.notesSpacing

            anchors.left: parent.left
            anchors.right: parent.right

            spacing: flickable.notesSpacing

            Repeater {
                id: previousReleasesNotesRepeater

                ExpandableBlank {
                    width: parent.width

                    title: qsTrc("update", "Read the %1 release notes").arg(modelData["version"])
                    titleFont: ui.theme.largeBodyBoldFont

                    isExpanded: false

                    contentItemComponent: Column {
                        height: implicitHeight
                        width: parent.width

                        StyledTextLabel {
                            width: parent.width

                            horizontalAlignment: Text.AlignLeft
                            font: ui.theme.largeBodyFont
                            wrapMode: Text.WordWrap
                            textFormat: Text.MarkdownText
                            lineHeight: 1.2

                            text: modelData["notes"]
                        }
                    }
                }
            }
        }

        ScrollBar.vertical: scrollBar
    }

    StyledScrollBar {
        id: scrollBar
        anchors.top: flickable.top
        anchors.right: flickable.right
        anchors.rightMargin: -prv.sideMargin
        anchors.bottom: flickable.bottom

        policy: ScrollBar.AlwaysOn
    }
}
