/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0

Row {
    id: root

    height: 72
    spacing: 6

    PercussionNotePopupContentModel {
        id: contentModel

        Component.onCompleted: {
            contentModel.init()
        }
    }

    Row {
        id: buttonRow

        visible: contentModel.shouldShowButtons

        height: root.height
        spacing: 6

        FlatButton {
            id: prevButton

            height: buttonRow.height
            width: 48

            contentItem: StyledIconLabel {
                iconCode: IconCode.CHEVRON_LEFT
                font.pixelSize: 36
            }

            onClicked: {
                contentModel.prevDrumNote()
            }
        }

        FlatButton {
            id: nextButton

            height: buttonRow.height
            width: 48

            contentItem: StyledIconLabel {
                iconCode: IconCode.CHEVRON_RIGHT
                font.pixelSize: 36
            }

            onClicked: {
                contentModel.nextDrumNote()
            }
        }
    }

    Row {
        id: labelRow

        height: root.height
        spacing: 18

        rightPadding: 18
        leftPadding: 18

        StyledTextLabel {
            id: percussionNoteName

            height: labelRow.height
            verticalAlignment: Text.AlignVCenter

            text: contentModel.percussionNoteName
            font: ui.theme.headerBoldFont
        }

        StyledTextLabel {
            id: keyboardShortcut

            height: labelRow.height
            verticalAlignment: Text.AlignVCenter

            text: contentModel.keyboardShortcut
            font: ui.theme.headerBoldFont
            opacity: 0.8
        }
    }
}

